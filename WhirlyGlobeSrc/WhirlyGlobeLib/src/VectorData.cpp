/*
 *  VectorData.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/7/11.
 *  Copyright 2011-2016 mousebird consulting
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

#import <string>
#import "VectorData.h"
#import "ShapeReader.h"
#import "libjson.h"

namespace WhirlyKit
{

// Calculate area of a single loop
float CalcLoopArea(const VectorRing &loop)
{
    float area = 0.0;
    for (unsigned int ii=0;ii<loop.size();ii++)
    {
        const Point2f &p1 = loop[ii];
        const Point2f &p2 = loop[(ii+1)%loop.size()];
        area += p1.x()*p2.y() - p1.y()*p2.x();
    }
    
    return area;
}
    
double CalcLoopArea(const Point2dVector &loop)
{
    double area = 0.0;
    for (unsigned int ii=0;ii<loop.size();ii++)
    {
        const Point2d &p1 = loop[ii];
        const Point2d &p2 = loop[(ii+1)%loop.size()];
        area += p1.x()*p2.y() - p1.y()*p2.x();
    }
    
    return area;    
}
    
Point2f CalcLoopCentroid(const VectorRing &loop)
{
    Point2f centroid(0,0);
    
    float area = 0.0;
    for (unsigned int ii=0;ii<loop.size()-1;ii++)
    {
        const Point2f p0 = loop[ii];
        const Point2f p1 = loop[(ii+1)%loop.size()];
        area += (p0.x()*p1.y()-p1.x()*p0.y());
    }
    area /= 2.0;
    
    Point2f sum(0,0);
    for (unsigned int ii=0;ii<loop.size()-1;ii++)
    {
        const Point2f p0 = loop[ii];
        const Point2f p1 = loop[(ii+1)%loop.size()];
        float b = (p0.x()*p1.y()-p1.x()*p0.y());
        sum.x() += (p0.x()+p1.x())*b;
        sum.y() += (p0.y()+p1.y())*b;
    }
    centroid.x() = sum.x()/(6*area);
    centroid.y() = sum.y()/(6*area);
    
    return centroid;
}
        
// Break any edge longer than the given length
// Returns true if it broke anything.  If it didn't, doesn't fill in outPts
void SubdivideEdges(const VectorRing &inPts,VectorRing &outPts,bool closed,float maxLen)
{
    float maxLen2 = maxLen*maxLen;
    
    for (int ii=0;ii<(closed ? inPts.size() : inPts.size()-1);ii++)
    {
        const Point2f &p0 = inPts[ii];
        const Point2f &p1 = inPts[(ii+1)%inPts.size()];
        outPts.push_back(p0);
        Point2f dir = p1-p0;
        float dist2 = dir.squaredNorm();
        if (dist2 > maxLen2)
        {
            float dist = sqrtf(dist2);
            dir /= dist;
            for (float pos=maxLen;pos<dist;pos+=maxLen)
            {                
                Point2f divPt = p0+dir*pos;
                outPts.push_back(divPt);
            }
        }
    }
    if (!closed)
        outPts.push_back(inPts.back());
}

void subdivideToSurfaceRecurse(Point2f p0,Point2f p1,VectorRing &outPts,CoordSystemDisplayAdapter *adapter,float eps)
{
    // If the difference is greater than 180, then this is probably crossing the date line
    //  in which case we'll just leave it alone.
    if (std::abs(p0.x() - p1.x()) > M_PI)
        return;
    
    Point3f dp0 = adapter->localToDisplay(adapter->getCoordSystem()->geographicToLocal(GeoCoord(p0.x(),p0.y())));
    Point3f dp1 = adapter->localToDisplay(adapter->getCoordSystem()->geographicToLocal(GeoCoord(p1.x(),p1.y())));
    Point2f midPt = (p0+p1)/2.0;
    Point3f dMidPt = adapter->localToDisplay(adapter->getCoordSystem()->geographicToLocal(GeoCoord(midPt.x(),midPt.y())));
    Point3f halfPt = (dp0+dp1)/2.0;
    float dist2 = (halfPt-dMidPt).squaredNorm();
    if (dist2 > eps*eps)
    {
        subdivideToSurfaceRecurse(p0, midPt, outPts, adapter, eps);
        subdivideToSurfaceRecurse(midPt, p1, outPts, adapter, eps);
    }
    outPts.push_back(p1);
}
    
void SubdivideEdgesToSurface(const VectorRing &inPts,VectorRing &outPts,bool closed,CoordSystemDisplayAdapter *adapter,float eps)
{
    for (int ii=0;ii<(closed ? inPts.size() : inPts.size()-1);ii++)
    {
        const Point2f &p0 = inPts[ii];
        const Point2f &p1 = inPts[(ii+1)%inPts.size()];
        outPts.push_back(p0);
        subdivideToSurfaceRecurse(p0,p1,outPts,adapter,eps);
    }
}
    
// Great circle version
void subdivideToSurfaceRecurseGC(Point3f p0,Point3f p1,Point3fVector &outPts,CoordSystemDisplayAdapter *adapter,float eps,float surfOffset,int minPts)
{
    // If the difference is greater than 180, then this is probably crossing the date line
    //  in which case we'll just leave it alone.
    // Note: Probably not right
//    if (std::abs(p0.x() - p1.x()) > M_PI)
//        return;
    
    Point3f midP = (p0+p1)/2.0;
    Point3f midOnSphere = midP.normalized() * (1.0 + surfOffset);
    float dist2 = (midOnSphere - midP).squaredNorm();
    if (dist2 > eps*eps || minPts > 0)
    {
        subdivideToSurfaceRecurseGC(p0, midOnSphere, outPts, adapter, eps, surfOffset,minPts/2);
        subdivideToSurfaceRecurseGC(midOnSphere, p1, outPts, adapter, eps, surfOffset,minPts/2);
    }
    outPts.push_back(p1);
}

void SubdivideEdgesToSurfaceGC(const VectorRing &inPts,Point3fVector &outPts,bool closed,CoordSystemDisplayAdapter *adapter,float eps,float surfOffset,int minPts)
{
    if (!adapter || inPts.empty())
        return;
    if (inPts.size() < 2)
    {
        const Point2f &p0 = inPts[0];
        Point3f dp0 = adapter->localToDisplay(adapter->getCoordSystem()->geographicToLocal(GeoCoord(p0.x(),p0.y())));
        outPts.push_back(dp0);        
        return;
    }
    
    for (int ii=0;ii<(closed ? inPts.size() : inPts.size()-1);ii++)
    {
        const Point2f &p0 = inPts[ii];
        const Point2f &p1 = inPts[(ii+1)%inPts.size()];
        Point3f dp0 = adapter->localToDisplay(adapter->getCoordSystem()->geographicToLocal(GeoCoord(p0.x(),p0.y())));
        dp0 = dp0.normalized() * (1.0 + surfOffset);
        Point3f dp1 = adapter->localToDisplay(adapter->getCoordSystem()->geographicToLocal(GeoCoord(p1.x(),p1.y())));
        dp1 = dp1.normalized() * (1.0 + surfOffset);
        outPts.push_back(dp0);
        subdivideToSurfaceRecurseGC(dp0,dp1,outPts,adapter,eps,surfOffset,minPts);
    }    
}

    
VectorShape::VectorShape()
{
}
   
VectorShape::~VectorShape()
{
}
    
void VectorShape::setAttrDict(const Dictionary &newDict)
{
    attrDict = newDict;
}
    
Dictionary *VectorShape::getAttrDict()
{
    return &attrDict;
}
    
VectorTriangles::VectorTriangles()
{
}
    
VectorTriangles::~VectorTriangles()
{
}
    
VectorTrianglesRef VectorTriangles::createTriangles()
{
    return VectorTrianglesRef(new VectorTriangles());
}
    
GeoMbr VectorTriangles::calcGeoMbr()
{
    return geoMbr;
}
    
void VectorTriangles::getTriangle(int which,VectorRing &ring)
{
    if (which < 0 || which >= tris.size())
        return;

    ring.reserve(3);
    Triangle &tri = tris[which];
    for (unsigned int ii=0;ii<3;ii++)
    {
        Point3f pt = pts[tri.pts[ii]];
        ring.push_back(Point2f(pt.x(),pt.y()));
    }
}
    
void VectorTriangles::initGeoMbr()
{
    for (unsigned int ii=0;ii<pts.size();ii++)
        geoMbr.addGeoCoord(GeoCoord(pts[ii].x(),pts[ii].y()));
}
    
VectorAreal::VectorAreal()
{
}
    
VectorAreal::~VectorAreal()
{
}
    
VectorArealRef VectorAreal::createAreal()
{
    return VectorArealRef(new VectorAreal());
}

    
bool VectorAreal::pointInside(GeoCoord coord)
{
    if (geoMbr.inside(coord))
    {
        for (unsigned int ii=0;ii<loops.size();ii++)
            if (PointInPolygon(coord,loops[ii]))
                return true;
    }
    
    return false;
}
    
GeoMbr VectorAreal::calcGeoMbr() 
{ 
    return geoMbr; 
}
    
void VectorAreal::initGeoMbr()
{
    for (unsigned int ii=0;ii<loops.size();ii++)
        geoMbr.addGeoCoords(loops[ii]);
}
    
void VectorAreal::subdivide(float maxLen)
{
    for (unsigned int ii=0;ii<loops.size();ii++)
    {
        VectorRing newPts;
        SubdivideEdges(loops[ii], newPts, true, maxLen);
        loops[ii] = newPts;
    }
}

VectorLinear::VectorLinear()
{
}

VectorLinear::~VectorLinear()
{
}
    
VectorLinearRef VectorLinear::createLinear()
{
    return VectorLinearRef(new VectorLinear());
}
    
GeoMbr VectorLinear::calcGeoMbr() 
{ 
    return geoMbr; 
}

void VectorLinear::initGeoMbr()
{
    geoMbr.addGeoCoords(pts);
}
    
void VectorLinear::subdivide(float maxLen)
{
    VectorRing newPts;
    SubdivideEdges(pts, newPts, false, maxLen);
    pts = newPts;
}
    
VectorPoints::VectorPoints()
{
}
    
VectorPoints::~VectorPoints()
{
}
    
VectorPointsRef VectorPoints::createPoints()
{
    return VectorPointsRef(new VectorPoints());
}

GeoMbr VectorPoints::calcGeoMbr() 
{ 
    if (!geoMbr.valid())
        initGeoMbr();
    return geoMbr;
}

void VectorPoints::initGeoMbr()
{
    geoMbr.addGeoCoords(pts);
}

typedef enum {FileVecPoints=20,FileVecLinear,FileVecAreal,FileVecMesh} VectorIdentType;
    
bool VectorWriteFile(const std::string &fileName,ShapeSet &shapes)
{
    FILE *fp = fopen(fileName.c_str(),"w");
    if (!fp)
        return false;

    try {
        int numFeatures = (int)shapes.size();
        if (fwrite(&numFeatures,sizeof(int),1, fp) != 1)
            throw 1;

        for (ShapeSet::iterator it = shapes.begin(); it != shapes.end(); ++it)
        {
            VectorShapeRef shape = *it;
            
            // They all have a dictionary
            Dictionary *dict = shape->getAttrDict();
            MutableRawData dictData;
            dict->asRawData(&dictData);
            int dataLen = (int)dictData.getLen();
            if (fwrite(&dataLen,sizeof(int),1,fp) != 1)
                throw 1;
            if (dataLen > 0)
                if (fwrite(dictData.getRawData(),dataLen,1,fp) != 1)
                    throw 1;
            
            VectorPointsRef pts = std::dynamic_pointer_cast<VectorPoints>(shape);
            VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(shape);
            VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(shape);
            VectorTrianglesRef mesh = std::dynamic_pointer_cast<VectorTriangles>(shape);
            if (pts.get())
            {
                unsigned short dataType = FileVecPoints;
                if (fwrite(&dataType,sizeof(short),1,fp) != 1)
                    throw 1;
                
                unsigned int numPts = (int)pts->pts.size();
                if (fwrite(&numPts,sizeof(unsigned int),1,fp) != 1)
                    throw 1;
                if (fwrite(&pts->pts[0],2*sizeof(float),numPts,fp) != numPts)
                    throw 1;
            } else if (lin.get())
            {
                unsigned short dataType = FileVecLinear;
                if (fwrite(&dataType,sizeof(short),1,fp) != 1)
                    throw 1;
                
                unsigned int numPts = (unsigned int)lin->pts.size();
                if (fwrite(&numPts,sizeof(unsigned int),1,fp) != 1)
                    throw 1;
                if (fwrite(&lin->pts[0],2*sizeof(float),numPts,fp) != numPts)
                    throw 1;
                
            } else if (ar.get())
            {
                unsigned short dataType = FileVecAreal;
                if (fwrite(&dataType,sizeof(short),1,fp) != 1)
                    throw 1;
                
                unsigned int numLoops = (unsigned int)ar->loops.size();
                if (fwrite(&numLoops,sizeof(int),1,fp) != 1)
                    throw 1;
                for (unsigned int ii=0;ii<numLoops;ii++)
                {
                    VectorRing &ring = ar->loops[ii];
                    unsigned int numPts = (unsigned int)ring.size();
                    if (fwrite(&numPts,sizeof(unsigned int),1,fp) != 1)
                        throw 1;
                    if (fwrite(&ring[0],2*sizeof(float),numPts,fp) != numPts)
                        throw 1;
                }
                
            } else if (mesh.get())
            {
                unsigned short dataType = FileVecMesh;
                if (fwrite(&dataType,sizeof(short),1,fp) != 1)
                    throw 1;
                
                unsigned int numPts = (unsigned int)mesh->pts.size();
                if (fwrite(&numPts,sizeof(unsigned int),1,fp) != 1)
                    throw 1;
                if (fwrite(&mesh->pts[0],3*sizeof(float),numPts,fp) != numPts)
                    throw 1;
                
                unsigned int numTri = (unsigned int)mesh->tris.size();
                if (fwrite(&numTri,sizeof(unsigned int),1,fp) != 1)
                    throw 1;
                if (fwrite(&mesh->tris[0],3*sizeof(unsigned int),numTri,fp) != numTri)
                    throw 1;
            } else {
//                NSLog(@"Tried to write unknown object in VectorWriteFile");
                throw 1;
            }
        }
    }
    catch (...)
    {
        fclose(fp);
        return false;
    }
    
    fclose(fp);
    return true;
}

bool VectorReadFile(const std::string &fileName,ShapeSet &shapes)
{
    FILE *fp = fopen(fileName.c_str(),"r");
    if (!fp)
        return false;
    
    try {
        int numFeatures;
        if (fread(&numFeatures, sizeof(int), 1, fp) != 1)
            throw 1;
        
        for (unsigned int ii=0;ii<numFeatures;ii++)
        {
            // Dictionary first
            int dataLen;
            if (fread(&dataLen, sizeof(int), 1, fp) != 1)
                throw 1;
            Dictionary *dict = NULL;
            if (dataLen > 0)
            {
                RawDataWrapper *rawData = RawDataFromFile(fp, dataLen);
                if (!rawData)
                    throw 1;
                dict = new Dictionary(rawData);
                delete rawData;
            }
            
            // Now for the type
            unsigned short dataType;
            if (fread(&dataType,sizeof(unsigned short),1,fp) != 1)
                throw 1;
            
            switch (dataType)
            {
                case FileVecPoints:
                {
                    VectorPointsRef pts(VectorPoints::createPoints());
                    if (dict)
                    	pts->setAttrDict(*dict);

                    unsigned int numPts;
                    if (fread(&numPts,sizeof(unsigned int),1,fp) != 1)
                        throw 1;
                    pts->pts.resize(numPts);
                    if (fread(&pts->pts[0],2*sizeof(float),numPts,fp) != numPts)
                        throw 1;
                    
                    pts->initGeoMbr();
                    shapes.insert(pts);
                }
                    break;
                case FileVecLinear:
                {
                    VectorLinearRef lin(VectorLinear::createLinear());
                    if (dict)
                    	lin->setAttrDict(*dict);

                    unsigned int numPts;
                    if (fread(&numPts,sizeof(unsigned int),1,fp) != 1)
                        throw 1;
                    lin->pts.resize(numPts);
                    if (fread(&lin->pts[0],2*sizeof(float),numPts,fp) != numPts)
                        throw 1;
                    
                    lin->initGeoMbr();
                    shapes.insert(lin);
                }
                    break;
                case FileVecAreal:
                {
                    VectorArealRef ar(VectorAreal::createAreal());
                    if (dict)
                    	ar->setAttrDict(*dict);

                    unsigned int numLoops;
                    if (fread(&numLoops,sizeof(unsigned int),1,fp) != 1)
                        throw 1;
                    ar->loops.resize(numLoops);
                    
                    for (unsigned int ii=0;ii<numLoops;ii++)
                    {
                        VectorRing &ring = ar->loops[ii];
                        unsigned int numPts;
                        if (fread(&numPts,sizeof(unsigned int),1,fp) != 1)
                            throw 1;
                        ring.resize(numPts);
                        if (fread(&ring[0],2*sizeof(float),numPts,fp) != numPts)
                            throw 1;
                    }
                    
                    ar->initGeoMbr();
                    shapes.insert(ar);
                }
                    break;
                case FileVecMesh:
                {
                    VectorTrianglesRef mesh(VectorTriangles::createTriangles());
                    if (dict)
                    	mesh->setAttrDict(*dict);
                    
                    unsigned int numPts;
                    if (fread(&numPts,sizeof(unsigned int),1,fp) != 1)
                        throw 1;
                    mesh->pts.resize(numPts);
                    if (fread(&mesh->pts[0],3*sizeof(float),numPts,fp) != numPts)
                        throw 1;
                    
                    unsigned int numTri;
                    if (fread(&numTri,sizeof(unsigned int),1,fp) != 1)
                        throw 1;
                    mesh->tris.resize(numTri);
                    if (fread(&mesh->tris[0],3*sizeof(unsigned int),numTri,fp) != numTri)
                        throw 1;
                    
                    mesh->initGeoMbr();
                    shapes.insert(mesh);
                }
                    break;
                default:
//                    NSLog(@"Unknown data type in VectorReadFile()");
                    throw 1;
                    break;
            }
            
            if (dict)
                delete dict;
        }
    }
    catch (...)
    {
        fclose(fp);
        return false;
    }
    
    fclose(fp);
    return true;
}


//// Parse a single coordinate out of an array
//bool VectorParseCoord(Point2f &coord,NSArray *coords)
//{
//    if (![coords isKindOfClass:[NSArray class]] || ([coords count] != 2 && [coords count] != 3))
//        return false;
//    coord.x() = DegToRad([[coords objectAtIndex:0] floatValue]);
//    coord.y() = DegToRad([[coords objectAtIndex:1] floatValue]);
//    
//    return true;
//}
//    
//// Parse coordinates out of a coordinate string
//bool VectorParseCoords(VectorRing &coords,NSArray *coordArray)
//{
//    if (![coordArray isKindOfClass:[NSArray class]])
//        return false;
//    
//    // Look at the type of the first object.  If it's not an array, we've got a coord.
//    NSObject *firstObj = [coordArray objectAtIndex:0];
//    if (![firstObj isKindOfClass:[NSArray class]])
//    {
//        coords.resize(1);
//        if (!VectorParseCoord(coords[0], coordArray))
//            return false;
//    } else {
//        coords.resize([coordArray count]);
//        int ci = 0;
//        for (NSArray *coord in coordArray)
//        {
//            if (!VectorParseCoord(coords[ci], coord))
//                return false;
//            ci++;
//        }
//    }
//    
//    return true;
//}
//
//// Parse geometry objects out of the JSON
//bool VectorParseGeometry(ShapeSet &shapes,NSDictionary *jsonDict)
//{
//    NSString *type = [jsonDict objectForKey:@"type"];
//    if (![type isKindOfClass:[NSString class]])
//        return false;
//
//    if (![type compare:@"Point"])
//    {
//        VectorRing coords;
//        if (!VectorParseCoords(coords,[jsonDict objectForKey:@"coordinates"]))
//            return false;
//        if (coords.size() != 1)
//            return false;
//        VectorPointsRef pts = VectorPoints::createPoints();
//        pts->pts.push_back(coords[0]);
//        pts->initGeoMbr();
//        shapes.insert(pts);
//    } else if (![type compare:@"LineString"])
//    {
//        VectorRing coords;
//        if (!VectorParseCoords(coords,[jsonDict objectForKey:@"coordinates"]))
//            return false;
//        if (coords.empty())
//            return false;
//        VectorLinearRef lin = VectorLinear::createLinear();
//        lin->pts = coords;
//        lin->initGeoMbr();
//        shapes.insert(lin);
//    } else if (![type compare:@"Polygon"])
//    {
//        NSArray *coordsArray = [jsonDict objectForKey:@"coordinates"];
//        if (![coordsArray isKindOfClass:[NSArray class]])
//            return false;
//        VectorArealRef ar = VectorAreal::createAreal();
//        for (NSArray *coordsEntry in coordsArray)
//        {
//            VectorRing coords;
//            if (!VectorParseCoords(coords, coordsEntry))
//                return false;
//            if (coords.empty())
//                return false;
//            ar->loops.push_back(coords);
//        }
//        ar->initGeoMbr();
//        shapes.insert(ar);
//    } else if (![type compare:@"MultiPoint"])
//    {
//        VectorRing coords;
//        if (!VectorParseCoords(coords,[jsonDict objectForKey:@"coordinates"]))
//            return false;
//        if (coords.empty())
//            return false;
//        VectorPointsRef pts = VectorPoints::createPoints();
//        pts->pts = coords;
//        pts->initGeoMbr();
//        shapes.insert(pts);
//    } else if (![type compare:@"MultiLineString"])
//    {
//        NSArray *coordsArray = [jsonDict objectForKey:@"coordinates"];
//        if (![coordsArray isKindOfClass:[NSArray class]])
//            return false;
//        for (NSArray *coordsEntry in coordsArray)
//        {
//            VectorRing coords;
//            if (!VectorParseCoords(coords, coordsEntry) || coords.empty())
//                return false;
//            VectorLinearRef lin = VectorLinear::createLinear();
//            lin->pts = coords;
//            lin->initGeoMbr();
//            shapes.insert(lin);
//        }
//    } else if (![type compare:@"MultiPolygon"])
//    {
//        NSArray *polyArray = [jsonDict objectForKey:@"coordinates"];
//        if (![polyArray isKindOfClass:[NSArray class]])
//            return false;
//        for (NSArray *polyEntry in polyArray)
//        {
//            if ([polyEntry isKindOfClass:[NSArray class]])
//            {
//                VectorArealRef ar = VectorAreal::createAreal();
//                for (NSArray *coordsEntry in polyEntry)
//                {
//                    VectorRing coords;
//                    if (!VectorParseCoords(coords, coordsEntry) || coords.empty())
//                        return false;
//                    ar->loops.push_back(coords);
//                }
//                ar->initGeoMbr();
//                shapes.insert(ar);                
//            } else
//                return false;
//        }
//    } else if (![type compare:@"GeometryCollection"])
//    {
//        // Recurse down for the other geometry
//        NSArray *geom = [jsonDict objectForKey:@"geometries"];
//        if (![geom isKindOfClass:[NSArray class]])
//            return false;
//        for (NSDictionary *geomDict in geom)
//        {
//            if (![geomDict isKindOfClass:[NSDictionary class]])
//                return false;
//            if (!VectorParseGeometry(shapes, geomDict))
//                return false;
//        }
//    } else
//        return false;
//    
//    return true;
//}
//    
//// Parse a single feature out of geoJSON
//bool VectorParseFeature(ShapeSet &shapes,NSDictionary *jsonDict)
//{
//    NSString *idStr = [jsonDict objectForKey:@"id"];
//    NSDictionary *geom = [jsonDict objectForKey:@"geometry"];
//    NSDictionary *prop = [jsonDict objectForKey:@"properties"];
//
//    if (![geom isKindOfClass:[NSDictionary class]])
//        return false;
//
//    // Parse out the geometry.  May result in multiple shapes
//    if (!VectorParseGeometry(shapes, geom))
//        return false;
//    
//    // Apply the attributes if there are any
//    if ([prop isKindOfClass:[NSDictionary class]])
//        for (ShapeSet::iterator it = shapes.begin(); it != shapes.end(); ++it)
//            (*it)->setAttrDict([NSMutableDictionary dictionaryWithDictionary:prop]);
//    
//    // Apply the identity if there is one
//    if ([idStr isKindOfClass:[NSString class]])
//        for (ShapeSet::iterator it = shapes.begin(); it != shapes.end(); ++it)
//            [(*it)->getAttrDict() setObject:idStr forKey:@"id"];
//    
//    return true;
//}
//    
//// Parse a set of features out of GeoJSON using an NSDictionary
//bool VectorParseGeoJSON(ShapeSet &shapes,NSDictionary *jsonDict)
//{
//    NSString *type = [jsonDict objectForKey:@"type"];
//    if (![type isKindOfClass:[NSString class]])
//        return false;
//    
//    if (![type compare:@"FeatureCollection"])
//    {
//        NSArray *features = [jsonDict objectForKey:@"features"];
//        if (![features isKindOfClass:[NSArray class]])
//            return false;
//        
//        for (NSDictionary *featDict in features)
//        {
//            if (![featDict isKindOfClass:[NSDictionary class]])
//                return false;
//            
//            ShapeSet featShapes;
//            if (VectorParseFeature(featShapes,featDict))
//                shapes.insert(featShapes.begin(),featShapes.end());
//            else
//                return false;
//        }
//    } 
//        
//    return true;
//}
    
using namespace libjson;
    
// Parse properties out of a node
bool VectorParseProperties(JSONNode node,Dictionary &dict)
{
    for (JSONNode::const_iterator it = node.begin();
         it != node.end(); ++it)
    {
        json_string name = it->name();
        if (!name.empty())
        {
            switch (it->type())
            {
                case JSON_STRING:
                {
                    json_string val = it->as_string();
                    dict.setString(name,val);
                }
                    break;
                case JSON_NUMBER:
                {
                    double val = it->as_float();
                    dict.setDouble(name, val);
                }
                    break;
                case JSON_BOOL:
                {
                    bool val = it->as_bool();
                    dict.setInt(name, (int)val);
                }
                    break;
            }
        }
    }
    
    return true;
}
    
// Parse coordinate list out of a node
bool VectorParseCoordinates(JSONNode node,VectorRing &pts)
{
    for (JSONNode::const_iterator it = node.begin();
         it != node.end(); ++it)
    {
        if (it->type() == JSON_ARRAY)
        {
            if (!VectorParseCoordinates(*it, pts))
                return false;
            continue;
        }

        // We're expecting two numbers here
        if (it->type() == JSON_NUMBER)
        {
            if (node.size() != 2)
                return false;
            
            float lon = it->as_float();  ++it;
            float lat = it->as_float();
            pts.push_back(GeoCoord::CoordFromDegrees(lon,lat));

            continue;
        }
        
        // Got something unexpected
        return false;
    }
    
    return true;
}

// Parse geometry out of a node
bool VectorParseGeometry(JSONNode node,ShapeSet &shapes)
{
    // Let's look for type and coordinates
    JSONNode::const_iterator typeIt = node.end();
    JSONNode::const_iterator coordIt = node.end();
    JSONNode::const_iterator geomCollectIt = node.end();
    for (JSONNode::const_iterator it = node.begin();
         it != node.end(); ++it)
    {
        if (!it->name().compare("type"))
            typeIt = it;
        else if (!it->name().compare("coordinates"))
            coordIt = it;
        else if (!it->name().compare("geometries"))
            geomCollectIt = it;
    }
    
    if (typeIt == node.end())
        return false;
    
    json_string type = typeIt->as_string();
    if (!type.compare("Point"))
    {
        if (coordIt == node.end() || coordIt->type() != JSON_ARRAY)
            return false;

        VectorPointsRef pts = VectorPoints::createPoints();
        if (!VectorParseCoordinates(*coordIt,pts->pts))
            return false;
        pts->initGeoMbr();
        shapes.insert(pts);
        
        return true;
    } else if (!type.compare("LineString"))
    {
        if (coordIt == node.end() || coordIt->type() != JSON_ARRAY)
            return false;
        
        VectorLinearRef lin = VectorLinear::createLinear();
        if (!VectorParseCoordinates(*coordIt,lin->pts))
            return false;
        lin->initGeoMbr();
        shapes.insert(lin);

        return true;
    } else if (!type.compare("Polygon"))
    {
        // This should be an array of array of coordinates
        if (coordIt == node.end() || coordIt->type() != JSON_ARRAY)
            return false;
        VectorArealRef ar = VectorAreal::createAreal();
        int numLoops = 0;
        for (JSONNode::const_iterator coordEntryIt = coordIt->begin();
             coordEntryIt != coordIt->end(); ++coordEntryIt, numLoops++)
        {
            if (coordEntryIt->type() != JSON_ARRAY)
                return false;
        
            ar->loops.resize(numLoops+1);
            if (!VectorParseCoordinates(*coordEntryIt,ar->loops[numLoops]))
                return false;
        }
        
        ar->initGeoMbr();
        shapes.insert(ar);
        
        return true;
    } else if (!type.compare("MultiPoint"))
    {
        if (coordIt == node.end() || coordIt->type() != JSON_ARRAY)
            return false;
        
        VectorPointsRef pts = VectorPoints::createPoints();
        if (!VectorParseCoordinates(*coordIt,pts->pts))
            return false;
        pts->initGeoMbr();
        shapes.insert(pts);
        
        return true;        
    } else if (!type.compare("MultiLineString"))
    {
        // This should be an array of array of coordinates
        if (coordIt == node.end() || coordIt->type() != JSON_ARRAY)
            return false;
        for (JSONNode::const_iterator coordEntryIt = coordIt->begin();
             coordEntryIt != coordIt->end(); ++coordEntryIt)
        {
            if (coordEntryIt->type() != JSON_ARRAY)
                return false;
            
            VectorLinearRef lin = VectorLinear::createLinear();
            if (!VectorParseCoordinates(*coordEntryIt, lin->pts))
                return false;
            lin->initGeoMbr();
            shapes.insert(lin);
        }
        
        return true;
    } else if (!type.compare("MultiPolygon"))
    {
        // This should be an array of array of coordinates
        if (coordIt == node.end() ||  coordIt->type() != JSON_ARRAY)
            return false;

        for (JSONNode::const_iterator polyIt = coordIt->begin();
             polyIt != coordIt->end(); ++polyIt)
        {
            VectorArealRef ar = VectorAreal::createAreal();
            int numLoops = 0;
            for (JSONNode::const_iterator coordEntryIt = polyIt->begin();
                 coordEntryIt != polyIt->end(); ++coordEntryIt, numLoops++)
            {
                if (coordEntryIt->type() != JSON_ARRAY)
                    return false;
                
                ar->loops.resize(numLoops+1);
                if (!VectorParseCoordinates(*coordEntryIt,ar->loops[numLoops]))
                    return false;
            }
            
            ar->initGeoMbr();
            shapes.insert(ar);
        }
        
        return true;        
    } else if (!type.compare("GeometryCollection"))
    {
        if (geomCollectIt == node.end() || geomCollectIt->type() != JSON_ARRAY)
            return false;
        for (JSONNode::const_iterator geomIt = geomCollectIt->begin();
             geomIt != geomCollectIt->end(); ++geomIt)
            if (!VectorParseGeometry(*geomIt,shapes))
                return false;
        
        return true;
    }
    
    return false;
}
    
// Parse a single feature
bool VectorParseFeature(JSONNode node,ShapeSet &shapes)
{
    JSONNode::const_iterator typeIt = node.end();
    JSONNode::const_iterator geomIt = node.end();
    JSONNode::const_iterator propIt = node.end();
    
    for (JSONNode::const_iterator it = node.begin();
         it != node.end(); ++it)
    {
        if (!it->name().compare("type"))
            typeIt = it;
        else if (!it->name().compare("geometry"))
            geomIt = it;
        else if (!it->name().compare("properties"))
            propIt = it;
    }
    if (typeIt == node.end() || geomIt == node.end() || propIt == node.end())
        return false;
    
    // Expecting this to be a feature with a geometry and properties node
    json_string type = typeIt->as_string();
    if (type.compare("Feature"))
        return false;

    // Parse the properties, then the geometry
    Dictionary properties;
    VectorParseProperties(*propIt,properties);
    ShapeSet newShapes;
    if (!VectorParseGeometry(*geomIt,newShapes))
        return false;
    // Apply the properties to the geometry
    for (ShapeSet::iterator sit = newShapes.begin(); sit != newShapes.end(); ++sit)
        (*sit)->setAttrDict(properties);
    
    shapes.insert(newShapes.begin(), newShapes.end());
    return true;
}

// Parse an array of features
bool VectorParseFeatures(JSONNode node,ShapeSet &shapes)
{
    for (JSONNode::const_iterator it = node.begin();it != node.end(); ++it)
    {
        // Not sure what this would be
        if (it->type() != JSON_NODE)
            return false;
        if (!VectorParseFeature(*it,shapes))
            return false;
    }
    
    return true;
}

// Recursively parse a feature collection
bool VectorParseTopNode(JSONNode node,ShapeSet &shapes,JSONNode &crs)
{
    JSONNode::const_iterator typeIt = node.end();
    JSONNode::const_iterator featIt = node.end();
    
    for (JSONNode::const_iterator it = node.begin();
         it != node.end(); ++it)
    {
        if (!it->name().compare("type"))
            typeIt = it;
        else if (!it->name().compare("features"))
            featIt = it;
        else if (!it->name().compare("crs"))
            crs = *it;
    }
    if (typeIt == node.end())
        return false;
    
    json_string type;
    type = typeIt->as_string();
    if (!type.compare("FeatureCollection"))
    {
        // Expecting a features node
        if (featIt == node.end() || featIt->type() != JSON_ARRAY)
            return false;
        return VectorParseFeatures(*featIt,shapes);
    } else
        return false;

    return false;
}

// Parse the name out of a CRS in a GeoJSON file
bool VectorParseGeoJSONCRS(JSONNode node,std::string &crsName)
{
    JSONNode::const_iterator typeIt = node.end();
    JSONNode::const_iterator propIt = node.end();
    
    for (JSONNode::const_iterator it = node.begin();
         it != node.end(); ++it)
    {
        if (!it->name().compare("type"))
            typeIt = it;
        else if (!it->name().compare("properties"))
            propIt = it;
    }
    if (typeIt == node.end())
        return false;
    
    json_string type;
    type = typeIt->as_string();
    if (!type.compare("name"))
    {
        // Expecting a features node
        if (propIt == node.end() || propIt->type() != JSON_NODE)
            return false;
        
        for (JSONNode::const_iterator it = propIt->begin(); it != propIt->end(); ++it)
        {
            if (!it->name().compare("name"))
            {
                if (it->type() != JSON_STRING)
                    return false;
                crsName = it->as_string();
                return true;
            }
        }
    } else
        return false;
    
    return false;
}
    
// Parse a set of features out of GeoJSON, using libjson
bool VectorParseGeoJSON(ShapeSet &shapes,const std::string &str,std::string &crs)
{
    json_string json = str;
    JSONNode topNode = libjson::parse(json);

    JSONNode crsNode;
    if (!VectorParseTopNode(topNode,shapes,crsNode))
    {
//        NSLog(@"Failed to parse JSON in VectorParseGeoJSON");
        return false;
    }

    std::string crsName;
    if (VectorParseGeoJSONCRS(crsNode,crsName))
    {
        if (!crsName.empty())
            crs = crsName;
    }
    
    return true;
}
    
bool VectorParseGeoJSONAssembly(const std::string &str,std::map<std::string,ShapeSet> &shapes)
{
    json_string json = str;
    
    JSONNode topNode = libjson::parse(json);
    JSONNode crsNode;

    for (JSONNode::iterator nodeIt = topNode.begin();
         nodeIt != topNode.end(); ++nodeIt)
    {
        if (nodeIt->type() == JSON_NODE)
        {
            ShapeSet theseShapes;
            if (VectorParseTopNode(*nodeIt,theseShapes,crsNode))
            {
                json_string name = nodeIt->name();
                std::string nameStr = to_std_string(name);
                shapes[nameStr] = theseShapes;
            } else
                return false;
        }
    }
    
    return true;
}
    
}
