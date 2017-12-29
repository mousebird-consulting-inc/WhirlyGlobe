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
#import "WhirlyKitLog.h"

namespace WhirlyKit
{
    
VectorObject::VectorObject()
{
}
    
#ifndef MAPLYMINIMAL
bool VectorObject::fromGeoJSON(const std::string &json)
{
    std::string crs;
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
#endif
    
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
    
Dictionary *VectorObject::getAttributes()
{
    if (shapes.empty())
        return NULL;
    
    return (*shapes.begin())->getAttrDict();
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
    
bool VectorObject::centroid(Point2f &center)
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
                center.x() = midCoord.x();
                center.y() = midCoord.y();
                return true;
            } else {
                VectorPointsRef pts = std::dynamic_pointer_cast<VectorPoints>(*it);
                if (pts)
                {
                    GeoCoord midCoord = pts->geoMbr.mid();
                    center.x() = midCoord.x();
                    center.y() = midCoord.y();
                    return true;
                }
            }
        }
    }
    
    if (bigArea < 0.0)
        return false;
    
    if (bigLoop)
    {
        Point2f centroid2f = CalcLoopCentroid(*bigLoop);
        center.x() = centroid2f.x();
        center.y() = centroid2f.y();
    } else
        return false;
    
    return true;
}

bool VectorObject::largestLoopCenter(Point2f &center,Point2f &ll,Point2f &ur)
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
    
    Point2f ctr;
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

bool VectorObject::linearMiddle(Point2f &middle,float &rot)
{
    if (shapes.empty())
        return false;
    
    VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*(shapes.begin()));
    if (!lin)
        return false;
    
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
    for (unsigned int ii=0;ii<pts.size();ii++)
    {
        Point2f &pt0 = pts[ii],&pt1 = pts[ii+1];
        float len = (pt1-pt0).norm();
        if (halfLen <= lenSoFar+len)
        {
            float t = (halfLen-lenSoFar)/len;
            Point2f thePt = (pt1-pt0)*t + pt0;
            middle.x() = thePt.x();
            middle.y() = thePt.y();
            rot = M_PI/2.0-atan2f(pt1.y()-pt0.y(),pt1.x()-pt0.x());
            return true;
        }
        
        lenSoFar += len;
    }
    
    middle.x() = pts.back().x();
    middle.y() = pts.back().y();
    rot = 0.0;
    
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
    
bool VectorObject::fromFile(const std::string &fileName)
{
    return VectorReadFile(fileName, shapes);
}

bool VectorObject::toFile(const std::string &file)
{
    return VectorWriteFile(file, shapes);
}

MaplyVectorObjectType VectorObject::getVectorType()
{
    if (shapes.empty())
        return MaplyVectorMultiType;

    MaplyVectorObjectType type = MaplyVectorNoneType;
    for (ShapeSet::iterator it = shapes.begin(); it != shapes.end(); ++it)
    {
        MaplyVectorObjectType thisType = MaplyVectorNoneType;
        VectorPointsRef points = std::dynamic_pointer_cast<VectorPoints>(*it);
        if (points)
            thisType = MaplyVectorPointType;
        else {
            VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*it);
            if (lin)
                thisType = MaplyVectorLinearType;
            else {
                VectorLinear3dRef lin3d = std::dynamic_pointer_cast<VectorLinear3d>(*it);
                if (lin3d)
                {
                    thisType = MaplyVectorLinear3dType;
                } else {
                    VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
                    if (ar)
                        thisType = MaplyVectorArealType;
                }
            }
        }

        if (type == MaplyVectorNoneType)                                                                                                                                                                                                                type = thisType;
        else
            if (type != thisType)
                return MaplyVectorMultiType;
    }

    return type;
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
            
            WHIRLYKIT_LOGD("lin pts = %lu, eps = %f",lin->pts.size(),epsilon);
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

}
