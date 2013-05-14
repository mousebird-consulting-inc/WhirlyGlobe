/*
 *  MaplyCoordinateSystem.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/13/13.
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

#import "MaplyCoordinateSystem_private.h"

using namespace WhirlyKit;

@implementation MaplyCoordinateSystem

- (id)initWithCoordSystem:(WhirlyKit::CoordSystem *)newCoordSystem
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

- (void)getBoundsLL:(MaplyCoordinate *)ret_ll ur:(MaplyCoordinate *)ret_ur
{
    if (ret_ll)
        *ret_ll = ll;
    if (ret_ur)
        *ret_ur = ur;
}

@end

@implementation MaplyPlateCarree

- (id)initFullCoverage
{
    PlateCarreeCoordSystem *coordSys = new PlateCarreeCoordSystem();
    self = [super initWithCoordSystem:coordSys];
    Point3f pt0 = coordSys->geographicToLocal(GeoCoord::CoordFromDegrees(-180, -90));
    Point3f pt1 = coordSys->geographicToLocal(GeoCoord::CoordFromDegrees(180, 90));
    ll.x = pt0.x();  ll.y = pt0.y();
    ur.x = pt1.x();  ur.y = pt1.y();
    
    return self;
}

@end

@implementation MaplySphericalMercator

- (id)initWebStandard
{
    SphericalMercatorCoordSystem *coordSys = new SphericalMercatorCoordSystem();
    self = [super initWithCoordSystem:coordSys];
    Point3f pt0 = coordSys->geographicToLocal(GeoCoord::CoordFromDegrees(-180,-85.05113));
    Point3f pt1 = coordSys->geographicToLocal(GeoCoord::CoordFromDegrees( 180, 85.05113));
    ll.x = pt0.x();  ll.y = pt0.y();
    ur.x = pt1.x();  ur.y = pt1.y();
    
    return self;
}

@end
