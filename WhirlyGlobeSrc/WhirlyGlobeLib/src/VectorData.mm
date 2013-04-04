/*
 *  VectorData.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/7/11.
 *  Copyright 2011-2012 mousebird consulting
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

#import "VectorData.h"
#import "ShapeReader.h"

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
void subdivideToSurfaceRecurseGC(Point3f p0,Point3f p1,std::vector<Point3f> &outPts,CoordSystemDisplayAdapter *adapter,float eps,float surfOffset)
{
    // If the difference is greater than 180, then this is probably crossing the date line
    //  in which case we'll just leave it alone.
    // Note: Probably not right
    if (std::abs(p0.x() - p1.x()) > M_PI)
        return;
    
    Point3f midP = (p0+p1)/2.0;
    Point3f midOnSphere = midP.normalized() * (1.0 + surfOffset);
    float dist2 = (midOnSphere - midP).squaredNorm();
    if (dist2 > eps*eps)
    {
        subdivideToSurfaceRecurseGC(p0, midOnSphere, outPts, adapter, eps, surfOffset);
        subdivideToSurfaceRecurseGC(midOnSphere, p1, outPts, adapter, eps, surfOffset);
    }
    outPts.push_back(p1);
}

void SubdivideEdgesToSurfaceGC(const VectorRing &inPts,std::vector<Point3f> &outPts,bool closed,CoordSystemDisplayAdapter *adapter,float eps,float surfOffset)
{
    for (int ii=0;ii<(closed ? inPts.size() : inPts.size()-1);ii++)
    {
        const Point2f &p0 = inPts[ii];
        const Point2f &p1 = inPts[(ii+1)%inPts.size()];
        Point3f dp0 = adapter->localToDisplay(adapter->getCoordSystem()->geographicToLocal(GeoCoord(p0.x(),p0.y())));
        dp0 = dp0.normalized() * (1.0 + surfOffset);
        Point3f dp1 = adapter->localToDisplay(adapter->getCoordSystem()->geographicToLocal(GeoCoord(p1.x(),p1.y())));
        dp1 = dp1.normalized() * (1.0 + surfOffset);
        outPts.push_back(dp0);
        subdivideToSurfaceRecurseGC(dp0,dp1,outPts,adapter,eps,surfOffset);
    }    
}

    
VectorShape::VectorShape()
{
    attrDict = nil;
}
   
VectorShape::~VectorShape()
{
}
    
void VectorShape::setAttrDict(NSMutableDictionary *newDict)
{ 
    attrDict = newDict;  
}
    
NSMutableDictionary *VectorShape::getAttrDict()    
{
    return attrDict;
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

// Parse a single coordinate out of an array
bool VectorParseCoord(Point2f &coord,NSArray *coords)
{
    if (![coords isKindOfClass:[NSArray class]] || [coords count] != 2)
        return false;
    coord.x() = DegToRad([[coords objectAtIndex:0] floatValue]);
    coord.y() = DegToRad([[coords objectAtIndex:1] floatValue]);
    
    return true;
}
    
// Parse coordinates out of a coordinate string
bool VectorParseCoords(VectorRing &coords,NSArray *coordArray)
{
    if (![coordArray isKindOfClass:[NSArray class]])
        return false;
    
    // Look at the type of the first object.  If it's not an array, we've got a coord.
    NSObject *firstObj = [coordArray objectAtIndex:0];
    if (![firstObj isKindOfClass:[NSArray class]])
    {
        coords.resize(1);
        if (!VectorParseCoord(coords[0], coordArray))
            return false;
    } else {
        coords.resize([coordArray count]);
        int ci = 0;
        for (NSArray *coord in coordArray)
        {
            if (!VectorParseCoord(coords[ci], coord))
                return false;
            ci++;
        }
    }
    
    return true;
}

// Parse geometry objects out of the JSON
bool VectorParseGeometry(ShapeSet &shapes,NSDictionary *jsonDict)
{
    NSString *type = [jsonDict objectForKey:@"type"];
    if (![type isKindOfClass:[NSString class]])
        return false;

    if (![type compare:@"Point"])
    {
        VectorRing coords;
        if (!VectorParseCoords(coords,[jsonDict objectForKey:@"coordinates"]))
            return false;
        if (coords.size() != 1)
            return false;
        VectorPointsRef pts = VectorPoints::createPoints();
        pts->pts.push_back(coords[0]);
        pts->initGeoMbr();
        shapes.insert(pts);
    } else if (![type compare:@"LineString"])
    {
        VectorRing coords;
        if (!VectorParseCoords(coords,[jsonDict objectForKey:@"coordinates"]))
            return false;
        if (coords.empty())
            return false;
        VectorLinearRef lin = VectorLinear::createLinear();
        lin->pts = coords;
        lin->initGeoMbr();
        shapes.insert(lin);
    } else if (![type compare:@"Polygon"])
    {
        NSArray *coordsArray = [jsonDict objectForKey:@"coordinates"];
        if (![coordsArray isKindOfClass:[NSArray class]])
            return false;
        VectorArealRef ar = VectorAreal::createAreal();
        for (NSArray *coordsEntry in coordsArray)
        {
            VectorRing coords;
            if (!VectorParseCoords(coords, coordsEntry))
                return false;
            if (coords.empty())
                return false;
            ar->loops.push_back(coords);
        }
        ar->initGeoMbr();
        shapes.insert(ar);
    } else if (![type compare:@"MultiPoint"])
    {
        VectorRing coords;
        if (!VectorParseCoords(coords,[jsonDict objectForKey:@"coordinates"]))
            return false;
        if (coords.empty())
            return false;
        VectorPointsRef pts = VectorPoints::createPoints();
        pts->pts = coords;
        pts->initGeoMbr();
        shapes.insert(pts);
    } else if (![type compare:@"MultiLineString"])
    {
        NSArray *coordsArray = [jsonDict objectForKey:@"coordinates"];
        if (![coordsArray isKindOfClass:[NSArray class]])
            return false;
        VectorArealRef ar = VectorAreal::createAreal();
        for (NSArray *coordsEntry in coordsArray)
        {
            VectorRing coords;
            if (!VectorParseCoords(coords, coordsEntry) || coords.empty())
                return false;
            VectorLinearRef lin = VectorLinear::createLinear();
            lin->pts = coords;
            lin->initGeoMbr();
            shapes.insert(lin);
        }
    } else if (![type compare:@"MultiPolygon"])
    {
        NSArray *polyArray = [jsonDict objectForKey:@"coordinates"];
        if (![polyArray isKindOfClass:[NSArray class]])
            return false;
        for (NSArray *polyEntry in polyArray)
        {
            if ([polyEntry isKindOfClass:[NSArray class]])
            {
                VectorArealRef ar = VectorAreal::createAreal();
                for (NSArray *coordsEntry in polyEntry)
                {
                    VectorRing coords;
                    if (!VectorParseCoords(coords, coordsEntry) || coords.empty())
                        return false;
                    ar->loops.push_back(coords);
                }
                ar->initGeoMbr();
                shapes.insert(ar);                
            } else
                return false;
        }
    } else if (![type compare:@"GeometryCollection"])
    {
        // Recurse down for the other geometry
        NSArray *geom = [jsonDict objectForKey:@"geometries"];
        if (![geom isKindOfClass:[NSArray class]])
            return false;
        for (NSDictionary *geomDict in geom)
        {
            if (![geomDict isKindOfClass:[NSDictionary class]])
                return false;
            if (!VectorParseGeometry(shapes, geomDict))
                return false;
        }
    } else
        return false;
    
    return true;
}
    
// Parse a single feature out of geoJSON
bool VectorParseFeature(ShapeSet &shapes,NSDictionary *jsonDict)
{
    NSString *idStr = [jsonDict objectForKey:@"id"];
    NSDictionary *geom = [jsonDict objectForKey:@"geometry"];
    NSDictionary *prop = [jsonDict objectForKey:@"properties"];

    if (![geom isKindOfClass:[NSDictionary class]])
        return false;

    // Parse out the geometry.  May result in multiple shapes
    if (!VectorParseGeometry(shapes, geom))
        return false;
    
    // Apply the attributes if there are any
    if ([prop isKindOfClass:[NSDictionary class]])
        for (ShapeSet::iterator it = shapes.begin(); it != shapes.end(); ++it)
            (*it)->setAttrDict([NSMutableDictionary dictionaryWithDictionary:prop]);
    
    // Apply the identity if there is one
    if ([idStr isKindOfClass:[NSString class]])
        for (ShapeSet::iterator it = shapes.begin(); it != shapes.end(); ++it)
            [(*it)->getAttrDict() setObject:idStr forKey:@"id"];
    
    return true;
}
    
// Parse a set of features out of GeoJSON
bool VectorParseGeoJSON(ShapeSet &shapes,NSDictionary *jsonDict)
{
    NSString *type = [jsonDict objectForKey:@"type"];
    if (![type isKindOfClass:[NSString class]])
        return false;
    
    if (![type compare:@"FeatureCollection"])
    {
        NSArray *features = [jsonDict objectForKey:@"features"];
        if (![features isKindOfClass:[NSArray class]])
            return false;
        
        for (NSDictionary *featDict in features)
        {
            if (![featDict isKindOfClass:[NSDictionary class]])
                return false;
            
            ShapeSet featShapes;
            if (VectorParseFeature(featShapes,featDict))
                shapes.insert(featShapes.begin(),featShapes.end());
            else
                return false;
        }
    } 
        
    return true;
}
    
}
