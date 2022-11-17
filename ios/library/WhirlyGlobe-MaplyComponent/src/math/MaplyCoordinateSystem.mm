/*  MaplyCoordinateSystem.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/13/13.
 *  Copyright 2011-2022 mousebird consulting
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
    coordSystem = nullptr;
}

- (MaplyCoordinate)ll
{
    if (coordSystem)
    {
        const auto &ll = coordSystem->getBounds().ll();
        return MaplyCoordinate { ll.x(), ll.y() };
    }
    return MaplyCoordinate();
}

- (MaplyCoordinate)ur
{
    if (coordSystem)
    {
        const auto &ur = coordSystem->getBounds().ur();
        return MaplyCoordinate { ur.x(), ur.y() };
    }
    return MaplyCoordinate();
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
    // Note: We don't have double versions of localToGeo exposed
    MaplyBoundingBox box;
    box.ll = MaplyCoordinateMake(boundsD.ll.x, boundsD.ll.y);
    box.ur = MaplyCoordinateMake(boundsD.ur.x, boundsD.ur.y);
    [self setBounds:box];
}

- (void)setBoundsDLocal:(MaplyBoundingBoxD)boundsD
{
    if (coordSystem)
    {
        coordSystem->setBounds(Point2f(boundsD.ll.x,boundsD.ll.y), Point2f(boundsD.ur.x,boundsD.ur.y));
    }
}

- (void)setBoundsLL:(const MaplyCoordinate *)inLL ur:(const MaplyCoordinate *)inUR
{
    if (coordSystem)
    {
        coordSystem->setBounds(Point2f(inLL->x,inLL->y), Point2f(inUR->x,inUR->y));
    }
}

- (MaplyBoundingBox)getBounds
{
    const Point3d llLoc = coordSystem->geographicToLocal(coordSystem->getBounds().ll().cast<double>());
    const Point3d urLoc = coordSystem->geographicToLocal(coordSystem->getBounds().ur().cast<double>());
    return {{(float)llLoc.x(),(float)llLoc.y()},{(float)urLoc.x(),(float)urLoc.y()}};
}

- (void)getBoundsLL:(MaplyCoordinate *)inLL ur:(MaplyCoordinate *)inUR
{
    if (inLL)
    {
        *inLL = self.ll;
    }
    if (inUR)
    {
        *inUR = self.ur;
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

- (void)setCanBeWrapped:(bool)b
{
    if (coordSystem)
    {
        coordSystem->setCanBeWrapped(b);
    }
}

@end

@implementation MaplyPlateCarree

- (instancetype)init
{
	return [self initFullCoverage];
}

- (instancetype)initWithBoundingBox:(MaplyBoundingBox)bbox
{
    if ((self = [super initWithCoordSystem:std::make_shared<PlateCarreeCoordSystem>()]))
    {
        [self setBounds:bbox];
    }
    return self;
}

- (nullable instancetype)initWithBoundingBoxD:(MaplyBoundingBoxD)bbox
{
    if ((self = [super initWithCoordSystem:std::make_shared<PlateCarreeCoordSystem>()]))
    {
        [self setBoundsD:bbox];
    }
    return self;
}

- (instancetype)initFullCoverage
{
    self = [super initWithCoordSystem:std::make_shared<PlateCarreeCoordSystem>()];
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
    self = [super initWithCoordSystem:SphericalMercatorCoordSystem::makeWebStandard()];
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
}

- (nonnull instancetype)initWithString:(NSString * __nonnull)proj4Str
{
    self = [super init];
    std::string str = [proj4Str cStringUsingEncoding:NSASCIIStringEncoding];
    coordSystem = std::make_shared<Proj4CoordSystem>(std::move(str));
    return self;
}

- (bool)valid
{
    if (auto cs = dynamic_cast<const Proj4CoordSystem*>(coordSystem.get()))
    {
        return cs->isValid();
    }
    return false;
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
