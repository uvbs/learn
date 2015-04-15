//
//  VideoEncoder.m
//  hardwareCodec
//
//  Created by IT－PC on 14-12-15.
//  Copyright (c) 2014年 YY. All rights reserved.
//

#import "VideoEncoder.h"

#import <AVFoundation/AVFoundation.h>
#import <VideoToolbox/VideoToolbox.h>
#import <CoreVideo/CoreVideo.h>

#include "hw_utils.h"

//#import <IOKit/IOKitLib.h>
//#import <IOSurface/IOSurfaceBase.h>

#define RELEASE_REF(ref)        \
        do {                    \
            if (ref != NULL) {  \
                CFRelease(ref); \
                ref = NULL;     \
            }                   \
        } while (0)

#define RELEASE_PTR(ptr)        \
    do {                        \
        if (ptr != NULL) {      \
            free((void *)ptr);  \
            ptr = NULL;         \
        }                       \
    } while (0)

#define USE_PIXEL_POOL  1
#define TIME_ANALYSIS   0
#define WRITE_ENCODE_TO_FILE    0

enum {
    ECreateNumberRef    = -1,
    ENULLSession        = -2,
    EInvaliedParam      = -3,
    ECreatePixelRet     = -4,
    ECreatePixelNULL    = -5,
    EMalloc             = -6,
    
};

enum {
    Level_Unk,
    Level_Low,
    Level_Mid,
    Level_High,
    Level_Max,
};

static NSString * _debug_filename = @"encode.bin";

@interface VideoEncoder()
{
    
}
@property VTCompressionSessionRef compressionSession;
//@property VTPixelTransferSessionRef pixelTransferSession;
@property int32_t picWidth;
@property int32_t picHeight;

@property int32_t bitrate;
@property int32_t depth;
@property BOOL progressiveScan;
@property float quality;
@property BOOL realtime;
@property int32_t level;
@property int32_t framerate;
@property int32_t iframerate;

@property int64_t count;

@property NSMutableArray * refArray;    //time array, for record encode time per frame.
@property NSMutableArray * frameArray;
@property CVPlanarPixelBufferInfo_YCbCrPlanar info;
@property CVPixelBufferPoolRef pool;

@property char * sendbuffer;
@property uint32_t sendbuffer_len;

@property NSMutableArray * encodeParameters;

@property CFDictionaryRef pixelBufferAttributes;

@property int savefd;

@property int32_t actWidth;
@property int32_t actHeight;

@property BOOL firstFrameEncode;
@end


@implementation VideoEncoder

- (id) initWithData:(int32_t)w height:(int32_t)h codeRate:(int32_t)cr frameRate:(int32_t)fr
{
    if (self = [super init]) {
        _compressionSession = NULL;
        
        _encWidth   = _actWidth = w;
        _encHeight  = _actHeight = h;
        _picHeight  = 0;
        _picWidth   = 0;
        _bitrate    = cr;
        _framerate  = fr;
        _depth      = 0;
        _progressiveScan = NO;
        _quality    = 0;
        _realtime   = NO;
        _level      = Level_Unk;
        _count      = 0;
        _callback   = NULL;
        _iframerate = 0;
        
        _refArray   = [[NSMutableArray alloc] init];
        _pool       = NULL;
        
        _encodeParameters = [[NSMutableArray alloc] init];
        _frameArray = [[NSMutableArray alloc] init];
        
        _savefd     = 0;

        _sendbuffer = (char *)malloc(_encWidth * _encHeight * 2);
        assert(_sendbuffer != NULL);
        _sendbuffer_len = 0;
        
        _pool       = NULL;
        
        _firstFrameEncode = YES;
        _onSizeChange = NULL;
        
#if USE_PIXEL_POOL
        CFMutableDictionaryRef pixelBufferAttributes = CFDictionaryCreateMutable( kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        
        CFNumberRef widthNum = CFNumberCreate( kCFAllocatorDefault, kCFNumberIntType, &_encWidth);
        CFDictionarySetValue( pixelBufferAttributes, kCVPixelBufferWidthKey, widthNum);
        CFRelease(widthNum);
        
        CFNumberRef heightNum = CFNumberCreate( kCFAllocatorDefault, kCFNumberIntType, &_encHeight);
        CFDictionarySetValue( pixelBufferAttributes, kCVPixelBufferHeightKey, heightNum);
        CFRelease(heightNum);
        
        OSType pixelFormat = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
        CFNumberRef pixelFormatNum = CFNumberCreate( kCFAllocatorDefault, kCFNumberIntType, &pixelFormat);
        CFDictionarySetValue( pixelBufferAttributes, kCVPixelBufferPixelFormatTypeKey, pixelFormatNum);
        CFRelease(pixelFormatNum);
        
        CVReturn status = CVPixelBufferPoolCreate( kCFAllocatorDefault,
                                                  NULL,
                                                  pixelBufferAttributes,
                                                  &_pool);
        if (kCVReturnSuccess != status) {
            printf("+ error: CVPixelBufferPoolCreate was not `noErr`: %d\n", (int)status);
        }
        
        CFRelease(pixelBufferAttributes);
#endif
    }
    return self;
}

- (void) dealloc
{
    NSLog(@"Hardware video encoder dealloc!");
    
    [self deinit];
}

- (void) deinit
{
    RELEASE_REF(_compressionSession);
    RELEASE_REF(_pool);

    _callback    = NULL;
    RELEASE_PTR(_sendbuffer);
    
    [self closeSavefile];
}

- (OSStatus) setupCompressionSession
{
    _compressionSession = NULL;
    NSString * errstr = @"";
    
    OSStatus status;
    
    status = VTCompressionSessionCreate(NULL,
                                        _encWidth,
                                        _encHeight,
                                        kCMVideoCodecType_H264,
                                        NULL,
                                        NULL,
                                        NULL,
                                        compressionOutputCallback,
                                        (__bridge void *)(self),
                                        &(_compressionSession));
    if (noErr != status) {
        errstr = @"VTCompressionSessionCreate failed";
        goto failed;
    }
    
    status = [self setAverageBitrate:_bitrate];
    if (status != noErr) {
        errstr = @"set average bitrate failed";
        goto failed;
    }
    
    status = VTSessionSetProperty(_compressionSession,
                                  kVTCompressionPropertyKey_ProfileLevel,
                                  kVTProfileLevel_H264_Baseline_AutoLevel);
    if (noErr != status) {
        errstr = @"set profile level failed";
        goto failed;
    }


    NSLog(@"create compression session, w:%u, h:%u, coderate:%u, framerate:%u",
          _encWidth, _encHeight, _bitrate, _framerate);
    
failed:
    if (status != noErr) {
        RELEASE_REF(_compressionSession);
    }
    NSLog(@"setupCompressionSession, %@, status = %d", errstr, (int)status);
    return  status;
}

- (OSStatus) setAverageBitrate:(int32_t)bitrate
{
    printf("[%s:%u][bitrate:%u]\n", __FUNCTION__, __LINE__, bitrate);
    _bitrate = bitrate;
    if (_bitrate < 0) {
        return EInvaliedParam;
    }
    return [self setCompressionSessionIntegerProperty:kVTCompressionPropertyKey_AverageBitRate
                                         value:_bitrate];
}

- (void) setFrameNum:(int32_t)framenum
{
    printf("[%s:%u][framenum:%u]\n", __FUNCTION__, __LINE__, framenum);
    _framerate = framenum;
}

- (OSStatus) setIFrameVal:(int32_t)iframeVal
{
    printf("[%s:%u][iframevale:%u]\n", __FUNCTION__, __LINE__, iframeVal);
    _iframerate = iframeVal;
    if (_iframerate < 0) {
        return EInvaliedParam;
    }
    return [self setCompressionSessionIntegerProperty:kVTCompressionPropertyKey_MaxKeyFrameInterval
                                                value:_iframerate];
}

- (OSStatus) setEncodeDepth:(int32_t)depth
{
    _depth = depth;
    return [self setCompressionSessionIntegerProperty:kVTCompressionPropertyKey_Depth
                                                value:_depth];
}

- (OSStatus) setProgressivescan:(BOOL)progressiveScan
{
    printf("[%s:%u]\n", __FUNCTION__, __LINE__);
    _progressiveScan = progressiveScan;
    return [self setCompressionSessionBooleanProperty:kVTCompressionPropertyKey_ProgressiveScan
                                                value:_progressiveScan];
}

- (OSStatus) forceKeyFrame
{
    printf("[%s:%u]\n", __FUNCTION__, __LINE__);
    if (_iframerate <= 1) {
        return false;
    }
    return [self setCompressionSessionBooleanProperty:kVTEncodeFrameOptionKey_ForceKeyFrame
                                                value:YES];
}

- (OSStatus) setquality:(float)quality
{
    if (quality >= 1 || quality < 0) {
        return EInvaliedParam;
    }
    _quality = quality;
    return [self setCompressionSessionFloatProperty:kVTCompressionPropertyKey_Quality
                                                value:_quality];
}

- (OSStatus) enableRealtime:(BOOL)realtime
{
    _realtime = realtime;
    return [self setCompressionSessionBooleanProperty:kVTCompressionPropertyKey_ProgressiveScan
                                                value:_realtime];
}

- (OSStatus) setProfileLevel:(int32_t) profile
{
    printf("[%s:%u]\n", __FUNCTION__, __LINE__);
    _level = profile;
    if (_level <= Level_Unk || _level >= Level_Max) {
        return EInvaliedParam;
    }
    return [self setCompressionSessionIntegerProperty:kVTCompressionPropertyKey_ProfileLevel
                                                value:_level];
}

- (OSStatus) forceCompleteEncode
{
    if (_compressionSession == NULL) {
        return ENULLSession;
    }
    
    return VTCompressionSessionCompleteFrames(_compressionSession, kCMTimeInvalid);
}

- (OSStatus) encode:(const char **)data strides:(const int32_t *)strides
             widths:(const int32_t *)widths heights:(const int32_t *)heights
{
    NSString * errstr = @"noErr";
    OSStatus status = noErr;
    CVPixelBufferRef pixelref = NULL;
    _count += (1000 / _framerate);
    CMTime presentation_ts = CMTimeMake(_count, 1000);
    CMTime duration = CMTimeMake(1, _framerate);
    
    if (_compressionSession == NULL) {
        status = ENULLSession;
        goto failed;
    }
    
    if (_actHeight != _encHeight || _actWidth != _encWidth) { //wait recreate.
        status = EInvaliedParam;
        goto failed;
    }
//    if (data == NULL || data[0] == NULL || data[1] == NULL || data[2] == NULL ||
//        widths[0] != _actWidth || widths[1] != widths[2] || widths[1] != _actWidth / 2 ||
//        heights[0] != _encHeight || heights[1] != heights[2] || heights[1] != _encHeight / 2) {
//        status = EInvaliedParam;
//        goto failed;
//    }
    
#if USE_PIXEL_POOL
    CVReturn ret = CVPixelBufferPoolCreatePixelBuffer(kCFAllocatorDefault, _pool, &pixelref);
#else
    _pixelBufferAttributes = (__bridge CFDictionaryRef)@{
                             (id)kCVPixelBufferExtendedPixelsTopKey: @(0),
                             (id)kCVPixelBufferExtendedPixelsBottomKey: @(0),
                             (id)kCVPixelBufferExtendedPixelsLeftKey: @(0),
                             (id)kCVPixelBufferExtendedPixelsRightKey: @(0),
                             (id)kCVPixelBufferIOSurfacePropertiesKey: @{},
                             };
    
    CVReturn ret = CVPixelBufferCreate(NULL,
                                       _encWidth,
                                       _encHeight,
                                       kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange,
                                       _pixelBufferAttributes,
                                       &pixelref);
#endif
    
    if (ret != kCVReturnSuccess) {
        status = ECreatePixelRet;
        errstr = @"create return failed";
        goto failed;
    }

    if (pixelref == NULL) {
        errstr = @"create return null";
        status = ECreatePixelNULL;
        goto failed;
    }
    
    CVPixelBufferLockBaseAddress( pixelref, 0 );
    size_t planeCount = CVPixelBufferGetPlaneCount(pixelref);
    if (planeCount > 2) {
        planeCount = 2;
    }
    
    uint8_t * yplane = CVPixelBufferGetBaseAddressOfPlane(pixelref, 0);
    memcpy(yplane, data[0], _encWidth * _encHeight);
    
    uint8_t * uvplane = CVPixelBufferGetBaseAddressOfPlane(pixelref, 1);
    int32_t uvHeight = _encHeight >> 1;
    int32_t uvWidth = _encWidth >> 1;
    for (int32_t i = 0; i < uvHeight; i++) {
        uint8_t * dst = uvplane + i * _encWidth;
        uint8_t * usrc = (uint8_t *)(data[1] + i * uvWidth);
        uint8_t * vsrc = (uint8_t *)(data[2] + i * uvWidth);
        
        for (int j = 0; j < uvWidth; j++) {
            * dst ++ = * usrc ++;
            * dst ++ = * vsrc ++;
        }
    }
    
    CVPixelBufferUnlockBaseAddress( pixelref, 0);
    
#if TIME_ANALYSIS
    @synchronized (_refArray) {
        [_refArray addObject:[NSDate date]];
    }
#endif
    
    @synchronized (_frameArray) {
        [_frameArray addObject:(__bridge id)(pixelref)];
    }
    status = VTCompressionSessionEncodeFrame(_compressionSession,
                                             (CVImageBufferRef)pixelref,
                                             presentation_ts,
                                             duration,
                                             NULL,
                                             NULL,
                                             NULL);
    if (status != noErr) {
        errstr = @"compress encode failed";
    }
    
failed:
    if (status != noErr) {
        RELEASE_REF(pixelref);
        NSLog(@"[%s:%u][(%@) staus = %d]\n", __FUNCTION__, __LINE__, errstr, (int)status);
    }
    
    return status;
}

#pragma mark callback
void compressionOutputCallback(void *outputCallbackRefCon, void *sourceFrameRefCon, OSStatus status,
                               VTEncodeInfoFlags infoFlags, CMSampleBufferRef sampleBuffer )
{
    __weak __block VideoEncoder * weakSelf = (__bridge VideoEncoder *)(outputCallbackRefCon);
    if (status != noErr) {
        NSLog(@"%s, status = %d", __FUNCTION__, (int)status);
        [weakSelf releaseFrame];
        return;
    }
    
    CMBlockBufferRef buffer;
    char * sampledata;
    size_t offset_length, buffer_length;
    
    buffer = CMSampleBufferGetDataBuffer(sampleBuffer);
    CMBlockBufferGetDataPointer(buffer, 0, &offset_length, &buffer_length, &sampledata);
    
    BOOL iskeyframe = [weakSelf isBufferKeyframe:sampleBuffer];
    BOOL output = YES;
    if (iskeyframe) {
        [weakSelf getSpsPps:sampleBuffer];
        output = [weakSelf docheck];
    }
    
    if (output) {
        [weakSelf combineData:sampledata size:(int)buffer_length keyframe:iskeyframe];
    }
    
#if TIME_ANALYSIS
    @synchronized(weakSelf.refArray) {
        NSDate * dec = [weakSelf.refArray objectAtIndex:0];
        [weakSelf.refArray removeObjectAtIndex:0];
        NSLog(@"encode time:%f, key:%s, size:%zu", [dec timeIntervalSinceNow] * -1, iskeyframe ? "YES" : "NO", buffer_length);
    }
#endif
    [weakSelf releaseFrame];
}

- (BOOL) docheck
{
    if (_actWidth != _encWidth || _actHeight != _encHeight) {
        if (_onSizeChange != NULL) {
            _onSizeChange(_encWidth, _encHeight, _actWidth, _actHeight);
        }
        return NO;
    }
    return YES;
}

- (void) releaseFrame
{
    @synchronized (_frameArray) {
        CVPixelBufferRef toRelease = (__bridge CVPixelBufferRef)([_frameArray objectAtIndex:0]);
        [_frameArray removeObjectAtIndex:0];
        RELEASE_REF(toRelease);
    }
}

- (void) combineData:(char *)data size:(int32_t)size keyframe:(BOOL)key
{
    const uint8_t startCode[] = {0x00, 0x00, 0x00, 0x01};
    
    uint32_t len = 0;
    uint32_t pos = 0;
    char * buffer = data;
    
    while (true) {
        len = [self getDataLength:(buffer + pos)];
        if (len == 0) {
            break;
        }
        memcpy((char *)(buffer + pos), (char *)startCode, 4);
        
        pos += 4 + len;
        if (pos == size) {
            break;
        }
    }
    
    int32_t sendsize = 0;
    
    if (key) {
        for (NSData * ps in _encodeParameters) {
            uint32_t len = (uint32_t)[ps length];
            const uint8_t * pps = (const uint8_t *)[ps bytes];
            
            memcpy((uint8_t *)(_sendbuffer + sendsize), (uint8_t *)startCode, 4);
            sendsize += 4;
            
            memcpy((uint8_t *)(_sendbuffer + sendsize), (uint8_t *)pps, len);
            sendsize += len;
        }
       memcpy((uint8_t *)(_sendbuffer + sendsize), (uint8_t *)data, size);
       sendsize += size;
    }
    
#if WRITE_ENCODE_TO_FILE //save encode data into file.
    if (key) {
        [self writeFile:_sendbuffer size:sendsize filename:_debug_filename];
    } else {

        [self writeFile:data size:size filename:_debug_filename];
    }
#endif
    
    if (_callback != NULL) {
        if (key) {
            _callback(_sendbuffer, sendsize, true);
        } else {
            _callback(data, size, false);
        }
    }
}

- (void) getSpsPps:(CMSampleBufferRef)sampleBuffer
{
    if ([_encodeParameters count] == 0) {
        CMVideoFormatDescriptionRef format = CMSampleBufferGetFormatDescription(sampleBuffer);
        size_t parameterSetCount = 0;
        int32_t NALUnitHeaderLengthOut = 0;
        CMVideoFormatDescriptionGetH264ParameterSetAtIndex(format, 0, NULL, NULL,
                                                           &parameterSetCount,
                                                           &NALUnitHeaderLengthOut);
        
        for (int i = 0; i < parameterSetCount; i++) {
            const uint8_t *parameterSetPointer;
            size_t parameterSetSize = 0;
            CMVideoFormatDescriptionGetH264ParameterSetAtIndex(format, i, &parameterSetPointer,
                                                               &parameterSetSize, NULL, NULL);
            NSLog(@"format:size:%zu, count:%zu, NALUnitHeaderLengthOut:%u", parameterSetSize, parameterSetCount, NALUnitHeaderLengthOut);
            NSData * data = [[NSData alloc] initWithBytes:parameterSetPointer length:parameterSetSize];
            [_encodeParameters addObject:data];
            
            if (_firstFrameEncode && parameterSetSize >= 1 && (parameterSetPointer[0] & 0x1F) == 0x07) {
                struct sps_parameters params;
                if (!analysis_sps(parameterSetPointer, &params)) {
                    //print_sps_parameters(&params);
                    _firstFrameEncode = NO;
                    
                    _actWidth = GET_WIDTH(params.pic_width_in_mbs_minus_1);
                    _actHeight = GET_WIDTH(params.pic_height_in_map_units_minus_1);
                }
            }
        }
        
        int index = 0;
        for (NSData * p in _encodeParameters) {
            NSUInteger len = [p length];
            
            NSString * print = @"";
            const uint8_t * b = (const uint8_t *)[p bytes];
            for (int i = 0; i < len; i++) {
                print = [NSString stringWithFormat:@"%@ 0x%02x", print, b[i]];
            }
            NSLog(@"param %d is %@", index ++, print);
        }
    }
}

void pixelBufferReleasePlanarBytesCallback( void *releaseRefCon, const void *dataPtr, size_t dataSize, size_t numberOfPlanes, const void *planeAddresses[] )
{
    int32_t count = *(int32_t *)releaseRefCon;
    NSLog(@"[%s][release num:%u", __FUNCTION__, count);
}

#pragma mark private functions
- (uint32_t) getDataLength:(char *)data {
    uint32_t len = 0;
    for (int i = 0; i < 4; i++) {
        uint8_t n = data[i];
        len = (len << 8) | n;
    }
    return len;
}

- (OSStatus) setCompressionSessionIntegerProperty:(CFStringRef)key value:(int32_t)value
{
    if (_compressionSession == NULL) {
        return ENULLSession;
    }
    
    CFNumberRef numref = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
    if (numref == NULL) {
        return ECreateNumberRef;
    }
    
    OSStatus status = VTSessionSetProperty(_compressionSession, key, numref);
    RELEASE_REF(numref);
    
    return status;
}

- (OSStatus) setCompressionSessionBooleanProperty:(CFStringRef)key value:(BOOL)value
{
    if (_compressionSession == NULL) {
        return ENULLSession;
    }
    
    CFBooleanRef boolref = kCFBooleanTrue;
    if (!value) {
        boolref = kCFBooleanFalse;
    }
    
    OSStatus status = VTSessionSetProperty(_compressionSession, key, boolref);
    
    return status;
}


- (OSStatus) setCompressionSessionFloatProperty:(CFStringRef)key value:(float)value
{
    if (_compressionSession == NULL) {
        return ENULLSession;
    }
    
    CFNumberRef numref = CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat32Type, &value);
    if (numref == NULL) {
        return ECreateNumberRef;
    }
    
    OSStatus status = VTSessionSetProperty(_compressionSession, key, numref);
    RELEASE_REF(numref);
    
    return status;
}

- (BOOL) isBufferKeyframe:(CMSampleBufferRef)theBuffer
{
    
    CFArrayRef sample_attachments;
    BOOL result = NO;
    
    sample_attachments = CMSampleBufferGetSampleAttachmentsArray(theBuffer, NO);
    if (sample_attachments)
    {
        CFDictionaryRef attach;
        CFBooleanRef depends_on_others;
        
        attach = CFArrayGetValueAtIndex(sample_attachments, 0);
        depends_on_others = CFDictionaryGetValue(attach, kCMSampleAttachmentKey_DependsOnOthers);
        result = depends_on_others == kCFBooleanFalse;
    }
    
    return result;
    
}

-(void) writeFile:(char *)data size:(int32_t)size filename:(NSString *)filename
{
    if (_savefd == 0) {
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask, YES) ;
        NSString *documentD = [paths objectAtIndex:0];
        NSString *configFile = [documentD stringByAppendingPathComponent:filename];
        NSLog(@"filiename: %@", configFile);
        
        NSFileManager *fileMgr = [NSFileManager defaultManager];
        BOOL bRet = [fileMgr fileExistsAtPath:configFile];
        if (bRet) {
            NSError *err;
            [fileMgr removeItemAtPath:configFile error:&err];
            NSLog(@"remove file, ret = %@", err);
        }
        
        
        if (access([configFile UTF8String], F_OK) == 0) {
            _savefd = open([configFile UTF8String], O_TRUNC | O_RDWR | O_SYNC);
            NSLog(@"file exists, just open, and truncate");
        } else {
            _savefd = open([configFile UTF8String], O_RDWR | O_CREAT | O_SYNC,
                               S_IRUSR |S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH |S_IXOTH);
            NSLog(@"file not exists, create");
        }
        
        
        if (_savefd <= 0) {
            NSLog(@"open file %@ failed", configFile);
        }
        NSLog(@"file description:%d", _savefd);
    }
    
    size_t res = write(_savefd, data, size);
    NSLog(@"write result = %zu", res);
}

- (void) closeSavefile
{
    if (_savefd > 0) {
        close(_savefd);
        _savefd = 0;
    }
}
@end
