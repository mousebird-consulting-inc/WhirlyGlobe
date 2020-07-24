/*
 *  MaplyCoordinate.m
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

#import "math/MaplyCoordinate.h"
#import <WhirlyGlobe_iOS.h>

using namespace WhirlyKit;

MaplyCoordinate MaplyCoordinateMake(float radLon,float radLat)
{
    MaplyCoordinate coord;
    coord.x = radLon;
    coord.y = radLat;
    
    return coord;
}

MaplyCoordinateD MaplyCoordinateDMake(double radLon,double radLat)
{
    MaplyCoordinateD coord;
    coord.x = radLon;
    coord.y = radLat;
    
    return coord;
}


MaplyCoordinate MaplyCoordinateMakeWithDegrees(float degLon,float degLat)
{
    MaplyCoordinate coord;
    coord.x = DegToRad(degLon);
    coord.y = DegToRad(degLat);
    
    return coord;
}

MaplyCoordinateD MaplyCoordinateDMakeWithDegrees(double degLon,double degLat)
{
    MaplyCoordinateD coord;
    coord.x = DegToRad(degLon);
    coord.y = DegToRad(degLat);
    
    return coord;
}

MaplyCoordinateD MaplyCoordinateDMakeWithMaplyCoordinate(MaplyCoordinate c)
{
    MaplyCoordinateD coord;
    coord.x = c.x;
    coord.y = c.y;
    return coord;
}

MaplyCoordinate3d MaplyCoordinate3dMake(float x,float y,float z)
{
    MaplyCoordinate3d coord;
    coord.x = x;  coord.y = y;  coord.z = z;
    return coord;
}

MaplyCoordinate3dD MaplyCoordinate3dDMake(double x,double y,double z)
{
    MaplyCoordinate3dD coord;
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

MaplyBoundingBoxD MaplyBoundingBoxDMakeWithDegrees(double degLon0,double degLat0,double degLon1,double degLat1)
{
    MaplyBoundingBoxD bbox;
    bbox.ll = MaplyCoordinateDMakeWithDegrees(degLon0, degLat0);
    bbox.ur = MaplyCoordinateDMakeWithDegrees(degLon1, degLat1);
    
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

bool MaplyBoundingBoxContains(MaplyBoundingBox bbox, MaplyCoordinate c)
{
    Mbr mbr;
    Point2f point = Point2f(c.x, c.y);
    mbr.ll() = Point2f(bbox.ll.x,bbox.ll.y);
    mbr.ur() = Point2f(bbox.ur.x,bbox.ur.y);

    return mbr.insideOrOnEdge(point);
}

MaplyBoundingBox MaplyBoundingBoxFromLocations(const CLLocationCoordinate2D locs[], unsigned int numLocs)
{
    Mbr mbr;

    for (unsigned int ii=0;ii<numLocs;ii++) {
        CLLocationCoordinate2D loc = locs[ii];
        MaplyCoordinate coord = MaplyCoordinateMakeWithDegrees(loc.longitude, loc.latitude);
        mbr.addPoint(Point2d(coord.x,coord.y));
    }
        
    MaplyBoundingBox ret;
    ret.ll.x = mbr.ll().x();  ret.ll.y = mbr.ll().y();
    ret.ur.x = mbr.ur().x();  ret.ur.y = mbr.ur().y();

    return ret;
}

MaplyBoundingBox MaplyBoundingBoxIntersection(MaplyBoundingBox bbox0,MaplyBoundingBox bbox1)
{
    Mbr mbr0;
    mbr0.ll() = Point2f(bbox0.ll.x,bbox0.ll.y);
    mbr0.ur() = Point2f(bbox0.ur.x,bbox0.ur.y);
    Mbr mbr1;
    mbr1.ll() = Point2f(bbox1.ll.x,bbox1.ll.y);
    mbr1.ur() = Point2f(bbox1.ur.x,bbox1.ur.y);
    Mbr inter = mbr0.intersect(mbr1);
    
    MaplyBoundingBox ret;
    ret.ll.x = inter.ll().x();  ret.ll.y = inter.ll().y();
    ret.ur.x = inter.ur().x();  ret.ur.y = inter.ur().y();
    
    return ret;
}

MaplyBoundingBox MaplyBoundingBoxExpandByFraction(MaplyBoundingBox bbox, float buffer)
{
    Mbr mbr;
    mbr.ll() = Point2f(bbox.ll.x,bbox.ll.y);
    mbr.ur() = Point2f(bbox.ur.x,bbox.ur.y);

    mbr.expandByFraction(buffer);

    MaplyBoundingBox r;
    r.ll = MaplyCoordinateMake(mbr.ll().x(), mbr.ll().y());
    r.ur = MaplyCoordinateMake(mbr.ur().x(), mbr.ur().y());

    return r;
}



double MaplyGreatCircleDistance(MaplyCoordinate p0,MaplyCoordinate p1)
{
    double delta = acos(sin(p0.y)*sin(p1.y) + cos(p0.y)*cos(p1.y)*cos(p1.x-p0.x));
    return delta * EarthRadius;
}

@implementation MaplyCoordinate3dWrapper

- (instancetype)initWithCoord:(MaplyCoordinate3d)coord
{
    self = [super init];
    _coord = coord;
    
    return self;
}

@end

@implementation MaplyCoordinate3dDWrapper

- (instancetype)initWithCoord:(MaplyCoordinate3dD)coord
{
    self = [super init];
    _coord = coord;
    
    return self;
}

@end
