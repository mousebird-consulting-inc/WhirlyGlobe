/*
 *  MaplyShape.m
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/28/12.
 *  Copyright 2012-2019 mousebird consulting
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

#import "visual_objects/MaplyShape.h"
#import "WhirlyGlobe.h"
#import "MaplySharedAttributes.h"
#import "MaplyMatrix_private.h"
#import "UIColor+Stuff.h"

using namespace WhirlyKit;

@implementation MaplyShape

- (Shape *)asWKShape:(NSDictionary *)desc
{
    return nil;
}

@end

@implementation MaplyShapeCircle

- (Shape *)asWKShape:(NSDictionary *)desc
{
    Circle *newCircle = new Circle();
    
    newCircle->loc.lon() = _center.x;
    newCircle->loc.lat() = _center.y;
    newCircle->radius = _radius;
    newCircle->height = _height;
    if (desc[kMaplyShapeSampleX] != nil)
        newCircle->sampleX = (int)[desc[kMaplyShapeSampleX] integerValue];
    else if (desc[kMaplySampleX] != nil)
        newCircle->sampleX = (int)[desc[kMaplySampleX] integerValue];
    if (self.color)
    {
        newCircle->useColor = true;
        RGBAColor color = [self.color asRGBAColor];
        newCircle->color = color;
    }
    
    return newCircle;
}

@end

@implementation MaplyShapeSphere

- (Sphere *)asWKShape:(NSDictionary *)desc
{
    Sphere *newSphere = new Sphere();
    newSphere->loc.lon() = _center.x;
    newSphere->loc.lat() = _center.y;
    newSphere->radius = _radius;
    newSphere->height = _height;
    if (desc[kMaplyShapeSampleX] != nil)
        newSphere->sampleX = [desc[kMaplyShapeSampleX] intValue];
    if (desc[kMaplyShapeSampleY] != nil)
        newSphere->sampleY = [desc[kMaplyShapeSampleY] intValue];
    if (self.color)
    {
        newSphere->useColor = true;
        RGBAColor color = [self.color asRGBAColor];
        newSphere->color = color;
    }
    
    return newSphere;
}

@end

@implementation MaplyShapeCylinder

- (Shape *)asWKShape:(NSDictionary *)desc
{
    Cylinder *newCyl = new Cylinder();
    newCyl->loc.lon() = _baseCenter.x;
    newCyl->loc.lat() = _baseCenter.y;
    newCyl->baseHeight = _baseHeight;
    newCyl->radius = _radius;
    newCyl->height = _height;
    newCyl->sampleX = (int)[desc[kMaplyShapeSampleX] integerValue];
    if (self.color)
    {
        newCyl->useColor = true;
        RGBAColor color = [self.color asRGBAColor];
        newCyl->color = color;
    }
    
    return newCyl;
}

@end

@implementation MaplyShapeGreatCircle

- (instancetype)init
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
    float ret = acosf(dot);
    if (std::isnan(ret))
        ret = 0.f;
    return ret;
}

@end

@implementation MaplyShapeRectangle

- (instancetype)init
{
    self = [super init];
    if (!self)
        return nil;
    _textures = [NSMutableArray array];
        
    return self;
}

- (void)addTexture:(MaplyTexture *)texture
{
    [_textures addObject:texture];
}

- (Rectangle *)asWKShape:(NSDictionary *)desc
{
    Rectangle *newRect = new Rectangle();
    newRect->ll.x() = _ll.x;  newRect->ll.y() = _ll.y;  newRect->ll.z() = _ll.z;
    newRect->ur.x() = _ur.x;  newRect->ur.y() = _ur.y;  newRect->ur.z() = _ur.z;
    newRect->clipCoords = self.clipCoords;
    
    return newRect;
}

@end

@implementation MaplyShapeLinear
{
    /// Number of coordinates to display in linear
    int numCoords;
    /// Coordinates we'll display for the linear (lon,lat,Z in display units)
    MaplyCoordinate3d *coords;    
}

- (instancetype)initWithCoords:(MaplyCoordinate3d *)inCoords numCoords:(int)inNumCoords
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

- (Shape *)asWKShape:(NSDictionary *)desc coordAdapter:(CoordSystemDisplayAdapter *)coordAdapter
{
    Linear *newLin = new Linear();
    MaplyCoordinate3d *coords = NULL;
    int numCoords = [self getCoords:&coords];
    for (unsigned int ii=0;ii<numCoords;ii++)
    {
        MaplyCoordinate3d &coord = coords[ii];
        Point3d pt = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(coord.x,coord.y)));
        if (coordAdapter->isFlat())
            pt.z() = coord.z;
        else
            pt *= (1.0+coord.z);
        newLin->pts.push_back(pt);
    }
    newLin->lineWidth = _lineWidth;
    if (self.color)
    {
        newLin->useColor = true;
        RGBAColor color = [self.color asRGBAColor];
        newLin->color = color;
    }
    
    return newLin;
}

@end

@implementation MaplyShapeExtruded
{
    int numCoords;
    NSData *coords;
}

- (instancetype)initWithOutline:(NSArray *)inCoords
{
	double* doubleCoords = (double *)malloc(sizeof(double)*[inCoords count]*2);

	for (int i = 0; i < [inCoords count]; ++i) {
		doubleCoords[i] = [inCoords[i] doubleValue];
	}

	self = [self initWithOutline:doubleCoords numCoordPairs:(int)[inCoords count]];

	free(doubleCoords);

	return self;
}

- (instancetype)initWithOutline:(double *)inCoords numCoordPairs:(int)numCoordPairs
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

- (Shape *)asWKShape:(NSDictionary *)desc
{
    Extruded *newEx = new Extruded();
    newEx->scale = self.scale;
    Point3d loc(self.center.x,self.center.y,self.height);
    newEx->loc = loc;
    newEx->thickness = self.thickness;
    if (self.transform)
        newEx->transform = self.transform.mat;
    int theNumCoords = self.numCoordPairs;
    double *theCoords = self.coordData;
    Point2dVector pts;
    pts.resize(theNumCoords);
    for (unsigned int ii=0;ii<theNumCoords;ii++)
    {
        Point2d pt(theCoords[2*ii],theCoords[2*ii+1]);
        pts[ii] = pt;
    }
    newEx->pts = pts;
    if (self.color)
    {
        newEx->useColor = true;
        RGBAColor color = [self.color asRGBAColor];
        newEx->color = color;
    }
    
    return newEx;
}

@end

