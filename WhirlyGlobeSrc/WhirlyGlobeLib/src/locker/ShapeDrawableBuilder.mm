/*
 *  ShapeDrawableBuilder.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/28/11.
 *  Copyright 2011-2013 mousebird consulting. All rights reserved.
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

#import "NSDictionary+Stuff.h"
#import "UIColor+Stuff.h"
#import "GlobeMath.h"
#import "VectorData.h"
#import "ShapeDrawableBuilder.h"

using namespace Eigen;
using namespace WhirlyKit;

/// Default priority for shapes.
static const int ShapeDrawPriority=1;

/// Maximum number of triangles we'll stick in a drawable
static const int MaxShapeDrawableTris=1<<15/3;


@implementation WhirlyKitShapeInfo

// Initialize with an array of shapes and parse out parameters
- (id)initWithShapes:(NSArray *)inShapes desc:(NSDictionary *)desc;
{
    self = [super init];
    
    if (self)
    {
        self.shapes = inShapes;
        [self parseDesc:desc];
        
        _shapeId = Identifiable::genId();
    }
    
    return self;
}


- (void)parseDesc:(NSDictionary *)desc
{
    self.color = [desc objectForKey:@"color" checkType:[UIColor class] default:[UIColor whiteColor]];
    _drawOffset = [desc floatForKey:@"drawOffset" default:0];
    _minVis = [desc floatForKey:@"minVis" default:DrawVisibleInvalid];
    _maxVis = [desc floatForKey:@"maxVis" default:DrawVisibleInvalid];
    _drawPriority = [desc intForKey:@"drawPriority" default:ShapeDrawPriority];
    _lineWidth = [desc floatForKey:@"width" default:1.0];
    _fade = [desc floatForKey:@"fade" default:0.0];
    _zBufferRead = [desc floatForKey:@"zbufferread" default:true];
    _zBufferWrite = [desc floatForKey:@"zbufferwrite" default:true];
    _enable = [desc boolForKey:@"enable" default:true];
    _shaderID = [desc intForKey:@"shader" default:EmptyIdentity];
}

@end


namespace WhirlyKit
{
    
ShapeDrawableBuilder::ShapeDrawableBuilder(CoordSystemDisplayAdapter *coordAdapter,WhirlyKitShapeInfo *shapeInfo,bool linesOrPoints)
        : coordAdapter(coordAdapter), shapeInfo(shapeInfo), drawable(NULL)
{
    primType = (linesOrPoints ? GL_LINES : GL_POINTS);
}
    
ShapeDrawableBuilder::~ShapeDrawableBuilder()
{
    for (unsigned int ii=0;ii<drawables.size();ii++)
        delete drawables[ii];
}

void ShapeDrawableBuilder::addPoints(std::vector<Point3f> &pts,RGBAColor color,Mbr mbr,float lineWidth,bool closed)
{
    // Decide if we'll appending to an existing drawable or
    //  create a new one
    int ptCount = 2*(pts.size()+1);
    if (!drawable || (drawable->getNumPoints()+ptCount > MaxDrawablePoints) || (drawable->getLineWidth() != lineWidth))
    {
        // We're done with it, toss it to the scene
        if (drawable)
            flush();
        
        drawable = new BasicDrawable("Shape Layer");
        drawMbr.reset();
        drawable->setType(primType);
        // Adjust according to the vector info
        drawable->setDrawOffset(shapeInfo.drawOffset);
        //            drawable->setColor([shapeInfo.color asRGBAColor]);
        drawable->setLineWidth(lineWidth);
        drawable->setDrawPriority(shapeInfo.drawPriority);
        drawable->setVisibleRange(shapeInfo.minVis,shapeInfo.maxVis);
        drawable->setRequestZBuffer(shapeInfo.zBufferRead);
        drawable->setWriteZBuffer(shapeInfo.zBufferWrite);
        drawable->setOnOff(shapeInfo.enable);
        drawable->setProgram(shapeInfo.shaderID);
    }
    drawMbr.expand(mbr);
    
    Point3f prevPt,prevNorm,firstPt,firstNorm;
    for (unsigned int jj=0;jj<pts.size();jj++)
    {
        // The point is already in display coordinates, so we have to project back
        Point3f pt = pts[jj];
        Point3f localPt = coordAdapter->displayToLocal(pt);
        Point3f norm = coordAdapter->normalForLocal(localPt);
        
        // Add to drawable
        // Depending on the type, we do this differently
        if (primType == GL_POINTS)
        {
            drawable->addPoint(pt);
            drawable->addNormal(norm);
        } else {
            if (jj > 0)
            {
                drawable->addPoint(prevPt);
                drawable->addNormal(prevNorm);
                drawable->addColor(color);
                drawable->addPoint(pt);
                drawable->addNormal(norm);
                drawable->addColor(color);
            } else {
                firstPt = pt;
                firstNorm = norm;
            }
            prevPt = pt;
            prevNorm = norm;
        }
    }
    
    // Close the loop
    if (closed && primType == GL_LINES)
    {
        drawable->addPoint(prevPt);
        drawable->addNormal(prevNorm);
        drawable->addColor(color);
        drawable->addPoint(firstPt);
        drawable->addNormal(firstNorm);
        drawable->addColor(color);
    }
}

void ShapeDrawableBuilder::flush()
{
    if (drawable)
    {
        if (drawable->getNumPoints() > 0)
        {
            drawable->setLocalMbr(drawMbr);
            
            if (shapeInfo.fade > 0.0)
            {
                NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
                drawable->setFade(curTime,curTime+shapeInfo.fade);
            }
            drawables.push_back(drawable);
        } else
            delete drawable;
        drawable = NULL;
    }
}

void ShapeDrawableBuilder::getChanges(ChangeSet &changeRequests,SimpleIDSet &drawIDs)
{
    flush();
    for (unsigned int ii=0;ii<drawables.size();ii++)
    {
        BasicDrawable *draw = drawables[ii];
        changeRequests.push_back(new AddDrawableReq(draw));
        drawIDs.insert(draw->getId());
    }
    drawables.clear();
}


ShapeDrawableBuilderTri::ShapeDrawableBuilderTri(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,WhirlyKitShapeInfo *shapeInfo)
: coordAdapter(coordAdapter), shapeInfo(shapeInfo), drawable(NULL)
{
}
    
ShapeDrawableBuilderTri::~ShapeDrawableBuilderTri()
{
    for (unsigned int ii=0;ii<drawables.size();ii++)
        delete drawables[ii];
}
    
void ShapeDrawableBuilderTri::setupNewDrawable()
{
    drawable = new BasicDrawable("Shape Layer");
    drawMbr.reset();
    drawable->setType(GL_TRIANGLES);
    // Adjust according to the vector info
    drawable->setDrawOffset(shapeInfo.drawOffset);
    drawable->setColor([shapeInfo.color asRGBAColor]);
    drawable->setDrawPriority(shapeInfo.drawPriority);
    drawable->setVisibleRange(shapeInfo.minVis,shapeInfo.maxVis);
    drawable->setRequestZBuffer(shapeInfo.zBufferRead);
    drawable->setWriteZBuffer(shapeInfo.zBufferWrite);
    drawable->setOnOff(shapeInfo.enable);
    drawable->setProgram(shapeInfo.shaderID);
}
    
// Add a triangle with normals
void ShapeDrawableBuilderTri::addTriangle(Point3f p0,Point3f n0,RGBAColor c0,Point3f p1,Point3f n1,RGBAColor c1,Point3f p2,Point3f n2,RGBAColor c2,Mbr shapeMbr)
{
    if (!drawable ||
        (drawable->getNumPoints()+3 > MaxDrawablePoints) ||
        (drawable->getNumTris()+1 > MaxDrawableTriangles))
    {
        // We're done with it, toss it to the scene
        if (drawable)
            flush();
        
        setupNewDrawable();
    }
    Mbr mbr = drawable->getLocalMbr();
    mbr.expand(shapeMbr);
    drawable->setLocalMbr(mbr);
    int baseVert = drawable->getNumPoints();
    drawable->addPoint(p0);
    drawable->addNormal(n0);
    drawable->addColor(c0);
    drawable->addPoint(p1);
    drawable->addNormal(n1);
    drawable->addColor(c1);
    drawable->addPoint(p2);
    drawable->addNormal(n2);
    drawable->addColor(c2);
    
    drawable->addTriangle(BasicDrawable::Triangle(0+baseVert,2+baseVert,1+baseVert));
    drawMbr.expand(shapeMbr);
}
    
// Add a group of pre-build triangles
void ShapeDrawableBuilderTri::addTriangles(std::vector<Point3f> &pts,std::vector<Point3f> &norms,std::vector<RGBAColor> &colors,std::vector<BasicDrawable::Triangle> &tris)
{
    if (!drawable ||
        (drawable->getNumPoints()+pts.size() > MaxDrawablePoints) ||
        (drawable->getNumTris()+tris.size() > MaxDrawableTriangles))
    {
        if (drawable)
            flush();
        
        setupNewDrawable();
    }
    
    int baseVert = drawable->getNumPoints();
    for (unsigned int ii=0;ii<pts.size();ii++)
    {
        drawable->addPoint(pts[ii]);
        drawable->addNormal(norms[ii]);
        drawable->addColor(colors[ii]);
    }
    for (unsigned int ii=0;ii<tris.size();ii++)
    {
        BasicDrawable::Triangle tri = tris[ii];
        for (unsigned int jj=0;jj<3;jj++)
            tri.verts[jj] += baseVert;
        drawable->addTriangle(tri);
    }
}
    
// Add a convex outline, triangulated
void ShapeDrawableBuilderTri::addConvexOutline(std::vector<Point3f> &pts,Point3f norm,RGBAColor color,Mbr shapeMbr)
{
    // It's convex, so we'll just triangulate it dumb style
    for (unsigned int ii = 2;ii<pts.size();ii++)
        addTriangle(pts[0], norm, color, pts[ii-1], norm, color, pts[ii], norm, color, shapeMbr);
}
    
void ShapeDrawableBuilderTri::flush()
{
    if (drawable)
    {
        if (drawable->getNumPoints() > 0)
        {
            drawable->setLocalMbr(drawMbr);
            
            if (shapeInfo.fade > 0.0)
            {
                NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
                drawable->setFade(curTime,curTime+shapeInfo.fade);
            }
            drawables.push_back(drawable);
        } else
            delete drawable;
        drawable = NULL;
    }
}
    
void ShapeDrawableBuilderTri::getChanges(ChangeSet &changeRequests,SimpleIDSet &drawIDs)
{
    flush();
    for (unsigned int ii=0;ii<drawables.size();ii++)
    {
        BasicDrawable *draw = drawables[ii];
        changeRequests.push_back(new AddDrawableReq(draw));
        drawIDs.insert(draw->getId());
    }
    drawables.clear();
}
    
    
}
