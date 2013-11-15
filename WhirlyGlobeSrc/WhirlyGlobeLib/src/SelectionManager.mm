/*
 *  SelectionManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/26/11.
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

#import "SelectionManager.h"
#import "NSDictionary+Stuff.h"
#import "UIColor+Stuff.h"
#import "GlobeMath.h"
#import "ScreenSpaceGenerator.h"
#import "MaplyView.h"
#import "WhirlyGeometry.h"
#import "Scene.h"
#import "SceneRendererES.h"

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

bool BillboardSelectable::operator < (const BillboardSelectable &that) const
{
    return selectID < that.selectID;
}

SelectionManager::SelectionManager(Scene *scene,float viewScale)
    : scene(scene), scale(viewScale)
{
    pthread_mutex_init(&mutex,NULL);
}

SelectionManager::~SelectionManager()
{
    pthread_mutex_destroy(&mutex);
}

// Add a rectangle (in 3-space) available for selection
void SelectionManager::addSelectableRect(SimpleIdentity selectId,Point3f *pts,bool enable)
{
    if (selectId == EmptyIdentity)
        return;
    
    RectSelectable3D newSelect;
    newSelect.selectID = selectId;
    newSelect.minVis = newSelect.maxVis = DrawVisibleInvalid;
    newSelect.norm = (pts[1] - pts[0]).cross(pts[3]-pts[0]).normalized();
    newSelect.enable = enable;
    for (unsigned int ii=0;ii<4;ii++)
        newSelect.pts[ii] = pts[ii];

    pthread_mutex_lock(&mutex);
    rect3Dselectables.insert(newSelect);
    pthread_mutex_unlock(&mutex);
}

// Add a rectangle (in 3-space) for selection, but only between the given visibilities
void SelectionManager::addSelectableRect(SimpleIdentity selectId,Point3f *pts,float minVis,float maxVis,bool enable)
{
    if (selectId == EmptyIdentity)
        return;
    
    RectSelectable3D newSelect;
    newSelect.selectID = selectId;
    newSelect.minVis = minVis;  newSelect.maxVis = maxVis;
    newSelect.norm = (pts[1] - pts[0]).cross(pts[3]-pts[0]).normalized();
    newSelect.enable = enable;
    for (unsigned int ii=0;ii<4;ii++)
        newSelect.pts[ii] = pts[ii];
    
    pthread_mutex_lock(&mutex);
    rect3Dselectables.insert(newSelect);
    pthread_mutex_unlock(&mutex);
}

/// Add a screen space rectangle (2D) for selection, between the given visibilities
void SelectionManager::addSelectableScreenRect(SimpleIdentity selectId,Point2f *pts,float minVis,float maxVis,bool enable)
{
    if (selectId == EmptyIdentity)
        return;
    
    RectSelectable2D newSelect;
    newSelect.selectID = selectId;
    newSelect.minVis = minVis;
    newSelect.maxVis = maxVis;
    newSelect.enable = enable;
    for (unsigned int ii=0;ii<4;ii++)
        newSelect.pts[ii] = pts[ii];
    
    pthread_mutex_lock(&mutex);
    rect2Dselectables.insert(newSelect);
    pthread_mutex_unlock(&mutex);
}

static const int corners[6][4] = {{0,1,2,3},{7,6,5,4},{1,0,4,5},{1,5,6,2},{2,6,7,3},{3,7,4,0}};

void SelectionManager::addSelectableRectSolid(SimpleIdentity selectId,Point3f *pts,float minVis,float maxVis,bool enable)
{
    if (selectId == EmptyIdentity)
        return;
    
    PolytopeSelectable newSelect;
    newSelect.selectID = selectId;
    newSelect.minVis = minVis;
    newSelect.maxVis = maxVis;
    newSelect.midPt = Point3f(0,0,0);
    newSelect.enable = enable;
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
    
    pthread_mutex_lock(&mutex);
    polytopeSelectables.insert(newSelect);
    pthread_mutex_unlock(&mutex);
}

void SelectionManager::addSelectableBillboard(SimpleIdentity selectId,Point3f center,Point3f norm,Point2f size,float minVis,float maxVis,bool enable)
{
    if (selectId == EmptyIdentity)
        return;
    
    BillboardSelectable newSelect;
    newSelect.selectID = selectId;
    newSelect.center = center;
    newSelect.normal = norm;
    newSelect.size = size;
    newSelect.enable = enable;
    newSelect.minVis = minVis;
    newSelect.maxVis = maxVis;
    
    pthread_mutex_lock(&mutex);
    billboardSelectables.insert(newSelect);
    pthread_mutex_unlock(&mutex);
}

void SelectionManager::enableSelectable(SimpleIdentity selectID,bool enable)
{
    pthread_mutex_lock(&mutex);
    
    RectSelectable3DSet::iterator it = rect3Dselectables.find(RectSelectable3D(selectID));
    
    if (it != rect3Dselectables.end())
    {
        RectSelectable3D sel = *it;
        rect3Dselectables.erase(it);
        sel.enable = enable;
        rect3Dselectables.insert(sel);
    }
    
    RectSelectable2DSet::iterator it2 = rect2Dselectables.find(RectSelectable2D(selectID));
    if (it2 != rect2Dselectables.end())
    {
        RectSelectable2D sel = *it2;
        rect2Dselectables.erase(it2);
        sel.enable = enable;
        rect2Dselectables.insert(sel);
    }
    
    PolytopeSelectableSet::iterator it3 = polytopeSelectables.find(PolytopeSelectable(selectID));
    if (it3 != polytopeSelectables.end())
    {
        PolytopeSelectable sel = *it3;
        polytopeSelectables.erase(it3);
        sel.enable = enable;
        polytopeSelectables.insert(sel);
    }
    
    BillboardSelectableSet::iterator it4 = billboardSelectables.find(BillboardSelectable(selectID));
    if (it4 != billboardSelectables.end())
    {
        BillboardSelectable sel = *it4;
        billboardSelectables.erase(it4);
        sel.enable = enable;
        billboardSelectables.insert(sel);
    }
    
    pthread_mutex_unlock(&mutex);
}

void SelectionManager::enableSelectables(const SimpleIDSet &selectIDs,bool enable)
{
    pthread_mutex_lock(&mutex);
    
    for (SimpleIDSet::iterator sit = selectIDs.begin(); sit != selectIDs.end(); ++sit)
    {
        SimpleIdentity selectID = *sit;
        RectSelectable3DSet::iterator it = rect3Dselectables.find(RectSelectable3D(selectID));
        
        if (it != rect3Dselectables.end())
        {
            RectSelectable3D sel = *it;
            rect3Dselectables.erase(it);
            sel.enable = enable;
            rect3Dselectables.insert(sel);
        }
        
        RectSelectable2DSet::iterator it2 = rect2Dselectables.find(RectSelectable2D(selectID));
        if (it2 != rect2Dselectables.end())
        {
            RectSelectable2D sel = *it2;
            rect2Dselectables.erase(it2);
            sel.enable = enable;
            rect2Dselectables.insert(sel);
        }
        
        PolytopeSelectableSet::iterator it3 = polytopeSelectables.find(PolytopeSelectable(selectID));
        if (it3 != polytopeSelectables.end())
        {
            PolytopeSelectable sel = *it3;
            polytopeSelectables.erase(it3);
            sel.enable = enable;
            polytopeSelectables.insert(sel);
        }
        
        BillboardSelectableSet::iterator it4 = billboardSelectables.find(BillboardSelectable(selectID));
        if (it4 != billboardSelectables.end())
        {
            BillboardSelectable sel = *it4;
            billboardSelectables.erase(it4);
            sel.enable = enable;
            billboardSelectables.insert(sel);
        }
    }
    
    pthread_mutex_unlock(&mutex);
}

// Remove the given selectable from consideration
void SelectionManager::removeSelectable(SimpleIdentity selectID)
{
    pthread_mutex_lock(&mutex);
    
    RectSelectable3DSet::iterator it = rect3Dselectables.find(RectSelectable3D(selectID));
    
    if (it != rect3Dselectables.end())
        rect3Dselectables.erase(it);
    
    RectSelectable2DSet::iterator it2 = rect2Dselectables.find(RectSelectable2D(selectID));
    if (it2 != rect2Dselectables.end())
        rect2Dselectables.erase(it2);
    
    PolytopeSelectableSet::iterator it3 = polytopeSelectables.find(PolytopeSelectable(selectID));
    if (it3 != polytopeSelectables.end())
        polytopeSelectables.erase(it3);
    
    BillboardSelectableSet::iterator it4 = billboardSelectables.find(BillboardSelectable(selectID));
    if (it4 != billboardSelectables.end())
        billboardSelectables.erase(it4);

    pthread_mutex_unlock(&mutex);
}

void SelectionManager::removeSelectables(const SimpleIDSet &selectIDs)
{
    pthread_mutex_lock(&mutex);
    
    for (SimpleIDSet::iterator sit = selectIDs.begin(); sit != selectIDs.end(); ++sit)
    {
        SimpleIdentity selectID = *sit;
        RectSelectable3DSet::iterator it = rect3Dselectables.find(RectSelectable3D(selectID));
        
        if (it != rect3Dselectables.end())
            rect3Dselectables.erase(it);
        
        RectSelectable2DSet::iterator it2 = rect2Dselectables.find(RectSelectable2D(selectID));
        if (it2 != rect2Dselectables.end())
            rect2Dselectables.erase(it2);
        
        PolytopeSelectableSet::iterator it3 = polytopeSelectables.find(PolytopeSelectable(selectID));
        if (it3 != polytopeSelectables.end())
            polytopeSelectables.erase(it3);

        BillboardSelectableSet::iterator it4 = billboardSelectables.find(BillboardSelectable(selectID));
        if (it4 != billboardSelectables.end())
            billboardSelectables.erase(it4);
    }
    
    pthread_mutex_unlock(&mutex);
}

/// Pass in the screen point where the user touched.  This returns the closest hit within the given distance
// Note: Should switch to a view state, rather than a view
SimpleIdentity SelectionManager::pickObject(Point2f touchPt,float maxDist,WhirlyKitView *theView)
{
    if (!renderer)
        return EmptyIdentity;
    
    float maxDist2 = maxDist * maxDist;
    
    // Precalculate the model matrix for use below
    Eigen::Matrix4d modelTrans = [theView calcFullMatrix];
    Point2f frameSize(renderer.framebufferWidth/scale,renderer.framebufferHeight/scale);
    Eigen::Matrix4d projTrans = [theView calcProjectionMatrix:frameSize margin:0.0];
    Eigen::Matrix4d modelTransInv = modelTrans.inverse();

    // And the eye vector for billboards
    Vector4d eyeVec4 = modelTransInv * Vector4d(0,0,1,0);
    Vector3d eyeVec(eyeVec4.x(),eyeVec4.y(),eyeVec4.z());

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
    
    pthread_mutex_lock(&mutex);
    
    // Work through the 2D rectangles
    for (unsigned int ii=0;ii<projPts.size();ii++)
    {
        ScreenSpaceGenerator::ProjectedPoint projPt = projPts[ii];
        // If we're on a retina display, we need to scale accordingly
        projPt.screenLoc.x() /= scale;
        projPt.screenLoc.y() /= scale;
        
        // Look for the corresponding selectable
        RectSelectable2DSet::iterator it = rect2Dselectables.find(RectSelectable2D(projPt.shapeID));
        if (it != rect2Dselectables.end() && it->enable)
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
            if (sel.selectID != EmptyIdentity && sel.enable)
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
    
    if (retId == EmptyIdentity && !rect3Dselectables.empty())
    {
        // Work through the 3D rectangles
        for (RectSelectable3DSet::iterator it = rect3Dselectables.begin();
             it != rect3Dselectables.end(); ++it)
        {
            RectSelectable3D sel = *it;
            if (sel.selectID != EmptyIdentity && sel.enable)
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
    
    if (retId == EmptyIdentity && !billboardSelectables.empty())
    {
        // We'll look for the closest object we can find
        float distToObj2 = MAXFLOAT;
        SimpleIdentity foundId = EmptyIdentity;
        Point3f eyePos;
        if (globeView)
            eyePos = Vector3dToVector3f(globeView.eyePos);
        else
            NSLog(@"Need to fill in eyePos for mapView");

        // Work through the billboards
        for (BillboardSelectableSet::iterator it = billboardSelectables.begin();
             it != billboardSelectables.end(); ++it)
        {
            BillboardSelectable sel = *it;
            if (sel.selectID != EmptyIdentity && sel.enable)
            {
                
                // Come up with a rectangle in display space
                std::vector<Point3d> poly(4);
                Vector3d normal3d = Vector3fToVector3d(sel.normal);
                Point3d axisX = eyeVec.cross(normal3d);
                Point3d center3d = Vector3fToVector3d(sel.center);
//                Point3d axisZ = axisX.cross(Vector3fToVector3d(sel.normal));
                poly[0] = -sel.size.x()/2.0 * axisX + center3d;
                poly[3] = sel.size.x()/2.0 * axisX + center3d;
                poly[2] = -sel.size.x()/2.0 * axisX + sel.size.y() * normal3d + center3d;
                poly[1] = sel.size.x()/2.0 * axisX + sel.size.y() * normal3d + center3d;
                
                BillboardSelectable sel = *it;

                std::vector<Point2f> screenPts;
                ClipAndProjectPolygon(modelTrans,projTrans,frameSize,poly,screenPts);
                
                if (screenPts.size() > 3)
                {
                    if (PointInPolygon(touchPt, screenPts))
                    {
                        float dist2 = (sel.center - eyePos).squaredNorm();
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
                            float objDist2 = (sel.center - eyePos).squaredNorm();
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

        retId = foundId;
    }
    
    pthread_mutex_unlock(&mutex);
    
    return retId;
}
