/*
 *  MaplyShape.m
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/28/12.
 *  Copyright 2012-2015 mousebird consulting
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

#import "MaplyShape.h"
#import "WhirlyGlobe.h"
#import "MaplySharedAttributes.h"
#import "MaplyMatrix_private.h"

using namespace WhirlyKit;

@implementation MaplyShape

- (WhirlyKitShape *)asWKShape:(NSDictionary *)desc
{
    return nil;
}

@end

@implementation MaplyShapeCircle

- (WhirlyKitCircle *)asWKShape:(NSDictionary *)desc
{
    WhirlyKitCircle *newCircle = [[WhirlyKitCircle alloc] init];\
    
    newCircle.loc.lon() = _center.x;
    newCircle.loc.lat() = _center.y;
    newCircle.radius = _radius;
    newCircle.height = _height;
    if (self.color)
    {
        newCircle.useColor = true;
        RGBAColor color = [self.color asRGBAColor];
        newCircle.color = color;
    }
    
    return newCircle;
}

@end

@implementation MaplyShapeSphere

- (WhirlyKitSphere *)asWKShape:(NSDictionary *)desc
{
    WhirlyKitSphere *newSphere = [[WhirlyKitSphere alloc] init];
    newSphere.loc.lon() = _center.x;
    newSphere.loc.lat() = _center.y;
    newSphere.radius = _radius;
    newSphere.height = _height;
    newSphere.sampleX = [desc[kMaplyShapeSampleX] intValue];
    newSphere.sampleY = [desc[kMaplyShapeSampleY] intValue];
    if (self.color)
    {
        newSphere.useColor = true;
        RGBAColor color = [self.color asRGBAColor];
        newSphere.color = color;
    }
    
    return newSphere;
}

@end

@implementation MaplyShapeCylinder

- (WhirlyKitCylinder *)asWKShape:(NSDictionary *)desc
{
    WhirlyKitCylinder *newCyl = [[WhirlyKitCylinder alloc] init];
    newCyl.loc.lon() = _baseCenter.x;
    newCyl.loc.lat() = _baseCenter.y;
    newCyl.baseHeight = _baseHeight;
    newCyl.radius = _radius;
    newCyl.height = _height;
    if (self.color)
    {
        newCyl.useColor = true;
        RGBAColor color = [self.color asRGBAColor];
        newCyl.color = color;
    }
    
    return newCyl;
}

@end

@implementation MaplyShapeGreatCircle

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    
    _lineWidth = 1.0;
    
    return self;
}

- (float)calcAngleBetween
{
    Point3f p0 = FakeGeocentricDisplayAdapter::LocalToDisplay(Point3f(_startPt.x,_startPt.y,0.0));
    Point3f p1 = FakeGeocentricDisplayAdapter::LocalToDisplay(Point3f(_endPt.x,_endPt.y,0.0));
    
    float dot = p0.dot(p1);
//    Point3f cross = p0.cross(p1);
//    float mag = cross.norm();

    // Note: Atan2 is the correct way, but it's not working right here
//    return atan2f(dot, mag);
    return acosf(dot);
}

@end

@implementation MaplyShapeLinear
{
    /// Number of coordinates to display in linear
    int numCoords;
    /// Coordinates we'll display for the linear (lon,lat,Z in display units)
    MaplyCoordinate3d *coords;    
}

- (id)initWithCoords:(MaplyCoordinate3d *)inCoords numCoords:(int)inNumCoords
{
    self = [super init];
    if (!self)
        return nil;
    
    numCoords = inNumCoords;
    coords = (MaplyCoordinate3d *)malloc(sizeof(MaplyCoordinate3d)*inNumCoords);
    for (unsigned int ii=0;ii<numCoords;ii++)
        coords[ii] = inCoords[ii];
    
    return self;
}

- (void)dealloc
{
    if (coords)
        free(coords);
    coords = NULL;
}

- (int)getCoords:(MaplyCoordinate3d **)retCoords
{
    *retCoords = coords;
    return numCoords;
}

@end

@implementation MaplyShapeExtruded
{
    int numCoords;
    NSData *coords;
}

- (id)initWithOutline:(double *)inCoords numCoordPairs:(int)numCoordPairs
{
    self = [super init];
    
    numCoords = numCoordPairs;
    coords = [NSData dataWithBytes:inCoords length:sizeof(double)*numCoordPairs*2];
    _scale = 1/WhirlyKit::EarthRadius;
    
    return self;
}

- (int)numCoordPairs
{
    return numCoords;
}

- (double *)coordData
{
    return (double *)[coords bytes];
}

- (WhirlyKitShapeExtruded *)asWKShape:(NSDictionary *)desc
{
    WhirlyKitShapeExtruded *newEx = [[WhirlyKitShapeExtruded alloc] init];
    Point3d loc(self.center.x,self.center.y,self.height*self.scale);
    newEx.loc = loc;
    newEx.thickness = self.thickness*self.scale;
    if (self.transform)
        newEx.transform = self.transform.mat;
    int theNumCoords = self.numCoordPairs;
    double *theCoords = self.coordData;
    std::vector<Point2d> pts;
    pts.resize(theNumCoords);
    for (unsigned int ii=0;ii<theNumCoords;ii++)
    {
        Point2d pt(theCoords[2*ii]*self.scale,theCoords[2*ii+1]*self.scale);
        pts[ii] = pt;
    }
    newEx.pts = pts;
    if (self.color)
    {
        newEx.useColor = true;
        RGBAColor color = [self.color asRGBAColor];
        newEx.color = color;
    }
    
    return newEx;
}

@end

