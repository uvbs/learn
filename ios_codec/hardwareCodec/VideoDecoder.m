//
//  VideoDecoder.m
//  hardwareCodec
//
//  Created by IT－PC on 14-12-15.
//  Copyright (c) 2014年 YY. All rights reserved.
//

#import "VideoDecoder.h"

#import <AVFoundation/AVFoundation.h>
#import <VideoToolbox/VideoToolbox.h>
#import <CoreVideo/CoreVideo.h>

#define RELEASE_REF(ref)            \
        do {                        \
            if (ref != NULL) {      \
                CFRelease(ref);     \
                ref = NULL;         \
            }                       \
        } while (0)


#define RELEASE_PTR(ptr)            \
        do {                        \
            if (ptr != NULL) {      \
                free((void *)ptr);  \
                ptr = NULL;         \
            }                       \
        } while (0)

#define DECODE_FILENAME     @"to_decode.bin"

@interface VideoDecoder()
{
}

@property VTDecompressionSessionRef decompression;
@property CMVideoFormatDescriptionRef format;
@property NSData * sps;
@property NSData * pps;
@property int32_t naluHeaderLen;
@property NSMutableArray * frameArray;

static void decompressionOutputCallback(void *decompressionOutputRefCon, void *sourceFrameRefCon,
                                        OSStatus status, VTDecodeInfoFlags infoFlags,
                                        CVImageBufferRef imageBuffer,
                                        CMTime presentationTimeStamp,
                                        CMTime presentationDuration);

@property char * decodebuf;
@property size_t width;
@property size_t height;

@property int savefd;
@end

@implementation VideoDecoder
#pragma mark public functions
- (id) initWithData:(const char *)sps sLen:(size_t)sLen pps:(const char *)pps pLen:(size_t)pLen
      naluHeaderLen:(int32_t)headerLen
{
    if (self = [super init]) {
        _decompression  = NULL;
        _format         = NULL;
        _naluHeaderLen  = headerLen;
        _decodebuf      = NULL;
        _width          = 0;
        _height         = 0;
        _callback       = NULL;
        _savefd         = 0;
        
        NSLog(@"sps:%zu, pps:%zu", sLen, pLen);
        _sps            = [[NSData alloc] initWithBytes:sps length:sLen];
        _pps            = [[NSData alloc] initWithBytes:pps length:pLen];
        _frameArray     = [[NSMutableArray alloc] initWithCapacity:10];
        
        NSString * output = @"sps:";
        for (size_t i = 0; i < sLen; i++) {
            output = [NSString stringWithFormat:@"%@, 0x%02x", output, (uint8_t)sps[i]];
        }
        NSLog(@"%@", output);
        
        output = @"pps:";
        for (size_t i = 0; i < pLen; i++) {
            output = [NSString stringWithFormat:@"%@, 0x%02x", output, (uint8_t)pps[i]];
        }
        NSLog(@"%@", output);

    }
    return self;
}

- (void) deinit
{
    NSLog(@"%s, %u", __FUNCTION__, __LINE__);
    RELEASE_REF(_decompression);
    RELEASE_REF(_format);
    
    RELEASE_PTR(_decodebuf);
    
    _sps    = nil;
    _pps    = nil;
    
    [self closeSavefile];
}

- (OSStatus) setupDecomressionSession
{
    _decompression  = NULL;
    _format         = NULL;
    
    OSStatus status = noErr;
    NSString * errstr = @"noErr";
    
    {
        const uint8_t * parameterSetPointers[2] = {
            (const uint8_t *)[_sps bytes], (const uint8_t *)[_pps bytes],
        };
        const size_t parameterSetSizes[2] = {
            [_sps length], [_pps length],
        };
        
        status = CMVideoFormatDescriptionCreateFromH264ParameterSets(kCFAllocatorDefault,
                                                                     2,
                                                                     parameterSetPointers,
                                                                     parameterSetSizes,
                                                                     _naluHeaderLen,
                                                                     &_format);
        if (status != noErr) {
            errstr = [NSString stringWithFormat:@"create format failed:%d", (int)status];
            goto failed;
        }
    }
    
    {
        CFMutableDictionaryRef decodeAttrs = CFDictionaryCreateMutable(NULL,
                                                                 0,
                                                                 &kCFTypeDictionaryKeyCallBacks,
                                                                 &kCFTypeDictionaryValueCallBacks);
        
        int32_t type = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
        CFNumberRef cfType = CFNumberCreate(NULL,
                                            kCFNumberSInt32Type,
                                            &type);
        CFDictionarySetValue(decodeAttrs,
                             kCVPixelBufferPixelFormatTypeKey,
                             cfType);
        RELEASE_REF(cfType);

        //    CFDictionarySetValue(destinationPixelBufferAttributes, kCVPixelBufferOpenGLCompatibilityKey, kCFBooleanTrue);
        VTDecompressionOutputCallbackRecord record;
        record.decompressionOutputCallback = decompressionOutputCallback;
        record.decompressionOutputRefCon   = (__bridge void *)self;
        
        status = VTDecompressionSessionCreate(kCFAllocatorDefault,
                                              _format,
                                              NULL,
                                              decodeAttrs,
                                              &record,
                                              &_decompression);
        RELEASE_REF(decodeAttrs);
    }
failed:
    if (status != noErr) {
        RELEASE_REF(_format);
        RELEASE_REF(_decompression);
    }
    NSLog(@"setupDecomressionSession, %@", errstr);
    
    return status;
}

- (BOOL) isSame:(const char *)sps sLen:(size_t)sLen pps:(const char *)pps pLen:(size_t)pLen
  naluHeaderLen:(int32_t)headerLen
{
    if (_naluHeaderLen != headerLen) {
        return NO;
    }

    if (sLen != [_sps length]) {
        return NO;
    }
    if (pLen != [_pps length]) {
        return NO;
    }
    
    if (memcmp(sps, (const char *)[_sps bytes], sLen)) {
        return NO;
    }
    
    if (memcmp(pps, (const char *)[_pps bytes], pLen)) {
        return NO;
    }
    return YES;
}

- (OSStatus) decode:(const char *)data size:(size_t)size
{
    OSStatus status = noErr;
    NSString * errstr = @"noErr";
    
    CMBlockBufferRef blockRef = NULL;
    CMSampleBufferRef sampleBuf = NULL;
    
    if (_decompression == NULL) {
        errstr = @"NULL Compression";
        status = -1;
        goto failed;
    }
    
//    static BOOL save = YES;
//    if (save) {
//        save = NO;
//        [self writeFile:(char *)data size:(int32_t)size filename:DECODE_FILENAME];
//        [self closeSavefile];
//    }
    
    status = CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault,
                                                (void *)data,
                                                size,
                                                kCFAllocatorNull,
                                                NULL,
                                                0,
                                                size,
                                                0,
                                                &blockRef);
    
    if (status != kCMBlockBufferNoErr) {
        errstr = [NSString stringWithFormat:@"create block buffer failed:%d", (int)status];
        goto failed;
    }
    
    static int64_t count = 1;
    CMSampleTimingInfo timeInfo;
    timeInfo.presentationTimeStamp = CMTimeMake(count ++, 15);
    
    status = CMSampleBufferCreate(kCFAllocatorDefault,
                                  blockRef,
                                  TRUE,
                                  NULL,
                                  NULL,
                                  _format,
                                  1,
                                  1,
                                  &timeInfo,
                                  0,
                                  NULL,
                                  &sampleBuf);
    if (status != noErr) {
        errstr = [NSString stringWithFormat:@"create sample buffer failed:%d", (int)status];
        goto failed;
    }
    
    VTDecodeFrameFlags decodeFlags = kVTDecodeFrame_EnableAsynchronousDecompression | kVTDecodeFrame_EnableTemporalProcessing;
    VTDecodeInfoFlags infoFlag;
    
//    @synchronized (_frameArray) {
//        [_frameArray addObject:(__bridge id)sampleBuf];
//    }

    status = VTDecompressionSessionDecodeFrame(_decompression,
                                               sampleBuf,
                                               decodeFlags,
                                               NULL,
                                               &infoFlag);
    if (status != noErr) {
        errstr = [NSString stringWithFormat:@"decode frame failed:%d", (int)status];
    }
    
failed:
    if (noErr != status) {
        RELEASE_REF(sampleBuf);
        NSLog(@"decode, %@", errstr);
    }
    RELEASE_REF(blockRef);
    return status;
}

#pragma mark decode callback
void decompressionOutputCallback(void *decompressionOutputRefCon,
                                 void *sourceFrameRefCon,
                                 OSStatus status,
                                 VTDecodeInfoFlags infoFlags,
                                 CVImageBufferRef imageBuffer,
                                 CMTime presentationTimeStamp,
                                 CMTime presentationDuration)
{
    __weak __block VideoDecoder * weakSelf = (__bridge VideoDecoder *)(decompressionOutputRefCon);
    if (status != noErr) {
        [weakSelf releaseFrame];
        return;
    }
    
    CVPixelBufferLockBaseAddress(imageBuffer, 0);
    size_t width            = CVPixelBufferGetWidth(imageBuffer);
    size_t height           = CVPixelBufferGetHeight(imageBuffer);
    size_t yStride          = CVPixelBufferGetBytesPerRowOfPlane(imageBuffer, 0);
    size_t uvStride         = CVPixelBufferGetBytesPerRowOfPlane(imageBuffer, 1);
    const uint8_t * yAddr   = CVPixelBufferGetBaseAddressOfPlane(imageBuffer, 0);
    const uint8_t * uvAddr  = CVPixelBufferGetBaseAddressOfPlane(imageBuffer, 1);

    if (width != weakSelf.width || height != weakSelf.height) {
        NSLog(@"%s, new[%zu, %zu], origin[%zu, %zu]", __FUNCTION__, width, height, weakSelf.width, weakSelf.height);
        
        if (width * height * 3 / 2 > weakSelf.width * weakSelf.height * 2) {
            RELEASE_PTR(weakSelf.decodebuf);
            weakSelf.width      = width;
            weakSelf.height     = height;
            weakSelf.decodebuf  = (char *)malloc(weakSelf.width * weakSelf.height * 2);
            assert(weakSelf.decodebuf);
        }
    }
    
    uint8_t * yBuf  = (uint8_t *)weakSelf.decodebuf;
    uint8_t * uBuf  = (uint8_t *)(weakSelf.decodebuf + width * height);
    uint8_t * vBuf  = (uint8_t *)(weakSelf.decodebuf + width * height * 5 / 4);
    
    if(width == yStride) {
        memcpy((uint8_t *)yBuf, yAddr, width * height);
    } else {
        for(size_t i = 0; i < height; i++) {
            memcpy((uint8_t *)yBuf + i * width, (uint8_t *)yAddr + i * yStride, width);
        }
    }
    
    size_t uvWidth     = width / 2;
    size_t uvHeight    = height / 2;
    
    for(size_t i = 0; i < uvHeight; i ++) {
        uint8_t * uvReadAddr  = (uint8_t *)(uvAddr + i * uvStride);

        uint8_t * uPos = uBuf + uvWidth * i;
        uint8_t * vPos = vBuf + uvWidth * i;
        
        for(int j = 0; j < width; j += 2) {
            *vPos ++ = *uvReadAddr ++;
            *uPos ++ = *uvReadAddr ++;
        }
    }
    CVPixelBufferUnlockBaseAddress(imageBuffer, 0);
    
    [weakSelf decodeEnd];
    [weakSelf releaseFrame];
    return;
}

#pragma mark implementation from NSobject
- (void) dealloc
{
    [self deinit];
}

#pragma mark private functions

- (void) releaseFrame
{
    return;
    @synchronized (_frameArray) {
        CMSampleBufferRef toDel = (__bridge CMSampleBufferRef)[_frameArray objectAtIndex:0];
        [_frameArray removeObjectAtIndex:0];
        RELEASE_REF(toDel);
    }
}

- (void) decodeEnd
{
    if (_callback != NULL) {
        _callback(_decodebuf, (int32_t)_width, (int32_t)_height);
    }
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
