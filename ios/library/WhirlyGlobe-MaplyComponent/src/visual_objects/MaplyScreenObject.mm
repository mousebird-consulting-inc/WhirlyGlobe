/*
 *  MaplyScreenObject
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 3/2/15
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

#import "WhirlyGlobe_iOS.h"
#import "MaplyRenderController_private.h"
#import "MaplyBaseInteractionLayer_private.h"
#import "MaplyScreenObject_private.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation MaplyScreenObject

- (void)addAttributedString:(NSAttributedString *)str
{
    StringWrapper_iOSRef strWrap(new StringWrapper_iOS());
    strWrap->str = str;
    CGSize size = [str size];
    strWrap->size = Point2d(size.width,size.height);
    
    screenObj.strings.push_back(strWrap);
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
    [self addImage:image color:color size:size offset:CGPointMake(0.0, 0.0)];
}

- (void)addImage:(id)image color:(UIColor *)color size:(CGSize)size offset:(CGPoint)offset
{
    SimplePoly_iOSRef poly(new SimplePoly_iOS);
    poly->texture = image;
    poly->color = [color asRGBAColor];

    poly->pts.resize(4);
    poly->texCoords.resize(4);

    CGFloat width2 = size.width/2.0, height2 = size.height/2.0;
    poly->pts[0] = Point2d(-width2+offset.x,-height2+offset.y);
    poly->texCoords[0] = TexCoord(0,1);
    poly->pts[1] = Point2d(width2+offset.x,-height2+offset.y);
    poly->texCoords[1] = TexCoord(1,1);
    poly->pts[2] = Point2d(width2+offset.x,height2+offset.y);
    poly->texCoords[2] = TexCoord(1,0);
    poly->pts[3] = Point2d(-width2+offset.x,height2+offset.y);
    poly->texCoords[3] = TexCoord(0,0);
    
    screenObj.polys.push_back(poly);
}

- (MaplyBoundingBox)getSize
{
    Mbr mbr;
    
    for (auto poly : screenObj.polys)
    {
        for (const Point2d &pt : poly->pts)
        {
            mbr.addPoint(pt);
        }
    }
    
    for (auto str : screenObj.strings)
    {
        Point3d p0 = str->mat * Point3d(0,0,1);
        Point3d p1 = str->mat * Point3d(str->size.x(),str->size.y(),1);
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
    
    for (auto poly : screenObj.polys)
    {
        for (Point2d &pt : poly->pts)
        {
            Point3d newPt = mat * Point3d(pt.x(),pt.y(),1.0);
            pt = Point2d(newPt.x(),newPt.y());
        }
    }
    
    for (auto str : screenObj.strings)
    {
        str->mat = mat * str->mat;
    }
}

- (void)translateX:(double)x y:(double)y
{
    Eigen::Affine2d trans(Translation2d(x,y));
    
    for (auto poly : screenObj.polys)
    {
        for (Point2d &pt : poly->pts)
            pt = trans * pt;
    }
    
    Eigen::Matrix3d mat = trans.matrix();
    for (auto str : screenObj.strings)
    {
        str->mat = mat * str->mat;
    }
}

- (void)addScreenObject:(MaplyScreenObject *)otherScreenObj
{
    screenObj.polys.insert(screenObj.polys.end(),otherScreenObj->screenObj.polys.begin(),otherScreenObj->screenObj.polys.end());
    screenObj.strings.insert(screenObj.strings.end(), otherScreenObj->screenObj.strings.begin(), otherScreenObj->screenObj.strings.end());
}

@end
