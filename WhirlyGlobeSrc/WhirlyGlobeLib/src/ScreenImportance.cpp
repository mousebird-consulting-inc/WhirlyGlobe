/*
 *  ScreenImportance.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/11/13.
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

#import "ScreenImportance.h"
#import "FlatMath.h"
#import "GlobeMath.h"
#import "VectorData.h"
#import "FlatMath.h"
#import "ViewState.h"

using namespace Eigen;

namespace WhirlyKit
{

// Let's not support tiles less than 10m on a side
static float const BoundsEps = 10.0 / EarthRadius;

DisplaySolid::DisplaySolid(const Quadtree::Identifier &nodeIdent,const Mbr &nodeMbr,float inMinZ,float inMaxZ,CoordSystem *srcSystem,CoordSystemDisplayAdapter *coordAdapter)
    : valid(true)
{
    // Start with the outline in the source coordinate system
    WhirlyKit::CoordSystem *displaySystem = coordAdapter->getCoordSystem();
    Point3dVector srcBounds;
    srcBounds.push_back(Point3d(nodeMbr.ll().x(),nodeMbr.ll().y(),inMinZ));
    srcBounds.push_back(Point3d(nodeMbr.ur().x(),nodeMbr.ll().y(),inMinZ));
    srcBounds.push_back(Point3d(nodeMbr.ur().x(),nodeMbr.ur().y(),inMinZ));
    srcBounds.push_back(Point3d(nodeMbr.ll().x(),nodeMbr.ur().y(),inMinZ));
    srcBounds.push_back(Point3d((nodeMbr.ll().x()+nodeMbr.ur().x())/2.0,(nodeMbr.ll().y()+nodeMbr.ur().y())/2.0,inMinZ));
    if (inMinZ != inMaxZ)
    {
        srcBounds.push_back(Point3d(nodeMbr.ll().x(),nodeMbr.ll().y(),inMaxZ));
        srcBounds.push_back(Point3d(nodeMbr.ur().x(),nodeMbr.ll().y(),inMaxZ));
        srcBounds.push_back(Point3d(nodeMbr.ur().x(),nodeMbr.ur().y(),inMaxZ));
        srcBounds.push_back(Point3d(nodeMbr.ll().x(),nodeMbr.ur().y(),inMaxZ));
        srcBounds.push_back(Point3d((nodeMbr.ll().x()+nodeMbr.ur().x())/2.0,(nodeMbr.ll().y()+nodeMbr.ur().y())/2.0,inMaxZ));
    }
    
    // Figure out where the bounds drop in display space
    Point3dVector dispBounds;
    dispBounds.reserve(srcBounds.size());
    Point3dVector srcPts;
    srcPts.reserve(srcBounds.size());
    for (unsigned int ii=0;ii<srcBounds.size();ii++)
    {
        Point3d localPt = CoordSystemConvert3d(srcSystem, displaySystem, srcBounds[ii]);
        Point3d dispPt = coordAdapter->localToDisplay(localPt);
        Point3d dispNorm = coordAdapter->normalForLocal(localPt);
        surfNormals.push_back(dispNorm);
        // If the previous one is too close, ditch this one
        if (ii > 0)
        {
            double dist2 = (dispBounds[dispBounds.size()-1] - dispPt).squaredNorm();
            if (dist2 > BoundsEps*BoundsEps)
            {
                dispBounds.push_back(dispPt);
                srcPts.push_back(srcBounds[ii]);
            }
        } else {
            dispBounds.push_back(dispPt);
            srcPts.push_back(srcBounds[ii]);
        }
    }
    
    // If we didn't get enough boundary points, this is degenerate
    if (dispBounds.size() < 3)
    {
        valid = false;
        return;
    }
    
    // We'll set up a plane and start working in that space
    Point3d localMidPt = CoordSystemConvert3d(srcSystem, displaySystem, (srcBounds[0]+srcBounds[2])/2.0);
    Point3d dispMidPt = coordAdapter->localToDisplay(localMidPt);
    Point3d zAxis = coordAdapter->normalForLocal(localMidPt);  zAxis.normalize();
    Point3d xAxis = dispBounds[1] - dispBounds[0];  xAxis.normalize();
    Point3d yAxis = zAxis.cross(xAxis); yAxis.normalize();
    Point3d org = dispMidPt;
    
    // Project the corner points onto the plane
    // We'll collect height at the same time
    Point2dVector planePts;
    planePts.reserve(dispBounds.size());
    double minZ=MAXFLOAT,maxZ =-MAXFLOAT;
    Point2d minPt,maxPt;
    for (unsigned int ii=0;ii<dispBounds.size();ii++)
    {
        Point3d dir = dispBounds[ii]-org;
        Point3d planePt(dir.dot(xAxis),dir.dot(yAxis),dir.dot(zAxis));
        minZ = std::min(minZ,planePt.z());
        maxZ = std::max(maxZ,planePt.z());
        planePts.push_back(Point2d(planePt.x(),planePt.y()));
        if (ii == 0)
        {
            minPt = maxPt = Point2d(planePt.x(),planePt.y());
        } else {
            minPt.x() = std::min(minPt.x(),planePt.x());
            minPt.y() = std::min(minPt.y(),planePt.y());
            maxPt.x() = std::max(maxPt.x(),planePt.x());
            maxPt.y() = std::max(maxPt.y(),planePt.y());
        }
    }
    
    // Now sample the edges back in the source coordinate system
    //  and see where they land in here
    
    // Surface normals only make sense if there is a surface, but there are other important
    //  calculations below
    if (inMinZ == inMaxZ)
        surfNormals.reserve(srcPts.size());
    
    for (unsigned int ii=0;ii<srcPts.size();ii++)
    {
        //        Point2f &planePt0 = planePts[ii], &planePt1 = planePts[(ii+1)%planePts.size()];
        // Project the test point all the way into our plane
        Point3d edgeSrcPt = (srcPts[ii]+srcPts[(ii+1)%srcPts.size()])/2.0;
        Point3d localPt = CoordSystemConvert3d(srcSystem, displaySystem, edgeSrcPt);
        Point3d edgeDispPt = coordAdapter->localToDisplay(localPt);
        Point3d dir = edgeDispPt-org;
        Point3d planePt(dir.dot(xAxis),dir.dot(yAxis),dir.dot(zAxis));
        // Update the min and max
        minZ = std::min(minZ,planePt.z());
        maxZ = std::max(maxZ,planePt.z());
        // Note: Trying MBR
        minPt.x() = std::min(minPt.x(),planePt.x());
        minPt.y() = std::min(minPt.y(),planePt.y());
        maxPt.x() = std::max(maxPt.x(),planePt.x());
        maxPt.y() = std::max(maxPt.y(),planePt.y());
        
        // And throw in another normal for the biggest tiles
        Point3d dispNorm = coordAdapter->normalForLocal(localPt);
        if (inMinZ == inMaxZ)
            surfNormals.push_back(Vector3d(dispNorm.x(),dispNorm.y(),dispNorm.z()));
        
#if 0
        // See if the plane pt is on the right of the two sample points
        if ((planePt1.x()-planePt0.x())*(planePt.y()-planePt0.y()) - (planePt1.y()-planePt0.y())*(planePt.x()-planePt0.x()) < 0.0)
        {
            // If it is, we need to nudge the line outward
            // Need the point previous to 0 and the one after 1
            Point2f &planePt_1 = planePts[(ii-1+srcPts.size())%srcPts.size()];
            Point2f &planePt2 = planePts[(ii+2)%srcPts.size()];
            Point2f dir = planePt1 - planePt0;
            Point2f ePt0 = Point2f(planePt.x(),planePt.y());
            Point2f ePt1 = ePt0 + dir;
            
            // Find the various intersections
            Point2f new_planePt0,new_planePt1;
            if (IntersectLines(ePt0, ePt1, planePt_1, planePt0, &new_planePt0))
                planePt0 = new_planePt0;
            if (IntersectLines(ePt0, ePt1, planePt1, planePt2, &new_planePt1))
                planePt1 = new_planePt1;
        }
#endif
    }
    
    // Now convert the plane points back into display space for the volume
    Point3dVector botCorners;
    Point3dVector topCorners;
    Point2dVector planeMbrPts;  planeMbrPts.reserve(4);
    planeMbrPts.push_back(minPt);
    planeMbrPts.push_back(Point2d(maxPt.x(),minPt.y()));
    planeMbrPts.push_back(maxPt);
    planeMbrPts.push_back(Point2d(minPt.x(),maxPt.y()));
    botCorners.reserve(planeMbrPts.size());
    topCorners.reserve(planeMbrPts.size());
    //    for (unsigned int ii=0;ii<planePts.size();ii++)
    for (unsigned int ii=0;ii<planeMbrPts.size();ii++)
    {
        Point2d planePt = planeMbrPts[ii];
        Point3d dispPt0 = xAxis * planePt.x() + yAxis * planePt.y() + zAxis * minZ + org;
        Point3d dispPt1 = xAxis * planePt.x() + yAxis * planePt.y() + zAxis * maxZ + org;
        botCorners.push_back(Point3d(dispPt0.x(),dispPt0.y(),dispPt0.z()));
        topCorners.push_back(Point3d(dispPt1.x(),dispPt1.y(),dispPt1.z()));
    }
    
    // Now let's go ahead and form the polygons for the planes
    // First the ones around the outside
    // If we've got a flat coord adapter and no height, one poly will do
    if (coordAdapter->isFlat() && minZ == maxZ)
    {
        polys.push_back(topCorners);
    } else {
      polys.reserve(planeMbrPts.size()+2);
      for (unsigned int ii=0;ii<planeMbrPts.size();ii++)
      {
          int thisPt = ii;
          int nextPt = (ii+1)%planeMbrPts.size();
          Point3dVector poly;  poly.reserve(4);
          poly.push_back(botCorners[thisPt]);
          poly.push_back(botCorners[nextPt]);
          poly.push_back(topCorners[nextPt]);
          poly.push_back(topCorners[thisPt]);
          polys.push_back(poly);
      }
      // Then top and bottom
      polys.push_back(topCorners);
      std::reverse(botCorners.begin(),botCorners.end());
      polys.push_back(botCorners);
    }
    
    // Now calculate normals for each of those
    normals.reserve(polys.size());
    for (unsigned int ii=0;ii<polys.size();ii++)
    {
        if (coordAdapter->isFlat())
            normals.push_back(Vector3d(0,0,1));
        else {
            Point3dVector &poly = polys[ii];
            Point3d &p0 = poly[0];
            Point3d &p1 = poly[1];
            Point3d &p2 = poly[poly.size()-1];
            Vector3d norm = (p1-p0).cross(p2-p0);
            norm.normalize();
            normals.push_back(norm);
        }
    }
    
    valid = true;
}

double PolyImportance(const Point3dVector &poly,const Point3d &norm,WhirlyKit::ViewState *viewState,WhirlyKit::Point2f frameSize)
{
    double import = 0.0;
    
    for (unsigned int offi=0;offi<viewState->viewMatrices.size();offi++)
    {
        double origArea = PolygonArea(poly,norm);
        origArea = std::abs(origArea);
    
        Vector4dVector pts;
        pts.reserve(poly.size());
        for (unsigned int ii=0;ii<poly.size();ii++)
        {
            const Point3d &pt = poly[ii];
            // Run through the model transform
            Vector4d modPt = viewState->fullMatrices[offi] * Vector4d(pt.x(),pt.y(),pt.z(),1.0);
            // And then the projection matrix.  Now we're in clip space
            Vector4d projPt = viewState->projMatrix * modPt;
            pts.push_back(projPt);
        }
    
        // The points are in clip space, so clip!
        Vector4dVector clipSpacePts;
        clipSpacePts.reserve(2*pts.size());
        ClipHomogeneousPolygon(pts,clipSpacePts);
    
        // Outside the viewing frustum, so ignore it
        if (clipSpacePts.empty())
            continue;
    
        // Project to the screen
        Point2dVector screenPts;
        screenPts.reserve(clipSpacePts.size());
        Point2d halfFrameSize(frameSize.x()/2.0,frameSize.y()/2.0);
        for (unsigned int ii=0;ii<clipSpacePts.size();ii++)
        {
            Vector4d &outPt = clipSpacePts[ii];
            Point2d screenPt(outPt.x()/outPt.w() * halfFrameSize.x()+halfFrameSize.x(),outPt.y()/outPt.w() * halfFrameSize.y()+halfFrameSize.y());
            screenPts.push_back(screenPt);
        }
    
        double screenArea = CalcLoopArea(screenPts);
        screenArea = std::abs(screenArea);
        if (std::isnan(screenArea))
            screenArea = 0.0;
    
        // Now project the screen points back into model space
        Point3dVector backPts;
        backPts.reserve(screenPts.size());
        for (unsigned int ii=0;ii<screenPts.size();ii++)
        {
            Vector4d modelPt = viewState->invProjMatrix * clipSpacePts[ii];
            Vector4d backPt = viewState->invFullMatrices[offi] * modelPt;
            backPts.push_back(Point3d(backPt.x(),backPt.y(),backPt.z()));
        }
        // Then calculate the area
        double backArea = PolygonArea(backPts,norm);
        backArea = std::abs(backArea);
    
        // Now we know how much of the original polygon made it out to the screen
        // We can scale its importance accordingly.
        // This gets rid of small slices of big tiles not getting loaded
        double scale = (backArea == 0.0) ? 1.0 : origArea / backArea;
    
        // Note: Turned off for the moment
        double newImport =  std::abs(screenArea) * scale;
        if (newImport > import)
        	import = newImport;
    }
    
    return import;
}

bool DisplaySolid::isInside(const Point3d &pt)
{
    // We should be on the inside of each plane
    for (unsigned int ii=0;ii<polys.size();ii++)
    {
        Point3d org = (polys[ii])[0];
        if ((pt-org).dot(normals[ii]) > 0.0)
            return false;
    }
    
    return true;
}

double DisplaySolid::importanceForViewState(ViewState *viewState,const Point2f &frameSize)
{
    Point3d eyePos = viewState->eyePos;
    
    if (!viewState->coordAdapter->isFlat())
    {
        // If the viewer is inside the bounds, the node is maximimally important (duh)
        if (isInside(eyePos))
            return MAXFLOAT;
        
        // Make sure that we're pointed toward the eye, even a bit
        if (!surfNormals.empty())
        {
            bool isFacing = false;
            for (unsigned int ii=0;ii<surfNormals.size();ii++)
            {
                const Vector3d &surfNorm = surfNormals[ii];
                if ((isFacing |= (surfNorm.dot(eyePos) >= 0.0)))
                    break;
            }
            if (!isFacing)
                return 0.0;
        }
    }
    
    // Now work through the polygons and project each to the screen
    double totalImport = 0.0;
    for (unsigned int ii=0;ii<polys.size();ii++)
    {
        double import = PolyImportance(polys[ii], normals[ii], viewState, frameSize);
        totalImport += import;
    }
    
    return totalImport/2.0;
}

bool DisplaySolid::isOnScreenForViewState(ViewState *viewState,const Point2f &frameSize)
{
    for (unsigned int offi=0;offi<viewState->viewMatrices.size();offi++)
    {
        for (unsigned int ii=0;ii<polys.size();ii++)
        {
            const Point3dVector &poly = polys[ii];
            double origArea = PolygonArea(poly,normals[ii]);
            origArea = std::abs(origArea);
        
            Vector4dVector pts;
            pts.reserve(poly.size());
            for (unsigned int ii=0;ii<poly.size();ii++)
            {
                const Point3d &pt = poly[ii];
                // Run through the model transform
                Vector4d modPt = viewState->fullMatrices[offi] * Vector4d(pt.x(),pt.y(),pt.z(),1.0);
                // And then the projection matrix.  Now we're in clip space
                Vector4d projPt = viewState->projMatrix * modPt;
                pts.push_back(projPt);
            }
        
            // The points are in clip space, so clip!
            Vector4dVector clipSpacePts;
            clipSpacePts.reserve(2*pts.size());
            ClipHomogeneousPolygon(pts,clipSpacePts);

            // Got something inside the viewing frustum.  Good enough.
            if (!clipSpacePts.empty())
                return true;
        }
    }
    
    return false;
}

bool TileIsOnScreen(WhirlyKit::ViewState *viewState,const WhirlyKit::Point2f &frameSize,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,const WhirlyKit::Mbr &nodeMbr,const WhirlyKit::Quadtree::Identifier &nodeIdent,Dictionary *attrs)
{
    DelayedDeletableRef objRef = attrs->getObject("DisplaySolid");
    DisplaySolidRef dispSolid = std::dynamic_pointer_cast<DisplaySolid>(objRef);
    if (!dispSolid)
    {
        dispSolid = DisplaySolidRef(new DisplaySolid(nodeIdent,nodeMbr,0.0,0.0,srcSystem,coordAdapter));
        attrs->setObject("DisplaySolid",dispSolid);
    }
    
    // This means the tile is degenerate (as far as we're concerned)
    if (!dispSolid->valid)
        return false;

    return dispSolid->isOnScreenForViewState(viewState,frameSize);
}


// Calculate the max pixel size for a tile
double ScreenImportance(WhirlyKit::ViewState *viewState,const WhirlyKit::Point2f &frameSize,const Point3d &notUsed,int pixelsSquare,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,const Mbr &nodeMbr,const WhirlyKit::Quadtree::Identifier &nodeIdent,Dictionary *attrs)
{
    DelayedDeletableRef objRef = attrs->getObject("DisplaySolid");
    DisplaySolidRef dispSolid = std::dynamic_pointer_cast<DisplaySolid>(objRef);
    if (!dispSolid)
    {
        dispSolid = DisplaySolidRef(new DisplaySolid(nodeIdent,nodeMbr,0.0,0.0,srcSystem,coordAdapter));
        attrs->setObject("DisplaySolid",dispSolid);
    }
    
    // This means the tile is degenerate (as far as we're concerned)
    if (!dispSolid->valid)
        return 0.0;
    
    double import = dispSolid->importanceForViewState(viewState,frameSize);
    // The system is expecting an estimate of pixel size on screen
    import = import/(pixelsSquare * pixelsSquare);
    
    //    NSLog(@"Import: %d: (%d,%d)  %f",nodeIdent.level,nodeIdent.x,nodeIdent.y,import);
    
    return import;
}

// This version is for volumes with height
double ScreenImportance(WhirlyKit::ViewState *viewState,const WhirlyKit::Point2f &frameSize,int pixelsSquare,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,const Mbr &nodeMbr,double minZ,double maxZ,const WhirlyKit::Quadtree::Identifier &nodeIdent,Dictionary *attrs)
{
    DelayedDeletableRef objRef = attrs->getObject("DisplaySolid");
    DisplaySolidRef dispSolid = std::dynamic_pointer_cast<DisplaySolid>(objRef);
    if (!dispSolid)
    {
        dispSolid = DisplaySolidRef(new DisplaySolid(nodeIdent,nodeMbr,minZ,maxZ,srcSystem,coordAdapter));
        attrs->setObject("DisplaySolid",dispSolid);
    }
    
    // This means the tile is degenerate (as far as we're concerned)
    if (!dispSolid->valid)
        return 0.0;
    
    double import = dispSolid->importanceForViewState(viewState,frameSize);
    // The system is expecting an estimate of pixel size on screen
    import = import/(pixelsSquare * pixelsSquare);
    
    //    NSLog(@"Import: %d: (%d,%d)  %f",nodeIdent.level,nodeIdent.x,nodeIdent.y,import);
    
    return import;
}
    
}
