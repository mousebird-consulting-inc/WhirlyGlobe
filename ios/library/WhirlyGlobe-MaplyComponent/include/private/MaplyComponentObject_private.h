/*
 *  WGComponentObject_private.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
 *  Copyright 2011-2019 mousebird consulting
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
#import <WhirlyGlobe_iOS.h>
#import "visual_objects/MaplyVectorObject.h"
#import "visual_objects/MaplyComponentObject.h"
#import "MaplyTexture_private.h"

/** The Component Object is used to track all the resources a user created
    to represent something.  We pass this back to them so they can remove
    those resources later.
 */
@interface MaplyComponentObject()
{
@public
    // A WhirlyKit object holds most of this stuff
    WhirlyKit::ComponentObject_iOSRef contents;
}

- (id)initWithRef:(WhirlyKit::ComponentObject_iOSRef)compObj;

@end
