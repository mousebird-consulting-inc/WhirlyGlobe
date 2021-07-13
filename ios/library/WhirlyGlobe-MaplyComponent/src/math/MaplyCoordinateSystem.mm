/*
 *  MaplyCoordinateSystem.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/13/13.
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

#import "MaplyCoordinateSystem_private.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation MaplyCoordinateSystem

- (instancetype)initWithCoordSystem:(WhirlyKit::CoordSystemRef)newCoordSystem
{
    self = [super init];
    if (!self)
    {
        return nil;
    }
    coordSystem = newCoordSystem;
    
    return self;
}

- (void)dealloc
{
    coordSystem = NULL;
}

- (WhirlyKit::CoordSystemRef)getCoordSystem
{
    return coordSystem;
}

- (NSString *)getSRS
{
    return @"";
}

- (bool)canBeDegrees
{
    return false;
}

- (void)setBounds:(MaplyBoundingBox)bounds
{
    auto ll = [self localToGeo:bounds.ll];
    auto ur = [self localToGeo:bounds.ur];
	[self setBoundsLL:&ll ur:&ur];
}

- (void)setBoundsD:(MaplyBoundingBoxD)boundsD
{
    ll.x = boundsD.ll.x;    ll.y = boundsD.ll.y;
    ur.x = boundsD.ur.x;    ur.y = boundsD.ur.y;
}

- (void)setBoundsLL:(MaplyCoordinate *)inLL ur:(MaplyCoordinate *)inUR
{
    ll.x = inLL->x;    ll.y = inLL->y;
    ur.x = inUR->x;    ur.y = inUR->y;
}

- (MaplyBoundingBox)getBounds
{
    const Point3d llLoc = coordSystem->geographicToLocal(Point2d(ll.x, ll.y));
    const Point3d urLoc = coordSystem->geographicToLocal(Point2d(ur.x, ur.y));
    return {{(float)llLoc.x(),(float)llLoc.y()},{(float)urLoc.x(),(float)urLoc.y()}};
}

- (void)getBoundsLL:(MaplyCoordinate *)ret_ll ur:(MaplyCoordinate *)ret_ur
{
    if (ret_ll)
    {
        ret_ll->x = ll.x; ret_ll->y = ll.y;
    }
    if (ret_ur)
    {
        ret_ur->x = ur.x; ret_ur->y = ur.y;
    }
}

- (MaplyCoordinate)geoToLocal:(MaplyCoordinate)coord
{
    GeoCoord pt(coord.x,coord.y);
    Point3d retPt = coordSystem->geographicToLocal3d(pt);
    
    MaplyCoordinate retCoord;
    retCoord.x = retPt.x();  retCoord.y = retPt.y();
    
    return retCoord;
}

- (MaplyCoordinate)localToGeo:(MaplyCoordinate)coord
{
    Point3d pt(coord.x,coord.y,0.0);
    GeoCoord retPt = coordSystem->localToGeographic(pt);
    
    MaplyCoordinate retCoord;
    retCoord.x = retPt.x();  retCoord.y = retPt.y();
    
    return retCoord;
}

- (MaplyCoordinate3dD)localToGeocentric:(MaplyCoordinate3dD)coord
{
    Point3d pt = coordSystem->localToGeocentric(Point3d(coord.x,coord.y,coord.z));
    MaplyCoordinate3dD ret;
    ret.x = pt.x();  ret.y = pt.y();  ret.z = pt.z();
    
    return ret;
}

- (MaplyCoordinate3dD)geocentricToLocal:(MaplyCoordinate3dD)coord
{
    Point3d pt = coordSystem->geocentricToLocal(Point3d(coord.x,coord.y,coord.z));
    MaplyCoordinate3dD ret;
    ret.x = pt.x();  ret.y = pt.y();  ret.z = pt.z();
    
    return ret;
}

@end

@implementation MaplyPlateCarree

- (instancetype)init
{
	return [self initFullCoverage];
}

- (instancetype)initWithBoundingBox:(MaplyBoundingBox)bbox
{
    PlateCarreeCoordSystem *coordSys = new PlateCarreeCoordSystem();
    self = [super initWithCoordSystem:CoordSystemRef(coordSys)];
    ll.x = bbox.ll.x;
    ll.y = bbox.ll.y;
    ur.x = bbox.ur.x;
    ur.y = bbox.ur.y;
    
    return self;
}

- (nullable instancetype)initWithBoundingBoxD:(MaplyBoundingBoxD)bbox
{
    PlateCarreeCoordSystem *coordSys = new PlateCarreeCoordSystem();
    self = [super initWithCoordSystem:CoordSystemRef(coordSys)];
    ll.x = bbox.ll.x;
    ll.y = bbox.ll.y;
    ur.x = bbox.ur.x;
    ur.y = bbox.ur.y;
    
    return self;
}

- (instancetype)initFullCoverage
{
    PlateCarreeCoordSystem *coordSys = new PlateCarreeCoordSystem();
    self = [super initWithCoordSystem:CoordSystemRef(coordSys)];
    Point3f pt0 = coordSys->geographicToLocal(GeoCoord::CoordFromDegrees(-180, -90));
    Point3f pt1 = coordSys->geographicToLocal(GeoCoord::CoordFromDegrees(180, 90));
    ll.x = pt0.x();  ll.y = pt0.y();
    ur.x = pt1.x();  ur.y = pt1.y();
    
    return self;
}

- (NSString *)getSRS
{
    return @"EPSG:4326";
}

- (bool)canBeDegrees
{
    return true;
}

@end

@implementation MaplySphericalMercator

- (instancetype)init
{
	return [self initWebStandard];
}

- (instancetype)initWebStandard
{
    SphericalMercatorCoordSystem *coordSys = new SphericalMercatorCoordSystem();
    self = [super initWithCoordSystem:CoordSystemRef(coordSys)];
    Point3d pt0 = coordSys->geographicToLocal(Point2d(-180/180.0 * M_PI,-85.05113/180.0 * M_PI));
    Point3d pt1 = coordSys->geographicToLocal(Point2d( 180/180.0 * M_PI, 85.05113/180.0 * M_PI));
    ll.x = pt0.x();  ll.y = pt0.y();
    ur.x = pt1.x();  ur.y = pt1.y();
    
    return self;
}

- (NSString *)getSRS
{
    return @"EPSG:3857";
}

- (bool)canBeDegrees
{
    return true;
}

@end

@implementation MaplyProj4CoordSystem
{
    Proj4CoordSystem *p4CoordSys;
}

- (nonnull instancetype)initWithString:(NSString * __nonnull)proj4Str
{
    self = [super init];
    std::string str = [proj4Str cStringUsingEncoding:NSASCIIStringEncoding];
    p4CoordSys = new Proj4CoordSystem(str);
    coordSystem = CoordSystemRef(p4CoordSys);
        
    return self;
}

- (bool)valid
{
    return p4CoordSys != nil && p4CoordSys->isValid();
}

@end

MaplyCoordinateSystem *MaplyCoordinateSystemFromEPSG(NSString *crs)
{
    if ([crs isEqualToString:@"EPSG:3857"])
    {
        return [[MaplySphericalMercator alloc] initWebStandard];
    } else if ([crs isEqualToString:@"EPSG:4326"])
    {
        return [[MaplyPlateCarree alloc] initFullCoverage];
    }
    
    return nil;
}
