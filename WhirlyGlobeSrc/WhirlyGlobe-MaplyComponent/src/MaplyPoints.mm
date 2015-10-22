/*
 *  MaplyPoints.mm
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 10/21/15
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

#import "MaplyPoints_private.h"

using namespace WhirlyKit;

@implementation MaplyPoints

- (__nonnull id)initWithNumPoints:(int)numPoints
{
    self = [super init];
    coordsAreGeo = false;
    coordIdx = -1;
    colorIdx = -1;
    
    return self;
}

- (void)addGeoCoordLon:(float)x lat:(float)y z:(float)z
{
    coordsAreGeo = true;

    if (coordIdx < 0)
        coordIdx = [self addAttributeType:@"a_position" type:MaplyShaderAttrTypeFloat3];

    [self addAttribute:coordIdx fValX:x fValY:y fValZ:z];
}

- (void)addDispCoordX:(float)x y:(float)y z:(float)z
{
    coordsAreGeo = false;
    
    if (coordIdx < 0)
        coordIdx = [self addAttributeType:@"a_position" type:MaplyShaderAttrTypeFloat3];
    
    [self addAttribute:coordIdx fValX:x fValY:y fValZ:z];
}

- (void)addDispCoordDoubleX:(double)x y:(double)y z:(double)z
{
    coordsAreGeo = false;
    
    if (coordIdx < 0)
        coordIdx = [self addAttributeType:@"a_position" type:MaplyShaderAttrTypeFloat3];
    
    [self addAttribute:coordIdx valX:x valY:y valZ:z];
}

- (void)addColorR:(float)r g:(float)g b:(float)b a:(float)a
{
    if (colorIdx < 0)
        colorIdx = [self addAttributeType:@"a_color" type:MaplyShaderAttrTypeFloat4];
    
    [self addAttribute:colorIdx fValX:r fValY:g fValZ:b fValW:a];
}

- (int)addAttributeType:(NSString *__nonnull)attrName type:(MaplyShaderAttrType)type
{
    std::string name = [attrName cStringUsingEncoding:NSASCIIStringEncoding];
    
    GeomRawDataType dataType = GeomRawTypeMax;
    switch (type)
    {
        case MaplyShaderAttrTypeInt:
            dataType = GeomRawIntType;
            break;
        case MaplyShaderAttrTypeFloat:
            dataType = GeomRawFloatType;
            break;
        case MaplyShaderAttrTypeFloat2:
            dataType = GeomRawFloat2Type;
            break;
        case MaplyShaderAttrTypeFloat3:
            dataType = GeomRawFloat3Type;
            break;
        case MaplyShaderAttrTypeFloat4:
            dataType = GeomRawFloat4Type;
            break;
    }
    
    if (dataType == GeomRawTypeMax)
        return -1;
    
    return points.addAttribute(name, dataType);
}

- (void)addAttribute:(int)whichAttr iVal:(int)val
{
    points.addValue(whichAttr,val);
}

- (void)addAttribute:(int)whichAttr fVal:(float)val
{
    points.addValue(whichAttr,val);
}

- (void)addAttribute:(int)whichAttr fValX:(float)valX fValY:(float)valY
{
    points.addPoint(whichAttr,Point2f(valX,valY));
}

- (void)addAttribute:(int)whichAttr fValX:(float)valX fValY:(float)valY fValZ:(float)valZ
{
    points.addPoint(whichAttr,Point3f(valX,valY,valZ));
}

- (void)addAttribute:(int)whichAttr valX:(double)valX valY:(double)valY valZ:(double)valZ
{
    points.addPoint(whichAttr,Point3d(valX,valY,valZ));
}

- (void)addAttribute:(int)whichAttr fValX:(float)valX fValY:(float)valY fValZ:(float)valZ fValW:(float)valW
{
    points.addPoint(whichAttr,Eigen::Vector4f(valX,valY,valZ,valW));
}

@end
