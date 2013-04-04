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

using namespace WhirlyKit;

bool RectSelectable3D::operator < (const RectSelectable3D &that) const
{
    return selectID < that.selectID;
}

bool RectSelectable2D::operator < (const RectSelectable2D &that) const
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

// Remove the given selectable from consideration
- (void)removeSelectable:(SimpleIdentity)selectID
{
    RectSelectable3DSet::iterator it = rect3Dselectables.find(RectSelectable3D(selectID));
    
    if (it != rect3Dselectables.end())
    {
        rect3Dselectables.erase(it);
    }
    
    RectSelectable2DSet::iterator it2 = rect2Dselectables.find(RectSelectable2D(selectID));
    if (it2 != rect2Dselectables.end())
    {
        rect2Dselectables.erase(it2);
    }
}

/// Pass in the screen point where the user touched.  This returns the closest hit within the given distance
- (SimpleIdentity)pickObject:(Point2f)touchPt view:(UIView *)view maxDist:(float)maxDist
{
    // Can only run in the layer thread
    if ([NSThread currentThread] != layerThread)
        return EmptyIdentity;
    
    float maxDist2 = maxDist * maxDist;
    
    // Precalculate the model matrix for use below
    Eigen::Matrix4f modelTrans = [theView calcFullMatrix];
    
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
                        if (globeView)
                            screenPt = [globeView pointOnScreenFromSphere:sel.pts[ii] transform:&modelTrans frameSize:Point2f(renderer.framebufferWidth/view.contentScaleFactor,renderer.framebufferHeight/view.contentScaleFactor)];
                        else
                            screenPt = [mapView pointOnScreenFromPlane:sel.pts[ii] transform:&modelTrans frameSize:Point2f(renderer.framebufferWidth/view.contentScaleFactor,renderer.framebufferHeight/view.contentScaleFactor)];
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
