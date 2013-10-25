/*
 *  MaplyCoordinate.m
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

#import "MaplyCoordinate.h"
#import <WhirlyGlobe.h>

using namespace WhirlyKit;

MaplyCoordinate MaplyCoordinateMakeWithDegrees(float degLon,float degLat)
{
    MaplyCoordinate coord;
    coord.x = DegToRad(degLon);
    coord.y = DegToRad(degLat);
    
    return coord;
}

MaplyCoordinate3d MaplyCoordinate3dMake(float x,float y,float z)
{
    MaplyCoordinate3d coord;
    coord.x = x;  coord.y = y;  coord.z = z;
    return coord;
}

MaplyBoundingBox MaplyBoundingBoxMakeWithDegrees(float degLon0,float degLat0,float degLon1,float degLat1)
{
    MaplyBoundingBox bbox;
    bbox.ll = MaplyCoordinateMakeWithDegrees(degLon0, degLat0);
    bbox.ur = MaplyCoordinateMakeWithDegrees(degLon1, degLat1);
    
    return bbox;
}

bool MaplyBoundingBoxesOverlap(MaplyBoundingBox bbox0,MaplyBoundingBox bbox1)
{
    Mbr mbr0,mbr1;
    mbr0.ll() = Point2f(bbox0.ll.x,bbox0.ll.y);
    mbr0.ur() = Point2f(bbox0.ur.x,bbox0.ur.y);
    mbr1.ll() = Point2f(bbox1.ll.x,bbox1.ll.y);
    mbr1.ur() = Point2f(bbox1.ur.x,bbox1.ur.y);
    
    return mbr0.overlaps(mbr1);
}