/*
 *  ScreenImportance.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/11/13.
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

#import "ScreenImportance.h"
#import "FlatMath.h"
#import "GlobeMath.h"
#import "VectorData.h"
#import "SceneRenderer.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{

// Calculate the number of samples required to represent the given line to a tolerance
int calcNumSamples(const Point3d &p0,const Point3d &p1,CoordSystem *srcSystem,CoordSystemDisplayAdapter *coordAdapter,int level)
{
    switch (level) {
    case 0:
        return 10;
    case 1:
        return 4;
    case 2:
        return 4;
    case 3:
        return 4;
    case 4:
        return 4;
    case 5:
        return 3;
    case 6:
        return 2;
    default:
        return 1;
    }
}

DisplaySolid::DisplaySolid(const QuadTreeIdentifier &nodeIdent,const Mbr &nodeMbr,float minZ,float maxZ,CoordSystem *srcSystem,CoordSystemDisplayAdapter *coordAdapter)
    : valid(false)
{
    // Start with the corner points in the source
    WhirlyKit::CoordSystem *displaySystem = coordAdapter->getCoordSystem();
    Point3dVector srcBounds;
    srcBounds.push_back(Point3d(nodeMbr.ll().x(),nodeMbr.ll().y(),minZ));
    srcBounds.push_back(Point3d(nodeMbr.ur().x(),nodeMbr.ll().y(),minZ));
    srcBounds.push_back(Point3d(nodeMbr.ur().x(),nodeMbr.ur().y(),minZ));
    srcBounds.push_back(Point3d(nodeMbr.ll().x(),nodeMbr.ur().y(),minZ));

    // Number of samples in X and Y we need for a decent surface
    int numSamplesX = std::max(calcNumSamples(srcBounds[0],srcBounds[1],srcSystem,coordAdapter,nodeIdent.level),
                               calcNumSamples(srcBounds[3],srcBounds[2],srcSystem,coordAdapter,nodeIdent.level))+1;
    int numSamplesY = std::max(calcNumSamples(srcBounds[0],srcBounds[3],srcSystem,coordAdapter,nodeIdent.level),
                               calcNumSamples(srcBounds[1],srcBounds[2],srcSystem,coordAdapter,nodeIdent.level))+1;

    // Build a grid of samples
    Point3dVector dispPoints;
    dispPoints.reserve(numSamplesX*numSamplesY);
    for (int ix=0;ix<numSamplesX;ix++) {
        double xt = ix/(double)(numSamplesX-1);
        Point3d srcPtx0 = (srcBounds[1] - srcBounds[0]) * xt + srcBounds[0];
        Point3d srcPtx1 = (srcBounds[2] - srcBounds[3]) * xt + srcBounds[3];
        for (int iy=0;iy<numSamplesY;iy++) {
            double yt = iy/(double)(numSamplesY-1);
            Point3d srcPt = (srcPtx1 - srcPtx0) * yt + srcPtx0;
            Point3d localPt = CoordSystemConvert3d(srcSystem, displaySystem, srcPt);
            Point3d dispPt = coordAdapter->localToDisplay(localPt);
            dispPoints.push_back(dispPt);
        }
    }
    
    // Build polygons out of those samples (in display space)
    bool boundingBoxValid = false;
    polys.reserve(numSamplesX*numSamplesY);
    for (int ix=0;ix<numSamplesX-1;ix++) {
        for (int iy=0;iy<numSamplesY-1;iy++) {
            // Surface polygon
            Point3dVector poly;
            poly.reserve(4);
            poly.push_back(dispPoints[iy*numSamplesX+ix]);
            poly.push_back(dispPoints[(iy+1)*numSamplesX+ix]);
            poly.push_back(dispPoints[(iy+1)*numSamplesX+(ix+1)]);
            poly.push_back(dispPoints[iy*numSamplesX+(ix+1)]);
            polys.push_back(poly);
            
            // Update bounding box
            for (auto pt: poly) {
                if (!boundingBoxValid) {
                    bbox0 = pt;  bbox1 = pt;
                    boundingBoxValid = true;
                } else {
                    bbox0.x() = std::min(pt.x(),bbox0.x());
                    bbox0.y() = std::min(pt.y(),bbox0.y());
                    bbox0.z() = std::min(pt.z(),bbox0.z());
                    bbox1.x() = std::max(pt.x(),bbox1.x());
                    bbox1.y() = std::max(pt.y(),bbox1.y());
                    bbox1.z() = std::max(pt.z(),bbox1.z());
                }
            }
            
            // And a normal
            if (coordAdapter->isFlat())
                normals.push_back(Vector3d(0,0,1));
            else {
                Point3d &p0 = poly[0];
                Point3d &p1 = poly[1];
                Point3d &p2 = poly[poly.size()-1];
                Vector3d norm = (p1-p0).cross(p2-p0);
                norm.normalize();
                normals.push_back(norm);
            }
        }
    }
    
    valid = true;
}

double PolyImportance(const Point3dVector &poly,const Point3d &norm,ViewState *viewState,const WhirlyKit::Point2f &frameSize)
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
        
        const double screenArea = CalcLoopArea(screenPts);
        // The polygon came out backwards, so toss it
        if (!std::isfinite(screenArea) || screenArea <= 0.0)
            continue;
        
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

        double newImport =  std::abs(screenArea) * scale;
        if (newImport > import)
            import = newImport;
    }
    
    return import;
}

bool DisplaySolid::isInside(const Point3d &pt)
{
    return bbox0.x() <= pt.x() && bbox0.y() <= pt.y() && bbox0.z() <= pt.z() &&
        pt.x() < bbox1.x() && pt.y() < bbox1.y() && pt.z() < bbox1.z();
}

double DisplaySolid::importanceForViewState(ViewState *viewState,const Point2f &frameSize)
{
    Point3d eyePos = viewState->eyePos;
//    eyePos.normalize();
    
    if (!viewState->coordAdapter->isFlat())
    {
        // If the viewer is inside the bounds, the node is maximimally important (duh)
        if (isInside(eyePos))
            return MAXFLOAT;
    }
    
    // Now work through the polygons and project each to the screen
    double totalImport = 0.0;
    for (unsigned int ii=0;ii<polys.size();ii++)
    {
        if (normals[ii].dot(eyePos) >= 0.0) {
            double import = PolyImportance(polys[ii], normals[ii], viewState, frameSize);
            totalImport += import;
        }
    }
    
    // The flat map case is optimized to only evaluate one poly, since there's no curvature
    double scaleFactor = (polys.size() > 1 ? 0.5 : 1.0);
    
    return totalImport*scaleFactor;
}

bool DisplaySolid::isOnScreenForViewState(ViewState *viewState,const Point2f &frameSize)
{
    if (!viewState->coordAdapter->isFlat())
    {
        // If the viewer is inside the bounds, the node is maximimally important (duh)
        if (isInside(viewState->eyePos))
            return MAXFLOAT;
    }
    
    for (unsigned int offi=0;offi<viewState->viewMatrices.size();offi++)
    {
        for (unsigned int ii=0;ii<polys.size();ii++)
        {
            const Point3dVector &poly = polys[ii];
            //double origArea = PolygonArea(poly,normals[ii]);
            //origArea = std::abs(origArea);
            
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

bool TileIsOnScreen(ViewState *viewState,const WhirlyKit::Point2f &frameSize,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,const WhirlyKit::Mbr &nodeMbr,const WhirlyKit::QuadTreeIdentifier &nodeIdent,DisplaySolidRef &dispSolid)
{
    if (!dispSolid)
        dispSolid = std::make_shared<DisplaySolid>(nodeIdent,nodeMbr,0.0,0.0,srcSystem,coordAdapter);
    
    // This means the tile is degenerate (as far as we're concerned)
    if (!dispSolid->valid)
        return false;
    
    return dispSolid->isOnScreenForViewState(viewState,frameSize);
}


// Calculate the max pixel size for a tile
double ScreenImportance(ViewState *viewState,const WhirlyKit::Point2f &frameSize,const Point3d &notUsed,int pixelsSquare,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,const Mbr &nodeMbr,const WhirlyKit::QuadTreeIdentifier &nodeIdent,DisplaySolidRef &dispSolid)
{
    if (!dispSolid)
        dispSolid = std::make_shared<DisplaySolid>(nodeIdent,nodeMbr,0.0,0.0,srcSystem,coordAdapter);

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
double ScreenImportance(ViewState *viewState,const WhirlyKit::Point2f &frameSize,int pixelsSquare,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,const Mbr &nodeMbr,double minZ,double maxZ,const WhirlyKit::QuadTreeIdentifier &nodeIdent,DisplaySolidRef &dispSolid)
{
    if (!dispSolid)
        dispSolid = std::make_shared<DisplaySolid>(nodeIdent,nodeMbr,minZ,maxZ,srcSystem,coordAdapter);
    
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
