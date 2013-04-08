/*
 *  SelectionLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/26/11.
 *  Copyright 2011-2012 mousebird consulting. All rights reserved.
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

#import "SelectionLayer.h"
#import "NSDictionary+Stuff.h"
#import "UIColor+Stuff.h"
#import "GlobeMath.h"
#import "ScreenSpaceGenerator.h"
#import "MaplyView.h"
#import "WhirlyGeometry.h"

using namespace Eigen;
using namespace WhirlyKit;

bool RectSelectable3D::operator < (const RectSelectable3D &that) const
{
    return selectID < that.selectID;
}

bool RectSelectable2D::operator < (const RectSelectable2D &that) const
{
    return selectID < that.selectID;
}

bool PolytopeSelectable::operator < (const PolytopeSelectable &that) const
{
    return selectID < that.selectID;
}

@implementation WhirlyKitSelectionLayer

- (id)initWithView:(WhirlyKitView *)inView renderer:(WhirlyKitSceneRendererES *)inRenderer
{
    self = [super init];
    
    if (self)
    {
        theView = inView;
        renderer = inRenderer;
    }
    
    return self;
}

// Called in the layer thread
- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene
{
    layerThread = inLayerThread;
    scene = inScene;
}

- (void)shutdown
{
    // No visual representation, so nothing to do
}

// Add a rectangle (in 3-space) available for selection
- (void)addSelectableRect:(SimpleIdentity)selectId rect:(Point3f *)pts
{
    if (selectId == EmptyIdentity)
        return;
    
    RectSelectable3D newSelect;
    newSelect.selectID = selectId;
    newSelect.minVis = newSelect.maxVis = DrawVisibleInvalid;
    newSelect.norm = (pts[1] - pts[0]).cross(pts[3]-pts[0]).normalized();
    for (unsigned int ii=0;ii<4;ii++)
        newSelect.pts[ii] = pts[ii];
    
    rect3Dselectables.insert(newSelect);
}

// Add a rectangle (in 3-space) for selection, but only between the given visibilities
- (void)addSelectableRect:(SimpleIdentity)selectId rect:(Point3f *)pts minVis:(float)minVis maxVis:(float)maxVis
{
    if (selectId == EmptyIdentity)
        return;
    
    RectSelectable3D newSelect;
    newSelect.selectID = selectId;
    newSelect.minVis = minVis;  newSelect.maxVis = maxVis;
    newSelect.norm = (pts[1] - pts[0]).cross(pts[3]-pts[0]).normalized();
    for (unsigned int ii=0;ii<4;ii++)
        newSelect.pts[ii] = pts[ii];
    
    rect3Dselectables.insert(newSelect);
}

/// Add a screen space rectangle (2D) for selection, between the given visibilities
- (void)addSelectableScreenRect:(WhirlyKit::SimpleIdentity)selectId rect:(WhirlyKit::Point2f *)pts minVis:(float)minVis maxVis:(float)maxVis
{
    if (selectId == EmptyIdentity)
        return;
    
    RectSelectable2D newSelect;
    newSelect.selectID = selectId;
    newSelect.minVis = minVis;
    newSelect.maxVis = maxVis;
    for (unsigned int ii=0;ii<4;ii++)
        newSelect.pts[ii] = pts[ii];
    
    rect2Dselectables.insert(newSelect);
}

static const int corners[6][4] = {{0,1,2,3},{7,6,5,4},{1,0,4,5},{1,5,6,2},{2,6,7,3},{3,7,4,0}};

- (void)addSelectableRectSolid:(WhirlyKit::SimpleIdentity)selectId rect:(WhirlyKit::Point3f *)pts minVis:(float)minVis maxVis:(float)maxVis
{
    if (selectId == EmptyIdentity)
        return;
    
    PolytopeSelectable newSelect;
    newSelect.selectID = selectId;
    newSelect.minVis = minVis;
    newSelect.maxVis = maxVis;
    newSelect.midPt = Point3f(0,0,0);
    for (unsigned int ii=0;ii<8;ii++)
        newSelect.midPt += pts[ii];
    newSelect.midPt /= 8.0;
    for (unsigned int ii=0;ii<6;ii++)
    {
        std::vector<Point3f> poly;
        for (unsigned int jj=0;jj<4;jj++)
            poly.push_back(pts[corners[ii][jj]]);
        newSelect.polys.push_back(poly);
    }
    
    polytopeSelectables.insert(newSelect);
}

// Remove the given selectable from consideration
- (void)removeSelectable:(SimpleIdentity)selectID
{
    RectSelectable3DSet::iterator it = rect3Dselectables.find(RectSelectable3D(selectID));
    
    if (it != rect3Dselectables.end())
        rect3Dselectables.erase(it);
    
    RectSelectable2DSet::iterator it2 = rect2Dselectables.find(RectSelectable2D(selectID));
    if (it2 != rect2Dselectables.end())
        rect2Dselectables.erase(it2);
    
    PolytopeSelectableSet::iterator it3 = polytopeSelectables.find(PolytopeSelectable(selectID));
    if (it3 != polytopeSelectables.end())
        polytopeSelectables.erase(it3);
}

/// Pass in the screen point where the user touched.  This returns the closest hit within the given distance
- (SimpleIdentity)pickObject:(Point2f)touchPt view:(UIView *)view maxDist:(float)maxDist
{
    // Can only run in the layer thread
    if ([NSThread currentThread] != layerThread)
        return EmptyIdentity;
    
    float maxDist2 = maxDist * maxDist;
    
    // Precalculate the model matrix for use below
    Eigen::Matrix4d modelTrans = [theView calcFullMatrix];
    Point2f frameSize(renderer.framebufferWidth/view.contentScaleFactor,renderer.framebufferHeight/view.contentScaleFactor);
    Eigen::Matrix4d projTrans = [theView calcProjectionMatrix:frameSize margin:0.0];
    
    SimpleIdentity retId = EmptyIdentity;
    float closeDist2 = MAXFLOAT;
    
    WhirlyGlobeView *globeView = (WhirlyGlobeView *)theView;
    if (![theView isKindOfClass:[WhirlyGlobeView class]])
        globeView = nil;
    MaplyView *mapView = (MaplyView *)theView;
    if (![theView isKindOfClass:[MaplyView class]])
        mapView = nil;
    
    if (!globeView && !mapView)
        return EmptyIdentity;
    
    // First we need to know where the things wound up, 2D wise
    std::vector<ScreenSpaceGenerator::ProjectedPoint> projPts;
    scene->getScreenSpaceGenerator()->getProjectedPoints(projPts);    
    
    // Work through the 2D rectangles
    for (unsigned int ii=0;ii<projPts.size();ii++)
    {
        ScreenSpaceGenerator::ProjectedPoint projPt = projPts[ii];
        // If we're on a retina display, we need to scale accordingly
        projPt.screenLoc.x() /= view.contentScaleFactor;
        projPt.screenLoc.y() /= view.contentScaleFactor;
        
        // Look for the corresponding selectable
        RectSelectable2DSet::iterator it = rect2Dselectables.find(RectSelectable2D(projPt.shapeID));
        if (it != rect2Dselectables.end())
        {
            RectSelectable2D sel = *it;
            if (sel.selectID != EmptyIdentity)
            {
                if (sel.minVis == DrawVisibleInvalid ||
                    (sel.minVis < [theView heightAboveSurface] && [theView heightAboveSurface] < sel.maxVis))
                {
                    std::vector<Point2f> screenPts;
                    for (unsigned int jj=0;jj<4;jj++)
                        screenPts.push_back(sel.pts[jj]+projPt.screenLoc);

                    // See if we fall within that polygon
                    if (PointInPolygon(touchPt, screenPts))
                    {
                        retId = sel.selectID;
                        break;
                    }
                    
                    // Now for a proximity check around the edges
                    for (unsigned int ii=0;ii<4;ii++)
                    {
                        Point2f closePt = ClosestPointOnLineSegment(screenPts[ii],screenPts[(ii+1)%4],touchPt);
                        float dist2 = (closePt-touchPt).squaredNorm();
                        if (dist2 <= maxDist2 && (dist2 < closeDist2))
                        {
                            retId = sel.selectID;
                            closeDist2 = dist2;
                        }
                    }                    
                }
            }
        }
    }

    if (retId == EmptyIdentity && !polytopeSelectables.empty())
    {
        // We'll look for the closest object we can find
        float distToObj2 = MAXFLOAT;
        SimpleIdentity foundId = EmptyIdentity;
        Point3f eyePos;
        if (globeView)
            eyePos = Vector3dToVector3f(globeView.eyePos);
        else
            NSLog(@"Need to fill in eyePos for mapView");
        
        // Work through the axis aligned rectangular solids
        for (PolytopeSelectableSet::iterator it = polytopeSelectables.begin();
             it != polytopeSelectables.end(); ++it)
        {
            PolytopeSelectable sel = *it;
            if (sel.selectID != EmptyIdentity)
            {
                if (sel.minVis == DrawVisibleInvalid ||
                    (sel.minVis < [theView heightAboveSurface] && [theView heightAboveSurface] < sel.maxVis))
                {
                    // Project each plane to the screen, including clipping
                    for (unsigned int ii=0;ii<sel.polys.size();ii++)
                    {
                        std::vector<Point3f> &poly3f = sel.polys[ii];
                        std::vector<Point3d> poly;
                        poly.reserve(poly3f.size());
                        for (unsigned int jj=0;jj<poly3f.size();jj++)
                        {
                            Point3f &pt = poly3f[jj];
                            poly.push_back(Point3d(pt.x(),pt.y(),pt.z()));
                        }
                        
                        std::vector<Point2f> screenPts;
                        ClipAndProjectPolygon(modelTrans,projTrans,frameSize,poly,screenPts);
                        
                        if (screenPts.size() > 3)
                        {
                            if (PointInPolygon(touchPt, screenPts))
                            {
                                float dist2 = (sel.midPt - eyePos).squaredNorm();
                                if (dist2 < distToObj2)
                                {
                                    distToObj2 = dist2;
                                    foundId = sel.selectID;
                                }
                                break;
                            }
                            
                            for (unsigned int jj=0;jj<screenPts.size();jj++)
                            {
                                Point2f closePt = ClosestPointOnLineSegment(screenPts[jj],screenPts[(jj+1)%4],touchPt);
                                float dist2 = (closePt-touchPt).squaredNorm();
                                if (dist2 <= maxDist2)
                                {
                                    float objDist2 = (sel.midPt - eyePos).squaredNorm();
                                    if (objDist2 < distToObj2)
                                    {
                                        distToObj2 = objDist2;
                                        foundId = sel.selectID;
                                        break;
                                    }
                                }
                            }                            
                        }
                    }
                }
            }
        }
        
        retId = foundId;
    }
    

    if (retId == EmptyIdentity)
    {
        // Work through the 3D rectangles
        for (RectSelectable3DSet::iterator it = rect3Dselectables.begin();
             it != rect3Dselectables.end(); ++it)
        {
            RectSelectable3D sel = *it;
            if (sel.selectID != EmptyIdentity)
            {
                if (sel.minVis == DrawVisibleInvalid ||
                    (sel.minVis < [theView heightAboveSurface] && [theView heightAboveSurface] < sel.maxVis))
                {
                    std::vector<Point2f> screenPts;
                    
                    for (unsigned int ii=0;ii<4;ii++)
                    {
                        CGPoint screenPt;
                        Point3d pt3d(sel.pts[ii].x(),sel.pts[ii].y(),sel.pts[ii].z());
                        if (globeView)
                            screenPt = [globeView pointOnScreenFromSphere:pt3d transform:&modelTrans frameSize:frameSize];
                        else
                            screenPt = [mapView pointOnScreenFromPlane:pt3d transform:&modelTrans frameSize:frameSize];
                        screenPts.push_back(Point2f(screenPt.x,screenPt.y));
                    }
                    
                    // See if we fall within that polygon
                    if (PointInPolygon(touchPt, screenPts))
                    {
                        retId = sel.selectID;
                        break;
                    }
                    
                    // Now for a proximity check around the edges
                    for (unsigned int ii=0;ii<4;ii++)
                    {
                        Point2f closePt = ClosestPointOnLineSegment(screenPts[ii],screenPts[(ii+1)%4],touchPt);
                        float dist2 = (closePt-touchPt).squaredNorm();
                        if (dist2 <= maxDist2 && (dist2 < closeDist2))
                        {
                            retId = sel.selectID;
                            closeDist2 = dist2;
                        }
                    }
                }
            }
        }
    }
    
    return retId;
}


@end
