//
//  VideoDecoder.h
//  hardwareCodec
//
//  Created by IT－PC on 14-12-15.
//  Copyright (c) 2014年 YY. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef void (* DecodeCallback)(const char * data, int32_t width, int32_t height);

@interface VideoDecoder : NSObject

@property DecodeCallback callback;

- (id) initWithData:(const char *)sps sLen:(size_t)sLen pps:(const char *)pps pLen:(size_t)pLen
      naluHeaderLen:(int32_t)headerLen;

- (BOOL) isSame:(const char *)sps sLen:(size_t)sLen pps:(const char *)pps pLen:(size_t)pLen
  naluHeaderLen:(int32_t)headerLen;

- (OSStatus) setupDecomressionSession;

- (OSStatus) decode:(const char *)data size:(size_t)size;

- (void) deinit;

@end
