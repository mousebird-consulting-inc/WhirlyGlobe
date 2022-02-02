/*  GlobeDoubleTapDragDelegate_private.h
 *
 *  Created by Steve Gifford on 2/7/14.
 *  Copyright 2011-2022 mousebird consulting
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
 */

#import <Foundation/Foundation.h>
#import "GlobeView_iOS.h"
#import "GlobeAnimateHeight.h"

@interface WhirlyGlobeDoubleTapDragDelegate ()

// If set, we calculate the tilt every time we update
@property (nonatomic) WhirlyGlobe::TiltCalculatorRef tiltDelegate;

/// Create a double tap gesture and a delegate and wire them up to the given UIView
+ (WhirlyGlobeDoubleTapDragDelegate *)doubleTapDragDelegateForView:(UIView *)wrapView
                                                         globeView:(WhirlyGlobe::GlobeView_iOS *)globeView;

@end
