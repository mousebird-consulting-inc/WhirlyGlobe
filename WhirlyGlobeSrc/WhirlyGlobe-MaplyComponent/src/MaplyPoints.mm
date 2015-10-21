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

- (void)dealloc
{
    for (PointAttrData *attrs : attrData)
        delete attrs;
    attrData.clear();
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
    
    // Make sure we don't already have it
    for (PointAttrData *data : attrData)
        if (name == data->name)
            return -1;
    
    int idx = -1;
    switch (type)
    {
        case MaplyShaderAttrTypeInt:
        {
            PointAttrDataInt *attrs = new PointAttrDataInt();
            attrs->name = name;
            idx = attrData.size();
            attrData.push_back(attrs);
        }
            break;
        case MaplyShaderAttrTypeFloat:
        {
            PointAttrDataFloat *attrs = new PointAttrDataFloat();
            attrs->name = name;
            idx = attrData.size();
            attrData.push_back(attrs);
        }
            break;
        case MaplyShaderAttrTypeFloat2:
        {
            PointAttrDataPoint2f *attrs = new PointAttrDataPoint2f();
            attrs->name = name;
            idx = attrData.size();
            attrData.push_back(attrs);
        }
            break;
        case MaplyShaderAttrTypeFloat3:
        {
            PointAttrDataPoint3f *attrs = new PointAttrDataPoint3f();
            attrs->name = name;
            idx = attrData.size();
            attrData.push_back(attrs);
        }
            break;
        case MaplyShaderAttrTypeFloat4:
        {
            PointAttrDataPoint4f *attrs = new PointAttrDataPoint4f();
            attrs->name = name;
            idx = attrData.size();
            attrData.push_back(attrs);
        }
            break;
    }
    
    return idx;
}

- (void)addAttribute:(int)whichAttr iVal:(int)val
{
    if (whichAttr >= attrData.size())
        return;
    
    PointAttrData *attrs = attrData[whichAttr];
    PointAttrDataInt *intAttrs = dynamic_cast<PointAttrDataInt *> (attrs);
    if (intAttrs)
        intAttrs->vals.push_back(val);
}

- (void)addAttribute:(int)whichAttr fVal:(float)val
{
    if (whichAttr >= attrData.size())
        return;
    
    PointAttrData *attrs = attrData[whichAttr];
    PointAttrDataFloat *fAttrs = dynamic_cast<PointAttrDataFloat *> (attrs);
    if (fAttrs)
        fAttrs->vals.push_back(val);
}

- (void)addAttribute:(int)whichAttr fValX:(float)valX fValY:(float)valY
{
    if (whichAttr >= attrData.size())
        return;
    
    PointAttrData *attrs = attrData[whichAttr];
    PointAttrDataPoint2f *f2Attrs = dynamic_cast<PointAttrDataPoint2f *> (attrs);
    if (f2Attrs)
        f2Attrs->vals.push_back(Point2f(valX,valY));
}

- (void)addAttribute:(int)whichAttr fValX:(float)valX fValY:(float)valY fValZ:(float)valZ
{
    if (whichAttr >= attrData.size())
        return;
    
    PointAttrData *attrs = attrData[whichAttr];
    PointAttrDataPoint3f *f3Attrs = dynamic_cast<PointAttrDataPoint3f *> (attrs);
    if (f3Attrs)
        f3Attrs->vals.push_back(Point3f(valX,valY,valZ));
}

- (void)addAttribute:(int)whichAttr valX:(double)valX valY:(double)valY valZ:(double)valZ
{
    if (whichAttr >= attrData.size())
        return;
    
    PointAttrData *attrs = attrData[whichAttr];
    PointAttrDataPoint3d *d3Attrs = dynamic_cast<PointAttrDataPoint3d *> (attrs);
    if (d3Attrs)
        d3Attrs->vals.push_back(Point3d(valX,valY,valZ));
}

- (void)addAttribute:(int)whichAttr fValX:(float)valX fValY:(float)valY fValZ:(float)valZ fValW:(float)valW
{
    if (whichAttr >= attrData.size())
        return;
    
    PointAttrData *attrs = attrData[whichAttr];
    PointAttrDataPoint4f *f4Attrs = dynamic_cast<PointAttrDataPoint4f *> (attrs);
    if (f4Attrs)
        f4Attrs->vals.push_back(Eigen::Vector4f(valX,valY,valZ,valW));
}

@end
