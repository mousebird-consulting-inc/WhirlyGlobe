/*  MaplyCoordinate.mm
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
 *  Copyright 2011-2021 mousebird consulting
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

#import "math/MaplyCoordinate.h"
#import <WhirlyGlobe_iOS.h>

using namespace WhirlyKit;

MaplyCoordinate MaplyCoordinateMake(float radLon,float radLat) { return { radLon, radLat }; }
MaplyCoordinateD MaplyCoordinateDMake(double radLon,double radLat) { return { radLon, radLat }; }
MaplyCoordinate MaplyCoordinateMakeWithDegrees(float degLon,float degLat) { return { DegToRad(degLon), DegToRad(degLat) }; }
MaplyCoordinateD MaplyCoordinateDMakeWithDegrees(double degLon, double degLat) { return { DegToRad(degLon), DegToRad(degLat) }; }
MaplyCoordinateD MaplyCoordinateDMakeWithMaplyCoordinate(MaplyCoordinate c) { return { c.x, c.y }; }
MaplyCoordinate MaplyCoordinateMakeWithMaplyCoordinateD(MaplyCoordinateD c) { return { (float)c.x, (float)c.y }; }
MaplyCoordinate3d MaplyCoordinate3dMake(float x, float y, float z) { return { x, y, z}; }
MaplyCoordinate3dD MaplyCoordinate3dDMake(double x, double y, double z) { return { x, y, z }; }

inline MaplyBoundingBox MaplyBoundingBoxMakeFromMbr(Mbr mbr) {
    return { MaplyCoordinateMake(mbr.ll().x(), mbr.ll().y()), MaplyCoordinateMake(mbr.ur().x(), mbr.ur().y()) };
}
inline MaplyBoundingBox MaplyBoundingBoxMakeFromMbrD(MbrD mbr) {
    return { MaplyCoordinateMake(mbr.ll().x(), mbr.ll().y()), MaplyCoordinateMake(mbr.ur().x(), mbr.ur().y()) };
}
inline MaplyBoundingBoxD MaplyBoundingBoxDMakeFromMbr(Mbr mbr) {
    return { MaplyCoordinateDMake(mbr.ll().x(), mbr.ll().y()), MaplyCoordinateDMake(mbr.ur().x(), mbr.ur().y()) };
}
inline MaplyBoundingBoxD MaplyBoundingBoxDMakeFromMbrD(MbrD mbr) {
    return { MaplyCoordinateDMake(mbr.ll().x(), mbr.ll().y()), MaplyCoordinateDMake(mbr.ur().x(), mbr.ur().y()) };
}

MaplyBoundingBox MaplyBoundingBoxMakeWithDegrees(float degLon0,float degLat0,float degLon1,float degLat1)
{
    return {
        MaplyCoordinateMakeWithDegrees(degLon0, degLat0),
        MaplyCoordinateMakeWithDegrees(degLon1, degLat1)
    };
}

MaplyBoundingBoxD MaplyBoundingBoxDMakeWithDegrees(double degLon0,double degLat0,double degLon1,double degLat1)
{
    return {
        MaplyCoordinateDMakeWithDegrees(degLon0, degLat0),
        MaplyCoordinateDMakeWithDegrees(degLon1, degLat1)
    };
}

bool MaplyBoundingBoxesOverlap(MaplyBoundingBox bbox0,MaplyBoundingBox bbox1)
{
    const Mbr mbr0(Point2f(bbox0.ll.x,bbox0.ll.y), Point2f(bbox0.ur.x,bbox0.ur.y));
    const Mbr mbr1(Point2f(bbox1.ll.x,bbox1.ll.y), Point2f(bbox1.ur.x,bbox1.ur.y));
    return mbr0.overlaps(mbr1);
}

bool MaplyBoundingBoxContains(MaplyBoundingBox bbox, MaplyCoordinate c)
{
    const Mbr mbr(Point2f(bbox.ll.x,bbox.ll.y), Point2f(bbox.ur.x,bbox.ur.y));
    return mbr.insideOrOnEdge(Point2f(c.x, c.y));
}

MaplyBoundingBox MaplyBoundingBoxFromLocations(const CLLocationCoordinate2D locs[], unsigned int numLocs)
{
    Mbr mbr;
    for (unsigned int ii=0;ii<numLocs;ii++) {
        const CLLocationCoordinate2D &loc = locs[ii];
        const MaplyCoordinate coord = MaplyCoordinateMakeWithDegrees(loc.longitude, loc.latitude);
        mbr.addPoint(Point2d(coord.x,coord.y));
    }
    return MaplyBoundingBoxMakeFromMbr(mbr);
}

namespace {
    template <typename TBox, typename TPoints>
    static TBox MbrAdd(TBox box, const TPoints pts[], unsigned count) {
        for (auto i = 0; i < count; ++i) {
            box.addPoint(Point2d(pts[i].x, pts[i].y));
        }
        return box;
    }
    template <typename TMbr>
    static MaplyBoundingBox ToBox(TMbr mbr) {
        return MaplyBoundingBox {
            MaplyCoordinate { mbr.ll().x(), mbr.ll().y() },
            MaplyCoordinate { mbr.ur().x(), mbr.ur().y() }
        };
    }
    template <typename TMbr>
    static MaplyBoundingBoxD ToBoxD(TMbr mbr) {
        return MaplyBoundingBoxD {
            MaplyCoordinateD { mbr.ll().x(), mbr.ll().y() },
            MaplyCoordinateD { mbr.ur().x(), mbr.ur().y() }
        };
    }
}

MaplyBoundingBox MaplyBoundingBoxFromCoordinates(const MaplyCoordinate coords[], unsigned int numCoords) {
    return ToBox(MbrAdd(Mbr(), coords, numCoords));
}
MaplyBoundingBox MaplyBoundingBoxFromCoordinatesD(const MaplyCoordinateD coords[], unsigned int numCoords) {
    return ToBox(MbrAdd(Mbr(), coords, numCoords));
}
MaplyBoundingBoxD MaplyBoundingBoxDFromCoordinates(const MaplyCoordinate coords[], unsigned int numCoords) {
    return ToBoxD(MbrAdd(MbrD(), coords, numCoords));
}
MaplyBoundingBoxD MaplyBoundingBoxDFromCoordinatesD(const MaplyCoordinateD coords[], unsigned int numCoords) {
    return ToBoxD(MbrAdd(MbrD(), coords, numCoords));
}
MaplyBoundingBox MaplyBoundingBoxAddCoordinates(MaplyBoundingBox box, const MaplyCoordinate coords[], unsigned int numCoords) {
    return ToBox(MbrAdd(Mbr(Point2f(box.ll.x, box.ll.y), Point2f(box.ur.x, box.ur.y)), coords, numCoords));
}
MaplyBoundingBox MaplyBoundingBoxAddCoordinatesD(MaplyBoundingBox box, const MaplyCoordinateD coords[], unsigned int numCoords) {
    return ToBox(MbrAdd(Mbr(Point2f(box.ll.x, box.ll.y), Point2f(box.ur.x, box.ur.y)), coords, numCoords));
}
MaplyBoundingBoxD MaplyBoundingBoxDAddCoordinates(MaplyBoundingBoxD box, const MaplyCoordinate coords[], unsigned int numCoords) {
    return ToBoxD(MbrAdd(MbrD(Point2d(box.ll.x, box.ll.y), Point2d(box.ur.x, box.ur.y)), coords, numCoords));
}
MaplyBoundingBoxD MaplyBoundingBoxDAddCoordinatesD(MaplyBoundingBoxD box, const MaplyCoordinateD coords[], unsigned int numCoords) {
    return ToBoxD(MbrAdd(MbrD(Point2d(box.ll.x, box.ll.y), Point2d(box.ur.x, box.ur.y)), coords, numCoords));
}

MaplyBoundingBox MaplyBoundingBoxIntersection(MaplyBoundingBox bbox0,MaplyBoundingBox bbox1)
{
    const Mbr mbr0(Point2f(bbox0.ll.x,bbox0.ll.y), Point2f(bbox0.ur.x,bbox0.ur.y));
    const Mbr mbr1(Point2f(bbox1.ll.x,bbox1.ll.y), Point2f(bbox1.ur.x,bbox1.ur.y));
    return MaplyBoundingBoxMakeFromMbr(mbr0.intersect(mbr1));
}

MaplyBoundingBox MaplyBoundingBoxExpandByFraction(MaplyBoundingBox bbox, float buffer)
{
    Mbr mbr(Point2f(bbox.ll.x,bbox.ll.y), Point2f(bbox.ur.x,bbox.ur.y));
    mbr.expandByFraction(buffer);

    return MaplyBoundingBoxMakeFromMbr(mbr);
}

double MaplyGreatCircleDistance(MaplyCoordinate p0,MaplyCoordinate p1)
{
    return EarthRadius * acos(sin(p0.y)*sin(p1.y) + cos(p0.y)*cos(p1.y)*cos(p1.x-p0.x));
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


@implementation NSValue (MaplyCoordinate)
+ (instancetype)valueWithMaplyCoordinate:(MaplyCoordinate)value {
    return [self valueWithBytes:&value objCType:@encode(MaplyCoordinate)];
}
- (MaplyCoordinate)maplyCoordinateValue {
    MaplyCoordinate c;
    [self getValue:&c];
    return c;
}
@end

@implementation NSValue (MaplyCoordinateD)
+ (instancetype)valueWithMaplyCoordinateD:(MaplyCoordinateD)value {
    return [self valueWithBytes:&value objCType:@encode(MaplyCoordinateD)];
}
- (MaplyCoordinateD)maplyCoordinateDValue {
    MaplyCoordinateD c;
    [self getValue:&c];
    return c;
}
@end

@implementation NSValue (MaplyBoundingBox)
+ (instancetype)valueWithMaplyBoundingBox:(MaplyBoundingBox)value {
    return [self valueWithBytes:&value objCType:@encode(MaplyBoundingBox)];
}
- (MaplyBoundingBox)maplyBoundingBoxValue {
    MaplyBoundingBox box;
    [self getValue:&box];
    return box;
}
@end

