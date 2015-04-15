//
//  hw_codec.h
//  hardwareCodec
//
//  Created by IT－PC on 14-12-16.
//  Copyright (c) 2014年 YY. All rights reserved.
//

#ifndef _HARDWARE_CODEC_H__
#define _HARDWARE_CODEC_H__

#import <string>

#ifdef __cplusplus
extern "C" {
#endif
    
    bool iOSHardwareCodecSupport();
    
    void enable_ios_hardware_video_codec(bool enable);

    void enable_hardware_codec_debug(bool enable);
    
    class IHardwareEncodeCallback {
    public:
        virtual void onHardwareEncodeCallback(const char * data, int32_t size, bool keyframe) = 0;
        virtual void onAdjustSize(int32_t encW, int32_t encH, int32_t actW, int32_t actH) = 0;
    };

#pragma mark encoder
    
    typedef void (* hw_encode_callback)(const char * data, int32_t size);
    typedef void (* hw_encode_resize)(int32_t width, int32_t height);

    bool hw_video_encoder_init(int width, int height, int codeRate, int frameRate, int rcMethod,
                            IHardwareEncodeCallback * cb);

    bool hw_video_encode(const char ** planes, const int32_t * strides,
                         const int32_t * widths, const int32_t * heights);
    
    void hw_video_encoder_deinit();
    
    void hw_video_set_code_rate(int32_t coderate);
    void hw_video_force_key_frame();
    void hw_video_set_frame_rate(int32_t framerate);
    void hw_video_enable_progress_scan(bool enable);
    void hw_video_set_iframe_value(int32_t iframeval);
    
#pragma mark decoder

    class IHardwareDecodeCallback {
    public:
        virtual void onHardwareDecodeCallback(const char * data, int32_t width, int32_t height) = 0;
    };

    bool hw_video_decoder_init(const char * sps, size_t sps_len, const char * pps, size_t pps_len,
                            int32_t naluHeaderLen, IHardwareDecodeCallback * cb);

    bool hw_video_decode(const char * data, size_t size);
    
    void hw_video_decoder_deinit();

#ifdef __cplusplus
}
#endif
    
#endif
