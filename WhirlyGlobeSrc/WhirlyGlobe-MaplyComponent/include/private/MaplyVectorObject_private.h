/*
 *  WGVectorObject_private.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 8/2/12.
 *  Copyright 2012-2015 mousebird consulting
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

#import "MaplyVectorObject.h"
#import <WhirlyGlobe.h>

namespace Maply
{

typedef enum {VectorAttrInt=0,VectorAttrString,VectorAttrReal,VectorAttrMax} VectorAttributeType;

// Vector attributes in a vector DB
class VectorAttribute
{
public:
    NSString *name;
    VectorAttributeType type;
};

}

@interface MaplyVectorObject()

@property (nonatomic,readonly) WhirlyKit::ShapeSet &shapes;
@property (nonatomic,readwrite) NSMutableDictionary *attributes;

// Construct a vector object from the Vector DB raw format
+ (MaplyVectorObject *)VectorObjectFromVectorDBRaw:(NSData *)data;
- (void)addShape:(WhirlyKit::VectorShapeRef)shape;

@end

@interface MaplyVectorDatabase() <WhirlyKitLoftedPolyCache>

@end
