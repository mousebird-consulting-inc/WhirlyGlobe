#include <utility>

/*  SelectionManager.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/26/11.
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

#import "SelectionManager.h"
#import "GlobeMath.h"
#import "MaplyView.h"
#import "WhirlyGeometry.h"
#import "Scene.h"
#import "SceneRenderer.h"
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

Point3d MovingRectSelectable2D::centerForTime(TimeInterval now) const
{
    double t = (now-startTime)/(endTime-startTime);
    return (endCenter-center)*t + center;
}

bool PolytopeSelectable::operator < (const PolytopeSelectable &that) const
{
    return selectID < that.selectID;
}

bool MovingPolytopeSelectable::operator < (const MovingPolytopeSelectable &that) const
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

SelectionManager::SelectedObject::SelectedObject(double distIn3D,double screenDist) :
    SelectedObject(std::vector<SimpleIdentity>(), distIn3D, screenDist)
{
}

SelectionManager::SelectedObject::SelectedObject(SimpleIdentity selectID,double distIn3D,double screenDist) :
    SelectedObject(std::vector<SimpleIdentity> { selectID }, distIn3D, screenDist)
{
}

SelectionManager::SelectedObject::SelectedObject(std::vector<SimpleIdentity> selectIDs,
                                                 double distIn3D, double screenDist) :
    distIn3D(distIn3D),
    screenDist(screenDist),
    isCluster(false),
    center(0.0f, 0.0f),
    selectIDs(std::move(selectIDs))
{
}

SelectionManager::SelectedObject &
SelectionManager::SelectedObject::SelectedObject::operator=(SelectedObject&& other) noexcept {
    selectIDs    = std::move(other.selectIDs);
    vecObj       = std::move(other.vecObj);
    center       = other.center;
    distIn3D     = other.distIn3D;
    screenDist   = other.screenDist;
    isCluster    = other.isCluster;
    clusterGroup = other.clusterGroup;
    clusterId    = other.clusterId;
    return *this;
}

SelectionManager::SelectionManager(Scene *scene)
    : scene(scene)
{
}

// Add a rectangle (in 3-space) available for selection
void SelectionManager::addSelectableRect(SimpleIdentity selectId,const Point3f *pts,bool enable)
{
    if (selectId == EmptyIdentity || !pts)
        return;
    
    RectSelectable3D newSelect;
    newSelect.selectID = selectId;
    newSelect.minVis = newSelect.maxVis = DrawVisibleInvalid;
    newSelect.norm = (pts[1] - pts[0]).cross(pts[3]-pts[0]).normalized();
    newSelect.enable = enable;

    for (unsigned int ii = 0; ii < 4; ii++)
    {
        newSelect.pts[ii] = pts[ii];
    }

    std::lock_guard<std::mutex> guardLock(lock);
    rect3Dselectables.insert(std::move(newSelect));
}

// Add a rectangle (in 3-space) for selection, but only between the given visibilities
void SelectionManager::addSelectableRect(SimpleIdentity selectId,const Point3f *pts,
                                         float minVis,float maxVis,bool enable)
{
    if (selectId == EmptyIdentity || !pts)
        return;
    
    RectSelectable3D newSelect;
    newSelect.selectID = selectId;
    newSelect.minVis = minVis;  newSelect.maxVis = maxVis;
    newSelect.norm = (pts[1] - pts[0]).cross(pts[3]-pts[0]).normalized();
    newSelect.enable = enable;

    for (unsigned int ii = 0; ii < 4; ii++)
    {
        newSelect.pts[ii] = pts[ii];
    }

    std::lock_guard<std::mutex> guardLock(lock);
    rect3Dselectables.insert(std::move(newSelect));
}

/// Add a screen space rectangle (2D) for selection, between the given visibilities
void SelectionManager::addSelectableScreenRect(SimpleIdentity selectId,const Point3d &center,
                                               const Point2f *pts,float minVis,float maxVis,bool enable)
{
    if (selectId == EmptyIdentity)
        return;
    
    RectSelectable2D newSelect;
    newSelect.center = center;
    newSelect.selectID = selectId;
    newSelect.minVis = minVis;
    newSelect.maxVis = maxVis;
    newSelect.enable = enable;

    if (pts)
    {
        for (unsigned int ii = 0; ii < 4; ii++)
        {
            newSelect.pts[ii] = pts[ii];
        }
    }
    
    std::lock_guard<std::mutex> guardLock(lock);
    rect2Dselectables.insert(std::move(newSelect));
}

/// Add a screen space rectangle (2D) for selection, between the given visibilities
void SelectionManager::addSelectableMovingScreenRect(SimpleIdentity selectId,const Point3d &startCenter,
                                                     const Point3d &endCenter,TimeInterval startTime,
                                                     TimeInterval endTime,const Point2f *pts,float minVis,
                                                     float maxVis,bool enable)
{
    if (selectId == EmptyIdentity)
        return;
    
    MovingRectSelectable2D newSelect;
    newSelect.center = startCenter;
    newSelect.endCenter = endCenter;
    newSelect.startTime = startTime;
    newSelect.endTime = endTime;
    newSelect.selectID = selectId;
    newSelect.minVis = minVis;
    newSelect.maxVis = maxVis;
    newSelect.enable = enable;

    if (pts)
    {
        for (unsigned int ii = 0; ii < 4; ii++)
        {
            newSelect.pts[ii] = pts[ii];
        }
    }
    
    std::lock_guard<std::mutex> guardLock(lock);
    movingRect2Dselectables.insert(std::move(newSelect));
}

static const int corners[6][4] = {{0,1,2,3},{7,6,5,4},{1,0,4,5},{1,5,6,2},{2,6,7,3},{3,7,4,0}};

void SelectionManager::addSelectableRectSolid(SimpleIdentity selectId,const Point3f *pts,
                                              float minVis,float maxVis,bool enable)
{
    if (selectId == EmptyIdentity)
        return;
    
    PolytopeSelectable newSelect;
    newSelect.selectID = selectId;
    newSelect.minVis = minVis;
    newSelect.maxVis = maxVis;
    newSelect.centerPt = Point3d(0,0,0);
    newSelect.enable = enable;

    if (pts)
    {
        for (unsigned int ii = 0; ii < 8; ii++)
        {
            newSelect.centerPt += pts[ii].cast<double>();
        }
    }
    newSelect.centerPt /= 8;

    newSelect.polys.reserve(sizeof(corners)/sizeof(corners[0]));
    for (const auto &corner : corners)
    {
        newSelect.polys.emplace_back();
        Point3fVector &poly = newSelect.polys.back();
        poly.reserve(4);
        const Point3f center3f = newSelect.centerPt.cast<float>();
        for (int jj : corner)
        {
            poly.push_back(pts[jj] - center3f);
        }
    }
    
    {
        std::lock_guard<std::mutex> guardLock(lock);
        polytopeSelectables.insert(std::move(newSelect));
    }
}

void SelectionManager::addSelectableRectSolid(SimpleIdentity selectId,const Point3d *pts,
                                              float minVis,float maxVis,bool enable)
{
    if (selectId == EmptyIdentity)
        return;
    
    PolytopeSelectable newSelect;
    newSelect.selectID = selectId;
    newSelect.minVis = minVis;
    newSelect.maxVis = maxVis;
    newSelect.centerPt = Point3d(0,0,0);
    newSelect.enable = enable;

    if (pts)
    {
        for (unsigned int ii = 0; ii < 8; ii++)
        {
            newSelect.centerPt += pts[ii].cast<double>();
        }
    }
    newSelect.centerPt /= 8;

    newSelect.polys.reserve(sizeof(corners)/sizeof(corners[0]));
    for (const auto &corner : corners)
    {
        newSelect.polys.emplace_back();
        Point3fVector &poly = newSelect.polys.back();
        poly.reserve(4);
        for (int jj : corner)
        {
            poly.push_back((pts[jj] - newSelect.centerPt).cast<float>());
        }
    }
    
    std::lock_guard<std::mutex> guardLock(lock);
    polytopeSelectables.insert(std::move(newSelect));
}

void SelectionManager::addSelectableRectSolid(SimpleIdentity selectId,const BBox &bbox,
                                              float minVis,float maxVis,bool enable)
{
    Point3fVector pts;
    bbox.asPoints(pts);
    addSelectableRect(selectId,&pts[0],minVis,maxVis,enable);
}

void SelectionManager::addPolytope(SimpleIdentity selectId,
                                   const std::vector<Point3dVector> &surfaces,
                                   float minVis,float maxVis,bool enable)
{
    if (selectId == EmptyIdentity)
        return;
    
    PolytopeSelectable newSelect;
    newSelect.selectID = selectId;
    newSelect.minVis = minVis;
    newSelect.maxVis = maxVis;
    newSelect.centerPt = Point3d(0,0,0);
    newSelect.enable = enable;

    int numPts = 0;
    for (const Point3dVector &surface : surfaces)
    {
        for (const Point3d &pt : surface)
        {
            newSelect.centerPt += pt;
            numPts++;
        }
    }
    newSelect.centerPt /= numPts;

    newSelect.polys.reserve(surfaces.size());
    for (const Point3dVector &surface : surfaces)
    {
        newSelect.polys.emplace_back();
        Point3fVector &surface3f = newSelect.polys.back();
        surface3f.reserve(surface.size());
        for (const Point3d &pt : surface)
        {
            surface3f.push_back((pt - newSelect.centerPt).cast<float>());
        }
    }
    
    std::lock_guard<std::mutex> guardLock(lock);
    polytopeSelectables.insert(std::move(newSelect));
}

void SelectionManager::addPolytopeFromBox(SimpleIdentity selectId,const Point3d &ll,const Point3d &ur,
                                          const Eigen::Matrix4d &mat,float minVis,float maxVis,bool enable)
{
    // Corners of the box
    const Point3d pts[8] = {
        { ll.x(), ll.y(), ll.z() },
        { ur.x(), ll.y(), ll.z() },
        { ur.x(), ur.y(), ll.z() },
        { ll.x(), ur.y(), ll.z() },
        { ll.x(), ll.y(), ur.z() },
        { ur.x(), ll.y(), ur.z() },
        { ur.x(), ur.y(), ur.z() },
        { ll.x(), ur.y(), ur.z() },
    };
    
    // Turn the box into a polytope
    std::vector<Point3dVector> polys(6);
    auto &bot = polys[0];  bot.resize(4);
    auto &side0 = polys[1];  side0.resize(4);
    auto &side1 = polys[2];  side1.resize(4);
    auto &side2 = polys[3];  side2.resize(4);
    auto &side3 = polys[4];  side3.resize(4);
    auto &top = polys[5];  top.resize(4);
    bot[0] = pts[0];  bot[1] = pts[1];  bot[2] = pts[2];  bot[3] = pts[3];
    side0[0] = pts[0];  side0[1] = pts[1];  side0[2] = pts[5];  side0[3] = pts[4];
    side1[0] = pts[1];  side1[1] = pts[2];  side1[2] = pts[6];  side1[3] = pts[5];
    side2[0] = pts[2];  side2[1] = pts[3];  side2[2] = pts[7];  side2[3] = pts[6];
    side3[0] = pts[3];  side3[1] = pts[6];  side3[2] = pts[4];  side3[3] = pts[7];
    top[0] = pts[4];  top[1] = pts[5];  top[2] = pts[6];  top[3] = pts[7];
    
    // Run through the matrix
    for (auto &side : polys)
    {
        for (auto &pt : side)
        {
            const Vector4d newPt = mat * Vector4d(pt.x(),pt.y(),pt.z(),1.0);
            pt = Point3d(newPt.x(),newPt.y(),newPt.z());
        }
    }

    addPolytope(selectId, polys, minVis, maxVis, enable);
}

void SelectionManager::addMovingPolytope(SimpleIdentity selectId,const std::vector<Point3dVector> &surfaces,
                                         const Point3d &startCenter,const Point3d &endCenter,
                                         TimeInterval startTime, TimeInterval duration,
                                         const Eigen::Matrix4d &mat,float minVis,float maxVis,bool enable)
{
    if (selectId == EmptyIdentity)
        return;
    
    MovingPolytopeSelectable newSelect;
    newSelect.selectID = selectId;
    newSelect.minVis = minVis;
    newSelect.maxVis = maxVis;
    newSelect.centerPt = startCenter;
    newSelect.endCenterPt = endCenter;
    newSelect.startTime = startTime;
    newSelect.duration = duration;
    newSelect.enable = enable;

    newSelect.polys.reserve(surfaces.size());
    for (const Point3dVector &surface : surfaces)
    {
        newSelect.polys.emplace_back();
        Point3fVector &surface3f = newSelect.polys.back();
        surface3f.reserve(surface.size());
        for (const Point3d &pt : surface)
        {
            surface3f.push_back(pt.cast<float>());
        }
    }
    
    std::lock_guard<std::mutex> guardLock(lock);
    movingPolytopeSelectables.insert(std::move(newSelect));
}

void SelectionManager::addMovingPolytopeFromBox(SimpleIdentity selectID, const Point3d &ll, const Point3d &ur,
                                                const Point3d &startCenter, const Point3d &endCenter,
                                                TimeInterval startTime,TimeInterval duration,
                                                const Eigen::Matrix4d &mat, float minVis, float maxVis, bool enable)
{
    // Corners of the box
    const Point3d pts[8] = {
        { ll.x(), ll.y(), ll.z() },
        { ur.x(), ll.y(), ll.z() },
        { ur.x(), ur.y(), ll.z() },
        { ll.x(), ur.y(), ll.z() },
        { ll.x(), ll.y(), ur.z() },
        { ur.x(), ll.y(), ur.z() },
        { ur.x(), ur.y(), ur.z() },
        { ll.x(), ur.y(), ur.z() },
    };
    
    // Turn the box into a polytope
    std::vector<Point3dVector> polys(6);
    auto &bot = polys[0];  bot.resize(4);
    auto &side0 = polys[1];  side0.resize(4);
    auto &side1 = polys[2];  side1.resize(4);
    auto &side2 = polys[3];  side2.resize(4);
    auto &side3 = polys[4];  side3.resize(4);
    auto &top = polys[5];  top.resize(4);
    bot[0] = pts[0];  bot[1] = pts[1];  bot[2] = pts[2];  bot[3] = pts[3];
    side0[0] = pts[0];  side0[1] = pts[1];  side0[2] = pts[5];  side0[3] = pts[4];
    side1[0] = pts[1];  side1[1] = pts[2];  side1[2] = pts[6];  side1[3] = pts[5];
    side2[0] = pts[2];  side2[1] = pts[3];  side2[2] = pts[7];  side2[3] = pts[6];
    side3[0] = pts[3];  side3[1] = pts[6];  side3[2] = pts[4];  side3[3] = pts[7];
    top[0] = pts[4];  top[1] = pts[5];  top[2] = pts[6];  top[3] = pts[7];
    
    // Run through the matrix
    for (auto &side : polys)
    {
        for (auto &pt : side)
        {
            const Vector4d newPt = mat * Vector4d(pt.x(),pt.y(),pt.z(),1.0);
            pt = Point3d(newPt.x(),newPt.y(),newPt.z());
        }
    }

    addMovingPolytope(selectID, polys, startCenter, endCenter, startTime, duration, mat, minVis, maxVis, enable);
}

void SelectionManager::addSelectableLinear(SimpleIdentity selectId,const Point3dVector &pts,
                                           float minVis,float maxVis,bool enable)
{
    if (selectId == EmptyIdentity)
        return;
    
    LinearSelectable newSelect;
    newSelect.selectID = selectId;
    newSelect.minVis = minVis;
    newSelect.maxVis = maxVis;
    newSelect.enable = enable;
    newSelect.pts = pts;

    std::lock_guard<std::mutex> guardLock(lock);
    linearSelectables.insert(std::move(newSelect));
}

void SelectionManager::addSelectableBillboard(SimpleIdentity selectId,const Point3d &center,
                                              const Point3d &norm,const Point2d &size,
                                              float minVis,float maxVis,bool enable)
{
    if (selectId == EmptyIdentity)
        return;
    
    BillboardSelectable newSelect;
    newSelect.selectID = selectId;
    newSelect.size = size;
    newSelect.center = center;
    newSelect.normal = norm;
    
    newSelect.enable = enable;
    newSelect.minVis = minVis;
    newSelect.maxVis = maxVis;
    
    std::lock_guard<std::mutex> guardLock(lock);
    billboardSelectables.insert(std::move(newSelect));
}

void SelectionManager::enableSelectable(SimpleIdentity selectID,bool enable)
{
    std::lock_guard<std::mutex> guardLock(lock);

    const auto it = rect3Dselectables.find(RectSelectable3D(selectID));
    
    if (it != rect3Dselectables.end())
    {
        RectSelectable3D sel = *it;
        rect3Dselectables.erase(it);
        sel.enable = enable;
        rect3Dselectables.insert(std::move(sel));
    }

    const auto it2 = rect2Dselectables.find(RectSelectable2D(selectID));
    if (it2 != rect2Dselectables.end())
    {
        RectSelectable2D sel = *it2;
        rect2Dselectables.erase(it2);
        sel.enable = enable;
        rect2Dselectables.insert(std::move(sel));
    }

    const auto itM = movingRect2Dselectables.find(MovingRectSelectable2D(selectID));
    if (itM != movingRect2Dselectables.end())
    {
        MovingRectSelectable2D sel = *itM;
        movingRect2Dselectables.erase(itM);
        sel.enable = enable;
        movingRect2Dselectables.insert(std::move(sel));
    }

    const auto it3 = polytopeSelectables.find(PolytopeSelectable(selectID));
    if (it3 != polytopeSelectables.end())
    {
        PolytopeSelectable sel = *it3;
        polytopeSelectables.erase(it3);
        sel.enable = enable;
        polytopeSelectables.insert(std::move(sel));
    }

    const auto it3a = movingPolytopeSelectables.find(MovingPolytopeSelectable(selectID));
    if (it3a != movingPolytopeSelectables.end())
    {
        MovingPolytopeSelectable sel = *it3a;
        movingPolytopeSelectables.erase(it3a);
        sel.enable = enable;
        movingPolytopeSelectables.insert(std::move(sel));
    }

    const auto it5 = linearSelectables.find(LinearSelectable(selectID));
    if (it5 != linearSelectables.end())
    {
        LinearSelectable sel = *it5;
        linearSelectables.erase(it5);
        sel.enable = enable;
        linearSelectables.insert(std::move(sel));
    }

    const auto it4 = billboardSelectables.find(BillboardSelectable(selectID));
    if (it4 != billboardSelectables.end())
    {
        BillboardSelectable sel = *it4;
        billboardSelectables.erase(it4);
        sel.enable = enable;
        billboardSelectables.insert(std::move(sel));
    }
}

void SelectionManager::enableSelectables(const SimpleIDSet &selectIDs,bool enable)
{
    std::lock_guard<std::mutex> guardLock(lock);

    for (const SimpleIdentity selectID : selectIDs)
    {
        const auto it = rect3Dselectables.find(RectSelectable3D(selectID));
        
        if (it != rect3Dselectables.end())
        {
            RectSelectable3D sel = *it;
            rect3Dselectables.erase(it);
            sel.enable = enable;
            rect3Dselectables.insert(std::move(sel));
        }

        const auto it2 = rect2Dselectables.find(RectSelectable2D(selectID));
        if (it2 != rect2Dselectables.end())
        {
            RectSelectable2D sel = *it2;
            rect2Dselectables.erase(it2);
            sel.enable = enable;
            rect2Dselectables.insert(std::move(sel));
        }

        const auto itM = movingRect2Dselectables.find(MovingRectSelectable2D(selectID));
        if (itM != movingRect2Dselectables.end())
        {
            MovingRectSelectable2D sel = *itM;
            movingRect2Dselectables.erase(itM);
            sel.enable = enable;
            movingRect2Dselectables.insert(std::move(sel));
        }

        const auto it3 = polytopeSelectables.find(PolytopeSelectable(selectID));
        if (it3 != polytopeSelectables.end())
        {
            PolytopeSelectable sel = *it3;
            polytopeSelectables.erase(it3);
            sel.enable = enable;
            polytopeSelectables.insert(std::move(sel));
        }

        const auto it3a = movingPolytopeSelectables.find(MovingPolytopeSelectable(selectID));
        if (it3a != movingPolytopeSelectables.end())
        {
            MovingPolytopeSelectable sel = *it3a;
            movingPolytopeSelectables.erase(it3a);
            sel.enable = enable;
            movingPolytopeSelectables.insert(std::move(sel));
        }

        const auto it5 = linearSelectables.find(LinearSelectable(selectID));
        if (it5 != linearSelectables.end())
        {
            LinearSelectable sel = *it5;
            linearSelectables.erase(it5);
            sel.enable = enable;
            linearSelectables.insert(std::move(sel));
        }

        const auto it4 = billboardSelectables.find(BillboardSelectable(selectID));
        if (it4 != billboardSelectables.end())
        {
            BillboardSelectable sel = *it4;
            billboardSelectables.erase(it4);
            sel.enable = enable;
            billboardSelectables.insert(std::move(sel));
        }
    }
}

// Remove the given selectable from consideration
void SelectionManager::removeSelectable(SimpleIdentity selectID)
{
    std::lock_guard<std::mutex> guardLock(lock);

    const auto it = rect3Dselectables.find(RectSelectable3D(selectID));
    if (it != rect3Dselectables.end())
        rect3Dselectables.erase(it);

    const auto it2 = rect2Dselectables.find(RectSelectable2D(selectID));
    if (it2 != rect2Dselectables.end())
        rect2Dselectables.erase(it2);

    const auto itM = movingRect2Dselectables.find(MovingRectSelectable2D(selectID));
    if (itM != movingRect2Dselectables.end())
        movingRect2Dselectables.erase(itM);

    const auto it3 = polytopeSelectables.find(PolytopeSelectable(selectID));
    if (it3 != polytopeSelectables.end())
        polytopeSelectables.erase(it3);

    const auto it3a = movingPolytopeSelectables.find(MovingPolytopeSelectable(selectID));
    if (it3a != movingPolytopeSelectables.end())
        movingPolytopeSelectables.erase(it3a);

    const auto it5 = linearSelectables.find(LinearSelectable(selectID));
    if (it5 != linearSelectables.end())
        linearSelectables.erase(it5);

    const auto it4 = billboardSelectables.find(BillboardSelectable(selectID));
    if (it4 != billboardSelectables.end())
        billboardSelectables.erase(it4);
}

void SelectionManager::removeSelectables(const SimpleIDSet &selectIDs)
{
    std::lock_guard<std::mutex> guardLock(lock);
    //bool found = false;
    
    for (const SimpleIdentity selectID : selectIDs)
    {
        const auto it = rect3Dselectables.find(RectSelectable3D(selectID));
        if (it != rect3Dselectables.end())
        {
            //found = true;
            rect3Dselectables.erase(it);
        }

        const auto it2 = rect2Dselectables.find(RectSelectable2D(selectID));
        if (it2 != rect2Dselectables.end())
        {
            //found = true;
            rect2Dselectables.erase(it2);
        }

        const auto itM = movingRect2Dselectables.find(MovingRectSelectable2D(selectID));
        if (itM != movingRect2Dselectables.end())
        {
            //found = true;
            movingRect2Dselectables.erase(itM);
        }

        const auto it3 = polytopeSelectables.find(PolytopeSelectable(selectID));
        if (it3 != polytopeSelectables.end())
        {
            //found = true;
            polytopeSelectables.erase(it3);
        }

        const auto it3a = movingPolytopeSelectables.find(MovingPolytopeSelectable(selectID));
        if (it3a != movingPolytopeSelectables.end())
        {
            //found = true;
            movingPolytopeSelectables.erase(it3a);
        }

        const auto it5 = linearSelectables.find(LinearSelectable(selectID));
        if (it5 != linearSelectables.end())
        {
            //found = true;
            linearSelectables.erase(it5);
        }

        const auto it4 = billboardSelectables.find(BillboardSelectable(selectID));
        if (it4 != billboardSelectables.end())
        {
            //found = true;
            billboardSelectables.erase(it4);
        }
    }
    
//    if (!found)
//        NSLog(@"Tried to delete selectable that doesn't exist.");
}

void SelectionManager::getScreenSpaceObjects(const PlacementInfo &pInfo,std::vector<ScreenSpaceObjectLocation> &screenPts,TimeInterval now)
{
    screenPts.reserve(screenPts.size() + rect2Dselectables.size() + movingRect2Dselectables.size());
    for (const auto &sel : rect2Dselectables)
    {
        if (sel.selectID != EmptyIdentity && sel.enable)
        {
            if (sel.minVis == DrawVisibleInvalid ||
                (sel.minVis < pInfo.heightAboveSurface && pInfo.heightAboveSurface < sel.maxVis))
            {
                screenPts.emplace_back();
                ScreenSpaceObjectLocation &objLoc = screenPts.back();
                objLoc.shapeIDs.push_back(sel.selectID);
                objLoc.dispLoc = sel.center;
                objLoc.offset = Point2d(0,0);
                for (const auto &pt : sel.pts)
                {
                    objLoc.pts.emplace_back(pt.x(),pt.y());
                    objLoc.mbr.addPoint(pt);
                }
            }
        }
    }

    for (const auto & sel : movingRect2Dselectables)
    {
        if (sel.selectID != EmptyIdentity)
        {
            if (sel.minVis == DrawVisibleInvalid ||
                (sel.minVis < pInfo.heightAboveSurface && pInfo.heightAboveSurface < sel.maxVis))
            {
                screenPts.emplace_back();
                ScreenSpaceObjectLocation &objLoc = screenPts.back();
                objLoc.shapeIDs.push_back(sel.selectID);
                objLoc.dispLoc = sel.centerForTime(now);
                objLoc.offset = Point2d(0,0);
                for (const auto &pt : sel.pts)
                {
                    objLoc.pts.emplace_back(pt.x(),pt.y());
                    objLoc.mbr.addPoint(pt);
                }
            }
        }
    }
}

SelectionManager::PlacementInfo::PlacementInfo(ViewStateRef inViewState,SceneRenderer *renderer)
    : viewState(std::move(inViewState))
{
    const float scale = renderer->getScale();
    
    // Sort out what kind of view it is
    globeViewState = dynamic_cast<WhirlyGlobe::GlobeViewState *>(viewState.get());
    mapViewState = dynamic_cast<Maply::MapViewState *>(viewState.get());
    heightAboveSurface = globeViewState ? globeViewState->heightAboveGlobe :
                            mapViewState ? mapViewState->heightAboveSurface : 0;
    
    // Calculate a slightly bigger framebuffer to grab nearby features
    frameSize = Point2f(renderer->framebufferWidth,renderer->framebufferHeight);
    frameSizeScale = Point2f((float)renderer->framebufferWidth/scale,
                             (float)renderer->framebufferHeight/scale);
    const float marginX = frameSize.x() * 0.25f;
    const float marginY = frameSize.y() * 0.25f;
    frameMbr.ll() = Point2f(0 - marginX,0 - marginY);
    frameMbr.ur() = Point2f(frameSize.x() + marginX,frameSize.y() + marginY);
}

void SelectionManager::projectWorldPointToScreen(const Point3d &worldLoc,const PlacementInfo &pInfo,Point2dVector &screenPts,float scale)
{
    for (unsigned int offi=0;offi<pInfo.viewState->fullMatrices.size();offi++)
    {
        // Project the world location to the screen
        const Eigen::Matrix4d &modelAndViewMat = pInfo.viewState->fullMatrices[offi];
        const Eigen::Matrix4d &viewModelNormalMat = pInfo.viewState->fullNormalMatrices[offi];

        Point2f screenPt;
        if (pInfo.globeViewState)
        {
            // Make sure this one is facing toward the viewer
            if (CheckPointAndNormFacing(worldLoc,worldLoc.normalized(),modelAndViewMat,viewModelNormalMat) < 0.0)
                return;
            
            screenPt = pInfo.globeViewState->pointOnScreenFromDisplay(worldLoc, &modelAndViewMat, pInfo.frameSize);
        }
        else if (pInfo.mapViewState)
        {
            screenPt = pInfo.mapViewState->pointOnScreenFromDisplay(worldLoc, &modelAndViewMat, pInfo.frameSize);
        }
        else
        {
            // No idea what this could be
            return;
        }

        // Isn't on the screen
        if (screenPt.x() < pInfo.frameMbr.ll().x() || screenPt.y() < pInfo.frameMbr.ll().y() ||
            screenPt.x() > pInfo.frameMbr.ur().x() || screenPt.y() > pInfo.frameMbr.ur().y())
        {
            continue;
        }
        
        screenPts.emplace_back(screenPt.x()/scale,screenPt.y()/scale);
    }
}

// Sorter for selected objects
static const struct SelectedSorter_t
{
    bool operator() (const SelectionManager::SelectedObject &a,const SelectionManager::SelectedObject &b) const
    {
        if (a.screenDist == b.screenDist)
        {
            if (a.isCluster == b.isCluster)
                return a.distIn3D < b.distIn3D;
            return a.isCluster < b.isCluster;
        }
        return a.screenDist < b.screenDist;
    }
} selectedSorter;

// Return a list of objects that pass the selection criteria
void SelectionManager::pickObjects(const Point2f &touchPt,float maxDist,
                                   const ViewStateRef &theView,
                                   std::vector<SelectedObject> &selObjs)
{
    pickObjects(touchPt, maxDist, theView, true, selObjs);

    std::sort(selObjs.begin(),selObjs.end(),selectedSorter);
}

// Look for the single closest object
SimpleIdentity SelectionManager::pickObject(const Point2f &touchPt,float maxDist,const ViewStateRef &theView)
{
    std::vector<SelectedObject> selObjs;
    pickObjects(touchPt, maxDist, theView, false, selObjs);

    std::sort(selObjs.begin(),selObjs.end(),selectedSorter);
    
    return selObjs.empty() ? EmptyIdentity : selObjs[0].selectIDs[0];
}

Matrix2d SelectionManager::calcScreenRot(float &screenRot,const ViewStateRef &viewState,
                                         const WhirlyGlobe::GlobeViewState *globeViewState,
                                         const ScreenSpaceObjectLocation *ssObj,
                                         const Point2f &objPt,const Matrix4d &modelTrans,
                                         const Matrix4d &normalMat,const Point2f &frameBufferSize)
{
    // Switch from counter-clockwise to clockwise
    const double rot = 2*M_PI-ssObj->rotation;
    
    Point3d upVec,northVec,eastVec;
    if (!globeViewState)
    {
        upVec = Point3d(0,0,1);
        northVec = Point3d(0,1,0);
        eastVec = Point3d(1,0,0);
    } else {
        upVec = ssObj->dispLoc.normalized();
        // Vector pointing north
        northVec = Point3d(-ssObj->dispLoc.x(),-ssObj->dispLoc.y(),1.0-ssObj->dispLoc.z());
        eastVec = northVec.cross(upVec);
        northVec = upVec.cross(eastVec);
    }
    
    // This vector represents the rotation in world space
    const Point3d rotVec = eastVec * sin(rot) + northVec * cos(rot);
    
    // Project down into screen space
    const Vector4d projRot = normalMat * Vector4d(rotVec.x(),rotVec.y(),rotVec.z(),0.0);
    
    // Use the resulting x & y
    screenRot = (float)(std::atan2(projRot.y(),projRot.x())-M_PI_2);

    // Keep the labels upright
    if (ssObj->keepUpright && screenRot > M_PI/2 && screenRot < 3*M_PI/2)
    {
        screenRot = (float)(screenRot + M_PI);
    }

    return Matrix2d(Eigen::Rotation2Dd(screenRot));
}

/// Pass in the screen point where the user touched.  This returns the closest hit within the given distance
void SelectionManager::pickObjects(const Point2f &touchPt,float maxDist,const ViewStateRef &viewState,
                                   bool multi,std::vector<SelectedObject> &selObjs)
{
    if (!renderer)
        return;

    // All the various parameters we need to evaluate... stuff
    PlacementInfo pInfo(viewState,renderer);
    if (!pInfo.globeViewState && !pInfo.mapViewState)
        return;
    
    const TimeInterval now = scene->getCurrentTime();
    const double maxDist2 = maxDist * maxDist;

    // And the eye vector for billboards
    const Vector4d eyeVec4 = pInfo.viewState->fullMatrices[0].inverse() * Vector4d(0,0,1,0);
    const Vector3d eyeVec(eyeVec4.x(),eyeVec4.y(),eyeVec4.z());
    const Matrix4d modelTrans = pInfo.viewState->fullMatrices[0];
    const Matrix4d normalMat = pInfo.viewState->fullMatrices[0].inverse().transpose();

    const Point2f frameBufferSize(renderer->framebufferWidth, renderer->framebufferHeight);

    const auto layoutManager = scene->getManager<LayoutManager>(kWKLayoutManager);

    std::lock_guard<std::mutex> guardLock(lock);

    // Figure out where the screen space objects are, both layout manager
    //  controlled and other
    std::vector<ScreenSpaceObjectLocation> ssObjs;
    getScreenSpaceObjects(pInfo,ssObjs,now);
    if (layoutManager)
        layoutManager->getScreenSpaceObjects(pInfo,ssObjs);
    
    // Work through the 2D rectangles
    for (const auto &screenObj : ssObjs)
    {
        Point2dVector projPts;
        projectWorldPointToScreen(screenObj.dispLoc, pInfo, projPts, renderer->getScale());
        
        double closeDist2 = std::numeric_limits<double>::max();
        // Work through the possible locations of the projected point
        for (const auto &projPt : projPts)
        {
            Mbr objMbr = screenObj.mbr;
            objMbr.ll() += Point2f(projPt.x(),projPt.y());
            objMbr.ur() += Point2f(projPt.x(),projPt.y());
            
            // Make sure it's on the screen at least
            if (!pInfo.frameMbr.overlaps(objMbr))
            {
                continue;
            }
            
            if (!screenObj.shapeIDs.empty())
            {
                Matrix2d screenRotMat;
                float screenRot = 0.0;
                Point2f objPt = projPt.cast<float>();
                if (screenObj.rotation != 0.0)
                {
                    screenRotMat = calcScreenRot(screenRot,pInfo.viewState,pInfo.globeViewState,&screenObj,objPt,modelTrans,normalMat,frameBufferSize);
                }

                Point2fVector screenPts;
                if (screenRot == 0.0)
                {
                    for (unsigned int kk=0;kk<screenObj.pts.size();kk++)
                    {
                        const Point2d &screenObjPt = screenObj.pts[kk];
                        Point2d theScreenPt = Point2d(screenObjPt.x(),-screenObjPt.y()) + projPt + Point2d(screenObj.offset.x(),-screenObj.offset.y());
                        screenPts.emplace_back(theScreenPt.x(),theScreenPt.y());
                    }
                }
                else
                {
                    for (unsigned int kk=0;kk<screenObj.pts.size();kk++)
                    {
                        const Point2d screenObjPt = screenRotMat * (screenObj.pts[kk] + Point2d(screenObj.offset.x(),screenObj.offset.y()));
                        Point2d theScreenPt = Point2d(screenObjPt.x(),-screenObjPt.y()) + projPt;
                        screenPts.emplace_back(theScreenPt.x(),theScreenPt.y());
                    }
                }
                
                // See if we fall within that polygon
                if (PointInPolygon(touchPt, screenPts))
                {
                    // Distance is zero since we're inside, but maybe distance from the center would be more useful...
                    closeDist2 = 0.0;
                    break;
                }
                
                // Now for a proximity check around the edges
                for (unsigned int kk=0; kk < screenObj.pts.size(); kk++)
                {
                    float t;
                    const Point2f closePt = ClosestPointOnLineSegment(screenPts[kk], screenPts[(kk + 1) % 4], touchPt, t);
                    const double dist2 = (closePt-touchPt).squaredNorm();
                    closeDist2 = std::min(dist2,closeDist2);
                }
            }
        }
        // Got close enough to this object to select it
        if (closeDist2 < maxDist2)
        {
            const auto coordAdapter = scene->getCoordAdapter();
            const auto coordSys = coordAdapter->getCoordSystem();
            const auto center = coordSys->localToGeographic(coordAdapter->displayToLocal(screenObj.dispLoc));

            for (SimpleIdentity shapeID : screenObj.shapeIDs)
            {
                selObjs.emplace_back(shapeID,0.0,std::sqrt(closeDist2));
                auto &selObj = selObjs.back();
                selObj.isCluster = screenObj.isCluster();
                selObj.center = center;
                selObj.clusterId = screenObj.clusterId;
                selObj.clusterGroup = screenObj.clusterGroup;
            }
        }
        
        if (!multi && !selObjs.empty())
        {
            return;
        }
    }

    const Point3d eyePos = pInfo.globeViewState ? pInfo.globeViewState->eyePos : pInfo.mapViewState->eyePos;

    if (!polytopeSelectables.empty())
    {
        // Work through the axis aligned rectangular solids
        for (const auto &sel : polytopeSelectables)
        {
            if (sel.selectID != EmptyIdentity && sel.enable)
            {
                if (sel.minVis == DrawVisibleInvalid ||
                    (sel.minVis < pInfo.heightAboveSurface && pInfo.heightAboveSurface < sel.maxVis))
                {
                    float closeDist2 = MAXFLOAT;
                    // Project each plane to the screen, including clipping
                    for (unsigned int ii=0;ii<sel.polys.size();ii++)
                    {
                        const Point3fVector &poly3f = sel.polys[ii];
                        Point3dVector poly;
                        poly.reserve(poly3f.size());
                        for (const auto &pt : poly3f)
                        {
                            poly.push_back(pt.cast<double>() + sel.centerPt);
                        }
                        
                        Point2fVector screenPts;
                        ClipAndProjectPolygon(pInfo.viewState->fullMatrices[0],pInfo.viewState->projMatrix,pInfo.frameSizeScale,poly,screenPts);
                        
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
                        float dist3d = (sel.centerPt - eyePos).norm();
                        SelectedObject selObj(sel.selectID,dist3d,sqrtf(closeDist2));
                        selObjs.push_back(selObj);
                    }
                }
            }
        }
    }
    
    if (!movingPolytopeSelectables.empty())
    {
        // Work through the axis aligned rectangular solids
        for (const auto &sel : movingPolytopeSelectables)
        {
            if (sel.selectID != EmptyIdentity && sel.enable)
            {
                if (sel.minVis == DrawVisibleInvalid ||
                    (sel.minVis < pInfo.heightAboveSurface && pInfo.heightAboveSurface < sel.maxVis))
                {
                    // Current center
                    const double t = (now-sel.startTime)/sel.duration;
                    const Point3d centerPt = (sel.endCenterPt - sel.centerPt)*t + sel.centerPt;
                    
                    float closeDist2 = MAXFLOAT;
                    // Project each plane to the screen, including clipping
                    for (const auto &poly3f : sel.polys)
                    {
                        Point3dVector poly;
                        poly.reserve(poly3f.size());
                        for (const auto &pt : poly3f)
                        {
                            poly.push_back(pt.cast<double>() + centerPt);
                        }
                        
                        Point2fVector screenPts;
                        ClipAndProjectPolygon(pInfo.viewState->fullMatrices[0],pInfo.viewState->projMatrix,pInfo.frameSizeScale,poly,screenPts);
                        
                        if (screenPts.size() > 3)
                        {
                            if (PointInPolygon(touchPt, screenPts))
                            {
                                closeDist2 = 0.0;
                                break;
                            }
                            
                            for (unsigned int jj=0;jj<screenPts.size();jj++)
                            {
                                float ti;
                                Point2f closePt = ClosestPointOnLineSegment(screenPts[jj], screenPts[(jj+1)%4], touchPt, ti);
                                const float dist2 = (closePt-touchPt).squaredNorm();
                                closeDist2 = std::min(dist2,closeDist2);
                            }
                        }
                    }
                    
                    if (closeDist2 < maxDist2)
                    {
                        const double dist3d = (centerPt - eyePos).norm();
                        selObjs.emplace_back(sel.selectID,dist3d,std::sqrt(closeDist2));
                    }
                }
            }
        }
    }
    
    if (!linearSelectables.empty())
    {
        for (const auto &sel : linearSelectables)
        {
            if (sel.selectID != EmptyIdentity && sel.enable)
            {
                if (sel.minVis == DrawVisibleInvalid ||
                    (sel.minVis < pInfo.heightAboveSurface && pInfo.heightAboveSurface < sel.maxVis))
                {
                    Point2dVector p0Pts;
                    projectWorldPointToScreen(sel.pts[0],pInfo,p0Pts,renderer->getScale());
                    float closeDist2 = MAXFLOAT;
                    float closeDist3d = MAXFLOAT;
                    for (unsigned int ip=1;ip<sel.pts.size();ip++)
                    {
                        Point2dVector p1Pts;
                        projectWorldPointToScreen(sel.pts[ip],pInfo,p1Pts,renderer->getScale());
                        
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
        for (const auto &sel : rect3Dselectables)
        {
            if (sel.selectID != EmptyIdentity && sel.enable)
            {
                if (sel.minVis == DrawVisibleInvalid ||
                    (sel.minVis < pInfo.heightAboveSurface && pInfo.heightAboveSurface < sel.maxVis))
                {
                    Point2fVector screenPts;
                    
                    for (const auto &pt : sel.pts)
                    {
                        const Point3d pt3d = pt.cast<double>();
                        const Point2f screenPt = pInfo.globeViewState ?
                            pInfo.globeViewState->pointOnScreenFromDisplay(pt3d, &pInfo.viewState->fullMatrices[0], pInfo.frameSizeScale) :
                            pInfo.mapViewState->pointOnScreenFromDisplay(pt3d, &pInfo.viewState->fullMatrices[0], pInfo.frameSizeScale);
                        screenPts.push_back(screenPt);
                    }
                    
                    float closeDist2 = MAXFLOAT;
                    float closeDist3d = MAXFLOAT;

                    // See if we fall within that polygon
                    if (PointInPolygon(touchPt, screenPts))
                    {
                        closeDist2 = 0.0;
                        Point3d midPt(0,0,0);
                        for (const auto &pt : sel.pts)
                        {
                            midPt += Vector3fToVector3d(pt);
                        }
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
        for (const auto &billboardSelectable : billboardSelectables)
        {
            BillboardSelectable sel = billboardSelectable;
            if (sel.selectID != EmptyIdentity && sel.enable)
            {
                // Come up with a rectangle in display space
                Point3dVector poly(4);
                const Vector3d normal3d = sel.normal;
                const Point3d axisX = eyeVec.cross(normal3d);
                const Point3d center3d = sel.center;
//                Point3d axisZ = axisX.cross(Vector3fToVector3d(sel.normal));
                poly[0] = -sel.size.x()/2.0 * axisX + center3d;
                poly[3] = sel.size.x()/2.0 * axisX + center3d;
                poly[2] = -sel.size.x()/2.0 * axisX + sel.size.y() * normal3d + center3d;
                poly[1] = sel.size.x()/2.0 * axisX + sel.size.y() * normal3d + center3d;
                
                const BillboardSelectable &bbSel = billboardSelectable;

                Point2fVector screenPts;
                ClipAndProjectPolygon(pInfo.viewState->fullMatrices[0],pInfo.viewState->projMatrix,pInfo.frameSizeScale,poly,screenPts);

                double closeDist2 = std::numeric_limits<double>::max();
                double closeDist3d = closeDist2;

                if (screenPts.size() > 3)
                {
                    if (PointInPolygon(touchPt, screenPts))
                    {
                        //closeDist3d = (sel.center - eyePos).norm();
                        break;
                    }
                    
                    for (unsigned int jj=0;jj<screenPts.size();jj++)
                    {
                        float t;
                        const Point2f closePt = ClosestPointOnLineSegment(screenPts[jj],screenPts[(jj+1)%4],touchPt,t);
                        const double dist2 = (closePt-touchPt).squaredNorm();
                        if (dist2 < maxDist2 && dist2 < closeDist2)
                        {
                            closeDist3d = (bbSel.center - eyePos).norm();
                            closeDist2 = dist2;
                        }
                    }
                }

                if (closeDist2 < maxDist2)
                {
                    selObjs.emplace_back(bbSel.selectID, closeDist3d, std::sqrt(closeDist2));
                }
            }
        }
    }
    
//    NSLog(@"Found %d selected objects",selObjs.size());    
}
