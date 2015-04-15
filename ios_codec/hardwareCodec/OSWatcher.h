//
//  OSWatcher.h
//  hardwareCodec
//
//  Created by IT－PC on 15-2-5.
//  Copyright (c) 2015年 YY. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef void (*appStateChangeListener)(bool);

@interface OSWatcher : NSObject

- (id) initWithListener:(appStateChangeListener)listener;

- (bool) isAppActive;

@end
