/*  MaplyCoordinateSystem.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/13/13.
 *  Copyright 2011-2023 mousebird consulting
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
#import "NSString+Stuff.h"

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
        return { ll.x(), ll.y() };
    }
    return {0,0};
}

- (MaplyCoordinate)ur
{
    if (coordSystem)
    {
        const auto &ur = coordSystem->getBounds().ur();
        return { ur.x(), ur.y() };
    }
    return {0,0};
}

- (MaplyCoordinateD)llD
{
    if (coordSystem)
    {
        const auto &ll = coordSystem->getBounds().ll();
        return { ll.x(), ll.y() };
    }
    return {0,0};
}

- (MaplyCoordinateD)urD
{
    if (coordSystem)
    {
        const auto &ur = coordSystem->getBounds().ur();
        return { ur.x(), ur.y() };
    }
    return {0,0};
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
    const auto ll = [self localToGeo:bounds.ll];
    const auto ur = [self localToGeo:bounds.ur];
	[self setBoundsLL:&ll ur:&ur];
}

- (void)setBoundsD:(MaplyBoundingBoxD)boundsD
{
    const auto ll = [self localToGeoD:boundsD.ll];
    const auto ur = [self localToGeoD:boundsD.ur];
    [self setBoundsDLL:&ll ur:&ur];
}

- (void)setBoundsDLocal:(MaplyBoundingBoxD)boundsD
{
    if (coordSystem)
    {
        coordSystem->setBounds(MbrD({boundsD.ll.x,boundsD.ll.y}, {boundsD.ur.x,boundsD.ur.y}));
    }
}

- (void)setBoundsLL:(const MaplyCoordinate *)inLL ur:(const MaplyCoordinate *)inUR
{
    if (coordSystem && inLL && inUR)
    {
        coordSystem->setBounds(MbrD({inLL->x, inLL->y}, {inUR->x, inUR->y}));
    }
}

- (void)setBoundsDLL:(const MaplyCoordinateD *)inLL ur:(const MaplyCoordinateD *)inUR
{
    if (coordSystem && inLL && inUR)
    {
        coordSystem->setBounds(MbrD({inLL->x, inLL->y}, {inUR->x, inUR->y}));
    }
}

- (MaplyBoundingBox)getBounds
{
    const Point3d llLoc = coordSystem->geographicToLocal(coordSystem->getBoundsD().ll());
    const Point3d urLoc = coordSystem->geographicToLocal(coordSystem->getBoundsD().ur());
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

- (void)getBoundsDLL:(MaplyCoordinateD * __nullable)inLL ur:(MaplyCoordinateD * __nullable)inUR
{
    if (inLL)
    {
        *inLL = self.llD;
    }
    if (inUR)
    {
        *inUR = self.urD;
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

- (MaplyCoordinateD)localToGeoD:(MaplyCoordinateD)coord
{
    const auto geo = coordSystem->localToGeographicD({coord.x,coord.y,0.0});
    return {geo.x(), geo.y()};
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
    if (!(self = [super init]))
    {
        return nil;
    }
    std::string str = [proj4Str cStringUsingEncoding:NSASCIIStringEncoding withDefault:""];
    if (str.empty())
    {
        return nil;
    }
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
