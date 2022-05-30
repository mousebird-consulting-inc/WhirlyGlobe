/*  ScreenImportance.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/11/13.
 *  Copyright 2011-2022 mousebird consulting
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

#import "ScreenImportance.h"
#import "FlatMath.h"
#import "GlobeMath.h"
#import "VectorData.h"
#import "SceneRenderer.h"
#import <array>

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{

// Calculate the number of samples required to represent the given line to a tolerance
static inline int calcNumSamples(const Point3d &p0,const Point3d &p1,int level)
{
    switch (level) {
    case 0:  return 10;
    case 1:
    case 2:
    case 3:
    case 4:  return 4;
    case 5:  return 3;
    case 6:  return 2;
    default: return 1;
    }
}

DisplaySolid::DisplaySolid(const QuadTreeIdentifier &nodeIdent,
                           const Mbr &nodeMbr,
                           float minZ, float maxZ,
                           const CoordSystem *srcSystem,
                           const CoordSystemDisplayAdapter *coordAdapter) :
    DisplaySolid(nodeIdent, MbrD(nodeMbr), minZ, maxZ, srcSystem, coordAdapter)
{
}

DisplaySolid::DisplaySolid(const QuadTreeIdentifier &nodeIdent,
                           const MbrD &nodeMbr,
                           float minZ,
                           __unused float maxZ,
                           const CoordSystem *srcSystem,
                           const CoordSystemDisplayAdapter *coordAdapter) :
    valid(false)
{
    // Start with the corner points in the source
    const WhirlyKit::CoordSystem *displaySystem = coordAdapter->getCoordSystem();
    const Point3d srcBounds[] = {
        {nodeMbr.ll().x(), nodeMbr.ll().y(), minZ},
        {nodeMbr.ur().x(), nodeMbr.ll().y(), minZ},
        {nodeMbr.ur().x(), nodeMbr.ur().y(), minZ},
        {nodeMbr.ll().x(), nodeMbr.ur().y(), minZ},
    };

    // Number of samples in X and Y we need for a decent surface
    const int numSamplesX = std::max(calcNumSamples(srcBounds[0],srcBounds[1],nodeIdent.level),
                                     calcNumSamples(srcBounds[3],srcBounds[2],nodeIdent.level))+1;
    const int numSamplesY = std::max(calcNumSamples(srcBounds[0],srcBounds[3],nodeIdent.level),
                                     calcNumSamples(srcBounds[1],srcBounds[2],nodeIdent.level))+1;

    // Build a grid of samples
    Point3dVector dispPoints;
    dispPoints.reserve(numSamplesX*numSamplesY);
    for (int ix=0;ix<numSamplesX;ix++)
    {
        const double xt = ix/(double)(numSamplesX-1);
        const Point3d srcPtx0 = (srcBounds[1] - srcBounds[0]) * xt + srcBounds[0];
        const Point3d srcPtx1 = (srcBounds[2] - srcBounds[3]) * xt + srcBounds[3];
        for (int iy=0;iy<numSamplesY;iy++)
        {
            const double yt = iy/(double)(numSamplesY-1);
            const Point3d srcPt = (srcPtx1 - srcPtx0) * yt + srcPtx0;
            const Point3d localPt = CoordSystemConvert3d(srcSystem, displaySystem, srcPt);
            dispPoints.push_back(coordAdapter->localToDisplay(localPt));
        }
    }
    
    // Build polygons out of those samples (in display space)
    bool boundingBoxValid = false;
    polys.reserve(numSamplesX*numSamplesY);
    normals.reserve(numSamplesX*numSamplesY);
    for (int ix=0;ix<numSamplesX-1;ix++)
    {
        for (int iy=0;iy<numSamplesY-1;iy++)
        {
            // Surface polygon
            polys.emplace_back();
            Point3dVector &poly = polys.back();
            poly.reserve(4);
            poly.push_back(dispPoints[iy*numSamplesX+ix]);
            poly.push_back(dispPoints[(iy+1)*numSamplesX+ix]);
            poly.push_back(dispPoints[(iy+1)*numSamplesX+(ix+1)]);
            poly.push_back(dispPoints[iy*numSamplesX+(ix+1)]);

            // Update bounding box
            for (const auto &pt: poly)
            {
                if (!boundingBoxValid)
                {
                    bbox0 = pt;
                    bbox1 = pt;
                    boundingBoxValid = true;
                }
                else
                {
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
            {
                normals.emplace_back(0,0,1);
            }
            else
            {
                const Point3d &p0 = poly[0];
                const Point3d &p1 = poly[1];
                const Point3d &p2 = poly[poly.size()-1];
                normals.push_back(((p1-p0).cross(p2-p0)).normalized());
            }
        }
    }
    
    valid = true;
}

double PolyImportance(const Point3dVector &poly,const Point3d &norm,ViewState *viewState,const WhirlyKit::Point2f &frameSize)
{
    double import = 0.0;
    const double origArea = std::abs(PolygonArea(poly,norm));

    for (unsigned int offi=0;offi<viewState->viewMatrices.size();offi++)
    {
        Vector4dVector pts;
        pts.reserve(poly.size());
        for (const auto &pt : poly)
        {
            // Run through the model transform
            const Vector4d modPt = viewState->fullMatrices[offi] * Vector4d(pt.x(),pt.y(),pt.z(),1.0);
            // And then the projection matrix.  Now we're in clip space
            pts.emplace_back(viewState->projMatrix * modPt);
        }
        
        // The points are in clip space, so clip!
        Vector4dVector clipSpacePts;
        clipSpacePts.reserve(2*pts.size());
        ClipHomogeneousPolygon(std::move(pts),clipSpacePts);
        
        // Outside the viewing frustum, so ignore it
        if (clipSpacePts.empty())
            continue;
        
        // Project to the screen
        Point2dVector screenPts;
        screenPts.reserve(clipSpacePts.size());

        const Point2d halfFrameSize(frameSize.x()/2.0,frameSize.y()/2.0);
        for (auto &outPt : clipSpacePts)
        {
            screenPts.emplace_back(outPt.x()/outPt.w() * halfFrameSize.x() + halfFrameSize.x(),
                                   outPt.y()/outPt.w() * halfFrameSize.y() + halfFrameSize.y());
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
            const Vector4d modelPt = viewState->invProjMatrix * clipSpacePts[ii];
            const Vector4d backPt = viewState->invFullMatrices[offi] * modelPt;
            backPts.emplace_back(backPt.x(),backPt.y(),backPt.z());
        }

        // Then calculate the area
        const double backArea = std::abs(PolygonArea(backPts,norm));

        // Now we know how much of the original polygon made it out to the screen
        // We can scale its importance accordingly.
        // This gets rid of small slices of big tiles not getting loaded
        const double scale = (backArea == 0.0) ? 1.0 : origArea / backArea;

        const double newImport =  std::abs(screenArea) * scale;
        if (newImport > import)
        {
            import = newImport;
        }
    }
    
    return import;
}

bool DisplaySolid::isInside(const Point3d &pt)
{
    return bbox0.x() <= pt.x() &&
           bbox0.y() <= pt.y() &&
           bbox0.z() <= pt.z() &&
           pt.x() < bbox1.x() &&
           pt.y() < bbox1.y() &&
           pt.z() < bbox1.z();
}

double DisplaySolid::importanceForViewState(ViewState *viewState,const Point2f &frameSize)
{
    const Point3d &eyePos = viewState->eyePos;
//    eyePos.normalize();
    
    if (!viewState->coordAdapter->isFlat())
    {
        // If the viewer is inside the bounds, the node is maximally important (duh)
        if (isInside(eyePos))
            return MAXFLOAT;
    }
    
    // Now work through the polygons and project each to the screen
    double totalImport = 0.0;
    for (unsigned int ii=0;ii<polys.size();ii++)
    {
        if (normals[ii].dot(eyePos) >= 0.0)
        {
            const double import = PolyImportance(polys[ii], normals[ii], viewState, frameSize);
            totalImport += import;
        }
    }
    
    // The flat map case is optimized to only evaluate one poly, since there's no curvature
    const double scaleFactor = (polys.size() > 1 ? 0.5 : 1.0);
    
    return totalImport*scaleFactor;
}

bool DisplaySolid::isOnScreenForViewState(ViewState *viewState,const Point2f &frameSize)
{
    if (!viewState->coordAdapter->isFlat())
    {
        // If the viewer is inside the bounds, the node is maximally important (duh)
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
            ClipHomogeneousPolygon(std::move(pts),clipSpacePts);

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
static double ScreenImportance(ViewState *viewState,const WhirlyKit::Point2f &frameSize,const Point3d &notUsed,int pixelsSquare,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,const Mbr &nodeMbr,const WhirlyKit::QuadTreeIdentifier &nodeIdent,DisplaySolid *dispSolid)
{
    // This means the tile is degenerate (as far as we're concerned)
    if (!dispSolid || !dispSolid->valid)
    {
        return 0.0;
    }

    const double import = dispSolid->importanceForViewState(viewState,frameSize);

    // The system is expecting an estimate of pixel size on screen
    return import / (pixelsSquare * pixelsSquare);
}

double ScreenImportance(ViewState *viewState,const WhirlyKit::Point2f &frameSize,const Point3d &notUsed,int pixelsSquare,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,const Mbr &nodeMbr,const WhirlyKit::QuadTreeIdentifier &nodeIdent,DisplaySolidRef &dispSolid)
{
    if (!dispSolid)
    {
        dispSolid = std::make_shared<DisplaySolid>(nodeIdent,nodeMbr,0.0,0.0,srcSystem,coordAdapter);
    }
    return ScreenImportance(viewState,frameSize,notUsed,pixelsSquare,srcSystem,coordAdapter,nodeMbr,nodeIdent,dispSolid.get());
}

double ScreenImportance(ViewState *viewState,const WhirlyKit::Point2f &frameSize,const Point3d &notUsed,int pixelsSquare,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,const Mbr &nodeMbr,const WhirlyKit::QuadTreeIdentifier &nodeIdent)
{
    DisplaySolid dispSolid(nodeIdent,nodeMbr,0.0,0.0,srcSystem,coordAdapter);
    return ScreenImportance(viewState,frameSize,notUsed,pixelsSquare,srcSystem,coordAdapter,nodeMbr,nodeIdent,&dispSolid);
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
