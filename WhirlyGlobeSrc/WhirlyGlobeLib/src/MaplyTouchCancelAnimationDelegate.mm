/*
 *  MaplyTouchCancelAnimationDelegate.mm
 *  WhirlyGlobeLib
 *
 *  Created by Jesse Crocker on 7/15/14.
 *  Copyright 2011-2015 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */


#import "MaplyTouchCancelAnimationDelegate.h"

@implementation MaplyTouchCancelAnimationDelegate

+ (MaplyTouchCancelAnimationDelegate*)touchDelegateForView:(UIView *)view mapView:(MaplyView*)mapView
{
    MaplyTouchCancelAnimationDelegate *touchDelegate = [[MaplyTouchCancelAnimationDelegate alloc] initWithMapView:mapView];
    touchDelegate.gestureRecognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:touchDelegate
                                                                                    action:@selector(touchGesture:)];

    ((UILongPressGestureRecognizer*)touchDelegate.gestureRecognizer).minimumPressDuration = 0.01;
    touchDelegate.gestureRecognizer.delegate = touchDelegate;
    [view addGestureRecognizer:touchDelegate.gestureRecognizer];
    return touchDelegate;
}


- (instancetype)initWithMapView:(MaplyView *)inView
{
    self = [super init];
    if(self)
    {
        self.mapView = inView;
    }
    return self;
}


- (void)touchGesture:(id)sender
{
    UIGestureRecognizer *recognizer = sender;
    if(recognizer.state == UIGestureRecognizerStateBegan) {
        if(self.mapView.delegate)
        {
            [self.mapView cancelAnimation];
        }
    }
    
    //Disable and reenable so other gesture recognizers will work
    self.gestureRecognizer.enabled = NO;
    self.gestureRecognizer.enabled = YES;
}


- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return YES;
}

@end
