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
#import "MaplyView.h"
#import "WhirlyGeometry.h"
#import "Scene.h"
#import "SceneRendererES.h"
#import "ScreenSpaceBuilder.h"
#import "LayoutManager.h"

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

bool LinearSelectable::operator < (const LinearSelectable &that) const
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
void SelectionManager::addSelectableScreenRect(SimpleIdentity selectId,const Point3d &center,Point2f *pts,float minVis,float maxVis,bool enable)
{
    if (selectId == EmptyIdentity)
        return;
    
    RectSelectable2D newSelect;
    newSelect.center = center;
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

void SelectionManager::addSelectableRectSolid(SimpleIdentity selectId,const BBox &bbox,float minVis,float maxVis,bool enable)
{
    std::vector<Point3f> pts;
    pts.reserve(8);
    bbox.asPoints(pts);
    addSelectableRect(selectId,&pts[0],minVis,maxVis,enable);
}

void SelectionManager::addPolytope(SimpleIdentity selectId,const std::vector<std::vector<Point3d> > &surfaces,float minVis,float maxVis,bool enable)
{
    if (selectId == EmptyIdentity)
        return;
    
    PolytopeSelectable newSelect;
    newSelect.selectID = selectId;
    newSelect.minVis = minVis;
    newSelect.maxVis = maxVis;
    newSelect.midPt = Point3f(0,0,0);
    newSelect.enable = enable;
    int numPts = 0;
    for (unsigned int si=0;si<surfaces.size();si++)
    {
        const std::vector<Point3d> &surface = surfaces[si];
        std::vector<Point3f> surface3f;
        surface3f.reserve(surface.size());
        for (unsigned int pi=0;pi<surface.size();pi++)
        {
            const Point3d &pt = surface[pi];
            Point3f pt3f(pt.x(),pt.y(),pt.z());
            newSelect.midPt += pt3f;
            surface3f.push_back(pt3f);
        }
        numPts += surfaces.size();
        newSelect.polys.push_back(surface3f);
    }
    newSelect.midPt /= numPts;
    
    pthread_mutex_lock(&mutex);
    polytopeSelectables.insert(newSelect);
    pthread_mutex_unlock(&mutex);
}

void SelectionManager::addSelectableLinear(SimpleIdentity selectId,const std::vector<Point3f> &pts,float minVis,float maxVis,bool enable)
{
    if (selectId == EmptyIdentity)
        return;
    
    LinearSelectable newSelect;
    newSelect.selectID = selectId;
    newSelect.minVis = minVis;
    newSelect.maxVis = maxVis;
    newSelect.enable = enable;
    newSelect.pts.resize(pts.size());
    for (unsigned int ii=0;ii<pts.size();ii++)
    {
        const Point3f &pt = pts[ii];
        newSelect.pts[ii] = Point3d(pt.x(),pt.y(),pt.z());
    }

    pthread_mutex_lock(&mutex);
    linearSelectables.insert(newSelect);
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
    
    LinearSelectableSet::iterator it5 = linearSelectables.find(LinearSelectable(selectID));
    if (it5 != linearSelectables.end())
    {
        LinearSelectable sel = *it5;
        linearSelectables.erase(it5);
        sel.enable = enable;
        linearSelectables.insert(sel);
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
        
        LinearSelectableSet::iterator it5 = linearSelectables.find(LinearSelectable(selectID));
        if (it5 != linearSelectables.end())
        {
            LinearSelectable sel = *it5;
            linearSelectables.erase(it5);
            sel.enable = enable;
            linearSelectables.insert(sel);
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
    
    LinearSelectableSet::iterator it5 = linearSelectables.find(LinearSelectable(selectID));
    if (it5 != linearSelectables.end())
        linearSelectables.erase(it5);
    
    BillboardSelectableSet::iterator it4 = billboardSelectables.find(BillboardSelectable(selectID));
    if (it4 != billboardSelectables.end())
        billboardSelectables.erase(it4);

    pthread_mutex_unlock(&mutex);
}

void SelectionManager::removeSelectables(const SimpleIDSet &selectIDs)
{
    pthread_mutex_lock(&mutex);
    bool found = false;
    
    for (SimpleIDSet::iterator sit = selectIDs.begin(); sit != selectIDs.end(); ++sit)
    {
        SimpleIdentity selectID = *sit;
        RectSelectable3DSet::iterator it = rect3Dselectables.find(RectSelectable3D(selectID));
        
        if (it != rect3Dselectables.end())
        {
            found = true;
            rect3Dselectables.erase(it);
        }
        
        RectSelectable2DSet::iterator it2 = rect2Dselectables.find(RectSelectable2D(selectID));
        if (it2 != rect2Dselectables.end())
        {
            found = true;
            rect2Dselectables.erase(it2);
        }
        
        PolytopeSelectableSet::iterator it3 = polytopeSelectables.find(PolytopeSelectable(selectID));
        if (it3 != polytopeSelectables.end())
        {
            found = true;
            polytopeSelectables.erase(it3);
        }

        LinearSelectableSet::iterator it5 = linearSelectables.find(LinearSelectable(selectID));
        if (it5 != linearSelectables.end())
        {
            found = true;
            linearSelectables.erase(it5);
        }

        BillboardSelectableSet::iterator it4 = billboardSelectables.find(BillboardSelectable(selectID));
        if (it4 != billboardSelectables.end())
        {
            found = true;
            billboardSelectables.erase(it4);
        }
    }
    
    if (!found)
        NSLog(@"Tried to delete selectable that doesn't exist.");
    
    pthread_mutex_unlock(&mutex);
}

void SelectionManager::getScreenSpaceObjects(const PlacementInfo &pInfo,std::vector<ScreenSpaceObjectLocation> &screenPts)
{
    for (RectSelectable2DSet::iterator it = rect2Dselectables.begin();
         it != rect2Dselectables.end(); ++it)
    {
        const RectSelectable2D &sel = *it;
        if (sel.selectID != EmptyIdentity)
        {
            if (sel.minVis == DrawVisibleInvalid ||
                (sel.minVis < pInfo.heightAboveSurface && pInfo.heightAboveSurface < sel.maxVis))
            {
                ScreenSpaceObjectLocation objLoc;
                objLoc.shapeID = sel.selectID;
                objLoc.dispLoc = sel.center;
                objLoc.offset = Point2d(0,0);
                for (unsigned int ii=0;ii<4;ii++)
                {
                    Point2f pt = sel.pts[ii];
                    objLoc.pts.push_back(Point2d(pt.x(),pt.y()));
                    objLoc.mbr.addPoint(pt);
                }
                screenPts.push_back(objLoc);
            }
        }
    }
}

SelectionManager::PlacementInfo::PlacementInfo(WhirlyKitView *view,WhirlyKitSceneRendererES *renderer)
: globeView(NULL), mapView(NULL)
{
    float scale = [UIScreen mainScreen].scale;
    
    // Sort out what kind of view it is
    if ([view isKindOfClass:[WhirlyGlobeView class]])
        globeView = (WhirlyGlobeView *)view;
    else if ([view isKindOfClass:[MaplyView class]])
        mapView = (MaplyView *)view;
    heightAboveSurface = view.heightAboveSurface;
    
    // Calculate a slightly bigger framebuffer to grab nearby features
    frameSize = Point2f(renderer.framebufferWidth,renderer.framebufferHeight);
    frameSizeScale = Point2f(renderer.framebufferWidth/scale,renderer.framebufferHeight/scale);
    float marginX = frameSize.x() * 0.25;
    float marginY = frameSize.y() * 0.25;
    frameMbr.ll() = Point2f(0 - marginX,0 - marginY);
    frameMbr.ur() = Point2f(frameSize.x() + marginX,frameSize.y() + marginY);

    // Now for the various matrices
    viewMat = [view calcViewMatrix];
    modelMat = [view calcModelMatrix];
    modelInvMat = modelMat.inverse();
    viewAndModelMat = viewMat * modelMat;
    viewAndModelInvMat = viewAndModelMat.inverse();
    viewModelNormalMat = viewAndModelMat.inverse().transpose();
    projMat = [view calcProjectionMatrix:frameSizeScale margin:0.0];
    [view getOffsetMatrices:offsetMatrices frameBuffer:frameSize];
}

void SelectionManager::projectWorldPointToScreen(const Point3d &worldLoc,const PlacementInfo &pInfo,std::vector<Point2d> &screenPts,float scale)
{
    for (unsigned int offi=0;offi<pInfo.offsetMatrices.size();offi++)
    {
        // Project the world location to the screen
        CGPoint screenPt;
        const Eigen::Matrix4d &offMatrix = pInfo.offsetMatrices[offi];
        Eigen::Matrix4d modelAndViewMat = pInfo.viewMat * offMatrix * pInfo.modelMat;
        
        if (pInfo.globeView)
        {
            // Make sure this one is facing toward the viewer
            if (CheckPointAndNormFacing(worldLoc,worldLoc.normalized(),pInfo.viewAndModelMat,pInfo.viewModelNormalMat) < 0.0)
                return;
            
            // Note: Should just use
            screenPt = [pInfo.globeView pointOnScreenFromSphere:worldLoc transform:&modelAndViewMat frameSize:pInfo.frameSize];
        } else {
            if (pInfo.mapView)
                screenPt = [pInfo.mapView pointOnScreenFromPlane:worldLoc transform:&modelAndViewMat frameSize:pInfo.frameSize];
            else
                // No idea what this could be
                return;
        }

        // Isn't on the screen
        if (screenPt.x < pInfo.frameMbr.ll().x() || screenPt.y < pInfo.frameMbr.ll().y() ||
            screenPt.x > pInfo.frameMbr.ur().x() || screenPt.y > pInfo.frameMbr.ur().y())
            continue;
        
        screenPts.push_back(Point2d(screenPt.x/scale,screenPt.y/scale));
    }
}

// Sorter for selected objects
struct selectedsorter
{
    bool operator() (const SelectionManager::SelectedObject &a,const SelectionManager::SelectedObject &b) const
    {
        if (a.screenDist == b.screenDist)
            return a.distIn3D < b.distIn3D;
        return a.screenDist < b.screenDist;
    }
} SelectedSorter;

// Return a list of objects that pass the selection criteria
void SelectionManager::pickObjects(Point2f touchPt,float maxDist,WhirlyKitView *theView,std::vector<SelectedObject> &selObjs)
{
    pickObjects(touchPt, maxDist, theView, true, selObjs);

    std::sort(selObjs.begin(),selObjs.end(),SelectedSorter);
}

// Look for the single closest object
SimpleIdentity SelectionManager::pickObject(Point2f touchPt,float maxDist,WhirlyKitView *theView)
{
    std::vector<SelectedObject> selObjs;
    pickObjects(touchPt, maxDist, theView, false, selObjs);

    std::sort(selObjs.begin(),selObjs.end(),SelectedSorter);
    
    if (selObjs.empty())
        return EmptyIdentity;
    return selObjs[0].selectID;
}

/// Pass in the screen point where the user touched.  This returns the closest hit within the given distance
// Note: Should switch to a view state, rather than a view
void SelectionManager::pickObjects(Point2f touchPt,float maxDist,WhirlyKitView *theView,bool multi,std::vector<SelectedObject> &selObjs)
{
    if (!renderer)
        return;
    float maxDist2 = maxDist * maxDist;
    
    // All the various parameters we need to evalute... stuff
    PlacementInfo pInfo(theView,renderer);
    if (!pInfo.globeView && !pInfo.mapView)
        return;

    // And the eye vector for billboards
    Vector4d eyeVec4 = pInfo.viewAndModelInvMat * Vector4d(0,0,1,0);
    Vector3d eyeVec(eyeVec4.x(),eyeVec4.y(),eyeVec4.z());

    LayoutManager *layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);
    
    pthread_mutex_lock(&mutex);

    // Figure out where the screen space objects are, both layout manager
    //  controlled and other
    std::vector<ScreenSpaceObjectLocation> ssObjs;
    getScreenSpaceObjects(pInfo,ssObjs);
    if (layoutManager)
        layoutManager->getScreenSpaceObjects(pInfo,ssObjs);
    
    // Work through the 2D rectangles
    for (unsigned int ii=0;ii<ssObjs.size();ii++)
    {
        ScreenSpaceObjectLocation &screenObj = ssObjs[ii];
        
        std::vector<Point2d> projPts;
        projectWorldPointToScreen(screenObj.dispLoc, pInfo, projPts,scale);
        
        float closeDist2 = MAXFLOAT;
        // Work through the possible locations of the projected point
        for (unsigned int jj=0;jj<projPts.size();jj++)
        {
            Point2d projPt = projPts[jj];
            Mbr objMbr = screenObj.mbr;
            objMbr.ll() += Point2f(projPt.x(),projPt.y());
            objMbr.ur() += Point2f(projPt.x(),projPt.y());
            
            // Make sure it's on the screen at least
            if (!pInfo.frameMbr.overlaps(objMbr))
                continue;
            
            if (screenObj.shapeID != EmptyIdentity)
            {
                std::vector<Point2f> screenPts;
                for (unsigned int kk=0;kk<4;kk++)
                {
                    const Point2d &screenObjPt = screenObj.pts[kk];
                    Point2d theScreenPt = Point2d(screenObjPt.x(),-screenObjPt.y()) + projPt + screenObj.offset;
                    screenPts.push_back(Point2f(theScreenPt.x(),theScreenPt.y()));
                }

                // See if we fall within that polygon
                if (PointInPolygon(touchPt, screenPts))
                {
                    SelectedObject selObj(screenObj.shapeID,0.0,0.0);
                    selObjs.push_back(selObj);
                    break;
                }
                
                // Now for a proximity check around the edges
                for (unsigned int ii=0;ii<4;ii++)
                {
                    float t;
                    Point2f closePt = ClosestPointOnLineSegment(screenPts[ii],screenPts[(ii+1)%4],touchPt,t);
                    float dist2 = (closePt-touchPt).squaredNorm();
                    closeDist2 = std::min(dist2,closeDist2);
                }
            }
        }
        // Got close enough to this object to select it
        if (closeDist2 < maxDist2)
        {
            SelectedObject selObj(screenObj.shapeID,0.0,sqrtf(closeDist2));
            selObjs.push_back(selObj);
        }
        
        if (!multi && !selObjs.empty())
            return;
    }

    Point3d eyePos;
    if (pInfo.globeView)
        eyePos = pInfo.globeView.eyePos;
    else
        NSLog(@"Need to fill in eyePos for mapView");

    if (!polytopeSelectables.empty())
    {
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
                    float closeDist2 = MAXFLOAT;
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
                        ClipAndProjectPolygon(pInfo.viewAndModelMat,pInfo.projMat,pInfo.frameSizeScale,poly,screenPts);
                        
                        if (screenPts.size() > 3)
                        {
                            if (PointInPolygon(touchPt, screenPts))
                            {
                                closeDist2 = 0.0;
                                break;
                            }
                            
                            for (unsigned int jj=0;jj<screenPts.size();jj++)
                            {
                                float t;
                                Point2f closePt = ClosestPointOnLineSegment(screenPts[jj],screenPts[(jj+1)%4],touchPt,t);
                                float dist2 = (closePt-touchPt).squaredNorm();
                                closeDist2 = std::min(dist2,closeDist2);
                            }
                        }
                    }

                    if (closeDist2 < maxDist2)
                    {
                        float dist3d = (Point3d(sel.midPt.x(),sel.midPt.y(),sel.midPt.z()) - eyePos).norm();
                        SelectedObject selObj(sel.selectID,dist3d,sqrtf(closeDist2));
                        selObjs.push_back(selObj);
                    }
                }
            }
        }
    }
    
    if (!linearSelectables.empty())
    {
        for (LinearSelectableSet::iterator it = linearSelectables.begin();
             it != linearSelectables.end(); ++it)
        {
            LinearSelectable sel = *it;
            
            if (sel.selectID != EmptyIdentity && sel.enable)
            {
                if (sel.minVis == DrawVisibleInvalid ||
                    (sel.minVis < [theView heightAboveSurface] && [theView heightAboveSurface] < sel.maxVis))
                {
                    std::vector<Point2d> p0Pts;
                    projectWorldPointToScreen(sel.pts[0],pInfo,p0Pts,scale);
                    float closeDist2 = MAXFLOAT;
                    float closeDist3d = MAXFLOAT;
                    for (unsigned int ip=1;ip<sel.pts.size();ip++)
                    {
                        std::vector<Point2d> p1Pts;
                        projectWorldPointToScreen(sel.pts[ip],pInfo,p1Pts,scale);
                        
                        if (p0Pts.size() == p1Pts.size())
                        {
                            // Look for a nearby hit along the line
                            for (unsigned int iw=0;iw<p0Pts.size();iw++)
                            {
                                float t;
                                Point2f closePt = ClosestPointOnLineSegment(Point2f(p0Pts[iw].x(),p0Pts[iw].y()),Point2f(p1Pts[iw].x(),p1Pts[iw].y()),touchPt,t);
                                float dist2 = (closePt-touchPt).squaredNorm();
                                if (dist2 < closeDist2)
                                {
                                    // Calculate the point in 3D we almost hit
                                    const Point3d &p0 = sel.pts[ip-1], &p1 = sel.pts[ip];
                                    Point3d midPt = (p1-p0)*t + p0;
                                    closeDist3d = (midPt-eyePos).norm();
                                    closeDist2 = dist2;
                                }
                            }
                        }
                        
                        p0Pts = p1Pts;
                    }
                    if (closeDist2 < maxDist2)
                    {
                        SelectedObject selObj(sel.selectID,closeDist3d,sqrtf(closeDist2));
                        selObjs.push_back(selObj);
                    }
                }
            }
        }
    }
    
    if (!rect3Dselectables.empty())
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
                        if (pInfo.globeView)
                            screenPt = [pInfo.globeView pointOnScreenFromSphere:pt3d transform:&pInfo.viewAndModelMat frameSize:pInfo.frameSizeScale];
                        else
                            screenPt = [pInfo.mapView pointOnScreenFromPlane:pt3d transform:&pInfo.viewAndModelMat frameSize:pInfo.frameSizeScale];
                        screenPts.push_back(Point2f(screenPt.x,screenPt.y));
                    }
                    
                    float closeDist2 = MAXFLOAT;
                    float closeDist3d = MAXFLOAT;

                    // See if we fall within that polygon
                    if (PointInPolygon(touchPt, screenPts))
                    {
                        closeDist2 = 0.0;
                        // Note: Lame way to calculate distance
                        Point3d midPt(0,0,0);
                        for (unsigned int ii=0;ii<4;ii++)
                            midPt += Vector3fToVector3d(sel.pts[ii]);
                        midPt /= 4.0;
                        closeDist3d = (midPt - eyePos).norm();
                    } else {
                        // Now for a proximity check around the edges
                        for (unsigned int ii=0;ii<4;ii++)
                        {
                            float t;
                            Point2f closePt = ClosestPointOnLineSegment(screenPts[ii],screenPts[(ii+1)%4],touchPt,t);
                            float dist2 = (closePt-touchPt).squaredNorm();
                            const Point3d p0 = Vector3fToVector3d(sel.pts[ii]), p1 = Vector3fToVector3d(sel.pts[(ii+1)%4]);
                            Point3d midPt = (p1-p0)*t + p0;
                            if (dist2 <= maxDist2 && (dist2 < closeDist2))
                            {
                                closeDist2 = dist2;
                                closeDist3d = (midPt-eyePos).norm();
                            }
                        }
                    }
                    
                    if (closeDist2 < maxDist2)
                    {
                        SelectedObject selObj(sel.selectID,closeDist3d,sqrtf(closeDist2));
                        selObjs.push_back(selObj);
                    }
                }
            }
        }
    }
    
    if (!billboardSelectables.empty())
    {
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
                ClipAndProjectPolygon(pInfo.viewAndModelMat,pInfo.projMat,pInfo.frameSizeScale,poly,screenPts);
                
                float closeDist2 = MAXFLOAT;
                float closeDist3d = MAXFLOAT;

                if (screenPts.size() > 3)
                {
                    if (PointInPolygon(touchPt, screenPts))
                    {
                        closeDist3d = (Vector3fToVector3d(sel.center) - eyePos).norm();
                        break;
                    }
                    
                    for (unsigned int jj=0;jj<screenPts.size();jj++)
                    {
                        float t;
                        Point2f closePt = ClosestPointOnLineSegment(screenPts[jj],screenPts[(jj+1)%4],touchPt,t);
                        float dist2 = (closePt-touchPt).squaredNorm();
                        if (dist2 < maxDist2 && dist2 < closeDist2)
                        {
                            closeDist3d = (Vector3fToVector3d(sel.center) - eyePos).norm();
                            closeDist2 = dist2;
                        }
                    }
                }

                if (closeDist2 < maxDist2)
                {
                    SelectedObject selObj(sel.selectID,closeDist3d,sqrtf(closeDist2));
                    selObjs.push_back(selObj);
                }
            }
        }
    }
    
    pthread_mutex_unlock(&mutex);
}
