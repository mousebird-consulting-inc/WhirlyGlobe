/*
 *  WGInteractionLayer_private.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
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

#import <Foundation/Foundation.h>
#import "MaplyBaseInteractionLayer_private.h"

// The view controller fills this in
@protocol WGInteractionLayerDelegate <NSObject>
// Called back on the main thread for selection
- (void)handleSelection:(WhirlyGlobeTapMessage *)msg didSelect:(NSArray *)selectedObjs;
@end

/** The Interaction Layer runs in the layer thread (mostly) and manages
    data added to the scene and various other layers.
 */
@interface WGInteractionLayer : MaplyBaseInteractionLayer
{
    WhirlyGlobeView * __weak globeView;

    // If set, we'll autorotate after a certain amount of time
    float autoRotateInterval,autoRotateDegrees;

    // Last time something was tapped
    NSTimeInterval lastTouched;
}

// The view controller, for various callbacks
@property (nonatomic,weak) NSObject<WGInteractionLayerDelegate> * viewController;

// Initialize with the globeView
-(id)initWithGlobeView:(WhirlyGlobeView *)globeView;

// Set the autorotate values
- (void)setAutoRotateInterval:(float)autoRotateInterval degrees:(float)autoRotateDegrees;

// Call this to process a tap with the selection layer
// It will call the given selector if there was no selection
- (void) userDidTap:(WhirlyGlobeTapMessage *)msg;

@end
