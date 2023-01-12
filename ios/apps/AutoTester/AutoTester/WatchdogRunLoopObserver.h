//
//  WatchdogRunLoopObserver.h
//  AutoTester
//
//  Created by Tim Sylvester on 1/12/23.
//  Copyright © 2023 mousebird consulting. All rights reserved.
//

#ifndef WatchdogRunLoopObserver_h
#define WatchdogRunLoopObserver_h

// See https://www.jessesquires.com/blog/2022/08/11/implementing-a-main-thread-watchdog-on-ios/

@protocol WatchdogRunLoopObserverDelegate <NSObject>
- (void)runLoopDidStallWithDuration:(NSTimeInterval)duration;
@end

@interface WatchdogRunLoopObserver : NSObject
@property (nonatomic, weak, nullable) id<WatchdogRunLoopObserverDelegate> delegate;
- (instancetype _Nullable)init;
- (instancetype _Nullable)initWithRunLoop:(CFRunLoopRef)runLoop stallingThreshold:(NSTimeInterval)threshold;
- (void)start;
- (void)stop;
@end

#endif /* WatchdogRunLoopObserver_h */
