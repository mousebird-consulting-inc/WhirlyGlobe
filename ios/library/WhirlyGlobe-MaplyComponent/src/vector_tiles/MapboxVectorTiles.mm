/*
 *  MaplyMapnikVectorTiles.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Jesse Crocker, Trailbehind inc. on 3/31/14.
 *  Recreated by Steve Gifford on 4/10/19.
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

#import "MapboxVectorTiles_private.h"
#import "MaplyComponentObject_private.h"

using namespace WhirlyKit;

@implementation MaplyVectorTileData

- (id)initWithTileData:(WhirlyKit::VectorTileDataRef)tileData
{
    self = [super init];
    data = tileData;
    
    return self;
}

- (id)initWithID:(MaplyTileID)tileID bbox:(MaplyBoundingBoxD)bbox geoBBox:(MaplyBoundingBoxD)geoBBox
{
    self = [super init];
    data = VectorTileDataRef(new VectorTileData());
    
    data->ident.x = tileID.x;
    data->ident.y = tileID.y;
    data->ident.level = tileID.level;
    
    data->bbox.ll() = Point2d(bbox.ll.x,bbox.ll.y);
    data->bbox.ur() = Point2d(bbox.ur.x,bbox.ur.y);
    data->geoBBox.ll() = Point2d(geoBBox.ll.x,geoBBox.ll.y);
    data->geoBBox.ur() = Point2d(geoBBox.ur.x,geoBBox.ur.y);
    
    return self;
}

- (MaplyTileID) tileID
{
    MaplyTileID newTileID;
    newTileID.x = data->ident.x;
    newTileID.y = data->ident.y;
    newTileID.level = data->ident.level;
    
    return newTileID;
}

- (MaplyBoundingBoxD)bounds
{
    MaplyBoundingBoxD ret;
    ret.ll.x = data->bbox.ll().x();  ret.ll.y = data->bbox.ll().y();
    ret.ur.x = data->bbox.ur().x();  ret.ur.y = data->bbox.ur().y();
    
    return ret;
}

- (MaplyBoundingBoxD)geoBounds
{
    MaplyBoundingBoxD ret;
    ret.ll.x = data->geoBBox.ll().x();  ret.ll.y = data->geoBBox.ll().y();
    ret.ur.x = data->geoBBox.ur().x();  ret.ur.y = data->geoBBox.ur().y();
    
    return ret;
}

- (void)addComponentObject:(MaplyComponentObject *)compObj
{
    if (!compObj)
        return;
    
    data->compObjs.push_back(compObj->contents);
}

- (void)addComponentObjects:(NSArray *)inCompObjs
{
    if (!inCompObjs)
        return;

    for (MaplyComponentObject *compObj in inCompObjs)
        data->compObjs.push_back(compObj->contents);
}

- (NSArray *)componentObjects
{
    NSMutableArray *ret = [[NSMutableArray alloc] init];
    for (auto compObj : data->compObjs) {
        ComponentObject_iOSRef compObjIOS = std::dynamic_pointer_cast<ComponentObject_iOS>(compObj);
        MaplyComponentObject *newCompObj = [[MaplyComponentObject alloc] initWithRef:compObjIOS];
        [ret addObject:newCompObj];
    }
    
    return ret;
}

- (void)mergeFrom:(MaplyVectorTileData *)tileData
{
    data->mergeFrom(tileData->data.get());
}

- (void)clear
{
    data->clear();
}

@end
