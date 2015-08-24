/*
 *  MaplyCoordinateSystem.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/13/13.
 *  Copyright 2011-2015 mousebird consulting
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

- (instancetype)initWithCoordSystem:(WhirlyKit::CoordSystem *)newCoordSystem
{
    self = [super init];
    if (!self)
    {
        delete newCoordSystem;
        return nil;
    }
    coordSystem = newCoordSystem;
    
    return self;
}

- (void)dealloc
{
    if (coordSystem)
    {
        delete coordSystem;
        coordSystem = NULL;
    }
}

- (WhirlyKit::CoordSystem *)getCoordSystem
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
	[self setBoundsLL:&bounds.ll ur:&bounds.ur];
}

- (void)setBoundsLL:(MaplyCoordinate *)inLL ur:(MaplyCoordinate *)inUR
{
    ll.x = inLL->x;    ll.y = inLL->y;
    ur.x = inUR->x;    ur.y = inUR->y;
}

- (MaplyBoundingBox)getBounds
{
	MaplyBoundingBox box;

	box.ll.x = ll.x;
	box.ll.y = ll.y;

	box.ur.x = ur.x;
	box.ur.y = ur.y;

	return box;
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

@end

@implementation MaplyPlateCarree

- (instancetype)init
{
	return [self initFullCoverage];
}

- (instancetype)initWithBoundingBox:(MaplyBoundingBox)bbox
{
    PlateCarreeCoordSystem *coordSys = new PlateCarreeCoordSystem();
    self = [super initWithCoordSystem:coordSys];
    ll.x = bbox.ll.x;
    ll.y = bbox.ll.y;
    ur.x = bbox.ur.x;
    ur.y = bbox.ur.y;
    
    return self;
}

- (instancetype)initFullCoverage
{
    PlateCarreeCoordSystem *coordSys = new PlateCarreeCoordSystem();
    self = [super initWithCoordSystem:coordSys];
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
    self = [super initWithCoordSystem:coordSys];
    Point3d pt0 = coordSys->geographicToLocal3d(GeoCoord::CoordFromDegrees(-180,-85.05113));
    Point3d pt1 = coordSys->geographicToLocal3d(GeoCoord::CoordFromDegrees( 180, 85.05113));
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
