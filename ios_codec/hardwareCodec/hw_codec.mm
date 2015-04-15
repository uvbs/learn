//
//  hw_codec.mm
//  hardwareCodec
//
//  Created by IT－PC on 14-12-16.
//  Copyright (c) 2014 YY. All rights reserved.
//
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#import "hw_codec.h"
#import "VideoEncoder.h"
#import "VideoDecoder.h"
#import "OSWatcher.h"

#ifdef __cplusplus
extern "C" {
#endif
    static bool sEnableHW = false;
    static bool sEnableDebug = false;

    
    bool iOSHardwareCodecSupport()
    {
        float version = [[[UIDevice currentDevice] systemVersion] floatValue];
        return (version >= 8.0) && (sEnableHW);
    }
    
    void enable_ios_hardware_video_codec(bool enable)
    {
        if (sEnableDebug)
            sEnableHW = true;
        else
            sEnableHW = enable;
    }

    void enable_hardware_codec_debug(bool enable)
    {
        sEnableDebug = enable;
        if (sEnableDebug)
            sEnableHW = true;
    }
    
#pragma mark os watcher, background & foreground switch.
    
    static VideoEncoder * sEncoder = nil;
    static VideoDecoder * sDecoder = nil;
    
    void appStateChange(bool active) {
        if (sEncoder != nil) {
            hw_video_encoder_deinit();
        }
        
        if (sDecoder != nil) {
            hw_video_decoder_deinit();
        }
    }
    
    static OSWatcher * sOSWatcher = [[OSWatcher alloc] initWithListener:appStateChange];
    
    class EncoderParamCache {
    public:
        EncoderParamCache() : width(0), height(0),
            coderate(0), framerate(0),
            callback(NULL),
            valid(false) {
        }
        
        ~EncoderParamCache() {
            valid = false;
            callback = NULL;
        }
        
        int width;
        int height;
        int coderate;
        int framerate;
        int rcmethod;
        IHardwareEncodeCallback * callback;
        bool valid;
    };
    
    static EncoderParamCache sEncodeParam;
    
#pragma mark video encode

    static IHardwareEncodeCallback * sHardwareCb = NULL;

    static void encode_callback(const char * data, int32_t size, bool keyFrame)
    {
        if (sHardwareCb != NULL) {
            sHardwareCb->onHardwareEncodeCallback(data, size, keyFrame);
        }
    }
    
    static void encode_size(int32_t encW, int32_t encH, int32_t actW, int32_t actH)
    {
        if (sHardwareCb != NULL) {
            sHardwareCb->onAdjustSize(encW, encH, actW, actH);
        }
    }

    bool hw_video_encoder_init(int width, int height, int codeRate, int frameRate, int rcMethod,
                            IHardwareEncodeCallback * cb)
    {
        printf("[%s:%u]\n", __FUNCTION__, __LINE__);
        
        sEncodeParam.width      = width;
        sEncodeParam.height     = height;
        sEncodeParam.coderate   = codeRate;
        sEncodeParam.framerate  = frameRate;
        sEncodeParam.callback   = cb;
        sEncodeParam.valid      = true;
        
        if (![sOSWatcher isAppActive]) {
            return true;
        }
        
        OSStatus status = noErr;
        if (sEncoder != nil && sEncoder.encWidth == width && sEncoder.encHeight == height) {
            [sEncoder setAverageBitrate:codeRate];
            [sEncoder setFrameNum:frameRate];
        } else {
            hw_video_encoder_deinit();
            
            sEncoder = [[VideoEncoder alloc] initWithData:width height:height codeRate:codeRate
                                                frameRate:frameRate];
            status = [sEncoder setupCompressionSession];
        }
        
        if (status != noErr) {
            return false;
        }
        sHardwareCb = cb;
        sEncoder.callback = encode_callback;
        sEncoder.onSizeChange = encode_size;
        return true;
    }

    bool hw_video_encode(const char ** planes, const int32_t * strides,
                         const int32_t * widths, const int32_t * heights)
    {
        if (![sOSWatcher isAppActive]) {
            if (sEncoder != nil) {
                hw_video_encoder_deinit();
            }
            return false;
        }

        OSStatus status;
        
        if (sEncoder == nil) {
            if (sEncodeParam.valid) {
                status = hw_video_encoder_init(sEncodeParam.width,
                                               sEncodeParam.height,
                                               sEncodeParam.coderate,
                                               sEncodeParam.framerate,
                                               sEncodeParam.rcmethod,
                                               sEncodeParam.callback);
                if (status != noErr) {
                    return false;
                }
            } else {
                return false;
            }
        }

        status = [sEncoder encode:planes strides:strides widths:widths heights:heights];

        return status == noErr ? true : false;
    }

    void hw_video_encoder_deinit()
    {
        printf("[%s:%u]\n", __FUNCTION__, __LINE__);
        if (sEncoder != nil) {
            [sEncoder forceCompleteEncode];
            [sEncoder deinit];
            sEncoder = nil;
        }
        sHardwareCb = NULL;
        sEncoder = nil;
    }
    
    void hw_video_set_code_rate(int32_t coderate)
    {
        if (sEncoder != nil) {
            [sEncoder setAverageBitrate:coderate];
        }
    }
    
    void hw_video_force_key_frame()
    {
        if (sEncoder != nil) {
            [sEncoder forceKeyFrame];
        }
    }
    
    void hw_video_set_frame_rate(int32_t framerate)
    {
        if (sEncoder != nil) {
            [sEncoder setFrameNum:framerate];
        }
    }
    
    void hw_video_enable_progress_scan(bool enable)
    {
        if (sEncoder != nil) {
            [sEncoder setProgressivescan:enable ? YES : NO];
        }
    }
    
    void hw_video_set_iframe_value(int32_t iframeval)
    {
        if (sEncoder != nil) {
            [sEncoder setIFrameVal:iframeval];
        }
    }

    #pragma mark video decoder

    static IHardwareDecodeCallback * sHardwareDecodeCb = NULL;

    static void decode_callback(const char * data, int32_t width, int32_t height)
    {
        if (sHardwareDecodeCb != NULL) {
            sHardwareDecodeCb->onHardwareDecodeCallback(data, width, height);
        }
    }

    bool hw_video_decoder_init(const char * sps, size_t sps_len, const char * pps, size_t pps_len,
                               int32_t naluHeaderLen, IHardwareDecodeCallback * cb)
    {
        if (![sOSWatcher isAppActive]) {
            return false;
        }

        BOOL needRecreate = YES;
        if (sDecoder != nil) {
            needRecreate = ([sDecoder isSame:sps sLen:sps_len pps:pps pLen:pps_len
                               naluHeaderLen:naluHeaderLen] == NO);
        }
        
        OSStatus status = noErr;
        if (needRecreate) {
            hw_video_decoder_deinit();
            
            sDecoder = [[VideoDecoder alloc] initWithData:sps sLen:sps_len pps:pps pLen:pps_len naluHeaderLen:naluHeaderLen];
            
            sHardwareDecodeCb = cb;
            sDecoder.callback = decode_callback;
            
            status = [sDecoder setupDecomressionSession];
        }
        
        return noErr == status;
    }
    
    bool hw_video_decode(const char * data, size_t size)
    {
        OSStatus status = noErr;
        
        if (![sOSWatcher isAppActive]) {
            if (sDecoder != nil) {
                hw_video_decoder_deinit();
            }
            return false;
        }
        
        if (sDecoder == nil) {
            NSLog(@"empty hardware decoder...");
            return false;
        }
        
        status = [sDecoder decode:data size:size];
        return status == noErr;
    }
    
    void hw_video_decoder_deinit()
    {
        if (sDecoder != nil) {
            [sDecoder deinit];
            sDecoder = nil;
        }
        sHardwareDecodeCb = NULL;
    }
    
#ifdef __cplusplus
}
#endif
