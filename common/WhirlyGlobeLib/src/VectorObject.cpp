/*
 *  VectorObject.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/17/11.
 *  Copyright 2011-2013 mousebird consulting
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

#import "VectorObject.h"
#import "GlobeMath.h"
#import "VectorData.h"
#import "ShapeReader.h"
#import "Tesselator.h"
#import "GridClipper.h"
#import "WhirlyKitLog.h"
#import "GlobeView.h"
#import "MaplyView.h"

namespace WhirlyKit
{
    
VectorObject::VectorObject()
    : selectable(true)
{
}

VectorObject::VectorObject(SimpleIdentity theId)
: Identifiable(theId), selectable(true)
{
}
    
bool VectorObject::fromGeoJSON(const std::string &json,std::string &crs)
{
    return VectorParseGeoJSON(shapes,json,crs);
}
    
bool VectorObject::FromGeoJSONAssembly(const std::string &json,std::map<std::string,VectorObject *> &vecData)
{
    std::map<std::string, ShapeSet> newShapes;
    if (!VectorParseGeoJSONAssembly(json, newShapes))
        return false;
    
    for (std::map<std::string, ShapeSet>::iterator it = newShapes.begin();
         it != newShapes.end(); ++it)
    {
        VectorObject *vecObj = new VectorObject();
        vecObj->shapes.insert(it->second.begin(),it->second.end());
        vecData[it->first] = vecObj;
    }
    
    return true;
}
    
bool VectorObject::fromShapeFile(const std::string &fileName)
{
    ShapeReader shapeReader(fileName);
    if (!shapeReader.isValid())
        return false;
    
    int numObj = shapeReader.getNumObjects();
    for (unsigned int ii=0;ii<numObj;ii++) {
        VectorShapeRef shape = shapeReader.getObjectByIndex(ii, NULL);
        shapes.insert(shape);
    }
    
    return true;
}
    
MutableDictionaryRef VectorObject::getAttributes()
{
    if (shapes.empty())
        return NULL;
    
    return (*(shapes.begin()))->getAttrDict();
}
    
void VectorObject::setAttributes(MutableDictionaryRef newDict)
{
    for (ShapeSet::iterator it = shapes.begin();it != shapes.end(); ++it)
        (*it)->setAttrDict(newDict);
}
    
void VectorObject::mergeVectorsFrom(VectorObject *otherVec)
{
    shapes.insert(otherVec->shapes.begin(),otherVec->shapes.end());
}
 
void VectorObject::splitVectors(std::vector<VectorObject *> &vecs)
{    
    for (WhirlyKit::ShapeSet::iterator it = shapes.begin();
         it != shapes.end(); ++it)
    {
        VectorObject *vecObj = new VectorObject();
        vecObj->shapes.insert(*it);
        vecs.push_back(vecObj);
    }
}
    
bool VectorObject::center(Point2d &center)
{
    Mbr mbr;
    for (ShapeSet::iterator it = shapes.begin();it != shapes.end();++it)
    {
        GeoMbr geoMbr = (*it)->calcGeoMbr();
        mbr.addPoint(geoMbr.ll());
        mbr.addPoint(geoMbr.ur());
    }
    
    center.x() = (mbr.ll().x() + mbr.ur().x())/2.0;
    center.y() = (mbr.ll().y() + mbr.ur().y())/2.0;
    
    return true;
}
    
bool VectorObject::centroid(Point2d &centroid)
{
    // Find the loop with the largest area
    float bigArea = -1.0;
    const VectorRing *bigLoop = NULL;
    for (ShapeSet::iterator it = shapes.begin();it != shapes.end();++it)
    {
        VectorArealRef areal = std::dynamic_pointer_cast<VectorAreal>(*it);
        if (areal && areal->loops.size() > 0)
        {
            for (unsigned int ii=0;ii<areal->loops.size();ii++)
            {
                float area = std::abs(CalcLoopArea(areal->loops[ii]));
                if (area > bigArea)
                {
                    bigLoop = &areal->loops[ii];
                    bigArea = area;
                }
            }
        } else {
            VectorLinearRef linear = std::dynamic_pointer_cast<VectorLinear>(*it);
            if (linear)
            {
                GeoCoord midCoord = linear->geoMbr.mid();
                centroid.x() = midCoord.x();
                centroid.y() = midCoord.y();
                return true;
            } else {
                VectorLinear3dRef linear3d = std::dynamic_pointer_cast<VectorLinear3d>(*it);
                if (linear3d)
                {
                    GeoCoord midCoord = linear3d->geoMbr.mid();
                    centroid.x() = midCoord.x();
                    centroid.y() = midCoord.y();
                    return true;
                } else {
                    VectorPointsRef pts = std::dynamic_pointer_cast<VectorPoints>(*it);
                    if (pts)
                    {
                        GeoCoord midCoord = pts->geoMbr.mid();
                        centroid.x() = midCoord.x();
                        centroid.y() = midCoord.y();
                        return true;
                    }
                }
            }
        }
    }
    
    if (bigArea < 0.0)
        return false;
    
    if (bigLoop)
    {
        Point2f centroid2f = CalcLoopCentroid(*bigLoop);
        centroid.x() = centroid2f.x();
        centroid.y() = centroid2f.y();
    } else
        return false;
    
    return true;
}

bool VectorObject::largestLoopCenter(Point2d &center,Point2d &ll,Point2d &ur)
{
    // Find the loop with the largest area
    float bigArea = -1.0;
    const VectorRing *bigLoop = NULL;
    for (ShapeSet::iterator it = shapes.begin();it != shapes.end();++it)
    {
        VectorArealRef areal = std::dynamic_pointer_cast<VectorAreal>(*it);
        if (areal && areal->loops.size() > 0)
        {
            for (unsigned int ii=0;ii<areal->loops.size();ii++)
            {
                float area = std::abs(CalcLoopArea(areal->loops[ii]));
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
    ctr.x() = 0;  ctr.y() = 0;
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

bool VectorObject::linearMiddle(Point2d &middle,double &rot)
{
    if (shapes.empty())
        return false;
    
    VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*(shapes.begin()));
    VectorLinear3dRef lin3d = std::dynamic_pointer_cast<VectorLinear3d>(*(shapes.begin()));
    if (!lin && !lin3d)
        return false;
    
    if (lin)
    {
        VectorRing pts = lin->pts;
        float totLen = 0;
        for (int ii=0;ii<pts.size()-1;ii++)
        {
            float len = (pts[ii+1]-pts[ii]).norm();
            totLen += len;
        }
        float halfLen = totLen / 2.0;
        
        // Now we'll walk along, looking for the middle
        float lenSoFar = 0.0;
        for (unsigned int ii=0;ii<pts.size()-1;ii++)
        {
            Point2f &pt0 = pts[ii],&pt1 = pts[ii+1];
            float len = (pt1-pt0).norm();
            if (len > 0.0 && halfLen <= lenSoFar+len)
            {
                float t = (halfLen-lenSoFar)/len;
                Point2f thePt = (pt1-pt0)*t + pt0;
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
        VectorRing3d pts = lin3d->pts;
        float totLen = 0;
        for (int ii=0;ii<pts.size()-1;ii++)
        {
            float len = (pts[ii+1]-pts[ii]).norm();
            totLen += len;
        }
        float halfLen = totLen / 2.0;
        
        // Now we'll walk along, looking for the middle
        float lenSoFar = 0.0;
        for (unsigned int ii=0;ii<pts.size()-1;ii++)
        {
            Point3d &pt0 = pts[ii],&pt1 = pts[ii+1];
            float len = (pt1-pt0).norm();
            if (len > 0.0 && halfLen <= lenSoFar+len)
            {
                float t = (halfLen-lenSoFar)/len;
                Point3d thePt = (pt1-pt0)*t + pt0;
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
    
bool VectorObject::linearMiddle(Point2d &middle,double &rot,CoordSystem *coordSys)
{
    if (shapes.empty())
        return false;
    
    VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*(shapes.begin()));
    VectorLinear3dRef lin3d = std::dynamic_pointer_cast<VectorLinear3d>(*(shapes.begin()));
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
            float len = (pts[ii+1]-pts[ii]).norm();
            totLen += len;
        }
        float halfLen = totLen / 2.0;
        
        // Now we'll walk along, looking for the middle
        float lenSoFar = 0.0;
        for (unsigned int ii=0;ii<pts.size()-1;ii++)
        {
            Point3d pt0 = coordSys->geographicToLocal3d(GeoCoord(pts[ii].x(),pts[ii].y()));
            Point3d pt1 = coordSys->geographicToLocal3d(GeoCoord(pts[ii+1].x(),pts[ii+1].y()));
            double len = (pt1-pt0).norm();
            if (halfLen <= lenSoFar+len)
            {
                double t = (halfLen-lenSoFar)/len;
                Point3d thePt = (pt1-pt0)*t + pt0;
                GeoCoord middleGeo = coordSys->localToGeographic(thePt);
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
        VectorRing3d &pts = lin3d->pts;
        
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
            float len = (pts[ii+1]-pts[ii]).norm();
            totLen += len;
        }
        float halfLen = totLen / 2.0;
        
        // Now we'll walk along, looking for the middle
        float lenSoFar = 0.0;
        for (unsigned int ii=0;ii<pts.size()-1;ii++)
        {
            GeoCoord geo0(pts[ii].x(),pts[ii].y());
            GeoCoord geo1(pts[ii+1].x(),pts[ii+1].y());
            Point3d pt0 = coordSys->geographicToLocal3d(geo0);
            Point3d pt1 = coordSys->geographicToLocal3d(geo1);
            double len = (pt1-pt0).norm();
            if (halfLen <= lenSoFar+len)
            {
                double t = (halfLen-lenSoFar)/len;
                Point3d thePt = (pt1-pt0)*t + pt0;
                GeoCoord middleGeo = coordSys->localToGeographic(thePt);
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
    
bool VectorObject::middleCoordinate(Point2d &middle)
{
    if (shapes.empty())
        return false;
    
    VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*(shapes.begin()));
    VectorLinear3dRef lin3d = std::dynamic_pointer_cast<VectorLinear3d>(*(shapes.begin()));
    if (!lin && !lin3d)
        return false;
    
    if (lin)
    {
        unsigned long index = lin->pts.size() / 2;
        middle.x() = lin->pts[index].x();
        middle.y() = lin->pts[index].y();
    } else {
        unsigned long index = lin3d->pts.size() / 2;
        middle.x() = lin3d->pts[index].x();
        middle.y() = lin3d->pts[index].y();
    }
    
    return true;
}
    
bool VectorObject::pointInside(const Point2d &pt)
{
    for (ShapeSet::iterator it = shapes.begin();it != shapes.end();++it)
    {
        VectorArealRef areal = std::dynamic_pointer_cast<VectorAreal>(*it);
        if (areal)
        {
            if (areal->pointInside(GeoCoord(pt.x(),pt.y())))
                return true;
        } else {
            VectorTrianglesRef tris = std::dynamic_pointer_cast<VectorTriangles>(*it);
            if (tris)
            {
                if (tris->pointInside(GeoCoord(pt.x(),pt.y())))
                    return true;
            }
        }
    }
    
    return false;
}
    
// Helper routine to convert and check geographic points (globe version)
static bool ScreenPointFromGeo(const Point2d &geoCoord,WhirlyGlobe::GlobeViewStateRef globeView,Maply::MapViewStateRef mapView,CoordSystemDisplayAdapter *coordAdapter,const Point2f &frameSize,const Eigen::Matrix4f &modelAndViewMat,const Eigen::Matrix4d &modelAndViewMat4d,const Eigen::Matrix4d &modelMatFull,const Eigen::Matrix4f &modelAndViewNormalMat,Point2d *screenPt)
{
    Point3d pt = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(geoCoord.x(),geoCoord.y())));
    Point3f pt3f(pt.x(),pt.y(),pt.z());
    
    Point2f screenPt2f;
    if (globeView) {
        if (CheckPointAndNormFacing(pt3f,pt3f.normalized(),modelAndViewMat,modelAndViewNormalMat) < 0.0)
            return false;
        
        screenPt2f = globeView->pointOnScreenFromDisplay(pt, &modelAndViewMat4d, frameSize);
    } else {
        screenPt2f = mapView->pointOnScreenFromDisplay(pt, &modelAndViewMat4d, frameSize);
    }
    screenPt->x() = screenPt2f.x();  screenPt->y() = screenPt2f.y();
    
    if (screenPt->x() < 0 || screenPt->y() < 0 || screenPt->x() > frameSize.x() || screenPt->y() > frameSize.y())
        return false;
    
    return true;
}
    
bool VectorObject::pointNearLinear(const Point2d &coord,float maxDistance,ViewStateRef viewState,const Point2f &frameSize)
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

    for (ShapeSet::iterator it = shapes.begin();it != shapes.end();++it)
    {
        VectorLinearRef linear = std::dynamic_pointer_cast<VectorLinear>(*it);
        if (linear)
        {
            GeoMbr geoMbr = linear->calcGeoMbr();
            if(geoMbr.inside(GeoCoord(coord.x(),coord.y())))
            {
                VectorRing pts = linear->pts;
                float distance;
                for (int ii=0;ii<pts.size()-1;ii++)
                {
                    distance = MAXFLOAT;
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
                    
                    Point2d aToP = a - p;
                    Point2d aToB = a - b;
                    double aToBMagitude = pow(hypot(aToB.x(), aToB.y()), 2);
                    double dot = aToP.x() * aToB.x() + aToP.y() * aToB.y();
                    double d = dot/aToBMagitude;
                    
                    if(d < 0)
                    {
                        distance = hypot(p.x() - a.x(), p.y() - a.y());
                    } else if(d > 1) {
                        distance = hypot(p.x() - b.x(), p.y() - b.y());
                    } else {
                        distance = hypot(p.x() - a.x() + (aToB.x() * d),
                                         p.y() - a.y() + (aToB.y() * d));
                    }
                    
                    if (distance < maxDistance)
                        return true;
                }
            }
        } else {
            VectorLinear3dRef linear3d = std::dynamic_pointer_cast<VectorLinear3d>(*it);
            if (linear3d)
            {
                GeoMbr geoMbr = linear3d->calcGeoMbr();
                if(geoMbr.inside(GeoCoord(coord.x(),coord.y())))
                {
                    VectorRing3d pts = linear3d->pts;
                    float distance;
                    for (int ii=0;ii<pts.size()-1;ii++)
                    {
                        distance = MAXFLOAT;
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

                        Point2d aToP = a - p;
                        Point2d aToB = a - b;
                        double aToBMagitude = pow(hypot(aToB.x(), aToB.y()), 2);
                        double dot = aToP.x() * aToB.x() + aToP.y() * aToB.y();
                        double d = dot/aToBMagitude;
                        
                        if(d < 0)
                        {
                            distance = hypot(p.x() - a.x(), p.y() - a.y());
                        } else if(d > 1) {
                            distance = hypot(p.x() - b.x(), p.y() - b.y());
                        } else {
                            distance = hypot(p.x() - a.x() + (aToB.x() * d),
                                             p.y() - a.y() + (aToB.y() * d));
                        }
                        
                        if (distance < maxDistance)
                            return true;
                    }
                }
            }
        }
    }
    
    return false;
}
    
double VectorObject::areaOfOuterLoops()
{
    double area = 0.0;
    for (ShapeSet::iterator it = shapes.begin();it != shapes.end();++it)
    {
        VectorArealRef areal = std::dynamic_pointer_cast<VectorAreal>(*it);
        if (areal && areal->loops.size() > 0)
        {
            area = CalcLoopArea(areal->loops[0]);
        }
    }
    
    return area;
}
    
bool VectorObject::boundingBox(Point2d &ll,Point2d &ur)
{
    bool valid = false;
    Mbr mbr;
    for (ShapeSet::iterator it = shapes.begin();it != shapes.end();++it)
    {
        GeoMbr geoMbr = (*it)->calcGeoMbr();
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
    VectorArealRef areal = std::dynamic_pointer_cast<VectorAreal>(*(shapes.begin()));
    if (areal)
    {
        areal->loops.push_back(hole);
    }
}
    
VectorObjectRef VectorObject::deepCopy()
{
    VectorObjectRef newVecObj(new VectorObject());
    
    for (ShapeSet::iterator it = shapes.begin(); it != shapes.end(); ++it)
    {
        VectorPointsRef points = std::dynamic_pointer_cast<VectorPoints>(*it);
        if (points)
        {
            VectorPointsRef newPts = VectorPoints::createPoints();
            newPts->pts = points->pts;
            newPts->setAttrDict(points->getAttrDict()->copy());
            newPts->initGeoMbr();
            newVecObj->shapes.insert(newPts);
        } else {
            VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*it);
            if (lin)
            {
                VectorLinearRef newLin = VectorLinear::createLinear();
                newLin->pts = lin->pts;
                newLin->setAttrDict(lin->getAttrDict()->copy());
                newLin->initGeoMbr();
                newVecObj->shapes.insert(newLin);
            } else {
                VectorLinear3dRef lin3d = std::dynamic_pointer_cast<VectorLinear3d>(*it);
                if (lin3d)
                {
                    VectorLinear3dRef newLin3d = VectorLinear3d::createLinear();
                    newLin3d->pts = lin3d->pts;
                    newLin3d->setAttrDict(lin3d->getAttrDict()->copy());
                    newLin3d->initGeoMbr();
                    newVecObj->shapes.insert(newLin3d);
                } else {
                    VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
                    if (ar)
                    {
                        VectorArealRef newAr = VectorAreal::createAreal();
                        newAr->loops = ar->loops;
                        newAr->setAttrDict(ar->getAttrDict()->copy());
                        newAr->initGeoMbr();
                        newVecObj->shapes.insert(newAr);
                    } else {
                        VectorTrianglesRef tri = std::dynamic_pointer_cast<VectorTriangles>(*it);
                        if (tri)
                        {
                            VectorTrianglesRef newTri = VectorTriangles::createTriangles();
                            newTri->geoMbr = tri->geoMbr;
                            newTri->pts = tri->pts;
                            newTri->tris = tri->tris;
                            newTri->setAttrDict(tri->getAttrDict()->copy());
                            newTri->initGeoMbr();
                            newVecObj->shapes.insert(newTri);
                        }
                    }
                }
            }
        }
    }
    
    return newVecObj;
}
    
VectorObjectType VectorObject::getVectorType()
{
    if (shapes.empty())
        return VectorMultiType;

    VectorObjectType type = VectorNoneType;
    for (ShapeSet::iterator it = shapes.begin(); it != shapes.end(); ++it)
    {
        VectorObjectType thisType = VectorNoneType;
        VectorPointsRef points = std::dynamic_pointer_cast<VectorPoints>(*it);
        if (points)
            thisType = VectorPointType;
        else {
            VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*it);
            if (lin)
                thisType = VectorLinearType;
            else {
                VectorLinear3dRef lin3d = std::dynamic_pointer_cast<VectorLinear3d>(*it);
                if (lin3d)
                {
                    thisType = VectorLinear3dType;
                } else {
                    VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
                    if (ar)
                        thisType = VectorArealType;
                }
            }
        }

        if (type == VectorNoneType)                                                                                                                                                                                                                type = thisType;
        else
            if (type != thisType)
                return VectorMultiType;
    }

    return type;
}
    
bool VectorObject::isSelectable()
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

std::string VectorObject::log()
{
    std::string outStr;
    
    for (ShapeSet::iterator it = shapes.begin(); it != shapes.end(); ++it)
    {
        VectorPointsRef points = std::dynamic_pointer_cast<VectorPoints>(*it);
        if (points)
        {
            outStr += "Points: ";
            for (unsigned int ii=0;ii<points->pts.size();ii++)
            {
                const Point2f &pt = points->pts[ii];
                outStr += " (" + to_string(pt.x()) + "," + to_string(pt.y()) + ")";
            }
            outStr += "\n";
        } else {
            VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*it);
            if (lin)
            {
                outStr += "Linear: ";
                for (unsigned int ii=0;ii<lin->pts.size();ii++)
                {
                    const Point2f &pt = lin->pts[ii];
                    outStr += " (" + to_string(pt.x()) + "," + to_string(pt.y()) + ")";
                }
                outStr += "\n";
            } else {
                VectorLinear3dRef lin3d = std::dynamic_pointer_cast<VectorLinear3d>(*it);
                if (lin3d)
                {
                    outStr += "Linear3d: ";
                    for (unsigned int ii=0;ii<lin3d->pts.size();ii++)
                    {
                        const Point3d &pt = lin3d->pts[ii];
                        outStr += " (" + to_string(pt.x()) + "," + to_string(pt.y()) + "," + to_string(pt.z()) + ")";
                    }
                    outStr += "\n";
                } else {
                    VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
                    if (ar)
                    {
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
            }
        }
    }
    
    return outStr;
}
    
void VectorObject::reproject(CoordSystem *inSystem,double scale,CoordSystem *outSystem)
{
    for (ShapeSet::iterator it = shapes.begin(); it != shapes.end(); ++it)
    {
        VectorPointsRef points = std::dynamic_pointer_cast<VectorPoints>(*it);
        if (points)
        {
            for (Point2f &pt : points->pts)
            {
                Point3f outPt = CoordSystemConvert(inSystem, outSystem, Point3f(pt.x()*scale,pt.y()*scale,0.0));
                pt.x() = outPt.x();  pt.y() = outPt.y();
            }
            points->calcGeoMbr();
        } else {
            VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*it);
            if (lin)
            {
                for (Point2f &pt : lin->pts)
                {
                    Point3f outPt = CoordSystemConvert(inSystem, outSystem, Point3f(pt.x()*scale,pt.y()*scale,0.0));
                    pt.x() = outPt.x();  pt.y() = outPt.y();
                }
                lin->calcGeoMbr();
            } else {
                VectorLinear3dRef lin3d = std::dynamic_pointer_cast<VectorLinear3d>(*it);
                if (lin3d)
                {
                    for (Point3d &pt : lin3d->pts)
                    {
                        Point3d outPt = CoordSystemConvert3d(inSystem, outSystem, pt * scale);
                        pt = outPt;
                    }
                    lin3d->calcGeoMbr();
                } else {
                    VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
                    if (ar)
                    {
                        for (VectorRing &loop : ar->loops)
                            for (Point2f &pt : loop)
                            {
                                
                                Point3f outPt = CoordSystemConvert(inSystem, outSystem, Point3f(pt.x()*scale,pt.y()*scale,0.0));
                                pt.x() = outPt.x() * 180 / M_PI;  pt.y() = outPt.y() * 180 / M_PI;
                            }
                        ar->calcGeoMbr();
                    } else {
                        VectorTrianglesRef tri = std::dynamic_pointer_cast<VectorTriangles>(*it);
                        if (tri)
                        {
                            for (Point3f &pt : tri->pts)
                            {
                                pt = CoordSystemConvert(inSystem, outSystem, Point3f(pt.x()*scale,pt.y()*scale,pt.z()));
                            }
                            tri->calcGeoMbr();
                        }
                    }
                }
            }
        }
    }
}
    
void VectorObject::subdivideToGlobe(float epsilon)
{
    FakeGeocentricDisplayAdapter adapter;
    
    for (ShapeSet::iterator it = shapes.begin();it!=shapes.end();it++)
    {
        VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*it);
        if (lin)
        {
            VectorRing outPts;
            SubdivideEdgesToSurface(lin->pts, outPts, false, &adapter, epsilon);
            lin->pts = outPts;
        } else {
            VectorLinear3dRef lin3d = std::dynamic_pointer_cast<VectorLinear3d>(*it);
            if (lin3d)
            {
                VectorRing3d outPts;
                SubdivideEdgesToSurface(lin3d->pts, outPts, false, &adapter, epsilon);
                lin3d->pts = outPts;
            } else {
                VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
                if (ar)
                {
                    for (unsigned int ii=0;ii<ar->loops.size();ii++)
                    {
                        VectorRing outPts;
                        SubdivideEdgesToSurface(ar->loops[ii], outPts, true, &adapter, epsilon);
                        ar->loops[ii] = outPts;
                    }
                }
            }
        }
    }    
}
   
void VectorObject::subdivideToInternal(float epsilon,WhirlyKit::CoordSystemDisplayAdapter *adapter,bool edgeMode)
{
    CoordSystem *coordSys = adapter->getCoordSystem();
    
    for (ShapeSet::iterator it = shapes.begin();it!=shapes.end();it++)
    {
        VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*it);
        if (lin)
        {
            VectorRing3d outPts;
            SubdivideEdgesToSurfaceGC(lin->pts, outPts, false, adapter, epsilon);
            VectorRing outPts2D;
            outPts2D.resize(outPts.size());
            for (unsigned int ii=0;ii<outPts.size();ii++)
                outPts2D[ii] = coordSys->localToGeographic(adapter->displayToLocal(outPts[ii]));
            if (lin->pts.size() > 0)
            {
                outPts2D.front() = lin->pts.front();
                outPts2D.back() = lin->pts.back();
            }
            
            if (edgeMode && outPts.size() > 1)
            {
                // See if they cross the edge of a wraparound coordinate system
                // Note: Only works for spherical mercator, most likely
                VectorRing offsetPts2D(outPts2D.size());
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
                
                lin->pts = offsetPts2D;
            } else
                lin->pts = outPts2D;
            
//            WHIRLYKIT_LOGD("lin pts = %lu, eps = %f",lin->pts.size(),epsilon);
        } else {
            VectorLinear3dRef lin3d = std::dynamic_pointer_cast<VectorLinear3d>(*it);
            if (lin3d)
            {
                VectorRing3d outPts;
                SubdivideEdgesToSurfaceGC(lin->pts, outPts, false, adapter, epsilon);
                for (unsigned int ii=0;ii<outPts.size();ii++)
                {
                    Point3d locPt = adapter->displayToLocal(outPts[ii]);
                    GeoCoord outPt = coordSys->localToGeographic(locPt);
                    outPts[ii] = Point3d(outPt.x(),outPt.y(),0.0);
                }
                lin3d->pts = outPts;
            } else {
                VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
                if (ar)
                {
                    for (unsigned int ii=0;ii<ar->loops.size();ii++)
                    {
                        VectorRing3d outPts;
                        SubdivideEdgesToSurfaceGC(ar->loops[ii], outPts, true, adapter, epsilon);
                        VectorRing outPts2D;
                        outPts2D.resize(outPts.size());
                        for (unsigned int ii=0;ii<outPts.size();ii++)
                        outPts2D[ii] = coordSys->localToGeographic(adapter->displayToLocal(outPts[ii]));
                        ar->loops[ii] = outPts2D;
                    }
                }
            }
        }
    }
}

void VectorObject::subdivideToGlobeGreatCircle(float epsilon)
{
    FakeGeocentricDisplayAdapter adapter;
    
    subdivideToInternal(epsilon,&adapter,true);
}
    
void VectorObject::subdivideToFlatGreatCircle(float epsilon)
{
    FakeGeocentricDisplayAdapter adapter;
    
    subdivideToInternal(epsilon,&adapter,false);
}

VectorObjectRef VectorObject::linearsToAreals()
{
    VectorObjectRef newVec(new VectorObject());
    
    for (ShapeSet::iterator it = shapes.begin();it!=shapes.end();it++)
    {
        VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
        if (ar)
        {
            newVec->shapes.insert(ar);
        } else {
            VectorLinearRef ln = std::dynamic_pointer_cast<VectorLinear>(*it);
            if (ln)
            {
                VectorArealRef newAr = VectorAreal::createAreal();
                newAr->loops.push_back(ln->pts);
                newVec->shapes.insert(newAr);
            }
        }
    }

    return newVec;
}

VectorObjectRef VectorObject::arealsToLinears()
{
    VectorObjectRef newVec(new VectorObject());

    for (ShapeSet::iterator it = shapes.begin();it!=shapes.end();it++)
    {
        VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
        if (ar)
        {
            for (auto loop : ar->loops) {
                VectorLinearRef newLn = VectorLinear::createLinear();
                newLn->setAttrDict(ar->getAttrDict());
                newLn->pts = loop;
                newVec->shapes.insert(newLn);
            }
        } else {
            VectorLinearRef ln = std::dynamic_pointer_cast<VectorLinear>(*it);
            if (ln)
            {
                newVec->shapes.insert(ln);
            }
        }
    }
    
    return newVec;
}
    
VectorObjectRef VectorObject::filterClippedEdges()
{
    VectorObjectRef newVec(new VectorObject());

    for (ShapeSet::iterator it = shapes.begin();it!=shapes.end();it++)
    {
        VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
        if (ar)
        {
            for (auto loop : ar->loops) {
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
                        VectorLinearRef newLn = VectorLinear::createLinear();
                        newLn->setAttrDict(ar->getAttrDict());
                        newLn->pts = pts;
                        newVec->shapes.insert(newLn);
                    }
                }
            }
        } else {
            VectorLinearRef ln = std::dynamic_pointer_cast<VectorLinear>(*it);
            if (ln)
            {
                newVec->shapes.insert(ln);
            }
        }
    }
    
    return newVec;
}
    
VectorObjectRef VectorObject::tesselate()
{
    VectorObjectRef newVec(new VectorObject());

    for (ShapeSet::iterator it = shapes.begin();it!=shapes.end();it++)
    {
        VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
        if (ar)
        {
            VectorTrianglesRef trisRef = VectorTriangles::createTriangles();
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
    VectorObjectRef newVec(new VectorObject());

    for (ShapeSet::iterator it = shapes.begin();it!=shapes.end();it++)
    {
        VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
        if (ar)
        {
            std::vector<VectorRing> newLoops;
            ClipLoopsToGrid(ar->loops, Point2f(0.0,0.0), Point2f(gridSize.x(),gridSize.y()), newLoops);
            for (unsigned int jj=0;jj<newLoops.size();jj++)
            {
                VectorArealRef newAr = VectorAreal::createAreal();
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
    VectorObjectRef newVec(new VectorObject());

    Mbr mbr(Point2f(ll.x(), ll.y()), Point2f(ur.x(), ur.y()));
    
    for (ShapeSet::iterator it = shapes.begin();it!=shapes.end();it++)
    {
        if(std::dynamic_pointer_cast<VectorLinear>(*it) != NULL)
        {
            VectorLinearRef linear = std::dynamic_pointer_cast<VectorLinear>(*it);
            std::vector<VectorRing> newLoops;
            ClipLoopToMbr(linear->pts, mbr, false, newLoops);
            for (std::vector<VectorRing>::iterator it = newLoops.begin(); it != newLoops.end(); it++)
            {
                VectorLinearRef newLinear = VectorLinear::createLinear();
                newLinear->setAttrDict(linear->getAttrDict());
                newLinear->pts = *it;
                newVec->shapes.insert(newLinear);
            }
        } else if(std::dynamic_pointer_cast<VectorLinear3d>(*it) != NULL)
        {
            wkLogLevel(Error, "Don't know how to clip linear3d objects");
        } else if(std::dynamic_pointer_cast<VectorAreal>(*it) != NULL)
        {
            VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
            if (ar)
            {
                for (int ii=0;ii<ar->loops.size();ii++)
                {
                    std::vector<VectorRing> newLoops;
                    ClipLoopToMbr(ar->loops[ii], mbr, true, newLoops);
                    for (unsigned int jj=0;jj<newLoops.size();jj++)
                    {
                        VectorArealRef newAr = VectorAreal::createAreal();
                        newAr->setAttrDict(ar->getAttrDict());
                        newAr->loops.push_back(newLoops[jj]);
                        newVec->shapes.insert(newAr);
                    }
                }
            }
        } else if(std::dynamic_pointer_cast<VectorPoints>(*it) != NULL)
        {
            VectorPointsRef points = std::dynamic_pointer_cast<VectorPoints>(*it);
            VectorPointsRef newPoints = VectorPoints::createPoints();
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
    bool isFlat = coordAdapter->isFlat();
    
    // We can subdivide the great circle with this routine
    if (isFlat)
    {
        pts.resize(2);
        pts[0] = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(startPt.x(),startPt.y())));
        pts[1] = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(endPt.x(),endPt.y())));
    } else {
        VectorRing inPts;
        inPts.push_back(Point2f(startPt.x(),startPt.y()));
        inPts.push_back(Point2f(endPt.x(),endPt.y()));
        VectorRing3d tmpPts;
        SubdivideEdgesToSurfaceGC(inPts, tmpPts, false, coordAdapter, eps);
        pts = tmpPts;
        
        // To apply the height, we'll need the total length
        float totLen = 0;
        for (int ii=0;ii<pts.size()-1;ii++)
        {
            float len = (pts[ii+1]-pts[ii]).norm();
            totLen += len;
        }
        
        // Now we'll walk along, apply the height (max at the middle)
        float lenSoFar = 0.0;
        for (unsigned int ii=0;ii<pts.size();ii++)
        {
            Point3d &pt = pts[ii];
            float len = (ii+1 < pts.size()) ? (pts[ii+1]-pt).norm() : 0.0;
            float t = lenSoFar/totLen;
            lenSoFar += len;
            
            // Parabolic curve
            float b = 4*height;
            float a = -b;
            float thisHeight = a*(t*t) + b*t;
            
            if (isFlat)
                pt.z() = thisHeight;
            else
                pt *= 1.0+thisHeight;
        }
    }
}

void SampleGreatCircleStatic(const Point2d &startPt,const Point2d &endPt,double height,Point3dVector &pts,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,double samples)
{
    bool isFlat = coordAdapter->isFlat();
    
    // We can subdivide the great circle with this routine
    if (isFlat)
    {
        pts.resize(2);
        pts[0] = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(startPt.x(),startPt.y())));
        pts[1] = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(endPt.x(),endPt.y())));
    } else {
        VectorRing inPts;
        inPts.push_back(Point2f(startPt.x(),startPt.y()));
        inPts.push_back(Point2f(endPt.x(),endPt.y()));
        VectorRing3d tmpPts;
        SubdivideEdgesToSurfaceGC(inPts, tmpPts, false, coordAdapter, 1.0, 0.0, samples);
        pts = tmpPts;
        
        // To apply the height, we'll need the total length
        float totLen = 0;
        for (int ii=0;ii<pts.size()-1;ii++)
        {
            float len = (pts[ii+1]-pts[ii]).norm();
            totLen += len;
        }
        
        // Now we'll walk along, apply the height (max at the middle)
        float lenSoFar = 0.0;
        for (unsigned int ii=0;ii<pts.size();ii++)
        {
            Point3d &pt = pts[ii];
            float len = (pts[ii+1]-pt).norm();
            float t = lenSoFar/totLen;
            lenSoFar += len;
            
            // Parabolic curve
            float b = 4*height;
            float a = -b;
            float thisHeight = a*(t*t) + b*t;
            
            if (isFlat)
                pt.z() = thisHeight;
            else
                pt *= 1.0+thisHeight;
        }
    }
}

}
