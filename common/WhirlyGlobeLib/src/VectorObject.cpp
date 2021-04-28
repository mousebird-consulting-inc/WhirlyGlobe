/*  VectorObject.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/17/11.
 *  Copyright 2011-2021 mousebird consulting
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

#import "VectorObject.h"
#import "GlobeMath.h"
#import "VectorData.h"
#import "ShapeReader.h"
#import "Tesselator.h"
#import "GridClipper.h"
#import "WhirlyKitLog.h"
#import "GlobeView.h"
#import "MaplyView.h"
#import "GeographicLib/Geodesic.hpp"

namespace WhirlyKit
{
    
VectorObject::VectorObject()
    : VectorObject(10)
{
}

VectorObject::VectorObject(SimpleIdentity theId)
    : VectorObject(theId, 10)
{
}

VectorObject::VectorObject(SimpleIdentity theId, int capacity)
    : Identifiable(theId)
    , shapes(capacity)
    , selectable(true)
{
}

bool VectorObject::fromGeoJSON(const std::string &json,std::string &crs)
{
    return VectorParseGeoJSON(shapes,json,crs);
}
    
bool VectorObject::FromGeoJSONAssembly(const std::string &json,std::map<std::string,VectorObject *> &vecData)
{
    // TODO: unordered_map?
    std::map<std::string, ShapeSet> newShapes;

    if (!VectorParseGeoJSONAssembly(json, newShapes))
        return false;
    
    for (auto const &it : newShapes)
    {
        VectorObject *vecObj = new VectorObject();
        vecObj->shapes.reserve(vecObj->shapes.size() + it.second.size());
        vecObj->shapes.insert(it.second.begin(),it.second.end());
        vecData[it.first] = vecObj;
    }
    
    return true;
}
    
bool VectorObject::fromShapeFile(const std::string &fileName)
{
    ShapeReader shapeReader(fileName);
    if (!shapeReader.isValid())
        return false;
    
    const int numObj = shapeReader.getNumObjects();
    shapes.reserve(shapes.size() + numObj);
    for (unsigned int ii=0;ii<numObj;ii++) {
        shapes.insert(shapeReader.getObjectByIndex(ii, NULL));
    }
    
    return true;
}

MutableDictionaryRef VectorObject::getAttributes() const
{
    return shapes.empty() ? MutableDictionaryRef() : (*shapes.begin())->getAttrDict();
}

void VectorObject::setAttributes(MutableDictionaryRef newDict)
{
    for (const auto &shape : shapes)
        shape->setAttrDict(newDict);
}

void VectorObject::mergeVectorsFrom(const VectorObject &otherVec)
{
    shapes.reserve(shapes.size() + otherVec.shapes.size());
    shapes.insert(otherVec.shapes.begin(),otherVec.shapes.end());
}

void VectorObject::splitVectors(std::vector<VectorObject *> &vecs)
{
    vecs.reserve(vecs.size() + shapes.size());
    for (const auto &shape : shapes)
    {
        VectorObject *vecObj = new VectorObject();
        vecObj->shapes.insert(shape);
        vecs.push_back(vecObj);
    }
}

bool VectorObject::center(Point2d &center) const
{
    Mbr mbr;
    for (const auto &shape : shapes)
    {
        GeoMbr geoMbr = shape->calcGeoMbr();
        mbr.addPoint(geoMbr.ll());
        mbr.addPoint(geoMbr.ur());
    }

    center.x() = (mbr.ll().x() + mbr.ur().x())/2.0;
    center.y() = (mbr.ll().y() + mbr.ur().y())/2.0;

    return true;
}

bool VectorObject::centroid(Point2d &centroid) const
{
    // Find the loop with the largest area
    float bigArea = 0.0;
    const VectorRing *bigLoop = nullptr;
    for (const auto &shapeRef : shapes)
    {
        const auto shape = shapeRef.get();
        if (const auto areal = dynamic_cast<const VectorAreal*>(shape)) {
            if (areal->loops.size() > 0) {
                for (unsigned int ii=0;ii<areal->loops.size();ii++)
                {
                    const float area = CalcLoopArea(areal->loops[ii]);
                    if (std::abs(area) > std::abs(bigArea))
                    {
                        bigLoop = &areal->loops[ii];
                        bigArea = area;
                    }
                }
            }
        } else if (const auto linear = dynamic_cast<const VectorLinear*>(shape)) {
            const GeoCoord midCoord = linear->geoMbr.mid();
            centroid.x() = midCoord.x();
            centroid.y() = midCoord.y();
            return true;
        } else if (const auto linear3d = dynamic_cast<const VectorLinear3d*>(shape)) {
            const GeoCoord midCoord = linear3d->geoMbr.mid();
            centroid.x() = midCoord.x();
            centroid.y() = midCoord.y();
            return true;
        } else if (const auto pts = dynamic_cast<const VectorPoints*>(shape)) {
            const GeoCoord midCoord = pts->geoMbr.mid();
            centroid.x() = midCoord.x();
            centroid.y() = midCoord.y();
            return true;
        }
    }

    if (bigLoop && bigArea != 0)
    {
        const Point2f centroid2f = CalcLoopCentroid(*bigLoop, bigArea);
        centroid.x() = centroid2f.x();
        centroid.y() = centroid2f.y();
        return true;
    }
    return false;
}

bool VectorObject::largestLoopCenter(Point2d &center,Point2d &ll,Point2d &ur) const
{
    // Find the loop with the largest area
    float bigArea = -1.0;
    const VectorRing *bigLoop = nullptr;
    for (const auto &shape : shapes)
    {
        const auto areal = dynamic_cast<VectorAreal*>(shape.get());
        if (areal && areal->loops.size() > 0)
        {
            for (unsigned int ii=0;ii<areal->loops.size();ii++)
            {
                const float area = std::abs(CalcLoopArea(areal->loops[ii]));
                if (area > bigArea)
                {
                    bigLoop = &areal->loops[ii];
                    bigArea = area;
                }
            }
        }
    }
    
    if (bigArea < 0.0)
        return false;
    
    Point2d ctr;
    ctr.x() = 0;
    ctr.y() = 0;
    
    if (bigLoop)
    {
        Mbr mbr;
        mbr.addPoints(*bigLoop);
        ctr.x() = (mbr.ll().x() + mbr.ur().x())/2.0;
        ctr.y() = (mbr.ll().y() + mbr.ur().y())/2.0;
        center = ctr;
        ll.x() = mbr.ll().x();  ll.y() = mbr.ll().y();
        ur.x() = mbr.ur().x();  ur.y() = mbr.ur().y();
    }

    return true;
}

bool VectorObject::linearMiddle(Point2d &middle,double &rot) const
{
    if (shapes.empty())
        return false;
    
    const auto lin = dynamic_cast<VectorLinear*>(shapes.begin()->get());
    const auto lin3d = dynamic_cast<VectorLinear3d*>(shapes.begin()->get());
    if (!lin && !lin3d)
        return false;
    
    if (lin)
    {
        VectorRing pts = lin->pts;
        float totLen = 0;
        for (int ii=0;ii<pts.size()-1;ii++)
        {
            const float len = (pts[ii+1]-pts[ii]).norm();
            totLen += len;
        }
        const float halfLen = totLen / 2.0;
        
        // Now we'll walk along, looking for the middle
        float lenSoFar = 0.0;
        for (unsigned int ii=0;ii<pts.size()-1;ii++)
        {
            const Point2f &pt0 = pts[ii];
            const Point2f &pt1 = pts[ii+1];
            const float len = (pt1-pt0).norm();
            if (len > 0.0 && halfLen <= lenSoFar+len)
            {
                const float t = (halfLen-lenSoFar)/len;
                const Point2f thePt = (pt1-pt0)*t + pt0;
                middle.x() = thePt.x();
                middle.y() = thePt.y();
                rot = M_PI/2.0-atan2(pt1.y()-pt0.y(),pt1.x()-pt0.x());
                return true;
            }
            
            lenSoFar += len;
        }
        
        middle.x() = pts.back().x();
        middle.y() = pts.back().y();
        rot = 0.0;
    } else {
        const VectorRing3d &pts = lin3d->pts;
        float totLen = 0;
        for (int ii=0;ii<pts.size()-1;ii++)
        {
            const float len = (pts[ii+1]-pts[ii]).norm();
            totLen += len;
        }
        const float halfLen = totLen / 2.0;
        
        // Now we'll walk along, looking for the middle
        float lenSoFar = 0.0;
        for (unsigned int ii=0;ii<pts.size()-1;ii++)
        {
            const Point3d &pt0 = pts[ii];
            const Point3d &pt1 = pts[ii+1];
            const float len = (pt1-pt0).norm();
            if (len > 0.0 && halfLen <= lenSoFar+len)
            {
                const float t = (halfLen-lenSoFar)/len;
                const Point3d thePt = (pt1-pt0)*t + pt0;
                middle.x() = thePt.x();
                middle.y() = thePt.y();
                rot = M_PI/2.0-atan2(pt1.y()-pt0.y(),pt1.x()-pt0.x());
                return true;
            }
            
            lenSoFar += len;
        }
        
        middle.x() = pts.back().x();
        middle.y() = pts.back().y();
        rot = 0.0;
    }
    
    return true;
}
    
bool VectorObject::linearMiddle(Point2d &middle,double &rot,CoordSystem *coordSys) const
{
    if (shapes.empty())
        return false;

    const auto lin = dynamic_cast<VectorLinear*>(shapes.begin()->get());
    const auto lin3d = dynamic_cast<VectorLinear3d*>(shapes.begin()->get());
    if (!lin && !lin3d)
        return false;
    
    if (lin)
    {
        const VectorRing &pts = lin->pts;
        
        if (pts.empty())
            return false;
        
        if (pts.size() == 1)
        {
            middle.x() = pts[0].x();
            middle.y() = pts[0].y();
            rot = 0.0;
            return true;
        }
        
        float totLen = 0;
        for (int ii=0;ii<pts.size()-1;ii++)
        {
            const float len = (pts[ii+1]-pts[ii]).norm();
            totLen += len;
        }
        const float halfLen = totLen / 2.0;
        
        // Now we'll walk along, looking for the middle
        float lenSoFar = 0.0;
        for (unsigned int ii=0;ii<pts.size()-1;ii++)
        {
            const Point3d pt0 = coordSys->geographicToLocal3d(GeoCoord(pts[ii].x(),pts[ii].y()));
            const Point3d pt1 = coordSys->geographicToLocal3d(GeoCoord(pts[ii+1].x(),pts[ii+1].y()));
            const double len = (pt1-pt0).norm();
            if (halfLen <= lenSoFar+len)
            {
                const double t = (halfLen-lenSoFar)/len;
                const Point3d thePt = (pt1-pt0)*t + pt0;
                const GeoCoord middleGeo = coordSys->localToGeographic(thePt);
                middle.x() = middleGeo.x();
                middle.y() = middleGeo.y();
                rot = M_PI/2.0-atan2(pt1.y()-pt0.y(),pt1.x()-pt0.x());
                return true;
            }
            
            lenSoFar += len;
        }
        
        middle.x() = pts.back().x();
        middle.y() = pts.back().y();
        rot = 0.0;
    } else {
        const VectorRing3d &pts = lin3d->pts;
        
        if (pts.empty())
            return false;
        
        if (pts.size() == 1)
        {
            middle.x() = pts[0].x();
            middle.y() = pts[0].y();
            rot = 0.0;
            return true;
        }
        
        float totLen = 0;
        for (int ii=0;ii<pts.size()-1;ii++)
        {
            const float len = (pts[ii+1]-pts[ii]).norm();
            totLen += len;
        }
        const float halfLen = totLen / 2.0;
        
        // Now we'll walk along, looking for the middle
        float lenSoFar = 0.0;
        for (unsigned int ii=0;ii<pts.size()-1;ii++)
        {
            const GeoCoord geo0(pts[ii].x(),pts[ii].y());
            const GeoCoord geo1(pts[ii+1].x(),pts[ii+1].y());
            const Point3d pt0 = coordSys->geographicToLocal3d(geo0);
            const Point3d pt1 = coordSys->geographicToLocal3d(geo1);
            const double len = (pt1-pt0).norm();
            if (halfLen <= lenSoFar+len)
            {
                const double t = (halfLen-lenSoFar)/len;
                const Point3d thePt = (pt1-pt0)*t + pt0;
                const GeoCoord middleGeo = coordSys->localToGeographic(thePt);
                middle.x() = middleGeo.x();
                middle.y() = middleGeo.y();
                rot = M_PI/2.0-atan2(pt1.y()-pt0.y(),pt1.x()-pt0.x());
                return true;
            }
            
            lenSoFar += len;
        }
        
        middle.x() = pts.back().x();
        middle.y() = pts.back().y();
        rot = 0.0;
    }
    
    return true;
}
    
bool VectorObject::middleCoordinate(Point2d &middle) const
{
    if (shapes.empty())
        return false;
    
    const auto lin = dynamic_cast<VectorLinear*>(shapes.begin()->get());
    const auto lin3d = dynamic_cast<VectorLinear3d*>(shapes.begin()->get());
    if (!lin && !lin3d)
        return false;
    
    if (lin)
    {
        const auto index = lin->pts.size() / 2;
        middle.x() = lin->pts[index].x();
        middle.y() = lin->pts[index].y();
    } else {
        const auto index = lin3d->pts.size() / 2;
        middle.x() = lin3d->pts[index].x();
        middle.y() = lin3d->pts[index].y();
    }
    
    return true;
}
    
bool VectorObject::pointInside(const Point2d &pt) const
{
    for (const auto &shape : shapes)
    {
        if (const auto areal = dynamic_cast<VectorAreal*>(shape.get()))
        {
            if (areal->pointInside(GeoCoord(pt.x(),pt.y())))
                return true;
        } else if (const auto tris = dynamic_cast<VectorTriangles*>(shape.get())) {
            if (tris->pointInside(GeoCoord(pt.x(),pt.y())))
                return true;
        }
    }

    return false;
}
    
// Helper routine to convert and check geographic points (globe version)
static bool ScreenPointFromGeo(const Point2d &geoCoord,
                               WhirlyGlobe::GlobeViewStateRef globeView,
                               Maply::MapViewStateRef mapView,
                               CoordSystemDisplayAdapter *coordAdapter,
                               const Point2f &frameSize,
                               const Eigen::Matrix4f &modelAndViewMat,
                               const Eigen::Matrix4d &modelAndViewMat4d,
                               const Eigen::Matrix4d &modelMatFull,
                               const Eigen::Matrix4f &modelAndViewNormalMat,
                               Point2d *screenPt)
{
    const Point3d pt = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(geoCoord.x(),geoCoord.y())));
    const Point3f pt3f(pt.x(),pt.y(),pt.z());
    
    Point2f screenPt2f;
    if (globeView) {
        if (CheckPointAndNormFacing(pt3f,pt3f.normalized(),modelAndViewMat,modelAndViewNormalMat) < 0.0)
            return false;
        
        screenPt2f = globeView->pointOnScreenFromDisplay(pt, &modelAndViewMat4d, frameSize);
    } else {
        screenPt2f = mapView->pointOnScreenFromDisplay(pt, &modelAndViewMat4d, frameSize);
    }
    screenPt->x() = screenPt2f.x();  screenPt->y() = screenPt2f.y();

    return !(screenPt->x() < 0 || screenPt->y() < 0 || screenPt->x() > frameSize.x() || screenPt->y() > frameSize.y());
}

// Return the square of the hypotenuse, i.e., hypot() without the sqrt()
inline static double hypotSq(double dx, double dy)
{
    return dx * dx + dy * dy;
}

bool VectorObject::pointNearLinear(const Point2d &coord,float maxDistance,ViewStateRef viewState,const Point2f &frameSize) const
{
    CoordSystemDisplayAdapter *coordAdapter = viewState->coordAdapter;
   
    WhirlyGlobe::GlobeViewStateRef globeView = std::dynamic_pointer_cast<WhirlyGlobe::GlobeViewState>(viewState);
    Maply::MapViewStateRef mapView = std::dynamic_pointer_cast<Maply::MapViewState>(viewState);
    
    Eigen::Matrix4d modelTrans4d = viewState->modelMatrix;
    // Note: This won't work if there's more than one matrix
    Eigen::Matrix4d viewTrans4d = viewState->viewMatrices[0];
    Eigen::Matrix4d modelAndViewMat4d = viewTrans4d * modelTrans4d;
    Eigen::Matrix4f modelAndViewMat = Matrix4dToMatrix4f(modelAndViewMat4d);
    Eigen::Matrix4f modelAndViewNormalMat = modelAndViewMat.inverse().transpose();
    // Note: This is probably redundant
    Eigen::Matrix4d modelMatFull = viewState->fullMatrices[0];

    // Point we're searching around
    Point2d p;
    if (!ScreenPointFromGeo(coord, globeView, mapView, coordAdapter, frameSize, modelAndViewMat, modelAndViewMat4d, modelMatFull, modelAndViewNormalMat, &p))
        return false;

    const double maxDistSq = (double)maxDistance * maxDistance;
    for (const auto &shape : shapes)
    {
        if (const auto linear = dynamic_cast<VectorLinear*>(shape.get()))
        {
            const GeoMbr geoMbr = linear->calcGeoMbr();
            if(geoMbr.inside(GeoCoord(coord.x(),coord.y())))
            {
                const VectorRing &pts = linear->pts;
                for (int ii=0;ii<pts.size()-1;ii++)
                {
                    const Point2f &p0 = pts[ii];
                    Point2d pc(p0.x(),p0.y());
                    Point2d a;
                    if (!ScreenPointFromGeo(pc, globeView, mapView, coordAdapter, frameSize, modelAndViewMat, modelAndViewMat4d, modelMatFull, modelAndViewNormalMat, &a))
                        continue;
                    
                    const Point2f &p1 = pts[ii + 1];
                    pc = Point2d(p1.x(),p1.y());
                    Point2d b;
                    if (!ScreenPointFromGeo(pc, globeView, mapView, coordAdapter, frameSize, modelAndViewMat, modelAndViewMat4d, modelMatFull, modelAndViewNormalMat, &b))
                        continue;
                    
                    const Point2d aToP = a - p;
                    const Point2d aToB = a - b;
                    const double aToBMagitude = hypotSq(aToB.x(), aToB.y());
                    const double dot = aToP.x() * aToB.x() + aToP.y() * aToB.y();
                    const double d = dot/aToBMagitude;

                    double distance = std::numeric_limits<double>::max();
                    if(d < 0)
                    {
                        distance = hypotSq(p.x() - a.x(), p.y() - a.y());
                    } else if(d > 1) {
                        distance = hypotSq(p.x() - b.x(), p.y() - b.y());
                    } else {
                        distance = hypotSq(p.x() - a.x() + (aToB.x() * d),
                                         p.y() - a.y() + (aToB.y() * d));
                    }
                    
                    if (distance < maxDistSq)
                        return true;
                }
            }
        } else if (const auto linear3d = dynamic_cast<VectorLinear3d*>(shape.get())) {
            const GeoMbr geoMbr = linear3d->calcGeoMbr();
            if(geoMbr.inside(GeoCoord(coord.x(),coord.y())))
            {
                const VectorRing3d &pts = linear3d->pts;
                for (int ii=0;ii<pts.size()-1;ii++)
                {
                    Point3d p0 = pts[ii];
                    Point2d pc(p0.x(),p0.y());
                    Point2d a;
                    if (!ScreenPointFromGeo(pc, globeView, mapView, coordAdapter, frameSize, modelAndViewMat, modelAndViewMat4d, modelMatFull, modelAndViewNormalMat, &a))
                        continue;

                    Point3d p1 = pts[ii + 1];
                    pc = Point2d(p1.x(),p1.y());
                    Point2d b;
                    if (!ScreenPointFromGeo(pc, globeView, mapView, coordAdapter, frameSize, modelAndViewMat, modelAndViewMat4d, modelMatFull, modelAndViewNormalMat, &b))
                        continue;

                    const Point2d aToP = a - p;
                    const Point2d aToB = a - b;
                    const double aToBMagitude = hypotSq(aToB.x(), aToB.y());
                    const double dot = aToP.x() * aToB.x() + aToP.y() * aToB.y();
                    const double d = dot/aToBMagitude;

                    double distance = std::numeric_limits<double>::max();
                    if(d < 0)
                    {
                        distance = hypotSq(p.x() - a.x(), p.y() - a.y());
                    } else if(d > 1) {
                        distance = hypotSq(p.x() - b.x(), p.y() - b.y());
                    } else {
                        distance = hypotSq(p.x() - a.x() + (aToB.x() * d),
                                         p.y() - a.y() + (aToB.y() * d));
                    }
                    
                    if (distance < maxDistSq)
                        return true;
                }
            }
        }
    }
    
    return false;
}
    
double VectorObject::areaOfOuterLoops() const
{
    double area = 0.0;
    for (const auto& shape : shapes)
    {
        const auto areal = dynamic_cast<VectorAreal*>(shape.get());
        if (areal && areal->loops.size() > 0)
        {
            area = CalcLoopArea(areal->loops[0]);
        }
    }
    
    return area;
}
    
bool VectorObject::boundingBox(Point2d &ll,Point2d &ur) const
{
    bool valid = false;
    Mbr mbr;
    for (const auto &shape : shapes)
    {
        const GeoMbr geoMbr = shape->calcGeoMbr();
        mbr.addPoint(geoMbr.ll());
        mbr.addPoint(geoMbr.ur());
        valid = true;
    }
    
    if (valid)
    {
        ll.x() = mbr.ll().x();
        ll.y() = mbr.ll().y();
        ur.x() = mbr.ur().x();
        ur.y() = mbr.ur().y();
    }
    
    return valid;
}

void VectorObject::addHole(const VectorRing &hole)
{
    const auto areal = dynamic_cast<VectorAreal*>(shapes.begin()->get());
    if (areal)
    {
        areal->loops.push_back(hole);
    }
}
    
VectorObjectRef VectorObject::deepCopy() const
{
    auto newVecObj = std::make_shared<VectorObject>();
    newVecObj->shapes.reserve(shapes.size());

    for (const auto &shapeRef : shapes)
    {
        const auto shape = shapeRef.get();
        if (const auto points = dynamic_cast<VectorPoints*>(shape))
        {
            const auto newPts = VectorPoints::createPoints();
            newPts->pts = points->pts;
            newPts->setAttrDict(points->getAttrDict()->copy());
            newPts->initGeoMbr();
            newVecObj->shapes.insert(newPts);
        } else if (const auto lin = dynamic_cast<VectorLinear*>(shape)) {
            const auto newLin = VectorLinear::createLinear();
            newLin->pts = lin->pts;
            newLin->setAttrDict(lin->getAttrDict()->copy());
            newLin->initGeoMbr();
            newVecObj->shapes.insert(newLin);
        } else if (const auto lin3d = dynamic_cast<VectorLinear3d*>(shape)) {
            const auto newLin3d = VectorLinear3d::createLinear();
            newLin3d->pts = lin3d->pts;
            newLin3d->setAttrDict(lin3d->getAttrDict()->copy());
            newLin3d->initGeoMbr();
            newVecObj->shapes.insert(newLin3d);
        } else if (const auto ar = dynamic_cast<VectorAreal*>(shape)) {
            const auto newAr = VectorAreal::createAreal();
            newAr->loops = ar->loops;
            newAr->setAttrDict(ar->getAttrDict()->copy());
            newAr->initGeoMbr();
            newVecObj->shapes.insert(newAr);
        } else if (const auto tri = dynamic_cast<VectorTriangles*>(shape)) {
            const auto newTri = VectorTriangles::createTriangles();
            newTri->geoMbr = tri->geoMbr;
            newTri->pts = tri->pts;
            newTri->tris = tri->tris;
            newTri->setAttrDict(tri->getAttrDict()->copy());
            newTri->initGeoMbr();
            newVecObj->shapes.insert(newTri);
        }
    }
    
    return newVecObj;
}

VectorObjectType VectorObject::getVectorType() const
{
    if (shapes.empty())
        return VectorMultiType;

    VectorObjectType type = VectorNoneType;
    for (const auto &shapeRef : shapes)
    {
        VectorObjectType thisType = VectorNoneType;
        const auto shape = shapeRef.get();
        if (const auto points = dynamic_cast<VectorPoints*>(shape))
            thisType = VectorPointType;
        else {
            if (const auto lin = dynamic_cast<VectorLinear*>(shape))
                thisType = VectorLinearType;
            else {
                if (const auto lin3d = dynamic_cast<VectorLinear3d*>(shape))
                    thisType = VectorLinear3dType;
                else if (const auto ar = dynamic_cast<VectorAreal*>(shape))
                    thisType = VectorArealType;
            }
        }

        if (type == VectorNoneType)
            type = thisType;
        else if (type != thisType)
            return VectorMultiType;
    }

    return type;
}

bool VectorObject::isSelectable() const
{
    return selectable;
}

void VectorObject::setIsSelectable(bool newSelect)
{
    selectable = newSelect;
}
    
// Really Android?  Really?
template <typename T>
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

std::string VectorObject::log() const
{
    std::string outStr;
    
    for (const auto &shapeRef : shapes)
    {
        const auto shape = shapeRef.get();
        if (const auto points = dynamic_cast<VectorPoints*>(shape))
        {
            outStr += "Points: ";
            for (unsigned int ii=0;ii<points->pts.size();ii++)
            {
                const Point2f &pt = points->pts[ii];
                outStr += " (" + to_string(pt.x()) + "," + to_string(pt.y()) + ")";
            }
            outStr += "\n";
        } else  if (const auto lin = dynamic_cast<VectorLinear*>(shape)) {
            outStr += "Linear: ";
            for (unsigned int ii=0;ii<lin->pts.size();ii++)
            {
                const Point2f &pt = lin->pts[ii];
                outStr += " (" + to_string(pt.x()) + "," + to_string(pt.y()) + ")";
            }
            outStr += "\n";
        } else if (const auto lin3d = dynamic_cast<VectorLinear3d*>(shape)) {
            outStr += "Linear3d: ";
            for (unsigned int ii=0;ii<lin3d->pts.size();ii++)
            {
                const Point3d &pt = lin3d->pts[ii];
                outStr += " (" + to_string(pt.x()) + "," + to_string(pt.y()) + "," + to_string(pt.z()) + ")";
            }
            outStr += "\n";
        } else if (const auto ar = dynamic_cast<VectorAreal*>(shape)) {
            outStr += "Areal:\n";
            for (unsigned int li=0;li<ar->loops.size();li++)
            {
                const VectorRing &ring = ar->loops[li];
                outStr += " loop (" + to_string(li) + "): ";
                for (unsigned int ii=0;ii<ring.size();ii++)
                {
                    const Point2f &pt = ring[ii];
                    outStr += " (" + to_string(pt.x()) + "," + to_string(pt.y()) + ")";
                }
                outStr += "\n";
            }
            outStr += "\n";
        }
    }
    
    return outStr;
}
    
void VectorObject::reproject(CoordSystem *inSystem,double scale,CoordSystem *outSystem)
{
    for (const auto &shapeRef : shapes)
    {
        const auto shape = shapeRef.get();
        if (const auto points = dynamic_cast<VectorPoints*>(shape))
        {
            for (Point2f &pt : points->pts)
            {
                const Point3f outPt = CoordSystemConvert(inSystem, outSystem, Point3f(pt.x()*scale,pt.y()*scale,0.0));
                pt.x() = outPt.x();  pt.y() = outPt.y();
            }
            points->calcGeoMbr();
        } else if (const auto lin = dynamic_cast<VectorLinear*>(shape)) {
            for (Point2f &pt : lin->pts)
            {
                const Point3f outPt = CoordSystemConvert(inSystem, outSystem, Point3f(pt.x()*scale,pt.y()*scale,0.0));
                pt.x() = outPt.x();  pt.y() = outPt.y();
            }
            lin->calcGeoMbr();
        } else if (const auto lin3d = dynamic_cast<VectorLinear3d*>(shape)) {
            for (Point3d &pt : lin3d->pts)
            {
                const Point3d outPt = CoordSystemConvert3d(inSystem, outSystem, pt * scale);
                pt = outPt;
            }
            lin3d->calcGeoMbr();
        } else if (const auto ar = dynamic_cast<VectorAreal*>(shape)) {
            for (auto &loop : ar->loops)
                for (Point2f &pt : loop) {
                    const Point3f outPt = CoordSystemConvert(inSystem, outSystem, Point3f(pt.x()*scale,pt.y()*scale,0.0));
                    pt.x() = outPt.x() * 180 / M_PI;  pt.y() = outPt.y() * 180 / M_PI;
                }
            ar->calcGeoMbr();
        } else if (const auto tri = dynamic_cast<VectorTriangles*>(shape)) {
            for (Point3f &pt : tri->pts)
            {
                pt = CoordSystemConvert(inSystem, outSystem, Point3f(pt.x()*scale,pt.y()*scale,pt.z()));
            }
            tri->calcGeoMbr();
        }
    }
}
    
void VectorObject::subdivideToGlobe(float epsilon)
{
    FakeGeocentricDisplayAdapter adapter;
    
    VectorRing outPts;
    VectorRing3d outPts3;
    for (const auto &shapeRef : shapes)
    {
        const auto shape = shapeRef.get();
        if (const auto lin = dynamic_cast<VectorLinear*>(shape))
        {
            outPts.clear();
            SubdivideEdgesToSurface(lin->pts, outPts, false, &adapter, epsilon);
            lin->pts = outPts;
        }
        else if (const auto lin3d = dynamic_cast<VectorLinear3d*>(shape))
        {
            outPts3.clear();
            SubdivideEdgesToSurface(lin3d->pts, outPts3, false, &adapter, epsilon);
            lin3d->pts = outPts3;
        }
        else if (const auto ar = dynamic_cast<VectorAreal*>(shape))
        {
            for (unsigned int ii=0;ii<ar->loops.size();ii++)
            {
                outPts.clear();
                SubdivideEdgesToSurface(ar->loops[ii], outPts, true, &adapter, epsilon);
                ar->loops[ii] = outPts;
            }
        }
    }
}

// SubdivideEdgesToSurfaceGC and convert back to geographic
static void SubdivideEdgesToSurfaceGCGeo(const VectorRing &inPts,VectorRing &outPts2D,bool closed,
                                         CoordSystemDisplayAdapter *adapter,CoordSystem* coordSys,
                                         float eps,float surfOffset=0,int minPts=0)
{
    Point3dVector outPts;
    outPts.reserve(std::max(20, 3 * minPts));
    SubdivideEdgesToSurfaceGC(inPts,outPts,closed,adapter,eps,surfOffset,minPts);

    outPts2D.resize(outPts.size());
    for (unsigned int ii=0;ii<outPts.size();ii++)
    {
        outPts2D[ii] = coordSys->localToGeographic(adapter->displayToLocal(outPts[ii]));
    }
    if (!inPts.empty())
    {
        outPts2D.front() = inPts.front();
        outPts2D.back() = inPts.back();
    }
}

#if 0
static void SubdivideEdgesToSurfaceGCGeo(const VectorRing3d &inPts,VectorRing &outPts2D,bool closed,
                                         CoordSystemDisplayAdapter *adapter,CoordSystem* coordSys,
                                         float eps,float surfOffset=0,int minPts=0)
{
    VectorRing inPts2D;
    inPts2D.reserve(inPts.size());
    for (const auto &p : inPts)
    {
        inPts2D.emplace_back(p.x(),p.y());
    }
    SubdivideEdgesToSurfaceGCGeo(inPts2D,outPts2D,closed,adapter,coordSys,eps,surfOffset,minPts);
}
#endif

static void SubdivideEdgesToSurfaceGCGeo(const VectorRing3d &inPts,Point3dVector &outPts,bool closed,
                                         CoordSystemDisplayAdapter *adapter,CoordSystem* coordSys,
                                         float eps,float surfOffset=0,int minPts=0)
{
    VectorRing inPts2D;
    inPts2D.reserve(inPts.size());
    for (const auto &p : inPts)
    {
        inPts2D.emplace_back(p.x(),p.y());
    }
    VectorRing outPts2d;
    outPts2d.resize(inPts.size() * 3);
    SubdivideEdgesToSurfaceGCGeo(inPts2D,outPts2d,closed,adapter,coordSys,eps,surfOffset,minPts);
    outPts.reserve(outPts.size() + outPts2d.size());
    for (const auto &p : outPts2d)
    {
        outPts.emplace_back(p.x(), p.y(), 0.0);
    }
}

static void fixEdges(const VectorRing& outPts2D, VectorRing& offsetPts2D)
{
    // See if they cross the edge of a wraparound coordinate system
    // Note: Only works for spherical mercator, most likely
    offsetPts2D.resize(outPts2D.size());
    double xOff = 0.0;
    for (unsigned int ii=0;ii<outPts2D.size()-1;ii++)
    {
        offsetPts2D[ii] = Point2f(outPts2D[ii].x()+xOff,outPts2D[ii].y());
        if (std::abs(outPts2D[ii].x() - outPts2D[ii+1].x()) > 1.1*M_PI)
        {
            if (outPts2D[ii].x() < 0.0)
                xOff -= 2*M_PI;
            else
                xOff += 2*M_PI;
        }
    }
    offsetPts2D.back() = outPts2D.back() + Point2f(xOff,0.0);
}

static const GeographicLib::Geodesic &wgs84Geodesic = GeographicLib::Geodesic::WGS84();

static std::pair<double,double> CalcInv(const Point2f &p1, const Point2f &p2)
{
    const auto lat1 = WhirlyKit::RadToDeg(p1.y());
    const auto lon1 = WhirlyKit::RadToDeg(p1.x());
    const auto lat2 = WhirlyKit::RadToDeg(p2.y());
    const auto lon2 = WhirlyKit::RadToDeg(p2.x());
    double dist = 0.0, az1 = 0.0, az2 = 0.0;
    wgs84Geodesic.Inverse(lat1, lon1, lat2, lon2, dist, az1, az2);
    return std::make_pair(dist,WhirlyKit::DegToRad(az1));
}

static std::pair<bool,Point2f> CalcDirect(const Point2f &origin, double az, double dist)
{
    const auto lat1 = WhirlyKit::RadToDeg(origin.y());
    const auto lon1 = WhirlyKit::RadToDeg(origin.x());
    const auto azDeg = WhirlyKit::RadToDeg(az);

    double lat2 = 0.0, lon2 = 0.0;
    const auto res = wgs84Geodesic.Direct(lat1, lon1, azDeg, dist, lat2, lon2);

    return std::make_pair(std::isfinite(res),
                          Point2f((float)WhirlyKit::DegToRad(lon2),
                                  (float)WhirlyKit::DegToRad(lat2)));
}

template <typename Tin,typename Tout>
static void SubdivideGeoLib(Tin beg, Tin end, Tout out, double maxDistMeters)
{
    const auto count = std::distance(beg, end);
    if (count < 2)
    {
        if (beg != end)
        {
            // special case for one point
            *out++ = *beg;
        }
        return;
    }
    while (beg != end)
    {
        const auto next = std::next(beg);
        if (next == end)
        {
            *out++ = *beg;
            break;
        }
        const auto &p0 = *beg;
        const auto &p1 = *next;
        ++beg;
        *out++ = p0;

        double dist,az;
        std::tie(dist,az) = CalcInv(p0,p1);

        if (dist > maxDistMeters)
        {
            const auto segs = (int)std::ceil(dist / maxDistMeters);
            const auto segLen = dist / segs;
            Point2f outPt;
            bool valid;
            for (int i = 1; i < segs; ++i)
            {
                std::tie(valid,outPt) = CalcDirect(p0,az,segLen * i);
                if (valid)
                {
                    *out++ = outPt;
                }
            }
        }
    }
}

template <typename T> struct Adapt3dTo2f : std::vector<Point2f>::const_iterator
{
    Adapt3dTo2f(T i) : _i(i) {}
    Adapt3dTo2f(const Adapt3dTo2f &i) : _i(i._i) {}
    Point2f operator*() const { return Point2f(_i->x(),_i->y()); }
    Adapt3dTo2f& operator++() { ++_i; return *this; }
    Adapt3dTo2f operator++(int) { auto x = *this; ++_i; return x; }
    bool operator==(const Adapt3dTo2f &other) const { return _i == other._i; }
    bool operator!=(const Adapt3dTo2f &other) const { return _i != other._i; }
    T _i;
};
template <typename T> struct Adapt2fTo3d
{
    Adapt2fTo3d(T i) : _i(i) {}
    Adapt2fTo3d& operator*() { return *this; }
    Adapt2fTo3d& operator++() { ++_i; return this; }
    Adapt2fTo3d operator++(int) { auto x = *this; ++_i; return x; }
    Adapt2fTo3d& operator=(const Point2f &p) { _i.operator=(Point3d(p.x(),p.y(),0.0)); return *this; }
    bool operator==(const Adapt2fTo3d &other) const { return _i == other._i; }
    bool operator!=(const Adapt2fTo3d &other) const { return _i != other._i; }
    T _i;
};
template <typename T> Adapt3dTo2f<T> make3to2(T iter) { return Adapt3dTo2f<T>(iter); }
template <typename T> Adapt2fTo3d<T> make2to3(T iter) { return Adapt2fTo3d<T>(iter); }

static void SubdivideGeoLib(const VectorRing &inPts, VectorRing &outPts, double maxDistMeters)
{
    outPts.reserve(outPts.size() + inPts.size() * 10);
    SubdivideGeoLib(inPts.begin(), inPts.end(), std::back_inserter(outPts), maxDistMeters);
}

#if 0
static void SubdivideGeoLib(const VectorRing3d &inPts, VectorRing &outPts, double maxDistMeters)
{
    outPts.reserve(outPts.size() + inPts.size() * 10);
    SubdivideGeoLib(make3to2(inPts.begin()), make3to2(inPts.end()), std::back_inserter(outPts), maxDistMeters);
}
#endif

static void SubdivideGeoLib(const VectorRing3d &inPts, VectorRing3d &outPts, double maxDistMeters)
{
    outPts.reserve(outPts.size() + inPts.size() * 10);
    SubdivideGeoLib(make3to2(inPts.begin()), make3to2(inPts.end()), make2to3(std::back_inserter(outPts)), maxDistMeters);
}

void VectorObject::subdivideToInternal(float epsilon,WhirlyKit::CoordSystemDisplayAdapter *adapter,bool useGeoLib,bool edgeMode)
{
    CoordSystem *coordSys = adapter->getCoordSystem();

    const auto geoDist = useGeoLib ? epsilon * wgs84Geodesic.EquatorialRadius() : 0.0;

    for (const auto &shapeRef : shapes)
    {
        const auto shape = shapeRef.get();
        if (const auto lin = dynamic_cast<VectorLinear*>(shape))
        {
            VectorRing outPts2D;
            if (useGeoLib) {
                SubdivideGeoLib(lin->pts,outPts2D,geoDist);
            } else {
                SubdivideEdgesToSurfaceGCGeo(lin->pts,outPts2D,false,adapter,coordSys,epsilon);
            }

            if (edgeMode && outPts2D.size() > 1)
            {
                // See if they cross the edge of a wraparound coordinate system
                // Note: Only works for spherical mercator, most likely
                lin->pts.clear();
                fixEdges(outPts2D,lin->pts);
            } else {
                lin->pts = outPts2D;
            }
        } else if (const auto lin3d = dynamic_cast<VectorLinear3d*>(shape)) {
            VectorRing3d outPts;
            if (useGeoLib) {
                SubdivideGeoLib(lin3d->pts, outPts, geoDist);
            } else {
                SubdivideEdgesToSurfaceGCGeo(lin3d->pts, outPts, false, adapter, coordSys, epsilon);
            }
            lin3d->pts = outPts;
        } else if (const auto ar = dynamic_cast<VectorAreal*>(shape)) {
            VectorRing outPts;
            for (unsigned int ii=0;ii<ar->loops.size();ii++)
            {
                outPts.clear();
                if (useGeoLib) {
                    SubdivideGeoLib(ar->loops[ii], outPts, geoDist);
                } else {
                    SubdivideEdgesToSurfaceGCGeo(ar->loops[ii], outPts, true, adapter, coordSys, epsilon);
                }
                ar->loops[ii] = outPts;
            }
        }
    }
}

void VectorObject::subdivideToGlobeGreatCircle(float epsilon)
{
    FakeGeocentricDisplayAdapter adapter;
    
    subdivideToInternal(epsilon,&adapter,false,true);
}
    
void VectorObject::subdivideToFlatGreatCircle(float epsilon)
{
    FakeGeocentricDisplayAdapter adapter;
    
    subdivideToInternal(epsilon,&adapter,false,false);
}

void VectorObject::subdivideToGlobeGreatCirclePrecise(float epsilon)
{
    FakeGeocentricDisplayAdapter adapter;
    subdivideToInternal(epsilon,&adapter,true,true);
}
    
void VectorObject::subdivideToFlatGreatCirclePrecise(float epsilon)
{
    FakeGeocentricDisplayAdapter adapter;
    subdivideToInternal(epsilon,&adapter,true,false);
}

VectorObjectRef VectorObject::linearsToAreals() const
{
    auto newVec = std::make_shared<VectorObject>();
    newVec->shapes.reserve(shapes.size());

    for (const auto &shape : shapes)
    {
        if (const auto ar = std::dynamic_pointer_cast<VectorAreal>(shape)) {
            newVec->shapes.insert(ar);
        } else if (const auto ln = std::dynamic_pointer_cast<VectorLinear>(shape)) {
            const auto newAr = VectorAreal::createAreal();
            newAr->loops.push_back(ln->pts);
            newVec->shapes.insert(newAr);
        }
    }

    return newVec;
}

VectorObjectRef VectorObject::arealsToLinears() const
{
    auto newVec = std::make_shared<VectorObject>();
    newVec->shapes.reserve(shapes.size());

    for (const auto &shape : shapes)
    {
        if (const auto ar = dynamic_cast<VectorAreal*>(shape.get()))
        {
            for (const auto &loop : ar->loops) {
                const auto newLn = VectorLinear::createLinear();
                newLn->setAttrDict(ar->getAttrDict());
                newLn->pts = loop;
                newVec->shapes.insert(newLn);
            }
        } else if (const auto ln = std::dynamic_pointer_cast<VectorLinear>(shape)) {
            newVec->shapes.insert(ln);
        }
    }
    
    return newVec;
}
    
VectorObjectRef VectorObject::filterClippedEdges() const
{
    auto newVec = std::make_shared<VectorObject>();
    newVec->shapes.reserve(shapes.size());

    for (const auto &shape : shapes)
    {
        if (const auto ar = dynamic_cast<VectorAreal*>(shape.get()))
        {
            for (const auto &loop : ar->loops) {
                if (loop.empty())
                    continue;

                // Compare segments against the bounding box
                Mbr mbr;
                mbr.addPoints(loop);
                
                int which = 0;
                while (which < loop.size()) {
                    VectorRing pts;
                    while (which < loop.size()) {
                        auto p0 = loop[which];
                        auto p1 = loop[(which+1)%loop.size()];
                        
                        which++;
                        if (p0 == p1)
                            continue;
                        
                        if ((p0.x() == p1.x() && (p0.x() == mbr.ll().x() || p0.x() == mbr.ur().x())) ||
                            (p0.y() == p1.y() && (p0.y() == mbr.ll().y() || p0.y() == mbr.ur().y()))) {
                            break;
                        } else {
                            if (pts.empty() || pts.back() != p0)
                                pts.push_back(p0);
                                if (pts.empty() || pts.back() != p1)
                                    pts.push_back(p1);
                                    }
                    }
                    
                    if (!pts.empty()) {
                        const auto newLn = VectorLinear::createLinear();
                        newLn->setAttrDict(ar->getAttrDict());
                        newLn->pts = pts;
                        newVec->shapes.insert(newLn);
                    }
                }
            }
        } else if (const auto ln = std::dynamic_pointer_cast<VectorLinear>(shape)) {
            newVec->shapes.insert(ln);
        }
    }
    
    return newVec;
}
    
VectorObjectRef VectorObject::tesselate() const
{
    auto newVec = std::make_shared<VectorObject>();
    newVec->shapes.reserve(shapes.size());

    for (const auto &shape : shapes)
    {
        if (const auto ar = dynamic_cast<VectorAreal*>(shape.get()))
        {
            const auto trisRef = VectorTriangles::createTriangles();
            TesselateLoops(ar->loops, trisRef);
            trisRef->setAttrDict(ar->getAttrDict());
            trisRef->initGeoMbr();
            newVec->shapes.insert(trisRef);
        }
    }
    
    return newVec;
}
    
VectorObjectRef VectorObject::clipToGrid(const Point2d &gridSize)
{
    auto newVec = std::make_shared<VectorObject>();
    newVec->shapes.reserve(shapes.size());

    for (const auto &shape : shapes)
    {
        if (const auto ar = dynamic_cast<VectorAreal*>(shape.get()))
        {
            std::vector<VectorRing> newLoops;
            newLoops.reserve(ar->loops.size());
            
            ClipLoopsToGrid(ar->loops, Point2f(0.0,0.0), Point2f(gridSize.x(),gridSize.y()), newLoops);
            for (unsigned int jj=0;jj<newLoops.size();jj++)
            {
                const auto newAr = VectorAreal::createAreal();
                newAr->setAttrDict(ar->getAttrDict());
                newAr->loops.push_back(newLoops[jj]);
                newVec->shapes.insert(newAr);
            }
        }
    }
    
    return newVec;
}

VectorObjectRef VectorObject::clipToMbr(const Point2d &ll,const Point2d &ur)
{
    auto newVec = std::make_shared<VectorObject>();
    newVec->shapes.reserve(shapes.size());

    Mbr mbr(Point2f(ll.x(), ll.y()), Point2f(ur.x(), ur.y()));
    
    for (const auto &shapeRef : shapes)
    {
        const auto shape = shapeRef.get();
        if (const auto linear = dynamic_cast<VectorLinear*>(shape))
        {
            std::vector<VectorRing> newLoops;
            newLoops.reserve(10);   // ?

            ClipLoopToMbr(linear->pts, mbr, false, newLoops);
            for (const auto &loop : newLoops)
            {
                const auto newLinear = VectorLinear::createLinear();
                newLinear->setAttrDict(linear->getAttrDict());
                newLinear->pts = loop;
                newVec->shapes.insert(newLinear);
            }
        } else if(dynamic_cast<VectorLinear3d*>(shape)) {
            wkLogLevel(Error, "Don't know how to clip linear3d objects");
        } else if (const auto ar = dynamic_cast<VectorAreal*>(shape)) {
            for (int ii=0;ii<ar->loops.size();ii++) {
                std::vector<VectorRing> newLoops;
                newLoops.reserve(ar->loops.size());
                ClipLoopToMbr(ar->loops[ii], mbr, true, newLoops);
                for (unsigned int jj=0;jj<newLoops.size();jj++)
                {
                    const auto newAr = VectorAreal::createAreal();
                    newAr->setAttrDict(ar->getAttrDict());
                    newAr->loops.push_back(newLoops[jj]);
                    newVec->shapes.insert(newAr);
                }
            }
        } else if(const auto points = dynamic_cast<VectorPoints*>(shape)) {
            const auto newPoints = VectorPoints::createPoints();
            newPoints->pts.reserve(points->pts.size());
            
            for (unsigned int ii=0;ii<points->pts.size();ii++)
            {
                const Point2f &pt = points->pts[ii];
                if(pt.x() >= ll.x() && pt.x() <= ur.x() &&
                   pt.y() >= ll.y() && pt.y() <= ur.y())
                {
                    newPoints->pts.push_back(pt);
                }
            }
        }
    }

    return newVec;
}
 
void SampleGreatCircle(const Point2d &startPt,const Point2d &endPt,double height,Point3dVector &pts,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,double eps)
{
    const bool isFlat = coordAdapter->isFlat();
    
    // We can subdivide the great circle with this routine
    if (isFlat)
    {
        pts.resize(2);
        pts[0] = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(startPt.x(),startPt.y())));
        pts[1] = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(endPt.x(),endPt.y())));
    } else {
        const VectorRing inPts { Point2f(startPt.x(),startPt.y()), Point2f(endPt.x(),endPt.y()) };
        
        VectorRing3d tmpPts;
        tmpPts.reserve(10); // ?
        SubdivideEdgesToSurfaceGC(inPts, tmpPts, false, coordAdapter, eps);
        pts = tmpPts;
        
        // To apply the height, we'll need the total length
        float totLen = 0;
        for (int ii=0;ii<pts.size()-1;ii++)
        {
            const float len = (pts[ii+1]-pts[ii]).norm();
            totLen += len;
        }
        
        // Now we'll walk along, apply the height (max at the middle)
        float lenSoFar = 0.0;
        for (unsigned int ii=0;ii<pts.size();ii++)
        {
            Point3d &pt = pts[ii];
            const float len = (ii+1 < pts.size()) ? (pts[ii+1]-pt).norm() : 0.0;
            const float t = lenSoFar/totLen;
            lenSoFar += len;
            
            // Parabolic curve
            const float b = 4*height;
            const float a = -b;
            const float thisHeight = a*(t*t) + b*t;
            
            if (isFlat)
                pt.z() = thisHeight;
            else
                pt *= 1.0+thisHeight;
        }
    }
}

void SampleGreatCircleStatic(const Point2d &startPt,const Point2d &endPt,double height,Point3dVector &pts,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,double samples)
{
    const bool isFlat = coordAdapter->isFlat();
    
    // We can subdivide the great circle with this routine
    if (isFlat)
    {
        pts.resize(2);
        pts[0] = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(startPt.x(),startPt.y())));
        pts[1] = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(endPt.x(),endPt.y())));
    } else {
        const VectorRing inPts { Point2f(startPt.x(),startPt.y()), Point2f(endPt.x(),endPt.y()) };
        
        VectorRing3d tmpPts;
        tmpPts.reserve(10); // ?
        SubdivideEdgesToSurfaceGC(inPts, tmpPts, false, coordAdapter, 1.0, 0.0, samples);
        pts = tmpPts;
        
        // To apply the height, we'll need the total length
        float totLen = 0;
        for (int ii=0;ii<pts.size()-1;ii++)
        {
            const float len = (pts[ii+1]-pts[ii]).norm();
            totLen += len;
        }
        
        // Now we'll walk along, apply the height (max at the middle)
        float lenSoFar = 0.0;
        for (unsigned int ii=0;ii<pts.size();ii++)
        {
            Point3d &pt = pts[ii];
            const float len = (pts[ii+1]-pt).norm();
            const float t = lenSoFar/totLen;
            lenSoFar += len;
            
            // Parabolic curve
            const float b = 4*height;
            const float a = -b;
            const float thisHeight = a*(t*t) + b*t;
            
            if (isFlat)
                pt.z() = thisHeight;
            else
                pt *= 1.0+thisHeight;
        }
    }
}

}
