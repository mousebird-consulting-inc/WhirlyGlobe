/*
 *  WGVectorObject.mm
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 8/2/12.
 *  Copyright 2012-2019 mousebird consulting
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

#import "visual_objects/MaplyVectorObject.h"
#import "MaplyVectorObject_private.h"
#import <WhirlyGlobe_iOS.h>
#import "Tesselator.h"
#import "GridClipper.h"
#import <CoreLocation/CoreLocation.h>
#import "MaplyRenderController_private.h"
#import "MaplyViewController.h"
#import "MaplyCoordinateSystem_private.h"
#import "MaplyCoordinateSystem_private.h"
#import "MaplyViewController.h"
#import "VectorData_iOS.h"
#import "Dictionary_NSDictionary.h"
#import "MaplyBaseViewController_private.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation MaplyVectorObject

+ (MaplyVectorObject *)VectorObjectFromGeoJSON:(NSData *)geoJSON
{
	return [[MaplyVectorObject alloc] initWithGeoJSON:geoJSON];
}

+ (NSDictionary *)VectorObjectsFromGeoJSONAssembly:(NSData *)geoJSON
{
    if ([geoJSON length] > 0)
    {
        NSString *nsStr = [[NSString alloc] initWithData:geoJSON encoding:NSUTF8StringEncoding];
        if (!nsStr)
            return nil;
        std::string str = [nsStr UTF8String];
        
        std::map<std::string,ShapeSet> shapes;
        if (!VectorParseGeoJSONAssembly(str, shapes))
            return nil;
        
        NSMutableDictionary *dict = [NSMutableDictionary dictionary];
        for (std::map<std::string,ShapeSet>::iterator it = shapes.begin();
             it != shapes.end(); ++it)
        {
            NSString *str = [NSString stringWithFormat:@"%s",it->first.c_str()];
            if (str)
            {
                MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] init];
                vecObj->vObj->shapes = it->second;
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

- (instancetype)init
{
    self = [super init];
    if (!self)
        return nil;
    
    vObj = VectorObjectRef(new VectorObject());
    
    return self;
}

- (id)initWithRef:(WhirlyKit::VectorObjectRef)vecObj
{
    self = [super init];
    
    vObj = vecObj;
    
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
        iosMutableDictionary *dict = new iosMutableDictionary([NSMutableDictionary dictionaryWithDictionary:attr]);

        vObj = VectorObjectRef(new VectorObject());

        pts->setAttrDict(MutableDictionaryRef(dict));
        pts->initGeoMbr();
        vObj->shapes.insert(pts);
    }
    
    return self;
}

- (instancetype)initWithLineString:(NSArray *)inCoords attributes:(NSDictionary *)attr
{
	MaplyCoordinate *coords = (MaplyCoordinate *) malloc(sizeof(int) * [inCoords count]/2);

	for (int i = 0; i < [inCoords count]/2; i++) {
		float x = [inCoords[2*i] floatValue];
		float y = [inCoords[2*i+1] floatValue];

		coords[i] = MaplyCoordinateMakeWithDegrees(x, y);
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
        vObj = VectorObjectRef(new VectorObject());

        VectorLinearRef lin = VectorLinear::createLinear();
        for (unsigned int ii=0;ii<numCoords;ii++)
            lin->pts.push_back(GeoCoord(coords[ii].x,coords[ii].y));
        iosMutableDictionary *dict = new iosMutableDictionary([NSMutableDictionary dictionaryWithDictionary:attr]);
        lin->setAttrDict(MutableDictionaryRef(dict));
        lin->initGeoMbr();
        vObj->shapes.insert(lin);
    }
    
    return self;
}

/// Construct as an areal with an exterior
- (instancetype)initWithAreal:(MaplyCoordinate *)coords numCoords:(int)numCoords attributes:(NSDictionary *)attr
{
    self = [super init];
    
    if (self)
    {
        vObj = VectorObjectRef(new VectorObject());

        VectorArealRef areal = VectorAreal::createAreal();
        VectorRing pts;
        for (unsigned int ii=0;ii<numCoords;ii++)
            pts.push_back(GeoCoord(coords[ii].x,coords[ii].y));
        areal->loops.push_back(pts);
        iosMutableDictionary *dict = new iosMutableDictionary([NSMutableDictionary dictionaryWithDictionary:attr]);
        areal->setAttrDict(MutableDictionaryRef(dict));
        areal->initGeoMbr();
        vObj->shapes.insert(areal);
    }
    
    return self;
}

/// Construct from GeoJSON
- (instancetype)initWithGeoJSON:(NSData *)geoJSON
{
	if ([geoJSON length] == 0)
		return nil;

	self = [super init];

    vObj = VectorObjectRef(new VectorObject());

    NSString *nsStr = [[NSString alloc] initWithData:geoJSON encoding:NSUTF8StringEncoding];
    if (!nsStr)
        return nil;
    std::string str = [nsStr UTF8String];
    std::string crs = "";
    if (!vObj->fromGeoJSON(str, crs))
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

    vObj = VectorObjectRef(new VectorObject());

	if (self = [super init]) {
		if (!VectorParseGeoJSON(vObj->shapes, jsonDict))
			return nil;
	}

	return self;
}

- (instancetype)initWithGeoJSONDictionary:(NSDictionary *)geoJSON
{
	if (![geoJSON isKindOfClass:[NSDictionary class]])
		return nil;

    vObj = VectorObjectRef(new VectorObject());

	if (self = [super init]) {
		if (!VectorParseGeoJSON(vObj->shapes, geoJSON))
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
    
    vObj = VectorObjectRef(new VectorObject());
    if (!vObj->fromShapeFile([fileName cStringUsingEncoding:NSASCIIStringEncoding]))
        return nil;

	return self;
}

- (void)setSelectable:(bool)selectable
{
    vObj->setIsSelectable(selectable);
}

- (bool)selectable
{
    return vObj->isSelectable();
}

- (NSMutableDictionary *)attributes
{
    if (vObj->shapes.empty())
        return nil;
    
    VectorShapeRef vec = *(vObj->shapes.begin());
    iosMutableDictionary *dict = (iosMutableDictionary *)vec->getAttrDict().get();
    return dict ? dict->dict : nil;
}

- (void)setAttributes:(NSDictionary *)attributes
{
    MutableDictionaryRef dict(new iosMutableDictionary([NSMutableDictionary dictionaryWithDictionary:attributes]));
    vObj->setAttributes(dict);
}

- (void)mergeVectorsFrom:(MaplyVectorObject *)otherVec
{
    vObj->mergeVectorsFrom(otherVec->vObj.get());
}

/// Add a hole to an existing areal feature
- (void)addHole:(MaplyCoordinate *)coords numCoords:(int)numCoords
{
    if (vObj->shapes.size() != 1)
        return;
    
    VectorRing pts;
    for (unsigned int ii=0;ii<numCoords;ii++)
        pts.push_back(GeoCoord(coords[ii].x,coords[ii].y));
    vObj->addHole(pts);
}

- (MaplyVectorObjectType)vectorType
{
    auto type = vObj->getVectorType();
    MaplyVectorObjectType retType = MaplyVectorNoneType;
    switch (type)
    {
        case VectorNoneType:
            retType = MaplyVectorNoneType;
            break;
        case VectorPointType:
            retType = MaplyVectorPointType;
            break;
        case VectorLinearType:
            retType = MaplyVectorLinearType;
            break;
        case VectorLinear3dType:
            retType = MaplyVectorLinear3dType;
            break;
        case VectorArealType:
            retType = MaplyVectorArealType;
            break;
        case VectorMultiType:
            retType = MaplyVectorMultiType;
            break;
    }
    
    return retType;
}

- (NSString *)log
{
    std::string info = vObj->log();
    return [NSString stringWithFormat:@"%s",info.c_str()];
}

- (MaplyVectorObject *)deepCopy2
{
    MaplyVectorObject *newVecObj = [[MaplyVectorObject alloc] init];
    newVecObj->vObj = vObj->deepCopy();
    
    return newVecObj;
}

- (void)reprojectFrom:(MaplyCoordinateSystem *)srcSystem to:(MaplyCoordinateSystem *)destSystem
{
    CoordSystem *inSystem = srcSystem->coordSystem.get();
    CoordSystem *outSystem = destSystem->coordSystem.get();

    // Note: Heinous hack for meters to radians conversion
    double scale = 1.0;
    if ([srcSystem isKindOfClass:[MaplySphericalMercator class]])
        scale = 1/EarthRadius;
    
    vObj->reproject(inSystem, scale, outSystem);
}

// Look for areals that this point might be inside
- (bool)pointInAreal:(MaplyCoordinate)coord
{
    return vObj->pointInside(Point2d(coord.x,coord.y));
}

//Fuzzy matching for selecting Linear features
- (bool)pointNearLinear:(MaplyCoordinate)coord distance:(float)maxDistance inViewController:(MaplyBaseViewController *)vc
{
    if (!vc->renderControl || !vc->renderControl->visualView)
        return false;
    
    ViewStateRef viewState = vc->renderControl->visualView->makeViewState(vc->renderControl->sceneRenderer.get());

    return vObj->pointNearLinear(Point2d(coord.x,coord.y),maxDistance,viewState,vc->renderControl->sceneRenderer->getFramebufferSizeScaled());
}

// Calculate a center
- (MaplyCoordinate)center
{
    Point2d ctr;
    MaplyCoordinate coord;
    if (vObj->center(ctr)) {
        coord.x = ctr.x();  coord.y = ctr.y();
    } else {
        coord.x = 0.0;  coord.y = 0.0;
    }
    
    
    return coord;
}

- (bool)linearMiddle:(MaplyCoordinate *)middle rot:(double *)rot
{
    Point2d mid;
    bool ret = vObj->linearMiddle(mid, *rot);
    if (ret) {
        middle->x = mid.x();
        middle->y = mid.y();
    } else {
        middle->x = 0.0;
        middle->y = 0.0;
    }
    
    return ret;
}

- (bool)linearMiddle:(MaplyCoordinate *)middle rot:(double *)rot displayCoordSys:(MaplyCoordinateSystem *)maplyCoordSys
{
    Point2d mid;
    bool ret = vObj->linearMiddle(mid, *rot, maplyCoordSys->coordSystem.get());
    if (ret) {
        middle->x = mid.x();
        middle->y = mid.y();
    } else {
        middle->x = 0.0;
        middle->y = 0.0;
    }
    
    return ret;
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
    Point2d mid;
    bool ret = vObj->middleCoordinate(mid);
    if (ret) {
        middle->x = mid.x();
        middle->y = mid.y();
    } else {
        middle->x = 0.0;
        middle->y = 0.0;
    }
    
    return ret;
}

- (bool)largestLoopCenter:(MaplyCoordinate *)center mbrLL:(MaplyCoordinate *)ll mbrUR:(MaplyCoordinate *)ur;
{
    Point2d center2d,ll2d,ur2d;
    
    if (vObj->largestLoopCenter(center2d, ll2d, ur2d))
    {
        center->x = center2d.x();  center->y = center2d.y();
        ll->x = ll2d.x();  ll->y = ll2d.y();
        ur->x = ur2d.x();  ur->y = ur2d.y();
        
        return true;
    } else {
        center->x = 0.0;  center->y = 0.0;
        ll->x = 0.0;  ll->y = 0.0;
        ur->x = 0.0;  ur->y = 0.0;
        
        return false;
    }
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
    Point2d centroid2d;
    if (vObj->centroid(centroid2d)) {
        centroid->x = centroid2d.x();
        centroid->y = centroid2d.y();

        return true;
    } else {
        centroid->x = 0.0;
        centroid->y = 0.0;
        
        return false;
    }
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
    return vObj->areaOfOuterLoops();
}


- (bool)boundingBoxLL:(MaplyCoordinate *)ll ur:(MaplyCoordinate *)ur
{
    Point2d ll2d,ur2d;
    if (vObj->boundingBox(ll2d, ur2d)) {
        ll->x = ll2d.x();  ll->y = ll2d.y();
        ur->x = ur2d.x();  ur->y = ur2d.y();
        return true;
    } else {
        ll->x = 0.0;  ll->y = 0.0;
        ur->x = 0.0;  ur->y = 0.0;
        return false;
    }
}

- (NSArray *)asCLLocationArrays
{
    if (vObj->shapes.size() < 1)
        return nil;

    NSMutableArray *loops = [NSMutableArray array];
    
    ShapeSet::iterator it = vObj->shapes.begin();
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
    if (vObj->shapes.size() < 1)
        return nil;
    
    NSMutableArray *outPts = [NSMutableArray array];
    
    ShapeSet::iterator it = vObj->shapes.begin();
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
    std::vector<VectorObject *> newVecs;
    vObj->splitVectors(newVecs);
    
    NSMutableArray *vecs = [NSMutableArray array];
    for (VectorObject *thisVecObj : newVecs) {
        MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] init];
        vecObj->vObj = VectorObjectRef(thisVecObj);
        [vecs addObject:vecObj];
    }
    
    return vecs;
}

- (void)subdivideToGlobe:(float)epsilon
{
    vObj->subdivideToGlobe(epsilon);
}

// For some reason this is not appearing in the build
- (void)subdivideToFlatGreatCircle:(float)epsilon
{
    vObj->subdivideToFlatGreatCircle(epsilon);
}

- (void)subdivideToGlobeGreatCircle:(float)epsilon
{
    vObj->subdivideToGlobeGreatCircle(epsilon);
}

- (MaplyVectorObject *)linearsToAreals
{
    MaplyVectorObject *newVec = [[MaplyVectorObject alloc] init];
    
    newVec->vObj = vObj->linearsToAreals();
    
    return newVec;
}

- (MaplyVectorObject *)arealsToLinears
{
    MaplyVectorObject *newVec = [[MaplyVectorObject alloc] init];
    
    newVec->vObj = vObj->arealsToLinears();
    
    return newVec;
}

- (MaplyVectorObject *__nonnull)filterClippedEdges
{
    MaplyVectorObject *newVec = [[MaplyVectorObject alloc] init];

    newVec->vObj = vObj->filterClippedEdges();
    
    return newVec;
}

- (MaplyVectorObject *) tesselate
{
    MaplyVectorObject *newVec = [[MaplyVectorObject alloc] init];
    
    newVec->vObj = vObj->tesselate();
    
    return newVec;
}

- (MaplyVectorObject *) clipToGrid:(CGSize)gridSize
{
    MaplyVectorObject *newVec = [[MaplyVectorObject alloc] init];
    
    newVec->vObj = vObj->clipToGrid(Point2d(gridSize.width,gridSize.height));
    
    return newVec;
}

- (MaplyVectorObject *) clipToMbr:(MaplyCoordinate)ll upperRight:(MaplyCoordinate)ur
{
    MaplyVectorObject *newVec = [[MaplyVectorObject alloc] init];
    
    newVec->vObj = vObj->clipToMbr(Point2d(ll.x,ll.y), Point2d(ur.x,ur.y));
    
    return newVec;
}

- (void)addShape:(WhirlyKit::VectorShapeRef)shape {
  vObj->shapes.insert(shape);
}


@end

