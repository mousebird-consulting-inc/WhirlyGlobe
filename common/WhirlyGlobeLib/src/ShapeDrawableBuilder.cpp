/*  ShapeDrawableBuilder.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/28/11.
 *  Copyright 2011-2021 mousebird consulting.
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

#import "GlobeMath.h"
#import "VectorData.h"
#import "ShapeDrawableBuilder.h"
#import "Tesselator.h"
#import "Scene.h"
#import "SharedAttributes.h"

using namespace Eigen;
using namespace WhirlyKit;

/// Maximum number of triangles we'll stick in a drawable
//static const int MaxShapeDrawableTris=1<<15/3;

namespace WhirlyKit
{

ShapeInfo::ShapeInfo()
    : color(RGBAColor(255,255,255,255))
    , lineWidth(1.0)
    , insideOut(false)
    , hasCenter(false)
    , center(0.0,0.0,0.0)
{
    zBufferRead = true;
}

ShapeInfo::ShapeInfo(const Dictionary &dict)
    : BaseInfo(dict)
    , hasCenter(false)
    , center(0,0,0)
{
    zBufferRead = dict.getBool(MaplyZBufferRead, true);
    color = dict.getColor(MaplyColor,RGBAColor(255,255,255,255));
    lineWidth = dict.getDouble(MaplyVecWidth,1.0);
    insideOut = dict.getBool(MaplyShapeInsideOut,false);
    if (dict.hasField(MaplyShapeCenterX) || dict.hasField(MaplyShapeCenterY) || dict.hasField(MaplyShapeCenterZ))
    {
        hasCenter = true;
        // Snap to float
        center.x() = (float)dict.getDouble(MaplyShapeCenterX, 0.0);
        center.y() = (float)dict.getDouble(MaplyShapeCenterY, 0.0);
        center.z() = (float)dict.getDouble(MaplyShapeCenterZ, 0.0);
    }
}

ShapeDrawableBuilder::ShapeDrawableBuilder(CoordSystemDisplayAdapter *coordAdapter, SceneRenderer *sceneRender, const ShapeInfo &shapeInfo, bool linesOrPoints, const Point3d &center)
    : coordAdapter(coordAdapter), sceneRender(sceneRender), shapeInfo(shapeInfo), drawable(NULL), center(center)
{
    primType = (linesOrPoints ? Lines : Points);
}

ShapeDrawableBuilder::~ShapeDrawableBuilder()
{
}

void ShapeDrawableBuilder::addPoints(Point3dVector &pts,RGBAColor color,Mbr mbr,float lineWidth,bool closed)
{
    // Decide if we'll appending to an existing drawable or
    //  create a new one
    int ptCount = (int)(2*(pts.size()+1));
    if (!drawable || (drawable->getNumPoints()+ptCount > MaxDrawablePoints) || (drawable->getLineWidth() != lineWidth))
    {
        // We're done with it, toss it to the scene
        if (drawable)
            flush();

        drawable = sceneRender->makeBasicDrawableBuilder("Shape Manager");
        shapeInfo.setupBasicDrawable(drawable);
        drawMbr.reset();
        drawable->setType(primType);
        // Adjust according to the vector info
        //            drawable->setColor([shapeInfo.color asRGBAColor]);
        drawable->setLineWidth(lineWidth);
        if (center.x() != 0.0 || center.y() != 0.0 || center.z() != 0.0)
        {
            Eigen::Affine3d trans(Eigen::Translation3d(center.x(),center.y(),center.z()));
            Matrix4d transMat = trans.matrix();
            drawable->setMatrix(&transMat);
        }
    }
    drawMbr.expand(mbr);

    Point3d prevPt {0,0,0},prevNorm,firstPt {0,0,0},firstNorm;
    for (unsigned int jj=0;jj<pts.size();jj++)
    {
        // The point is already in display coordinates, so we have to project back
        const Point3d &pt = pts[jj];
        const Point3d localPt = coordAdapter->displayToLocal(pt);
        const Point3d norm = coordAdapter->normalForLocal(localPt);

        // Add to drawable
        // Depending on the type, we do this differently
        if (primType == Points)
        {
            drawable->addPoint(Point3d(pt-center));
            drawable->addNormal(norm);
        } else {
            if (jj > 0)
            {
                drawable->addPoint(Point3d(prevPt-center));
                drawable->addNormal(prevNorm);
                drawable->addColor(color);
                drawable->addPoint(Point3d(pt-center));
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
    if (closed && primType == Lines)
    {
        drawable->addPoint(Point3d(prevPt-center));
        drawable->addNormal(prevNorm);
        drawable->addColor(color);
        drawable->addPoint(Point3d(firstPt-center));
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
                TimeInterval curTime = time_t();
                drawable->setFade(curTime,curTime+shapeInfo.fade);
            }
            drawables.push_back(drawable);
        }
        drawable = NULL;
    }
}

void ShapeDrawableBuilder::getChanges(WhirlyKit::ChangeSet &changes,SimpleIDSet &drawIDs)
{
    flush();
    for (unsigned int ii=0;ii<drawables.size();ii++)
    {
        BasicDrawableBuilderRef draw = drawables[ii];
        changes.push_back(new AddDrawableReq(draw->getDrawable()));
        drawIDs.insert(draw->getDrawableID());
    }
    drawables.clear();
}


ShapeDrawableBuilderTri::ShapeDrawableBuilderTri(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter, SceneRenderer *sceneRender, const ShapeInfo &shapeInfo, const Point3d &center)
    : coordAdapter(coordAdapter), sceneRender(sceneRender), shapeInfo(shapeInfo), drawable(NULL), center(center), clipCoords(false)
{
}

ShapeDrawableBuilderTri::~ShapeDrawableBuilderTri()
{
}

void ShapeDrawableBuilderTri::setupNewDrawable()
{
    drawable = sceneRender->makeBasicDrawableBuilder("Shape Layer");
    shapeInfo.setupBasicDrawable(drawable);
    if (clipCoords)
        drawable->setClipCoords(true);
    drawMbr.reset();
    drawable->setType(Triangles);
    // Adjust according to the vector info
    drawable->setColor(shapeInfo.color);
    int which = 0;
    for (auto texID : texIDs)
    {
        drawable->setTexId(which, texID);
        which++;
    }
    if (center.x() != 0.0 || center.y() != 0.0 || center.z() != 0.0)
    {
        Eigen::Affine3d trans(Eigen::Translation3d(center.x(),center.y(),center.z()));
        Matrix4d transMat = trans.matrix();
        drawable->setMatrix(&transMat);
    }
}

void ShapeDrawableBuilderTri::setClipCoords(bool newClipCoords)
{
    if (clipCoords != newClipCoords)
    {
        // Different values of clipCoords aren't compatible, unsurprisingly
        if (drawable)
            flush();
    }
    
    clipCoords = newClipCoords;
}
    
void ShapeDrawableBuilderTri::setTexIDs(const std::vector<SimpleIdentity> &newTexIDs)
{
    if (texIDs != newTexIDs)
    {
        if (drawable)
            flush();
    }
    
    texIDs = newTexIDs;
}

// Add a triangle with normals
void ShapeDrawableBuilderTri::addTriangle(Point3f p0,Point3f n0,RGBAColor c0,Point3f p1,Point3f n1,RGBAColor c1,Point3f p2,Point3f n2,RGBAColor c2,Mbr shapeMbr)
{
    Point3f center3f(center.x(),center.y(),center.z());

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
    drawable->addPoint((Point3f)(p0-center3f));
    drawable->addNormal(n0);
    drawable->addColor(c0);
    drawable->addPoint((Point3f)(p1-center3f));
    drawable->addNormal(n1);
    drawable->addColor(c1);
    drawable->addPoint((Point3f)(p2-center3f));
    drawable->addNormal(n2);
    drawable->addColor(c2);

    drawable->addTriangle(BasicDrawable::Triangle(0+baseVert,2+baseVert,1+baseVert));
    drawMbr.expand(shapeMbr);
}

// Add a triangle with normals and texture coords
void ShapeDrawableBuilderTri::addTriangle(const Point3d &p0,const Point3d &n0,RGBAColor c0,const TexCoord &tx0,const Point3d &p1,const Point3d &n1,RGBAColor c1,const TexCoord &tx1,const Point3d &p2,const Point3d &n2,RGBAColor c2,const TexCoord &tx2,Mbr shapeMbr)
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
    drawable->addPoint((Point3d)(p0-center));
    drawable->addNormal(n0);
    drawable->addColor(c0);
    drawable->addTexCoord(0,tx0);
    drawable->addPoint((Point3d)(p1-center));
    drawable->addNormal(n1);
    drawable->addColor(c1);
    drawable->addTexCoord(0,tx1);
    drawable->addPoint((Point3d)(p2-center));
    drawable->addNormal(n2);
    drawable->addColor(c2);
    drawable->addTexCoord(0,tx2);
    
    drawable->addTriangle(BasicDrawable::Triangle(0+baseVert,2+baseVert,1+baseVert));
    drawMbr.expand(shapeMbr);
}

// Add a triangle with normals
void ShapeDrawableBuilderTri::addTriangle(Point3d p0,Point3d n0,RGBAColor c0,Point3d p1,Point3d n1,RGBAColor c1,Point3d p2,Point3d n2,RGBAColor c2,Mbr shapeMbr)
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
    drawable->addPoint((Point3d)(p0-center));
    drawable->addNormal(n0);
    drawable->addColor(c0);
    drawable->addPoint((Point3d)(p1-center));
    drawable->addNormal(n1);
    drawable->addColor(c1);
    drawable->addPoint((Point3d)(p2-center));
    drawable->addNormal(n2);
    drawable->addColor(c2);

    drawable->addTriangle(BasicDrawable::Triangle(0+baseVert,2+baseVert,1+baseVert));
    drawMbr.expand(shapeMbr);
}

// Add a group of pre-build triangles
void ShapeDrawableBuilderTri::addTriangles(Point3dVector &pts,Point3dVector &norms,std::vector<RGBAColor> &colors,std::vector<BasicDrawable::Triangle> &tris)
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
        const Point3d &pt = pts[ii];
        drawable->addPoint(Point3d(pt.x()-center.x(),pt.y()-center.y(),pt.z()-center.z()));
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
void ShapeDrawableBuilderTri::addConvexOutline(Point3fVector &pts,Point3f norm,RGBAColor color,Mbr shapeMbr)
{
    // It's convex, so we'll just triangulate it dumb style
    for (unsigned int ii = 2;ii<pts.size();ii++)
        addTriangle(pts[0], norm, color, pts[ii-1], norm, color, pts[ii], norm, color, shapeMbr);
}

// Add a convex outline, triangulated
void ShapeDrawableBuilderTri::addConvexOutline(Point3dVector &pts,Point3d norm,RGBAColor color,Mbr shapeMbr)
{
    // It's convex, so we'll just triangulate it dumb style
    for (unsigned int ii = 2;ii<pts.size();ii++)
        addTriangle(pts[0], norm, color, pts[ii-1], norm, color, pts[ii], norm, color, shapeMbr);
}

// Add a convex outline with tex coords, triangulated
void ShapeDrawableBuilderTri::addConvexOutline(Point3dVector &pts,std::vector<TexCoord> &texCoords,Point3d norm,RGBAColor color,Mbr shapeMbr)
{
    // It's convex, so we'll just triangulate it dumb style
    for (unsigned int ii = 2;ii<pts.size();ii++)
        addTriangle(pts[0], norm, color, texCoords[0], pts[ii-1], norm, color, texCoords[ii-1], pts[ii], norm, color, texCoords[ii], shapeMbr);
}

void ShapeDrawableBuilderTri::addComplexOutline(Point3dVector &pts,Point3d norm,RGBAColor color,Mbr shapeMbr)
{
    Point3f norm3f(norm.x(),norm.y(),norm.z());

    VectorRing ring;
    ring.resize(pts.size());
    for (unsigned int ii=0;ii<pts.size();ii++)
    {
        const Point3d &pt = pts[ii];
        ring[ii] = Point2f(pt.x()-center.x(),pt.y()-center.y());
    }

    VectorTrianglesRef trisRef = VectorTriangles::createTriangles();
    TesselateRing(ring,trisRef);

    for (unsigned int ii=0;ii<trisRef->tris.size();ii++)
    {
        const VectorTriangles::Triangle &tri = trisRef->tris[ii];
        Point3f thePts[3];
        thePts[0] = trisRef->pts[tri.pts[0]];
        thePts[1] = trisRef->pts[tri.pts[1]];
        thePts[2] = trisRef->pts[tri.pts[2]];
        addTriangle(thePts[0], norm3f, color, thePts[1], norm3f, color, thePts[2], norm3f, color, shapeMbr);
    }
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
                TimeInterval curTime = time_t(NULL);
                drawable->setFade(curTime,curTime+shapeInfo.fade);
            }
            drawables.push_back(drawable);
        }
        drawable = NULL;
    }
}

void ShapeDrawableBuilderTri::getChanges(ChangeSet &changeRequests,SimpleIDSet &drawIDs)
{
    flush();
    for (unsigned int ii=0;ii<drawables.size();ii++)
    {
        BasicDrawableBuilderRef draw = drawables[ii];
        changeRequests.push_back(new AddDrawableReq(draw->getDrawable()));
        drawIDs.insert(draw->getDrawableID());
    }
    drawables.clear();
}

}
