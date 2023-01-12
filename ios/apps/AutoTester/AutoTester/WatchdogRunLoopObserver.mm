//
//  WatchdogRunLoopObserver.mm
//  AutoTester
//
//  Created by Tim Sylvester on 1/12/23.
//  Copyright © 2023 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "WatchdogRunLoopObserver.h"

#include <mach/mach_time.h>


static const NSTimeInterval DefaultStallingThreshold = 4;
@interface WatchdogRunLoopObserver ()
@property (nonatomic, assign, readonly) CFRunLoopRef runLoop;
@property (nonatomic, assign, readonly) CFRunLoopObserverRef observer;
@property (nonatomic, assign, readonly) NSTimeInterval threshold;
@property (nonatomic, assign) uint64_t startTime;
@end

@implementation WatchdogRunLoopObserver
- (instancetype)init {
    return [self initWithRunLoop:CFRunLoopGetMain() stallingThreshold:DefaultStallingThreshold];
}

- (instancetype)initWithRunLoop:(CFRunLoopRef)runLoop
              stallingThreshold:(NSTimeInterval)threshold {
    NSParameterAssert(runLoop != NULL);
    NSParameterAssert(threshold > 0);

    if (!(self = [super init])) {
        return nil;
    }

    _runLoop = (CFRunLoopRef)CFRetain(runLoop);
    _threshold = threshold;
    _startTime = 0;

    // Pre-calculate timebase information.
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    const NSTimeInterval secondsPerMachTime = timebase.numer / timebase.denom / 1e9;

    // Observe at an extremely low order so that we can catch stalling even in
    // high-priority operations (like UI redrawing or animation).
    __weak WatchdogRunLoopObserver *weakSelf = self;
    _observer = CFRunLoopObserverCreateWithHandler(NULL, kCFRunLoopAllActivities, YES, INT_MIN,
                                                   ^(CFRunLoopObserverRef observer, CFRunLoopActivity activity) {
        WatchdogRunLoopObserver *strongSelf = weakSelf;
        if (!strongSelf) {
            return;
        }
        switch (activity) {
            // What we consider one "iteration" might start with any one of these events.
            case kCFRunLoopEntry:
            case kCFRunLoopBeforeTimers:
            case kCFRunLoopAfterWaiting:
            case kCFRunLoopBeforeSources:
                if (strongSelf.startTime == 0) {
                    strongSelf.startTime = mach_absolute_time();
                }
                break;
            case kCFRunLoopBeforeWaiting:
            case kCFRunLoopExit: {
                uint64_t endTime = mach_absolute_time();
                if (strongSelf.startTime <= 0) {
                    break;
                }
                const uint64_t elapsed = endTime - strongSelf.startTime;
                const NSTimeInterval duration = elapsed * secondsPerMachTime;
                if (duration > strongSelf.threshold) {
                    [strongSelf iterationStalledWithDuration:duration];
                }
                strongSelf.startTime = 0;
                break;
            }
            default:
                NSAssert(NO, @"WatchdogRunLoopObserver should not have been triggered for activity %i", (int)activity);
        }
    });

    return _observer ? self : nil;
}

- (void)dealloc {
    if (_observer != NULL) {
        CFRunLoopObserverInvalidate(_observer);

        CFRelease(_observer);
        _observer = NULL;
    }

    if (_runLoop != NULL) {
        CFRelease(_runLoop);
        _runLoop = NULL;
    }
}

- (void)start {
    CFRunLoopAddObserver(self.runLoop, self.observer, kCFRunLoopCommonModes);
}

- (void)stop {
    CFRunLoopRemoveObserver(self.runLoop, self.observer, kCFRunLoopCommonModes);
}

- (void)iterationStalledWithDuration:(NSTimeInterval)duration {
    [self.delegate runLoopDidStallWithDuration:duration];
}

@end
