//
//  OSWatcher.m
//  hardwareCodec
//
//  Created by IT－PC on 15-2-5.
//  Copyright (c) 2015年 YY. All rights reserved.
//

#import "OSWatcher.h"

#import <UIKit/UIKit.h>

@interface OSWatcher()
@property bool mAppForeground;
@property appStateChangeListener mListener;
@end

@implementation OSWatcher

- (id) initWithListener:(appStateChangeListener)listener
{
    if (self = [super init]) {
        _mAppForeground = [UIApplication sharedApplication].applicationState == UIApplicationStateActive;
        
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(onUIApplicationDidEnterBackgroundNotification)
                                                     name:UIApplicationDidEnterBackgroundNotification
                                                   object:nil];
        
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(onUIApplicationWillEnterForegroundNotification)
                                                     name:UIApplicationWillEnterForegroundNotification
                                                   object:nil];
        _mListener = listener;
    }
    return self;
}

- (void) dealloc
{
    _mListener = NULL;
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void) notify
{
    if (_mListener != NULL) {
        _mListener(_mAppForeground);
    }
}

- (void) onUIApplicationDidEnterBackgroundNotification
{
    @synchronized (self) {
        _mAppForeground = false;
        [self notify];
    }
}

- (void) onUIApplicationWillEnterForegroundNotification
{
    @synchronized (self) {
        _mAppForeground = true;
        [self notify];
    }
}

- (bool) isAppActive
{
    bool active = true;
    @synchronized (self) {
        active = _mAppForeground;
    }
    return active;
}

@end
