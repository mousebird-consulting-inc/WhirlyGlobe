/*
 *  MaplyScreenObject
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 3/2/15
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

#import "WhirlyGlobe.h"
#import "MaplyBaseViewController_private.h"
#import "MaplyBaseInteractionLayer_private.h"
#import "MaplyScreenObject_private.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation MaplyScreenObject

- (void)addAttributedString:(NSAttributedString *)str
{
    StringWrapper strWrap;
    strWrap.str = str;
    strWrap.size = [str size];
    
    strings.push_back(strWrap);
}

- (void)addString:(NSString *)str font:(UIFont *)font color:(UIColor *)color
{
    NSMutableAttributedString *attrStr = [[NSMutableAttributedString alloc] initWithString:str];
    NSInteger strLen = [attrStr length];
    [attrStr addAttribute:NSFontAttributeName value:font range:NSMakeRange(0, strLen)];
    [attrStr addAttribute:NSForegroundColorAttributeName value:color range:NSMakeRange(0, strLen)];
    
    [self addAttributedString:attrStr];
}

- (void)addImage:(id)image color:(UIColor *)color size:(CGSize)size
{
    SimplePoly poly;
    poly.texture = image;
    poly.color = color;

    poly.pts.resize(4);
    poly.texCoords.resize(4);

    poly.pts[0] = Point2d(0,0);
    poly.texCoords[0] = TexCoord(0,1);
    poly.pts[1] = Point2d(size.width,0);
    poly.texCoords[1] = TexCoord(1,1);
    poly.pts[2] = Point2d(size.width,size.height);
    poly.texCoords[2] = TexCoord(1,0);
    poly.pts[3] = Point2d(0,size.height);
    poly.texCoords[3] = TexCoord(0,0);
    
    polys.push_back(poly);
}

- (MaplyBoundingBox)getSize
{
    Mbr mbr;
    
    for (const SimplePoly &poly : polys)
    {
        for (const Point2d &pt : poly.pts)
        {
            mbr.addPoint(pt);
        }
    }
    
    for (const StringWrapper &str : strings)
    {
        Point3d p0 = str.mat * Point3d(0,0,1);
        Point3d p1 = str.mat * Point3d(str.size.width,str.size.height,1);
        mbr.addPoint(Point2d(p0.x(),p0.y()));
        mbr.addPoint(Point2d(p1.x(),p1.y()));
    }
    
    MaplyBoundingBox bbox;
    bbox.ll.x = mbr.ll().x();  bbox.ll.y = mbr.ll().y();
    bbox.ur.x = mbr.ur().x();  bbox.ur.y = mbr.ur().y();
    
    return bbox;
}

- (void)scaleX:(double)x y:(double)y
{
    Affine2d scale(Eigen::Scaling(x, y));
    Matrix3d mat = scale.matrix();
    
    for (SimplePoly &poly : polys)
    {
        for (Point2d &pt : poly.pts)
        {
            Point3d newPt = mat * Point3d(pt.x(),pt.y(),1.0);
            pt = Point2d(newPt.x(),newPt.y());
        }
    }
    
    for (StringWrapper &str : strings)
    {
        str.mat = mat * str.mat;
    }
}

- (void)translateX:(double)x y:(double)y
{
    Eigen::Affine2d trans(Translation2d(x,y));
    
    for (SimplePoly &poly : polys)
    {
        for (Point2d &pt : poly.pts)
            pt = trans * pt;
    }
    
    Eigen::Matrix3d mat = trans.matrix();
    for (StringWrapper &str : strings)
    {
        str.mat = mat * str.mat;
    }
}

- (void)addScreenObject:(MaplyScreenObject *)screenObj
{
    polys.insert(polys.end(),screenObj->polys.begin(),screenObj->polys.end());
    strings.insert(strings.end(), screenObj->strings.begin(), screenObj->strings.end());
}

@end
