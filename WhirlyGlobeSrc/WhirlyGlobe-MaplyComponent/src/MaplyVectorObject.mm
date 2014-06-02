/*
 *  WGVectorObject.mm
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 8/2/12.
 *  Copyright 2012 mousebird consulting
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

#import "MaplyVectorObject.h"
#import "MaplyVectorObject_private.h"
#import <WhirlyGlobe.h>
<<<<<<< HEAD
#import <CoreLocation/CoreLocation.h>
#import "DictionaryWrapper_private.h"
=======
#import "Tesselator.h"
#import "GridClipper.h"
#import <CoreLocation/CoreLocation.h>
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation MaplyVectorObject

+ (WGVectorObject *)VectorObjectFromGeoJSON:(NSData *)geoJSON
{
    if ([geoJSON length] > 0)
    {
        MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] init];
        
<<<<<<< HEAD
        // Note: Kind of an extra step here
        NSString *str = [[NSString alloc] initWithData:geoJSON encoding:NSUTF8StringEncoding];
        std::string cStr = [str UTF8String];
        if (!VectorParseGeoJSON(vecObj->_shapes, cStr))
=======
        if (!VectorParseGeoJSON(vecObj->_shapes, geoJSON))
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
            return nil;
        
        return vecObj;
    }
    
    return nil;
}

+ (NSDictionary *)VectorObjectsFromGeoJSONAssembly:(NSData *)geoJSON
{
    if ([geoJSON length] > 0)
    {
        std::map<std::string,ShapeSet> shapes;
<<<<<<< HEAD
        // Note: Kind of an extra step here
        NSString *str = [[NSString alloc] initWithData:geoJSON encoding:NSUTF8StringEncoding];
        std::string cStr = [str UTF8String];
        if (!VectorParseGeoJSONAssembly(cStr, shapes))
=======
        if (!VectorParseGeoJSONAssembly(geoJSON, shapes))
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
            return nil;
        
        NSMutableDictionary *dict = [NSMutableDictionary dictionary];
        for (std::map<std::string,ShapeSet>::iterator it = shapes.begin();
             it != shapes.end(); ++it)
        {
            NSString *str = [NSString stringWithFormat:@"%s",it->first.c_str()];
            if (str)
            {
                MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] init];
                vecObj.shapes = it->second;
                dict[str] = vecObj;
            }
        }
        
        return dict;
    }
    
    return nil;
}

<<<<<<< HEAD
///// Parse vector data from geoJSON.  Returns one object to represent
////   the whole thing, which might include multiple different vectors.
//+ (WGVectorObject *)VectorObjectFromGeoJSONApple:(NSData *)geoJSON
//{
//    if([geoJSON length] > 0)
//    {
//        NSError *error = nil;
//        NSDictionary *jsonDict = [NSJSONSerialization JSONObjectWithData:geoJSON options:NULL error:&error];
//        if (error || ![jsonDict isKindOfClass:[NSDictionary class]])
//            return nil;
//        
//        WGVectorObject *vecObj = [[WGVectorObject alloc] init];
//
//        if (!VectorParseGeoJSON(vecObj->_shapes,jsonDict))
//            return nil;
//
//      return vecObj;
//    }
//    
//    return nil;
//}
//
//+ (MaplyVectorObject *)VectorObjectFromGeoJSONDictionary:(NSDictionary *)jsonDict
//{
//    if (![jsonDict isKindOfClass:[NSDictionary class]])
//        return nil;
//    
//    WGVectorObject *vecObj = [[WGVectorObject alloc] init];
//    
//    if (!VectorParseGeoJSON(vecObj->_shapes,jsonDict))
//        return nil;
//    
//    return vecObj;
//}
=======
/// Parse vector data from geoJSON.  Returns one object to represent
//   the whole thing, which might include multiple different vectors.
+ (WGVectorObject *)VectorObjectFromGeoJSONApple:(NSData *)geoJSON
{
    if([geoJSON length] > 0)
    {
        NSError *error = nil;
        NSDictionary *jsonDict = [NSJSONSerialization JSONObjectWithData:geoJSON options:NULL error:&error];
        if (error || ![jsonDict isKindOfClass:[NSDictionary class]])
            return nil;
        
        WGVectorObject *vecObj = [[WGVectorObject alloc] init];

        if (!VectorParseGeoJSON(vecObj->_shapes,jsonDict))
            return nil;

      return vecObj;
    }
    
    return nil;
}

+ (MaplyVectorObject *)VectorObjectFromGeoJSONDictionary:(NSDictionary *)jsonDict
{
    if (![jsonDict isKindOfClass:[NSDictionary class]])
        return nil;
    
    WGVectorObject *vecObj = [[WGVectorObject alloc] init];
    
    if (!VectorParseGeoJSON(vecObj->_shapes,jsonDict))
        return nil;
    
    return vecObj;
}
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

+ (MaplyVectorObject *)VectorObjectFromShapeFile:(NSString *)fileName
{
    if (![[NSFileManager defaultManager] fileExistsAtPath:[NSString stringWithFormat:@"%@.shp",fileName]])
    {
        fileName = [[NSBundle mainBundle] pathForResource:fileName ofType:@"shp"];
    }
    if (!fileName)
        return nil;
<<<<<<< HEAD

    const char *cFileName = [fileName cStringUsingEncoding:NSASCIIStringEncoding];
    if (!cFileName)
        return nil;
    ShapeReader shapeReader(cFileName);
    if (!shapeReader.isValid())
        return NULL;
=======
    
    ShapeReader shapeReader(fileName);
    if (!shapeReader.isValid())
        return false;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    
    MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] init];
    int numObj = shapeReader.getNumObjects();
    for (unsigned int ii=0;ii<numObj;ii++)
    {
        VectorShapeRef shape = shapeReader.getObjectByIndex(ii, nil);
        vecObj.shapes.insert(shape);
    }
    
    return vecObj;
}

<<<<<<< HEAD
// Note: Porting
//+ (MaplyVectorObject *)VectorObjectFromFile:(NSString *)fileName
//{
//    MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] init];
//    
//    if (!VectorReadFile([fileName cStringUsingEncoding:NSASCIIStringEncoding], vecObj.shapes))
//        return nil;
//    
//    return vecObj;
//}
//
//- (bool)writeToFile:(NSString *)fileName
//{
//    return VectorWriteFile([fileName cStringUsingEncoding:NSASCIIStringEncoding], _shapes);
//}

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    
    _selectable = true;
    
    return self;
}

// Note: Porting.  This is horribly inefficient
=======
+ (MaplyVectorObject *)VectorObjectFromFile:(NSString *)fileName
{
    MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] init];
    
    if (!VectorReadFile([fileName cStringUsingEncoding:NSASCIIStringEncoding], vecObj.shapes))
        return nil;
    
    return vecObj;
}

- (bool)writeToFile:(NSString *)fileName
{
    return VectorWriteFile([fileName cStringUsingEncoding:NSASCIIStringEncoding], _shapes);
}

>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
- (NSDictionary *)attributes
{
    if (_shapes.empty())
        return nil;
    
    VectorShapeRef vec = *(_shapes.begin());
<<<<<<< HEAD
    return [NSMutableDictionary DictionaryWithMaplyDictionary:vec->getAttrDict()];
=======
    return vec->getAttrDict();
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
}

- (void)setAttributes:(NSDictionary *)attributes
{
    for (ShapeSet::iterator it = _shapes.begin();
         it != _shapes.end(); ++it)
<<<<<<< HEAD
    {
        WhirlyKit::Dictionary *dict = (*it)->getAttrDict();
        dict->clear();
        [attributes copyToMaplyDictionary:dict];
    }
=======
        (*it)->setAttrDict([NSMutableDictionary dictionaryWithDictionary:attributes]);
}

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    
    _selectable = true;
    
    return self;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
}

/// Construct with a single point
- (id)initWithPoint:(MaplyCoordinate *)coord attributes:(NSDictionary *)attr
{
    self = [super init];
    
    if (self)
    {
        VectorPointsRef pts = VectorPoints::createPoints();
        pts->pts.push_back(GeoCoord(coord->x,coord->y));
<<<<<<< HEAD
        [attr copyToMaplyDictionary:pts->getAttrDict()];
=======
        pts->setAttrDict([NSMutableDictionary dictionaryWithDictionary:attr]);
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
        pts->initGeoMbr();
        _shapes.insert(pts);
        
        _selectable = true;
    }
    
    return self;
}

/// Construct with a linear feature (e.g. line string)
- (id)initWithLineString:(MaplyCoordinate *)coords numCoords:(int)numCoords attributes:(NSDictionary *)attr
{
    self = [super init];
    
    if (self)
    {
        VectorLinearRef lin = VectorLinear::createLinear();
        for (unsigned int ii=0;ii<numCoords;ii++)
            lin->pts.push_back(GeoCoord(coords[ii].x,coords[ii].y));
<<<<<<< HEAD
        [attr copyToMaplyDictionary:lin->getAttrDict()];
=======
        lin->setAttrDict([NSMutableDictionary dictionaryWithDictionary:attr]);
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
        lin->initGeoMbr();
        _shapes.insert(lin);
        
        _selectable = true;
    }
    
    return self;
}

/// Construct as an areal with an exterior
- (id)initWithAreal:(MaplyCoordinate *)coords numCoords:(int)numCoords attributes:(NSDictionary *)attr
{
    self = [super init];
    
    if (self)
    {
        VectorArealRef areal = VectorAreal::createAreal();
        VectorRing pts;
        for (unsigned int ii=0;ii<numCoords;ii++)
            pts.push_back(GeoCoord(coords[ii].x,coords[ii].y));
        areal->loops.push_back(pts);
<<<<<<< HEAD
        [attr copyToMaplyDictionary:areal->getAttrDict()];
=======
        areal->setAttrDict([NSMutableDictionary dictionaryWithDictionary:attr]);
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
        areal->initGeoMbr();
        _shapes.insert(areal);
        
        _selectable = true;
    }
    
    return self;
}

- (void)mergeVectorsFrom:(MaplyVectorObject *)otherVec
{
    _shapes.insert(otherVec.shapes.begin(),otherVec.shapes.end());
}

/// Add a hole to an existing areal feature
- (void)addHole:(MaplyCoordinate *)coords numCoords:(int)numCoords
{
    if (_shapes.size() != 1)
        return;
    
    VectorArealRef areal = boost::dynamic_pointer_cast<VectorAreal>(*(_shapes.begin()));
    if (areal)
    {
        VectorRing pts;
        for (unsigned int ii=0;ii<numCoords;ii++)
            pts.push_back(GeoCoord(coords[ii].x,coords[ii].y));
        areal->loops.push_back(pts);        
    }
}

- (MaplyVectorObjectType)vectorType
{
    if (_shapes.empty())
        return MaplyVectorMultiType;

    MaplyVectorObjectType type = MaplyVectorNoneType;
    for (ShapeSet::iterator it = _shapes.begin(); it != _shapes.end(); ++it)
    {
        MaplyVectorObjectType thisType = MaplyVectorNoneType;
        VectorPointsRef points = boost::dynamic_pointer_cast<VectorPoints>(*it);
        if (points)
            thisType = MaplyVectorPointType;
        else {
            VectorLinearRef lin = boost::dynamic_pointer_cast<VectorLinear>(*it);
            if (lin)
                thisType = MaplyVectorLinearType;
            else {
                VectorArealRef ar = boost::dynamic_pointer_cast<VectorAreal>(*it);
                if (ar)
                    thisType = MaplyVectorArealType;
            }
        }
        
        if (type == MaplyVectorNoneType)
            type = thisType;
        else
            if (type != thisType)
                return MaplyVectorMultiType;
    }
    
    return type;
}

- (NSString *)log
{
    NSMutableString *outStr = [NSMutableString string];
    
    for (ShapeSet::iterator it = _shapes.begin(); it != _shapes.end(); ++it)
    {
        VectorPointsRef points = boost::dynamic_pointer_cast<VectorPoints>(*it);
        if (points)
        {
            [outStr appendString:@"Points: "];
            for (unsigned int ii=0;ii<points->pts.size();ii++)
            {
                const Point2f &pt = points->pts[ii];
                [outStr appendFormat:@" (%f,%f)",pt.x(),pt.y()];
            }
            [outStr appendString:@"\n"];
        } else {
            VectorLinearRef lin = boost::dynamic_pointer_cast<VectorLinear>(*it);
            if (lin)
            {
                [outStr appendString:@"Linear: "];
                for (unsigned int ii=0;ii<lin->pts.size();ii++)
                {
                    const Point2f &pt = lin->pts[ii];
                    [outStr appendFormat:@" (%f,%f)",pt.x(),pt.y()];
                }
                [outStr appendString:@"\n"];
            } else {
                VectorArealRef ar = boost::dynamic_pointer_cast<VectorAreal>(*it);
                if (ar)
                {
                    [outStr appendString:@"Areal:\n"];
                    for (unsigned int li=0;li<ar->loops.size();li++)
                    {
                        const VectorRing &ring = ar->loops[li];
                        [outStr appendFormat:@" loop (%d): ",li];
                        for (unsigned int ii=0;ii<ring.size();ii++)
                        {
                            const Point2f &pt = ring[ii];
                            [outStr appendFormat:@" (%f,%f)",pt.x(),pt.y()];
                        }
                        [outStr appendString:@"\n"];
                    }
                    [outStr appendString:@"\n"];
                }
            }
        }        
    }
    
    return outStr;
}

- (MaplyVectorObject *)deepCopy
{
    MaplyVectorObject *newVecObj = [[MaplyVectorObject alloc] init];
    
    for (ShapeSet::iterator it = _shapes.begin(); it != _shapes.end(); ++it)
    {
        VectorPointsRef points = boost::dynamic_pointer_cast<VectorPoints>(*it);
        if (points)
        {
            VectorPointsRef newPts = VectorPoints::createPoints();
<<<<<<< HEAD
            newPts->setAttrDict(*points->getAttrDict());
=======
            [newPts->getAttrDict() addEntriesFromDictionary:points->getAttrDict()];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
            newPts->pts = points->pts;
            newVecObj.shapes.insert(newPts);
        } else {
            VectorLinearRef lin = boost::dynamic_pointer_cast<VectorLinear>(*it);
            if (lin)
            {
                VectorLinearRef newLin = VectorLinear::createLinear();
<<<<<<< HEAD
                newLin->setAttrDict(*lin->getAttrDict());
=======
                [newLin->getAttrDict() addEntriesFromDictionary:lin->getAttrDict()];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
                newLin->pts = lin->pts;
                newVecObj.shapes.insert(newLin);
            } else {
                VectorArealRef ar = boost::dynamic_pointer_cast<VectorAreal>(*it);
                if (ar)
                {
                    VectorArealRef newAr = VectorAreal::createAreal();
<<<<<<< HEAD
                    newAr->setAttrDict(*ar->getAttrDict());
=======
                    [newAr->getAttrDict() addEntriesFromDictionary:ar->getAttrDict()];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
                    newAr->loops = ar->loops;
                    newVecObj.shapes.insert(newAr);
                }
            }
        }
    }
    
    return newVecObj;
}

// Look for areals that this point might be inside
- (bool)pointInAreal:(MaplyCoordinate)coord
{
    for (ShapeSet::iterator it = _shapes.begin();it != _shapes.end();++it)
    {
        VectorArealRef areal = boost::dynamic_pointer_cast<VectorAreal>(*it);
        if (areal)
        {
            if (areal->pointInside(GeoCoord(coord.x,coord.y)))
                return true;
        }
    }
                
    return false;
}

// Calculate a center
- (MaplyCoordinate)center
{
    Mbr mbr;
    for (ShapeSet::iterator it = _shapes.begin();it != _shapes.end();++it)
    {
        GeoMbr geoMbr = (*it)->calcGeoMbr();
        mbr.addPoint(geoMbr.ll());
        mbr.addPoint(geoMbr.ur());
    }
    
    MaplyCoordinate ctr;
    ctr.x = (mbr.ll().x() + mbr.ur().x())/2.0;
    ctr.y = (mbr.ll().y() + mbr.ur().y())/2.0;

    return ctr;
}

- (bool)linearMiddle:(MaplyCoordinate *)middle rot:(float *)rot
{
    if (_shapes.empty())
        return false;
    
    VectorLinearRef lin = boost::dynamic_pointer_cast<VectorLinear>(*(_shapes.begin()));
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
            middle->x = thePt.x();
            middle->y = thePt.y();
            *rot = M_PI/2.0-atan2f(pt1.y()-pt0.y(),pt1.x()-pt0.x());
            return true;
        }

        lenSoFar += len;
    }
    
    middle->x = pts.back().x();
    middle->y = pts.back().y();
    if (rot)
        *rot = 0.0;
    
    return true;
}


- (bool)largestLoopCenter:(MaplyCoordinate *)center mbrLL:(MaplyCoordinate *)ll mbrUR:(MaplyCoordinate *)ur;
{
    // Find the loop with the largest area
    float bigArea = -1.0;
    const VectorRing *bigLoop = NULL;
    for (ShapeSet::iterator it = _shapes.begin();it != _shapes.end();++it)
    {
        VectorArealRef areal = boost::dynamic_pointer_cast<VectorAreal>(*it);
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

    MaplyCoordinate ctr;
    ctr.x = 0;  ctr.y = 0;
    if (bigLoop)
    {
        Mbr mbr;
        mbr.addPoints(*bigLoop);
        ctr.x = (mbr.ll().x() + mbr.ur().x())/2.0;
        ctr.y = (mbr.ll().y() + mbr.ur().y())/2.0;
        if (center)
            *center = ctr;
        if (ll)
        {
            ll->x = mbr.ll().x();  ll->y = mbr.ll().y();
        }
        if (ur)
        {
            ur->x = mbr.ur().x();  ur->y = mbr.ur().y();
        }
    }

    return true;
}

- (bool)centroid:(MaplyCoordinate *)centroid
{
    // Find the loop with the largest area
    float bigArea = -1.0;
    const VectorRing *bigLoop = NULL;
    for (ShapeSet::iterator it = _shapes.begin();it != _shapes.end();++it)
    {
        VectorArealRef areal = boost::dynamic_pointer_cast<VectorAreal>(*it);
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

    if (bigLoop)
    {
        Point2f centroid2f = CalcLoopCentroid(*bigLoop);
        centroid->x = centroid2f.x();
        centroid->y = centroid2f.y();
    } else
        return false;
    
    return true;
}

- (bool)boundingBoxLL:(MaplyCoordinate *)ll ur:(MaplyCoordinate *)ur
{
    bool valid = false;
    Mbr mbr;
    for (ShapeSet::iterator it = _shapes.begin();it != _shapes.end();++it)
    {
        GeoMbr geoMbr = (*it)->calcGeoMbr();
        mbr.addPoint(geoMbr.ll());
        mbr.addPoint(geoMbr.ur());
        valid = true;
    }

    if (valid)
    {
        ll->x = mbr.ll().x();
        ll->y = mbr.ll().y();
        ur->x = mbr.ur().x();
        ur->y = mbr.ur().y();
    }
    
    return valid;
}

- (NSArray *)asCLLocationArrays
{
    if (_shapes.size() < 1)
        return nil;

    NSMutableArray *loops = [NSMutableArray array];
    
    ShapeSet::iterator it = _shapes.begin();
    VectorArealRef ar = boost::dynamic_pointer_cast<VectorAreal>(*it);
    if (ar)
    {
        for (unsigned int ii=0;ii<ar->loops.size();ii++)
        {
            const VectorRing &loop = ar->loops[ii];
            NSMutableArray *pts = [NSMutableArray array];
            for (unsigned int ii=0;ii<loop.size();ii++)
            {
                const Point2f &coord = loop[ii];
                CLLocation *loc = [[CLLocation alloc] initWithLatitude:RadToDeg(coord.y()) longitude:RadToDeg(coord.x())];
                [pts addObject:loc];
            }
            [loops addObject:pts];
        }
    }
    
    return loops;
}

- (NSArray *)splitVectors
{
    NSMutableArray *vecs = [NSMutableArray array];
    
    for (WhirlyKit::ShapeSet::iterator it = _shapes.begin();
         it != _shapes.end(); ++it)
    {
        MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] init];
        vecObj.shapes.insert(*it);
        [vecs addObject:vecObj];
    }
    
    return vecs;
}

- (void)subdivideToGlobe:(float)epsilon
{
    FakeGeocentricDisplayAdapter adapter;
    
    for (ShapeSet::iterator it = _shapes.begin();it!=_shapes.end();it++)
    {
        VectorLinearRef lin = boost::dynamic_pointer_cast<VectorLinear>(*it);
        if (lin)
        {
<<<<<<< HEAD
            Point2fVector outPts;
=======
            std::vector<Point2f> outPts;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
            SubdivideEdgesToSurface(lin->pts, outPts, false, &adapter, epsilon);
            lin->pts = outPts;
        } else {
            VectorArealRef ar = boost::dynamic_pointer_cast<VectorAreal>(*it);
            if (ar)
            {
                for (unsigned int ii=0;ii<ar->loops.size();ii++)
                {
<<<<<<< HEAD
                    Point2fVector outPts;
=======
                    std::vector<Point2f> outPts;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
                    SubdivideEdgesToSurface(ar->loops[ii], outPts, true, &adapter, epsilon);
                    ar->loops[ii] = outPts;
                }
            }
        }
    }
}

- (void)subdivideToGlobeGreatCircle:(float)epsilon
{
    FakeGeocentricDisplayAdapter adapter;
    CoordSystem *coordSys = adapter.getCoordSystem();
    
    for (ShapeSet::iterator it = _shapes.begin();it!=_shapes.end();it++)
    {
        VectorLinearRef lin = boost::dynamic_pointer_cast<VectorLinear>(*it);
        if (lin)
        {
<<<<<<< HEAD
            Point3fVector outPts;
=======
            std::vector<Point3f> outPts;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
            SubdivideEdgesToSurfaceGC(lin->pts, outPts, false, &adapter, epsilon);
            VectorRing outPts2D;
            outPts2D.resize(outPts.size());
            for (unsigned int ii=0;ii<outPts.size();ii++)
                outPts2D[ii] = coordSys->localToGeographic(adapter.displayToLocal(outPts[ii]));
            lin->pts = outPts2D;
        } else {
            VectorArealRef ar = boost::dynamic_pointer_cast<VectorAreal>(*it);
            if (ar)
            {
                for (unsigned int ii=0;ii<ar->loops.size();ii++)
                {
<<<<<<< HEAD
                    Point3fVector outPts;
=======
                    std::vector<Point3f> outPts;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
                    SubdivideEdgesToSurfaceGC(ar->loops[ii], outPts, true, &adapter, epsilon);
                    VectorRing outPts2D;
                    outPts2D.resize(outPts.size());
                    for (unsigned int ii=0;ii<outPts.size();ii++)
                        outPts2D[ii] = coordSys->localToGeographic(adapter.displayToLocal(outPts[ii]));
                    ar->loops[ii] = outPts2D;
                }
            }
        }
    }    
}

- (MaplyVectorObject *) tesselate
{
    MaplyVectorObject *newVec = [[MaplyVectorObject alloc] init];
    
    for (ShapeSet::iterator it = _shapes.begin();it!=_shapes.end();it++)
    {
        VectorArealRef ar = boost::dynamic_pointer_cast<VectorAreal>(*it);
        if (ar)
        {
            VectorTrianglesRef trisRef = VectorTriangles::createTriangles();
            TesselateLoops(ar->loops, trisRef);
<<<<<<< HEAD
            trisRef->setAttrDict(*ar->getAttrDict());
=======
            trisRef->setAttrDict(ar->getAttrDict());
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
            newVec->_shapes.insert(trisRef);
        }
    }
    
    return newVec;
}

- (MaplyVectorObject *) clipToGrid:(CGSize)gridSize
{
    MaplyVectorObject *newVec = [[MaplyVectorObject alloc] init];
    
    for (ShapeSet::iterator it = _shapes.begin();it!=_shapes.end();it++)
    {
        VectorArealRef ar = boost::dynamic_pointer_cast<VectorAreal>(*it);
        if (ar)
        {
            for (int ii=0;ii<ar->loops.size();ii++)
            {
                std::vector<VectorRing> newLoops;
                ClipLoopToGrid(ar->loops[ii], Point2f(0.0,0.0), Point2f(gridSize.width,gridSize.height), newLoops);
                for (unsigned int jj=0;jj<newLoops.size();jj++)
                {
                    VectorArealRef newAr = VectorAreal::createAreal();
<<<<<<< HEAD
                    newAr->setAttrDict(*ar->getAttrDict());
=======
                    newAr->setAttrDict(ar->getAttrDict());
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
                    newAr->loops.push_back(newLoops[jj]);
                    newVec->_shapes.insert(newAr);
                }
            }
        }
    }
    
    return newVec;
}

@end

<<<<<<< HEAD
// Note: Porting
//@implementation MaplyVectorDatabase
//{
//    VectorDatabase *vectorDb;
//    NSString *baseName;
//}
//
//- (id)initWithVectorDatabase:(VectorDatabase *)inVectorDb
//{
//    self = [super init];
//    if (!self)
//        return nil;
//    
//    vectorDb = inVectorDb;
//    
//    return self;
//}
//
///// Construct from a shapefile in the bundle
//+ (MaplyVectorDatabase *) vectorDatabaseWithShape:(NSString *)shapeName
//{
//    NSString *fileName = [[NSBundle mainBundle] pathForResource:shapeName ofType:@"shp"];
//    VectorDatabase *vecDb = new VectorDatabase([[NSBundle mainBundle] resourcePath],[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject],shapeName,new ShapeReader(fileName),NULL);
//    
//    MaplyVectorDatabase *mVecDb = [[MaplyVectorDatabase alloc] initWithVectorDatabase:vecDb];
//    mVecDb->baseName = shapeName;
//    return mVecDb;
//}
//
///// Return vectors that match the given SQL query
//- (MaplyVectorObject *)fetchMatchingVectors:(NSString *)sqlQuery
//{
//    MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] init];
//    vectorDb->getMatchingVectors(sqlQuery, vecObj.shapes);
//    
//    if (vecObj.shapes.empty())
//        return nil;
//    
//    return vecObj;
//}
//
///// Search for all the areals that surround the given point (in geographic)
//- (MaplyVectorObject *)fetchArealsForPoint:(MaplyCoordinate)coord
//{
//    MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] init];
//    vectorDb->findArealsForPoint(GeoCoord(coord.x,coord.y), vecObj.shapes);
//    
//    if (vecObj.shapes.empty())
//        return nil;
//    
//    return vecObj;
//}
//
//- (MaplyVectorObject *)fetchAllVectors
//{
//    MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] init];
//    int numVecs = vectorDb->numVectors();
//    for (unsigned int ii=0;ii<numVecs;ii++)
//    {
//        VectorShapeRef shapeRef = vectorDb->getVector(ii);
//        vecObj.shapes.insert(shapeRef);
//    }
//    
//    if (vecObj.shapes.empty())
//        return nil;
//    
//    return vecObj;
//}
//
//#pragma mark - WhirlyKitLoftedPolyCache delegate
//
//// We'll look for the lofted poly data in the bundle first, then the cache dir
//- (NSData *)readLoftedPolyData:(NSString *)key
//{
//    // Look for cache files in the doc and bundle dirs
//    NSString *docDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) lastObject];
//    NSString *bundleDir = [[NSBundle mainBundle] resourcePath];
//
//    NSString *cache0 = [NSString stringWithFormat:@"%@/%@_%@.loftcache",bundleDir,baseName,key];
//    NSString *cache1 = [NSString stringWithFormat:@"%@/%@_%@.loftcache",docDir,baseName,key];
//    
//    // Look for an existing file
//    NSString *cacheFile = nil;
//    NSFileManager *fileManager = [NSFileManager defaultManager];
//    if ([fileManager fileExistsAtPath:cache0])
//        cacheFile = cache0;
//    else
//        if ([fileManager fileExistsAtPath:cache1])
//            cacheFile = cache1;
//
//    if (!cacheFile)
//        return nil;
//    
//    return [NSData dataWithContentsOfFile:cacheFile];
//}
//
//// We'll write the lofted poly data to the cache dir with the base name and key
//- (bool)writeLoftedPolyData:(NSData *)data cacheName:(NSString *)key
//{
//    NSString *docDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) lastObject];
//    NSString *cacheFile = [NSString stringWithFormat:@"%@/%@_%@.loftcache",docDir,baseName,key];
//    
//    return [data writeToFile:cacheFile atomically:YES];
//}
//
//@end
=======
@implementation MaplyVectorDatabase
{
    VectorDatabase *vectorDb;
    NSString *baseName;
}

- (id)initWithVectorDatabase:(VectorDatabase *)inVectorDb
{
    self = [super init];
    if (!self)
        return nil;
    
    vectorDb = inVectorDb;
    
    return self;
}

/// Construct from a shapefile in the bundle
+ (MaplyVectorDatabase *) vectorDatabaseWithShape:(NSString *)shapeName
{
    NSString *fileName = [[NSBundle mainBundle] pathForResource:shapeName ofType:@"shp"];
    VectorDatabase *vecDb = new VectorDatabase([[NSBundle mainBundle] resourcePath],[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject],shapeName,new ShapeReader(fileName),NULL);
    
    MaplyVectorDatabase *mVecDb = [[MaplyVectorDatabase alloc] initWithVectorDatabase:vecDb];
    mVecDb->baseName = shapeName;
    return mVecDb;
}

/// Return vectors that match the given SQL query
- (MaplyVectorObject *)fetchMatchingVectors:(NSString *)sqlQuery
{
    MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] init];
    vectorDb->getMatchingVectors(sqlQuery, vecObj.shapes);
    
    if (vecObj.shapes.empty())
        return nil;
    
    return vecObj;
}

/// Search for all the areals that surround the given point (in geographic)
- (MaplyVectorObject *)fetchArealsForPoint:(MaplyCoordinate)coord
{
    MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] init];
    vectorDb->findArealsForPoint(GeoCoord(coord.x,coord.y), vecObj.shapes);
    
    if (vecObj.shapes.empty())
        return nil;
    
    return vecObj;
}

- (MaplyVectorObject *)fetchAllVectors
{
    MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] init];
    int numVecs = vectorDb->numVectors();
    for (unsigned int ii=0;ii<numVecs;ii++)
    {
        VectorShapeRef shapeRef = vectorDb->getVector(ii);
        vecObj.shapes.insert(shapeRef);
    }
    
    if (vecObj.shapes.empty())
        return nil;
    
    return vecObj;
}

#pragma mark - WhirlyKitLoftedPolyCache delegate

// We'll look for the lofted poly data in the bundle first, then the cache dir
- (NSData *)readLoftedPolyData:(NSString *)key
{
    // Look for cache files in the doc and bundle dirs
    NSString *docDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) lastObject];
    NSString *bundleDir = [[NSBundle mainBundle] resourcePath];

    NSString *cache0 = [NSString stringWithFormat:@"%@/%@_%@.loftcache",bundleDir,baseName,key];
    NSString *cache1 = [NSString stringWithFormat:@"%@/%@_%@.loftcache",docDir,baseName,key];
    
    // Look for an existing file
    NSString *cacheFile = nil;
    NSFileManager *fileManager = [NSFileManager defaultManager];
    if ([fileManager fileExistsAtPath:cache0])
        cacheFile = cache0;
    else
        if ([fileManager fileExistsAtPath:cache1])
            cacheFile = cache1;

    if (!cacheFile)
        return nil;
    
    return [NSData dataWithContentsOfFile:cacheFile];
}

// We'll write the lofted poly data to the cache dir with the base name and key
- (bool)writeLoftedPolyData:(NSData *)data cacheName:(NSString *)key
{
    NSString *docDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) lastObject];
    NSString *cacheFile = [NSString stringWithFormat:@"%@/%@_%@.loftcache",docDir,baseName,key];
    
    return [data writeToFile:cacheFile atomically:YES];
}

@end
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
