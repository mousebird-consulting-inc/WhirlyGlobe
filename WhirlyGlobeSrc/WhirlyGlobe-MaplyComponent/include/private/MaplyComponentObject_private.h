/*
 *  WGComponentObject_private.h
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
#import <WhirlyGlobe.h>
#import "MaplyVectorObject.h"
#import "MaplyComponentObject.h"
#import "MaplyTexture_private.h"

/** The Component Object is used to track all the resources a user created
    to represent something.  We pass this back to them so they can remove
    those resources later.
 */
@interface MaplyComponentObject()

@property (nonatomic,assign) WhirlyKit::SimpleIDSet &markerIDs;
@property (nonatomic,assign) WhirlyKit::SimpleIDSet &labelIDs;
@property (nonatomic,assign) WhirlyKit::SimpleIDSet &vectorIDs;
@property (nonatomic,assign) WhirlyKit::SimpleIDSet &wideVectorIDs;
@property (nonatomic,assign) WhirlyKit::SimpleIDSet &shapeIDs;
@property (nonatomic,assign) WhirlyKit::SimpleIDSet &chunkIDs;
@property (nonatomic,assign) WhirlyKit::SimpleIDSet &loftIDs;
@property (nonatomic,assign) WhirlyKit::SimpleIDSet &billIDs;
@property (nonatomic,assign) WhirlyKit::SimpleIDSet &geomIDs;
@property (nonatomic,assign) WhirlyKit::SimpleIDSet &selectIDs;
@property (nonatomic,assign) std::set<MaplyTexture *> &textures;
@property (nonatomic,strong) NSArray *vectors;
@property (nonatomic) WhirlyKit::Point2d &vectorOffset;
@property (nonatomic,assign) bool isSelectable;
@property (nonatomic,assign) bool enable;
@property (nonatomic,assign) bool underConstruction;

@end
