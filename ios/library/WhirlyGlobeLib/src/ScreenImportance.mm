/*
 *  ScreenImportance.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/11/13.
 *  Copyright 2011-2017 mousebird consulting
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
#import "GlobeLayerViewWatcher.h"
#import "MaplyLayerViewWatcher.h"
#import "UIImage+Stuff.h"
#import "VectorData.h"
#import "SceneRendererES2.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation WhirlyKitDisplaySolid

// Let's not support tiles less than 10m on a side
//static float const BoundsEps = 10.0 / EarthRadius;

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

+ (WhirlyKitDisplaySolid *)displaySolidWithNodeIdent:(WhirlyKit::Quadtree::Identifier &)nodeIdent mbr:(WhirlyKit::Mbr)nodeMbr minZ:(float)inMinZ maxZ:(float)inMaxZ srcSystem:(WhirlyKit::CoordSystem *)srcSystem adapter:(WhirlyKit::CoordSystemDisplayAdapter *)coordAdapter;
{
    WhirlyKitDisplaySolid *dispSolid = [[WhirlyKitDisplaySolid alloc] init];

    // Start with the corner points in the source
    WhirlyKit::CoordSystem *displaySystem = coordAdapter->getCoordSystem();
    std::vector<Point3d> srcBounds;
    srcBounds.push_back(Point3d(nodeMbr.ll().x(),nodeMbr.ll().y(),inMinZ));
    srcBounds.push_back(Point3d(nodeMbr.ur().x(),nodeMbr.ll().y(),inMinZ));
    srcBounds.push_back(Point3d(nodeMbr.ur().x(),nodeMbr.ur().y(),inMinZ));
    srcBounds.push_back(Point3d(nodeMbr.ll().x(),nodeMbr.ur().y(),inMinZ));

    // Number of samples in X and Y we need for a decent surface
    int numSamplesX = std::max(calcNumSamples(srcBounds[0],srcBounds[1],srcSystem,coordAdapter,nodeIdent.level),
                               calcNumSamples(srcBounds[3],srcBounds[2],srcSystem,coordAdapter,nodeIdent.level))+1;
    int numSamplesY = std::max(calcNumSamples(srcBounds[0],srcBounds[3],srcSystem,coordAdapter,nodeIdent.level),
                               calcNumSamples(srcBounds[1],srcBounds[2],srcSystem,coordAdapter,nodeIdent.level))+1;

    // Build a grid of samples
    std::vector<Point3d> dispPoints;
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
    dispSolid.polys.reserve(numSamplesX*numSamplesY);
    for (int ix=0;ix<numSamplesX-1;ix++) {
        for (int iy=0;iy<numSamplesY-1;iy++) {
            // Surface polygon
            std::vector<Point3d> poly;
            poly.reserve(4);
            poly.push_back(dispPoints[iy*numSamplesX+ix]);
            poly.push_back(dispPoints[(iy+1)*numSamplesX+ix]);
            poly.push_back(dispPoints[(iy+1)*numSamplesX+(ix+1)]);
            poly.push_back(dispPoints[iy*numSamplesX+(ix+1)]);
            dispSolid.polys.push_back(poly);
            
            // And a normal
            if (coordAdapter->isFlat())
                dispSolid.normals.push_back(Vector3d(0,0,1));
            else {
                Point3d &p0 = poly[0];
                Point3d &p1 = poly[1];
                Point3d &p2 = poly[poly.size()-1];
                Vector3d norm = (p1-p0).cross(p2-p0);
                norm.normalize();
                dispSolid.normals.push_back(norm);
            }
        }
    }
    
    return dispSolid;
}

double PolyImportance(const std::vector<Point3d> &poly,const Point3d &norm,WhirlyKitViewState *viewState,WhirlyKit::Point2f frameSize)
{
    double import = 0.0;
    
    for (unsigned int offi=0;offi<viewState.viewMatrices.size();offi++)
    {
        double origArea = PolygonArea(poly,norm);
        origArea = std::abs(origArea);
        
        std::vector<Eigen::Vector4d> pts;
        pts.reserve(poly.size());
        for (unsigned int ii=0;ii<poly.size();ii++)
        {
            const Point3d &pt = poly[ii];
            // Run through the model transform
            Vector4d modPt = viewState.fullMatrices[offi] * Vector4d(pt.x(),pt.y(),pt.z(),1.0);
            // And then the projection matrix.  Now we're in clip space
            Vector4d projPt = viewState.projMatrix * modPt;
            pts.push_back(projPt);
        }
        
        // The points are in clip space, so clip!
        std::vector<Eigen::Vector4d> clipSpacePts;
        clipSpacePts.reserve(2*pts.size());
        ClipHomogeneousPolygon(pts,clipSpacePts);
        
        // Outside the viewing frustum, so ignore it
        if (clipSpacePts.empty())
            continue;
        
        // Project to the screen
        std::vector<Point2d> screenPts;
        screenPts.reserve(clipSpacePts.size());
        Point2d halfFrameSize(frameSize.x()/2.0,frameSize.y()/2.0);
        for (unsigned int ii=0;ii<clipSpacePts.size();ii++)
        {
            Vector4d &outPt = clipSpacePts[ii];
            Point2d screenPt(outPt.x()/outPt.w() * halfFrameSize.x()+halfFrameSize.x(),outPt.y()/outPt.w() * halfFrameSize.y()+halfFrameSize.y());
            screenPts.push_back(screenPt);
        }
        
        double screenArea = CalcLoopArea(screenPts);
        if (std::isnan(screenArea))
            screenArea = 0.0;
        // The polygon came out backwards, so toss it
        if (screenArea <= 0.0)
            continue;
        
        // Now project the screen points back into model space
        std::vector<Point3d> backPts;
        backPts.reserve(screenPts.size());
        for (unsigned int ii=0;ii<screenPts.size();ii++)
        {
            Vector4d modelPt = viewState.invProjMatrix * clipSpacePts[ii];
            Vector4d backPt = viewState.invFullMatrices[offi] * modelPt;
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

- (bool)isInside:(WhirlyKit::Point3d)pt
{
    // Note: Fix this.  This will do weird things when we're very close.
    return false;
}

- (double)importanceForViewState:(WhirlyKitViewState *)viewState frameSize:(WhirlyKit::Point2f)frameSize;
{
    Point3d eyePos = viewState.eyePos;
//    eyePos.normalize();
    
    if (!viewState.coordAdapter->isFlat())
    {
        // If the viewer is inside the bounds, the node is maximimally important (duh)
        if ([self isInside:eyePos])
            return MAXFLOAT;
    }
    
    // Now work through the polygons and project each to the screen
    double totalImport = 0.0;
    for (unsigned int ii=0;ii<_polys.size();ii++)
    {
        if (_normals[ii].dot(eyePos) >= 0.0) {
            double import = PolyImportance(_polys[ii], _normals[ii], viewState, frameSize);
            totalImport += import;
        }
    }
    
    // The flat map case is optimized to only evaluate one poly, since there's no curvature
    double scaleFactor = (_polys.size() > 1 ? 0.5 : 1.0);
    
    return totalImport*scaleFactor;
}

- (bool)isOnScreenForViewState:(WhirlyKitViewState *)viewState frameSize:(WhirlyKit::Point2f)frameSize
{
    if (!viewState.coordAdapter->isFlat())
    {
        // If the viewer is inside the bounds, the node is maximimally important (duh)
        if ([self isInside:viewState.eyePos])
            return MAXFLOAT;
    }
    
    for (unsigned int offi=0;offi<viewState.viewMatrices.size();offi++)
    {
        for (unsigned int ii=0;ii<_polys.size();ii++)
        {
            const std::vector<Point3d> &poly = _polys[ii];
            double origArea = PolygonArea(poly,_normals[ii]);
            origArea = std::abs(origArea);
            
            std::vector<Eigen::Vector4d> pts;
            pts.reserve(poly.size());
            for (unsigned int ii=0;ii<poly.size();ii++)
            {
                const Point3d &pt = poly[ii];
                // Run through the model transform
                Vector4d modPt = viewState.fullMatrices[offi] * Vector4d(pt.x(),pt.y(),pt.z(),1.0);
                // And then the projection matrix.  Now we're in clip space
                Vector4d projPt = viewState.projMatrix * modPt;
                pts.push_back(projPt);
            }
            
            // The points are in clip space, so clip!
            std::vector<Eigen::Vector4d> clipSpacePts;
            clipSpacePts.reserve(2*pts.size());
            ClipHomogeneousPolygon(pts,clipSpacePts);

            // Got something inside the viewing frustum.  Good enough.
            if (!clipSpacePts.empty())
                return true;
        }
    }
    
    return false;
}

@end

namespace WhirlyKit
{
    
bool TileIsOnScreen(WhirlyKitViewState *viewState,WhirlyKit::Point2f frameSize,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,WhirlyKit::Mbr nodeMbr,WhirlyKit::Quadtree::Identifier &nodeIdent,NSMutableDictionary *attrs)
{
    WhirlyKitDisplaySolid *dispSolid = attrs[@"DisplaySolid"];
    if (!dispSolid)
    {
        dispSolid = [WhirlyKitDisplaySolid displaySolidWithNodeIdent:nodeIdent mbr:nodeMbr minZ:0.0 maxZ:0.0 srcSystem:srcSystem adapter:coordAdapter];
        if (dispSolid)
        attrs[@"DisplaySolid"] = dispSolid;
        else
        attrs[@"DisplaySolid"] = [NSNull null];
    }
    
    // This means the tile is degenerate (as far as we're concerned)
    if ([dispSolid isKindOfClass:[NSNull null]])
        return false;

    return [dispSolid isOnScreenForViewState:viewState frameSize:frameSize];
}


// Calculate the max pixel size for a tile
double ScreenImportance(WhirlyKitViewState *viewState,WhirlyKit::Point2f frameSize,const Point3d &notUsed,int pixelsSquare,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,Mbr nodeMbr,WhirlyKit::Quadtree::Identifier &nodeIdent,NSMutableDictionary *attrs)
{
    WhirlyKitDisplaySolid *dispSolid = attrs[@"DisplaySolid"];
    if (!dispSolid)
    {
        dispSolid = [WhirlyKitDisplaySolid displaySolidWithNodeIdent:nodeIdent mbr:nodeMbr minZ:0.0 maxZ:0.0 srcSystem:srcSystem adapter:coordAdapter];
        if (!dispSolid)
            dispSolid = (WhirlyKitDisplaySolid *)[NSNull null];
        attrs[@"DisplaySolid"] = dispSolid;
    }
    
    // This means the tile is degenerate (as far as we're concerned)
    if ([dispSolid isKindOfClass:[NSNull class]])
        return 0.0;
    
    double import = [dispSolid importanceForViewState:viewState frameSize:frameSize];
    // The system is expecting an estimate of pixel size on screen
    import = import/(pixelsSquare * pixelsSquare);
    
//    NSLog(@"Import: %d: (%d,%d)  %f",nodeIdent.level,nodeIdent.x,nodeIdent.y,import);
    
    return import;
}

// This version is for volumes with height
double ScreenImportance(WhirlyKitViewState *viewState,WhirlyKit::Point2f frameSize,int pixelsSquare,WhirlyKit::CoordSystem *srcSystem,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,Mbr nodeMbr,double minZ,double maxZ,WhirlyKit::Quadtree::Identifier &nodeIdent,NSMutableDictionary *attrs)
{
    WhirlyKitDisplaySolid *dispSolid = attrs[@"DisplaySolid"];
    if (!dispSolid)
    {
        dispSolid = [WhirlyKitDisplaySolid displaySolidWithNodeIdent:nodeIdent mbr:nodeMbr minZ:minZ maxZ:maxZ srcSystem:srcSystem adapter:coordAdapter];
        if (dispSolid)
            attrs[@"DisplaySolid"] = dispSolid;
        else
            attrs[@"DisplaySolid"] = [NSNull null];
    }
    
    // This means the tile is degenerate (as far as we're concerned)
    if ([dispSolid isKindOfClass:[NSNull class]])
        return 0.0;
    
    double import = [dispSolid importanceForViewState:viewState frameSize:frameSize];
    // The system is expecting an estimate of pixel size on screen
    import = import/(pixelsSquare * pixelsSquare);
    
    //    NSLog(@"Import: %d: (%d,%d)  %f",nodeIdent.level,nodeIdent.x,nodeIdent.y,import);
    
    return import;
}
    
}
