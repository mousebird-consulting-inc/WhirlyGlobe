/*
 *  MaplyActiveSticker.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 4/8/13.
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

#import "MaplyActiveObject.h"
#import "MaplyCoordinate.h"
#import "MaplySticker.h"

/** The Active Sticker can be modified on the main thread.
    Changes will shown up in the next frame.  You can use
    this for animation.
  */
@interface MaplyActiveSticker : MaplyActiveObject

/** This is what the current sticker looks like.
    Assign a new one to change things.  Don't just change
     the sticker itself.  If you do, assign it here again
     so we know that you did.
  */
- (void)setActiveSticker:(MaplySticker *)newSticker desc:(NSDictionary *)desc;

@end
