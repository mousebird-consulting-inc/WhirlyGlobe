/*
 *  WGVectorObject.mm
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 8/2/12.
 *  Copyright 2012-2015 mousebird consulting
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
#import "Tesselator.h"
#import "GridClipper.h"
#import <CoreLocation/CoreLocation.h>
#import "MaplyBaseViewController.h"
#import "MaplyViewController.h"
#import "MaplyCoordinateSystem_private.h"
#import "MaplyCoordinateSystem_private.h"
#import "MaplyViewController.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

namespace Maply
{
class DataReader
{
public:
    DataReader(NSData *data)
    : pos(0)
    {
        len = (int)[data length];
        bytes = (unsigned char *)[data bytes];
    }
    
    int getInt()
    {
        int size = sizeof(int);
        if (pos+size > len)
            throw 1;
        int iVal = *((int *)&bytes[pos]);
        pos += size;
        
        return iVal;
    }
    
    void rangeCheck(int iVal,int minVal,int maxVal)
    {
        if (iVal < minVal || iVal >= maxVal)
            throw 1;
    }
    
    float getFloat()
    {
        int size = sizeof(float);
        if (pos+size > len)
            throw 1;
        float fVal = *((float *)&bytes[pos]);
        pos += size;
        return fVal;
    }
    
    NSString *getString()
    {
        int strLen = getInt();
        if (strLen == 0)
            return @"";
        const char *str = (const char *)str;
        if (pos+strLen > len)
            throw 1;
        char *simpleStr = (char *)malloc(strLen+1);
        bcopy(&bytes[pos],simpleStr,strLen);
        simpleStr[strLen] = 0;
        
        NSString *retStr = [NSString stringWithCString:simpleStr encoding:NSUTF8StringEncoding];
        free(simpleStr);
        
        pos += strLen;
        return retStr;
    }
    
    unsigned char *bytes;
    int pos;
    int len;
};
            
}

@implementation MaplyVectorObject

+ (MaplyVectorObject *)VectorObjectFromGeoJSON:(NSData *)geoJSON
{
	return [[MaplyVectorObject alloc] initWithGeoJSON:geoJSON];
}

+ (NSDictionary *)VectorObjectsFromGeoJSONAssembly:(NSData *)geoJSON
{
    if ([geoJSON length] > 0)
    {
        std::map<std::string,ShapeSet> shapes;
        if (!VectorParseGeoJSONAssembly(geoJSON, shapes))
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

/// Parse vector data from geoJSON.  Returns one object to represent
//   the whole thing, which might include multiple different vectors.
+ (MaplyVectorObject *)VectorObjectFromGeoJSONApple:(NSData *)geoJSON
{
	return [[MaplyVectorObject alloc] initWithGeoJSONApple:geoJSON];
}

+ (MaplyVectorObject *)VectorObjectFromGeoJSONDictionary:(NSDictionary *)jsonDict
{
	return [[MaplyVectorObject alloc] initWithGeoJSONDictionary:jsonDict];
}

+ (MaplyVectorObject *)VectorObjectFromShapeFile:(NSString *)fileName
{
	return [[MaplyVectorObject alloc] initWithShapeFile:fileName];
}

+ (MaplyVectorObject *)VectorObjectFromFile:(NSString *)fileName
{
	return [[MaplyVectorObject alloc] initWithFile:fileName];
}

- (bool)writeToFile:(NSString *)fileName
{
    return VectorWriteFile([fileName cStringUsingEncoding:NSASCIIStringEncoding], _shapes);
}

- (NSMutableDictionary *)attributes
{
    if (_shapes.empty())
        return nil;
    
    VectorShapeRef vec = *(_shapes.begin());
    return vec->getAttrDict();
}

- (void)setAttributes:(NSDictionary *)attributes
{
    for (ShapeSet::iterator it = _shapes.begin();
         it != _shapes.end(); ++it)
        (*it)->setAttrDict([NSMutableDictionary dictionaryWithDictionary:attributes]);
}

- (instancetype)init
{
    self = [super init];
    if (!self)
        return nil;
    
    _selectable = true;
    
    return self;
}

/// Construct with a single point
- (instancetype)initWithPoint:(MaplyCoordinate)coord attributes:(NSDictionary *)attr
{
	return [self initWithPointRef:&coord attributes:attr];
}

/// Construct with a single point ref
- (instancetype)initWithPointRef:(MaplyCoordinate *)coord attributes:(NSDictionary *)attr
{
    self = [super init];
    
    if (self)
    {
        VectorPointsRef pts = VectorPoints::createPoints();
        pts->pts.push_back(GeoCoord(coord->x,coord->y));
        pts->setAttrDict([NSMutableDictionary dictionaryWithDictionary:attr]);
        pts->initGeoMbr();
        _shapes.insert(pts);
        
        _selectable = true;
    }
    
    return self;
}

- (instancetype)initWithLineString:(NSArray *)inCoords attributes:(NSDictionary *)attr
{
	MaplyCoordinate *coords = (MaplyCoordinate *) malloc(sizeof(int) * [inCoords count]/2);

	for (int i = 0; i < [inCoords count]/2; i += 2) {
		float x = [inCoords[i] floatValue];
		float y = [inCoords[i+1] floatValue];

		coords[i / 2] = MaplyCoordinateMakeWithDegrees(x, y);
	}

	self = [self initWithLineString:coords numCoords:(int)[inCoords count]/2 attributes:attr];

	free(coords);

	return self;
}

/// Construct with a linear feature (e.g. line string)
- (instancetype)initWithLineString:(MaplyCoordinate *)coords numCoords:(int)numCoords attributes:(NSDictionary *)attr
{
    self = [super init];
    
    if (self)
    {
        VectorLinearRef lin = VectorLinear::createLinear();
        for (unsigned int ii=0;ii<numCoords;ii++)
            lin->pts.push_back(GeoCoord(coords[ii].x,coords[ii].y));
        lin->setAttrDict([NSMutableDictionary dictionaryWithDictionary:attr]);
        lin->initGeoMbr();
        _shapes.insert(lin);
        
        _selectable = true;
    }
    
    return self;
}

/// Construct as an areal with an exterior
- (instancetype)initWithAreal:(MaplyCoordinate *)coords numCoords:(int)numCoords attributes:(NSDictionary *)attr
{
    self = [super init];
    
    if (self)
    {
        VectorArealRef areal = VectorAreal::createAreal();
        VectorRing pts;
        for (unsigned int ii=0;ii<numCoords;ii++)
            pts.push_back(GeoCoord(coords[ii].x,coords[ii].y));
        areal->loops.push_back(pts);
        areal->setAttrDict([NSMutableDictionary dictionaryWithDictionary:attr]);
        areal->initGeoMbr();
        _shapes.insert(areal);
        
        _selectable = true;
    }
    
    return self;
}

/// Construct from GeoJSON
- (instancetype)initWithGeoJSON:(NSData *)geoJSON
{
	if ([geoJSON length] == 0)
		return nil;

	self = [super init];

	NSString *crs = nil;
	if (!VectorParseGeoJSON(_shapes, geoJSON, &crs))
		return nil;

	// Reproject to a destination system
	// Note: Not working
	//        if (crs)
	//        {
	//            MaplyCoordinateSystem *srcSys = MaplyCoordinateSystemFromEPSG(crs);
	//            MaplyCoordinateSystem *destSys = [[MaplyPlateCarree alloc] initFullCoverage];
	//            if (srcSys && destSys)
	//            {
	//                [vecObj reprojectFrom:srcSys to:destSys];
	//            } else
	//                NSLog(@"VectorObjectFromGeoJSON: Unable to reproject to CRS (%@)",crs);
	//        }

	return self;
}

- (instancetype)initWithGeoJSONApple:(NSData *)geoJSON
{
	if([geoJSON length] == 0)
		return nil;

	NSError *error = nil;
	NSDictionary *jsonDict = [NSJSONSerialization JSONObjectWithData:geoJSON options:NULL error:&error];
	if (error || ![jsonDict isKindOfClass:[NSDictionary class]])
		return nil;

	if (self = [super init]) {
		if (!VectorParseGeoJSON(_shapes, jsonDict))
			return nil;
	}

	return self;
}

- (instancetype)initWithGeoJSONDictionary:(NSDictionary *)geoJSON
{
	if (![geoJSON isKindOfClass:[NSDictionary class]])
		return nil;

	if (self = [super init]) {
		if (!VectorParseGeoJSON(_shapes, geoJSON))
			return nil;
	}

	return self;
}

- (instancetype)initWithFile:(NSString *)fileName
{
	if (self = [super init]) {
		if (!VectorReadFile([fileName cStringUsingEncoding:NSASCIIStringEncoding], _shapes))
			return nil;
	}

	return self;
}

- (instancetype)initWithShapeFile:(NSString *)fileName
{
	if (![[NSFileManager defaultManager] fileExistsAtPath:[NSString stringWithFormat:@"%@.shp",fileName]]) {
		fileName = [[NSBundle mainBundle] pathForResource:fileName ofType:@"shp"];
	}
	if (!fileName)
		return nil;

	ShapeReader shapeReader(fileName);
	if (!shapeReader.isValid())
		return nil;

	if (self = [super init]) {
		int numObj = shapeReader.getNumObjects();
		for (unsigned int ii=0;ii<numObj;ii++) {
			VectorShapeRef shape = shapeReader.getObjectByIndex(ii, nil);
			_shapes.insert(shape);
		}
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
    
    VectorArealRef areal = std::dynamic_pointer_cast<VectorAreal>(*(_shapes.begin()));
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
        VectorPointsRef points = std::dynamic_pointer_cast<VectorPoints>(*it);
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
            VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*it);
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
                VectorLinear3dRef lin3d = std::dynamic_pointer_cast<VectorLinear3d>(*it);
                if (lin3d)
                {
                    [outStr appendString:@"Linear3d: "];
                    for (unsigned int ii=0;ii<lin3d->pts.size();ii++)
                    {
                        const Point3d &pt = lin3d->pts[ii];
                        [outStr appendFormat:@" (%f,%f,%f)",pt.x(),pt.y(),pt.z()];
                    }
                    [outStr appendString:@"\n"];
                } else {
                    VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
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
    }
    
    return outStr;
}

- (MaplyVectorObject *)deepCopy2
{
    MaplyVectorObject *newVecObj = [[MaplyVectorObject alloc] init];
    
    for (ShapeSet::iterator it = _shapes.begin(); it != _shapes.end(); ++it)
    {
        VectorPointsRef points = std::dynamic_pointer_cast<VectorPoints>(*it);
        if (points)
        {
            VectorPointsRef newPts = VectorPoints::createPoints();
            newPts->pts = points->pts;
            newPts->setAttrDict([NSMutableDictionary dictionaryWithDictionary:points->getAttrDict()]);
            newVecObj.shapes.insert(newPts);
        } else {
            VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*it);
            if (lin)
            {
                VectorLinearRef newLin = VectorLinear::createLinear();
                newLin->pts = lin->pts;
                newLin->setAttrDict([NSMutableDictionary dictionaryWithDictionary:lin->getAttrDict()]);
                newVecObj.shapes.insert(newLin);
            } else {
                VectorLinear3dRef lin3d = std::dynamic_pointer_cast<VectorLinear3d>(*it);
                if (lin3d)
                {
                    VectorLinear3dRef newLin3d = VectorLinear3d::createLinear();
                    newLin3d->pts = lin3d->pts;
                    newLin3d->setAttrDict([NSMutableDictionary dictionaryWithDictionary:lin3d->getAttrDict()]);
                    newVecObj.shapes.insert(newLin3d);
                } else {
                    VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
                    if (ar)
                    {
                        VectorArealRef newAr = VectorAreal::createAreal();
                        newAr->loops = ar->loops;
                        newAr->setAttrDict([NSMutableDictionary dictionaryWithDictionary:ar->getAttrDict()]);
                        newVecObj.shapes.insert(newAr);
                    } else {
                        VectorTrianglesRef tri = std::dynamic_pointer_cast<VectorTriangles>(*it);
                        if (tri)
                        {
                            VectorTrianglesRef newTri = VectorTriangles::createTriangles();
                            newTri->geoMbr = tri->geoMbr;
                            newTri->pts = tri->pts;
                            newTri->tris = tri->tris;
                            newTri->setAttrDict([NSMutableDictionary dictionaryWithDictionary:tri->getAttrDict()]);
                            newVecObj.shapes.insert(newTri);
                        }
                    }
                }
            }
        }
    }
    
    return newVecObj;
}

- (void)reprojectFrom:(MaplyCoordinateSystem *)srcSystem to:(MaplyCoordinateSystem *)destSystem
{
    CoordSystem *inSystem = srcSystem->coordSystem;
    CoordSystem *outSystem = destSystem->coordSystem;

    // Note: Heinous hack for meters to radians conversion
    double scale = 1.0;
    if ([srcSystem isKindOfClass:[MaplySphericalMercator class]])
        scale = 1/EarthRadius;
    
    for (ShapeSet::iterator it = _shapes.begin(); it != _shapes.end(); ++it)
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

// Look for areals that this point might be inside
- (bool)pointInAreal:(MaplyCoordinate)coord
{
    for (ShapeSet::iterator it = _shapes.begin();it != _shapes.end();++it)
    {
        VectorArealRef areal = std::dynamic_pointer_cast<VectorAreal>(*it);
        if (areal)
        {
            if (areal->pointInside(GeoCoord(coord.x,coord.y)))
                return true;
        } else {
            VectorTrianglesRef tris = std::dynamic_pointer_cast<VectorTriangles>(*it);
            if (tris)
            {
                if (tris->pointInside(GeoCoord(coord.x,coord.y)))
                    return true;
            }
        }
    }
                
    return false;
}

//Fuzzy matching for selecting Linear features
- (bool)pointNearLinear:(MaplyCoordinate)coord distance:(float)maxDistance inViewController:(MaplyBaseViewController*)vc
{
    for (ShapeSet::iterator it = _shapes.begin();it != _shapes.end();++it)
    {
        VectorLinearRef linear = std::dynamic_pointer_cast<VectorLinear>(*it);
        if (linear)
        {
            GeoMbr geoMbr = linear->calcGeoMbr();
            if(geoMbr.inside(GeoCoord(coord.x,coord.y)))
            {
                CGPoint p = [vc screenPointFromGeo:coord];
                VectorRing pts = linear->pts;
                float distance;
                for (int ii=0;ii<pts.size()-1;ii++)
                {
                    distance = MAXFLOAT;
                    Point2f p0 = pts[ii];
                    MaplyCoordinate pc;
                    pc.x = p0.x();
                    pc.y = p0.y();
                    CGPoint a = [vc screenPointFromGeo:pc];

                    Point2f p1 = pts[ii + 1];
                    pc.x = p1.x();
                    pc.y = p1.y();
                    CGPoint b = [vc screenPointFromGeo:pc];
                    
                    CGPoint aToP = CGPointMake(a.x - p.x, a.y - p.y);
                    CGPoint aToB = CGPointMake(a.x - b.x, a.y - b.y);
                    double aToBMagitude = pow(hypot(aToB.x, aToB.y), 2);
                    double dot = aToP.x * aToB.x + aToP.y * aToB.y;
                    double d = dot/aToBMagitude;
                    
                    if(d < 0)
                    {
                        distance = hypot(p.x - a.x, p.y - a.y);
                    } else if(d > 1) {
                        distance = hypot(p.x - b.x, p.y - b.y);
                    } else {
                        distance = hypot(p.x - a.x + (aToB.x * d),
                                         p.y - a.y + (aToB.y * d));
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
                if(geoMbr.inside(GeoCoord(coord.x,coord.y)))
                {
                    CGPoint p = [vc screenPointFromGeo:coord];
                    VectorRing3d pts = linear3d->pts;
                    float distance;
                    for (int ii=0;ii<pts.size()-1;ii++)
                    {
                        distance = MAXFLOAT;
                        Point3d p0 = pts[ii];
                        MaplyCoordinate pc;
                        pc.x = p0.x();
                        pc.y = p0.y();
                        CGPoint a = [vc screenPointFromGeo:pc];
                        
                        Point3d p1 = pts[ii + 1];
                        pc.x = p1.x();
                        pc.y = p1.y();
                        CGPoint b = [vc screenPointFromGeo:pc];
                        
                        CGPoint aToP = CGPointMake(a.x - p.x, a.y - p.y);
                        CGPoint aToB = CGPointMake(a.x - b.x, a.y - b.y);
                        double aToBMagitude = pow(hypot(aToB.x, aToB.y), 2);
                        double dot = aToP.x * aToB.x + aToP.y * aToB.y;
                        double d = dot/aToBMagitude;
                        
                        if(d < 0)
                        {
                            distance = hypot(p.x - a.x, p.y - a.y);
                        } else if(d > 1) {
                            distance = hypot(p.x - b.x, p.y - b.y);
                        } else {
                            distance = hypot(p.x - a.x + (aToB.x * d),
                                             p.y - a.y + (aToB.y * d));
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

- (bool)linearMiddle:(MaplyCoordinate *)middle rot:(double *)rot
{
    if (_shapes.empty())
        return false;
    
    VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*(_shapes.begin()));
    VectorLinear3dRef lin3d = std::dynamic_pointer_cast<VectorLinear3d>(*(_shapes.begin()));
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
                middle->x = thePt.x();
                middle->y = thePt.y();
                *rot = M_PI/2.0-atan2(pt1.y()-pt0.y(),pt1.x()-pt0.x());
                return true;
            }

            lenSoFar += len;
        }
        
        middle->x = pts.back().x();
        middle->y = pts.back().y();
        *rot = 0.0;
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
                middle->x = thePt.x();
                middle->y = thePt.y();
                *rot = M_PI/2.0-atan2(pt1.y()-pt0.y(),pt1.x()-pt0.x());
                return true;
            }
            
            lenSoFar += len;
        }
        
        middle->x = pts.back().x();
        middle->y = pts.back().y();
        *rot = 0.0;
    }
    
    return true;
}

- (bool)linearMiddle:(MaplyCoordinate *)middle rot:(double *)rot displayCoordSys:(MaplyCoordinateSystem *)maplyCoordSys
{
    if (_shapes.empty())
        return false;
    
    VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*(_shapes.begin()));
    VectorLinear3dRef lin3d = std::dynamic_pointer_cast<VectorLinear3d>(*(_shapes.begin()));
    if (!lin && !lin3d)
        return false;
    
    if (lin)
    {
        const VectorRing &pts = lin->pts;
        
        if (pts.empty())
            return false;
        
        if (pts.size() == 1)
        {
            if (middle) {
                middle->x = pts[0].x();
                middle->y = pts[0].y();
            }
            if (rot) {
                *rot = 0.0;
            }
            return true;
        }
        
        float totLen = 0;
        for (int ii=0;ii<pts.size()-1;ii++)
        {
            float len = (pts[ii+1]-pts[ii]).norm();
            totLen += len;
        }
        float halfLen = totLen / 2.0;
        
        WhirlyKit::CoordSystem *coordSys = maplyCoordSys->coordSystem;
        
        // Now we'll walk along, looking for the middle
        float lenSoFar = 0.0;
        for (unsigned int ii=0;ii<pts.size()-1;ii++)
        {
            Point3d pt0 = coordSys->geographicToLocal3d(GeoCoord(pts[ii].x(),pts[ii].y()));
            Point3d pt1 = coordSys->geographicToLocal3d(GeoCoord(pts[ii+1].x(),pts[ii+1].y()));
            double len = (pt1-pt0).norm();
            if (halfLen <= lenSoFar+len)
            {
                if (middle) {
                    double t = (halfLen-lenSoFar)/len;
                    Point3d thePt = (pt1-pt0)*t + pt0;
                    GeoCoord middleGeo = coordSys->localToGeographic(thePt);
                    middle->x = middleGeo.x();
                    middle->y = middleGeo.y();
                }
                if (rot) {
                    *rot = M_PI/2.0-atan2(pt1.y()-pt0.y(),pt1.x()-pt0.x());
                }
                return true;
            }
            
            lenSoFar += len;
        }

        if (middle){
            middle->x = pts.back().x();
            middle->y = pts.back().y();
        }
        if (rot) {
            *rot = 0.0;
        }
    } else {
        VectorRing3d &pts = lin3d->pts;
        
        if (pts.empty())
            return false;
        
        if (pts.size() == 1)
        {
            if (middle) {
                middle->x = pts[0].x();
                middle->y = pts[0].y();
            }
            if (rot) {
                *rot = 0.0;
            }
            return true;
        }
        
        float totLen = 0;
        for (int ii=0;ii<pts.size()-1;ii++)
        {
            float len = (pts[ii+1]-pts[ii]).norm();
            totLen += len;
        }
        float halfLen = totLen / 2.0;
        
        WhirlyKit::CoordSystem *coordSys = maplyCoordSys->coordSystem;
        
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
                if (middle) {
                    double t = (halfLen-lenSoFar)/len;
                    Point3d thePt = (pt1-pt0)*t + pt0;
                    GeoCoord middleGeo = coordSys->localToGeographic(thePt);
                    middle->x = middleGeo.x();
                    middle->y = middleGeo.y();
                }
                if (rot) {
                    *rot = M_PI/2.0-atan2(pt1.y()-pt0.y(),pt1.x()-pt0.x());
                }
                return true;
            }
            
            lenSoFar += len;
        }
        
        if (middle){
            middle->x = pts.back().x();
            middle->y = pts.back().y();
        }
        if (rot) {
            *rot = 0.0;
        }
    }

    return true;
}

- (MaplyCoordinate)linearMiddle:(MaplyCoordinateSystem *)coordSys
{
	MaplyCoordinate coord;

	if (![self linearMiddle:&coord rot:nil displayCoordSys:coordSys]) {
		return kMaplyNullCoordinate;
	}

	return coord;
}

- (double)linearMiddleRotation:(MaplyCoordinateSystem *)coordSys
{
	double rot;

	if (![self linearMiddle:nil rot:&rot displayCoordSys:coordSys]) {
		return DBL_MIN;
	}

	return rot;
}


- (MaplyCoordinate)middleCoordinate {
	MaplyCoordinate middle;

	if (![self middleCoordinate:&middle]) {
		return kMaplyNullCoordinate;
	}

	return middle;
}

- (bool)middleCoordinate:(MaplyCoordinate *)middle
{
    if (_shapes.empty())
        return false;

    VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*(_shapes.begin()));
    VectorLinear3dRef lin3d = std::dynamic_pointer_cast<VectorLinear3d>(*(_shapes.begin()));
    if (!lin && !lin3d)
        return false;

    if (lin)
    {
        unsigned long index = lin->pts.size() / 2;
        middle->x = lin->pts[index].x();
        middle->y = lin->pts[index].y();
    } else {
        unsigned long index = lin3d->pts.size() / 2;
        middle->x = lin3d->pts[index].x();
        middle->y = lin3d->pts[index].y();
    }

    return true;
}

- (bool)largestLoopCenter:(MaplyCoordinate *)center mbrLL:(MaplyCoordinate *)ll mbrUR:(MaplyCoordinate *)ur;
{
    // Find the loop with the largest area
    float bigArea = -1.0;
    const VectorRing *bigLoop = NULL;
    for (ShapeSet::iterator it = _shapes.begin();it != _shapes.end();++it)
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

- (MaplyCoordinate)centroid
{
	MaplyCoordinate centroidCoord;

	if (![self centroid:&centroidCoord]) {
		return kMaplyNullCoordinate;
	}

	return centroidCoord;
}

- (bool)centroid:(MaplyCoordinate *)centroid
{
    // Find the loop with the largest area
    float bigArea = -1.0;
    const VectorRing *bigLoop = NULL;
    for (ShapeSet::iterator it = _shapes.begin();it != _shapes.end();++it)
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
                centroid->x = midCoord.x();
                centroid->y = midCoord.y();
                return true;
            } else {
                VectorLinear3dRef linear3d = std::dynamic_pointer_cast<VectorLinear3d>(*it);
                if (linear3d)
                {
                    GeoCoord midCoord = linear3d->geoMbr.mid();
                    centroid->x = midCoord.x();
                    centroid->y = midCoord.y();
                    return true;
                } else {
                    VectorPointsRef pts = std::dynamic_pointer_cast<VectorPoints>(*it);
                    if (pts)
                    {
                        GeoCoord midCoord = pts->geoMbr.mid();
                        centroid->x = midCoord.x();
                        centroid->y = midCoord.y();
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
        centroid->x = centroid2f.x();
        centroid->y = centroid2f.y();
    } else
        return false;
    
    return true;
}

- (MaplyBoundingBox)boundingBox
{
	MaplyBoundingBox bounds;

	if (![self boundingBoxLL:&bounds.ll ur:&bounds.ur]) {
		return kMaplyNullBoundingBox;
	}

	return bounds;
}

- (double)areaOfOuterLoops
{
    // Find the loop with the largest area
    double area = 0.0;
    for (ShapeSet::iterator it = _shapes.begin();it != _shapes.end();++it)
    {
        VectorArealRef areal = std::dynamic_pointer_cast<VectorAreal>(*it);
        if (areal && areal->loops.size() > 0)
        {
            area = CalcLoopArea(areal->loops[0]);
        }
    }
    
    return area;
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
    VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
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
    } else {
        VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*it);
        if (lin)
        {
            const VectorRing &loop = lin->pts;
            NSMutableArray *pts = [NSMutableArray array];
            for (unsigned int ii=0;ii<loop.size();ii++)
            {
                const Point2f &coord = loop[ii];
                CLLocation *loc = [[CLLocation alloc] initWithLatitude:RadToDeg(coord.y()) longitude:RadToDeg(coord.x())];
                [pts addObject:loc];
            }
            [loops addObject:pts];
        } else {
            VectorLinear3dRef lin3d = std::dynamic_pointer_cast<VectorLinear3d>(*it);
            if (lin3d)
            {
                const VectorRing3d &loop = lin3d->pts;
                NSMutableArray *pts = [NSMutableArray array];
                for (unsigned int ii=0;ii<loop.size();ii++)
                {
                    const Point3d &coord = loop[ii];
                    CLLocation *loc = [[CLLocation alloc] initWithLatitude:RadToDeg(coord.y()) longitude:RadToDeg(coord.x())];
                    [pts addObject:loc];
                }
                [loops addObject:pts];
            }
        }
    }
    
    return loops;
}

- (NSArray *)asNumbers
{
    if (_shapes.size() < 1)
        return nil;
    
    NSMutableArray *outPts = [NSMutableArray array];
    
    ShapeSet::iterator it = _shapes.begin();
    VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*it);
    if (lin)
    {
        const VectorRing &loop = lin->pts;
        for (unsigned int ii=0;ii<loop.size();ii++)
        {
            const Point2f &coord = loop[ii];
            [outPts addObject:@(coord.x())];
            [outPts addObject:@(coord.y())];
        }
    } else {
        VectorLinear3dRef lin3d = std::dynamic_pointer_cast<VectorLinear3d>(*it);
        if (lin3d)
        {
            const VectorRing3d &loop = lin3d->pts;
            for (unsigned int ii=0;ii<loop.size();ii++)
            {
                const Point3d &coord = loop[ii];
                [outPts addObject:@(coord.x())];
                [outPts addObject:@(coord.y())];
            }
        }
    }
    
    return outPts;
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
        VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*it);
        if (lin)
        {
            std::vector<Point2f> outPts;
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
                        std::vector<Point2f> outPts;
                        SubdivideEdgesToSurface(ar->loops[ii], outPts, true, &adapter, epsilon);
                        ar->loops[ii] = outPts;
                    }
                }
            }
        }
    }
}

- (void)subdivideToInternal:(float)epsilon adapter:(WhirlyKit::CoordSystemDisplayAdapter *)adapter edgeMode:(bool)edgeMode
{
    CoordSystem *coordSys = adapter->getCoordSystem();
    
    for (ShapeSet::iterator it = _shapes.begin();it!=_shapes.end();it++)
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

// For some reason this is not appearing in the build
- (void)subdivideToFlatGreatCircle:(float)epsilon
{
    FakeGeocentricDisplayAdapter adapter;
    
    [self subdivideToInternal:epsilon adapter:&adapter edgeMode:true];
}

- (void)subdivideToGlobeGreatCircle:(float)epsilon
{
    FakeGeocentricDisplayAdapter adapter;
    
    [self subdivideToInternal:epsilon adapter:&adapter edgeMode:false];
}

- (MaplyVectorObject *) tesselate
{
    MaplyVectorObject *newVec = [[MaplyVectorObject alloc] init];
    
    for (ShapeSet::iterator it = _shapes.begin();it!=_shapes.end();it++)
    {
        VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
        if (ar)
        {
            VectorTrianglesRef trisRef = VectorTriangles::createTriangles();
            TesselateLoops(ar->loops, trisRef);
            trisRef->setAttrDict(ar->getAttrDict());
            trisRef->initGeoMbr();
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
        VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
        if (ar)
        {
            std::vector<VectorRing> newLoops;
            ClipLoopsToGrid(ar->loops, Point2f(0.0,0.0), Point2f(gridSize.width,gridSize.height), newLoops);
            for (unsigned int jj=0;jj<newLoops.size();jj++)
            {
                VectorArealRef newAr = VectorAreal::createAreal();
                newAr->setAttrDict(ar->getAttrDict());
                newAr->loops.push_back(newLoops[jj]);
                newVec->_shapes.insert(newAr);
            }
        }
    }
    
    return newVec;
}

- (MaplyVectorObject *) clipToMbr:(MaplyCoordinate)ll upperRight:(MaplyCoordinate)ur
{
    MaplyVectorObject *newVec = [[MaplyVectorObject alloc] init];
    
    Mbr mbr(Point2f(ll.x, ll.y), Point2f(ur.x, ur.y));
    
    for (ShapeSet::iterator it = _shapes.begin();it!=_shapes.end();it++)
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
                newVec->_shapes.insert(newLinear);
            }
        } else if(std::dynamic_pointer_cast<VectorLinear3d>(*it) != NULL)
        {
            NSLog(@"Don't know how to clip linear3d objects");
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
                        newVec->_shapes.insert(newAr);
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
                if(pt.x() >= ll.x && pt.x() <= ur.x &&
                   pt.y() >= ll.y && pt.y() <= ur.y)
                {
                    newPoints->pts.push_back(pt);
                }
            }
        }
    }
    
    return newVec;
}


// Read from the raw vector format in the Vector DB
// Note: Put this somewhere else.
// Note: Bullet proof this if we start getting these over the network
+ (MaplyVectorObject *)VectorObjectFromVectorDBRaw:(NSData *)data
{
    MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] init];
    
    try
    {
        // Wrap a reader around the NSData
        Maply::DataReader dataReader(data);
        
        // String table first
        std::vector<NSString *> strings;
        int numStrings = dataReader.getInt();
        dataReader.rangeCheck(numStrings, 0, 1000000);
        strings.resize(numStrings,nil);
        for (unsigned int ii=0;ii<numStrings;ii++)
            strings[ii] = dataReader.getString();
        
        // Each chunk has a group of shared attributes
        int numChunks = dataReader.getInt();
        dataReader.rangeCheck(numChunks, 0, 1000000);
        for (int ii=0;ii<numChunks;ii++)
        {
            // All the attributes are shared within the chunk
            NSMutableDictionary *attrDict = [NSMutableDictionary dictionary];
            int numAttrs = dataReader.getInt();
            dataReader.rangeCheck(numAttrs, 0, 10000);
            for (unsigned int jj=0;jj<numAttrs;jj++)
            {
                // Name is index into the string table
                int nameIdx = dataReader.getInt();
                dataReader.rangeCheck(nameIdx, 0, (int)strings.size());
                NSString *name = strings[nameIdx];
                
                // Type
                int type = dataReader.getInt();
                switch (type)
                {
                    case 0:
                        attrDict[name] = @(dataReader.getInt());
                        break;
                    case 1:
                        attrDict[name] = @(dataReader.getFloat());
                        break;
                    case 2:
                    {
                        int valIdx = dataReader.getInt();
                        dataReader.rangeCheck(valIdx, 0, (int)strings.size());
                        attrDict[name] = strings[valIdx];
                    }
                        break;
                    default:
                        break;
                }
            }
            
            // Geometry type and number of features in chunk
            int geomType = dataReader.getInt();
            int numFeat = dataReader.getInt();
            dataReader.rangeCheck(numFeat, 0, 1000000);
            
            // Work through the features
            for (int jj=0;jj<numFeat;jj++)
            {
                VectorShapeRef shape;
                switch (geomType)
                {
                        // Point
                    case 1:
                    {
                        VectorPointsRef pts = VectorPoints::createPoints();
                        shape = pts;
                        Point2f pt;
                        pt.x() = dataReader.getFloat();
                        pt.y() = dataReader.getFloat();
                        pts->pts.push_back(pt);
                        pts->initGeoMbr();
                    }
                        break;
                        // Line string
                    case 2:
                    {
                        int numPts = dataReader.getInt();
                        dataReader.rangeCheck(numPts, 0, 100000);
                        VectorLinearRef lin = VectorLinear::createLinear();
                        shape = lin;
                        lin->pts.reserve(numPts);
                        for (int pi=0;pi<numPts;pi++)
                        {
                            Point2f pt;
                            pt.x() = dataReader.getFloat();
                            pt.y() = dataReader.getFloat();
                            lin->pts.push_back(pt);
                        }
                        lin->initGeoMbr();
                    }
                        break;
                        // Polygon
                    case 3:
                    {
                        int numLoops = dataReader.getInt();
                        dataReader.rangeCheck(numLoops, 0, 100000);
                        VectorArealRef ar = VectorAreal::createAreal();
                        shape = ar;
                        ar->loops.resize(numLoops);
                        for (int li=0;li<numLoops;li++)
                        {
                            VectorRing &ring = ar->loops[li];
                            int numPts = dataReader.getInt();
                            dataReader.rangeCheck(numPts, 0, 100000);
                            ring.reserve(numPts);
                            for (int pi=0;pi<numPts;pi++)
                            {
                                Point2f pt;
                                pt.x() = dataReader.getFloat();
                                pt.y() = dataReader.getFloat();
                                ring.push_back(pt);
                            }
                        }
                        ar->initGeoMbr();
                    }
                        break;
                    default:
                        throw 1;
                        break;
                }
                
                // Note: Might want to copy this if we're going to mess with it later
                shape->setAttrDict(attrDict);
                vecObj.shapes.insert(shape);
            }
        }

        if (dataReader.pos < dataReader.len-1)
            NSLog(@"Failed to read full tile.");
    }
    catch (...)
    {
        NSLog(@"Failed to read VectorDB tile.");
        return nil;
    }
    
    return vecObj;
}


- (void)addShape:(WhirlyKit::VectorShapeRef)shape {
  self.shapes.insert(shape);
}


@end

@implementation MaplyVectorDatabase
{
    VectorDatabase *vectorDb;
    NSString *baseName;
}

- (instancetype)initWithShape:(NSString *)shapeName
{
	if (self = [super init]) {
		NSString *fileName = [[NSBundle mainBundle] pathForResource:shapeName ofType:@"shp"];
		VectorDatabase *vecDb = new VectorDatabase([[NSBundle mainBundle] resourcePath],[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject],shapeName,new ShapeReader(fileName),NULL);

		vectorDb = vecDb;
		baseName = shapeName;
	}

	return self;
}

- (instancetype)initWithShapefileInDocuments:(NSString *)shapeName
{
    if (self = [super init]) {

        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *documentsDirectory = [paths objectAtIndex:0];
        NSString *filePath = [documentsDirectory stringByAppendingPathComponent: shapeName];
        VectorDatabase *vecDb = new VectorDatabase(documentsDirectory, documentsDirectory, shapeName, new ShapeReader(filePath), NULL);

        vectorDb = vecDb;
        baseName = shapeName;
    }

    return self;
}



/// Construct from a shapefile in the bundle
+ (MaplyVectorDatabase *) vectorDatabaseWithShape:(NSString *)shapeName
{
    return [[MaplyVectorDatabase alloc] initWithShape:shapeName];
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
