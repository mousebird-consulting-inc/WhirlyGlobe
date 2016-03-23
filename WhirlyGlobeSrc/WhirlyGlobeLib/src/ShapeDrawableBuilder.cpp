/*
 *  ShapeDrawableBuilder.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/28/11.
 *  Copyright 2011-2016 mousebird consulting. All rights reserved.
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

#import "GlobeMath.h"
#import "VectorData.h"
#import "ShapeDrawableBuilder.h"
#import "Tesselator.h"
#import "Scene.h"

using namespace Eigen;
using namespace WhirlyKit;

/// Maximum number of triangles we'll stick in a drawable
//static const int MaxShapeDrawableTris=1<<15/3;

namespace WhirlyKit
{

WhirlyKitShapeInfo::WhirlyKitShapeInfo()
{
}

WhirlyKitShapeInfo::~WhirlyKitShapeInfo()
{
}


ShapeDrawableBuilder::ShapeDrawableBuilder(CoordSystemDisplayAdapter *coordAdapter, WhirlyKitShapeInfo *shapeInfo, bool linesOrPoints, const Point3d &center)
    : coordAdapter(coordAdapter), shapeInfo(shapeInfo), drawable(NULL), center(center)
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
    Point3f center3f(center.x(),center.y(),center.z());

    // Decide if we'll appending to an existing drawable or
    //  create a new one
    int ptCount = (int)(2*(pts.size()+1));
    if (!drawable || (drawable->getNumPoints()+ptCount > MaxDrawablePoints) || (drawable->getLineWidth() != lineWidth))
    {
        // We're done with it, toss it to the scene
        if (drawable)
            flush();

        drawable = new BasicDrawable("Shape Manager");
        shapeInfo->setupBasicDrawable(drawable);
        drawMbr.reset();
        drawable->setType(primType);
        // Adjust according to the vector info
        //            drawable->setColor([shapeInfo.color asRGBAColor]);
        drawable->setLineWidth(lineWidth);
        drawable->setRequestZBuffer(shapeInfo->getZBufferRead());
        drawable->setWriteZBuffer(shapeInfo->getZBufferWrite());
        drawable->setProgram(shapeInfo->programID);
        if (center.x() != 0.0 || center.y() != 0.0 || center.z() != 0.0)
        {
            Eigen::Affine3d trans(Eigen::Translation3d(center.x(),center.y(),center.z()));
            Matrix4d transMat = trans.matrix();
            drawable->setMatrix(&transMat);
        }
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
            drawable->addPoint((Point3f)(pt-center3f));
            drawable->addNormal(norm);
        } else {
            if (jj > 0)
            {
                drawable->addPoint((Point3f)(prevPt-center3f));
                drawable->addNormal(prevNorm);
                drawable->addColor(color);
                drawable->addPoint((Point3f)(pt-center3f));
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
        drawable->addPoint((Point3f)(prevPt-center3f));
        drawable->addNormal(prevNorm);
        drawable->addColor(color);
        drawable->addPoint((Point3f)(firstPt-center3f));
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

            if (shapeInfo->fade > 0.0)
            {
                TimeInterval curTime = time_t();
                drawable->setFade(curTime,curTime+shapeInfo->fade);
            }
            drawables.push_back(drawable);
        } else
            delete drawable;
        drawable = NULL;
    }
}

void ShapeDrawableBuilder::getChanges(WhirlyKit::ChangeSet &changes,SimpleIDSet &drawIDs)
{
    flush();
    for (unsigned int ii=0;ii<drawables.size();ii++)
    {
        BasicDrawable *draw = drawables[ii];
        changes.push_back(new AddDrawableReq(draw));
        drawIDs.insert(draw->getId());
    }
    drawables.clear();
}


ShapeDrawableBuilderTri::ShapeDrawableBuilderTri(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter, WhirlyKitShapeInfo *shapeInfo, const Point3d &center)
    : coordAdapter(coordAdapter), shapeInfo(shapeInfo), drawable(NULL), center(center)
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
    shapeInfo->setupBasicDrawable(drawable);
    drawMbr.reset();
    drawable->setType(GL_TRIANGLES);
    // Adjust according to the vector info
    drawable->setColor(shapeInfo->getColor());
    drawable->setRequestZBuffer(shapeInfo->getZBufferRead());
    drawable->setWriteZBuffer(shapeInfo->getZBufferWrite());
    drawable->setProgram(shapeInfo->programID);
    if (center.x() != 0.0 || center.y() != 0.0 || center.z() != 0.0)
    {
        Eigen::Affine3d trans(Eigen::Translation3d(center.x(),center.y(),center.z()));
        Matrix4d transMat = trans.matrix();
        drawable->setMatrix(&transMat);
    }
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
        const Point3f &pt = pts[ii];
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
void ShapeDrawableBuilderTri::addConvexOutline(std::vector<Point3f> &pts,Point3f norm,RGBAColor color,Mbr shapeMbr)
{
    // It's convex, so we'll just triangulate it dumb style
    for (unsigned int ii = 2;ii<pts.size();ii++)
        addTriangle(pts[0], norm, color, pts[ii-1], norm, color, pts[ii], norm, color, shapeMbr);
}

// Add a convex outline, triangulated
void ShapeDrawableBuilderTri::addConvexOutline(std::vector<Point3d> &pts,Point3d norm,RGBAColor color,Mbr shapeMbr)
{
    // It's convex, so we'll just triangulate it dumb style
    for (unsigned int ii = 2;ii<pts.size();ii++)
        addTriangle(pts[0], norm, color, pts[ii-1], norm, color, pts[ii], norm, color, shapeMbr);
}

void ShapeDrawableBuilderTri::addComplexOutline(std::vector<Point3d> &pts,Point3d norm,RGBAColor color,Mbr shapeMbr)
{
    Point3f norm3f(norm.x(),norm.y(),norm.z());

    // Note: Deal with doubles
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

            if (shapeInfo->fade > 0.0)
            {
                TimeInterval curTime = time_t(NULL);
                drawable->setFade(curTime,curTime+shapeInfo->fade);
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
