/*
 *  WGVectorObject_private.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 8/2/12.
 *  Copyright 2012-2019 mousebird consulting
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

#import "visual_objects/MaplyVectorObject.h"
#import <WhirlyGlobe_iOS.h>

@interface MaplyVectorObject()
{
@public
    WhirlyKit::VectorObjectRef vObj;
    // If the developer requests attributes, we need to convert them to
    //  a friendly representation
    NSMutableDictionary *attrCache;
}

// Construct as a wrapper
- (id)initWithRef:(WhirlyKit::VectorObjectRef)vecObj;

// Construct a vector object from the Vector DB raw format
- (void)addShape:(WhirlyKit::VectorShapeRef)shape;

@end
