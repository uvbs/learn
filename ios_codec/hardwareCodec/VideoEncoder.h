//
//  VideoEncoder.h
//  hardwareCodec
//
//  Created by IT－PC on 14-12-15.
//  Copyright (c) 2014年 YY. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef void (* EncodeCallback)(const char * data, int32_t size, bool keyframe);
typedef void (* onSizeChangeCallback)(int32_t encW, int32_t encH, int32_t actW, int32_t actH);

@interface VideoEncoder : NSObject

@property onSizeChangeCallback onSizeChange;
@property EncodeCallback callback;
@property int32_t encWidth;
@property int32_t encHeight;

- (id) initWithData:(int32_t)w height:(int32_t)h codeRate:(int32_t)cr frameRate:(int32_t)fr;

- (OSStatus) setupCompressionSession;

- (OSStatus) encode:(const char **)data strides:(const int32_t * )strides
             widths:(const int32_t *)widths heights:(const int32_t *)heights;

- (void) deinit;

- (OSStatus) setAverageBitrate:(int32_t)bitrate;
- (OSStatus) setProgressivescan:(BOOL)progressiveScan;
- (OSStatus) forceKeyFrame;
- (void) setFrameNum:(int32_t)framenum;
- (OSStatus) setIFrameVal:(int32_t)iframeVal;

- (OSStatus) forceCompleteEncode;
@end
