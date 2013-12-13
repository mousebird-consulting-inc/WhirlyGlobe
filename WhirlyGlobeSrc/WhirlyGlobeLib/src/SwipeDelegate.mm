/*
 *  SwipeDelegate.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/17/11.
 *  Copyright 2011-2013 mousebird consulting
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

#import "SwipeDelegate.h"


@implementation WhirlyGlobeSwipeDelegate
{
    WhirlyGlobe::GlobeView *view;
}

- (id)initWithGlobeView:(WhirlyGlobe::GlobeView *)inView
{
	if ((self = [super init]))
	{
		view = inView;
	}
	
	return self;
}

+ (WhirlyGlobeSwipeDelegate *)swipeDelegateForView:(UIView *)view globeView:(WhirlyGlobe::GlobeView *)globeView
{
	WhirlyGlobeSwipeDelegate *swipeDelegate = [[WhirlyGlobeSwipeDelegate alloc] initWithGlobeView:globeView];
	[view addGestureRecognizer:[[UISwipeGestureRecognizer alloc] initWithTarget:swipeDelegate action:@selector(swipeGesture:)]];
	return swipeDelegate;
}

// We'll let other gestures run
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return TRUE;
}

// Called for swipe actions
- (void)swipeGesture:(id)sender
{
//	UISwipeGestureRecognizer *swipe = sender;
	
	// Only one state, since this is not continuous
}	


@end
