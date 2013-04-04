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

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation MaplyVectorObject

@synthesize userObject;
@synthesize selectable;
@synthesize shapes;

/// Parse vector data from geoJSON.  Returns one object to represent
//   the whole thing, which might include multiple different vectors.
+ (WGVectorObject *)VectorObjectFromGeoJSON:(NSData *)geoJSON
{
    if([geoJSON length])
    {
    NSError *error = nil;
    NSDictionary *jsonDict = [NSJSONSerialization JSONObjectWithData:geoJSON options:NULL error:&error];
    if (error || ![jsonDict isKindOfClass:[NSDictionary class]])
        return nil;
    
    WGVectorObject *vecObj = [[WGVectorObject alloc] init];

    if (!VectorParseGeoJSON(vecObj->shapes,jsonDict))
        return nil;

      return vecObj;
    }
    return nil;
}

- (NSDictionary *)attributes
{
    if (shapes.empty())
        return nil;
    
    VectorShapeRef vec = *(shapes.begin());
    return vec->getAttrDict();
}

- (void)setAttributes:(NSDictionary *)attributes
{
    for (ShapeSet::iterator it = shapes.begin();
         it != shapes.end(); ++it)
        (*it)->setAttrDict([NSMutableDictionary dictionaryWithDictionary:attributes]);
}

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    
    selectable = true;
    
    return self;
}

/// Construct with a single point
- (id)initWithPoint:(MaplyCoordinate *)coord attributes:(NSDictionary *)attr
{
    self = [super init];
    
    if (self)
    {
        VectorPointsRef pts = VectorPoints::createPoints();
        pts->pts.push_back(GeoCoord(coord->x,coord->y));
        pts->setAttrDict([NSMutableDictionary dictionaryWithDictionary:attr]);
        pts->initGeoMbr();
        shapes.insert(pts);
        
        selectable = true;
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
        lin->setAttrDict([NSMutableDictionary dictionaryWithDictionary:attr]);
        lin->initGeoMbr();
        shapes.insert(lin);
        
        selectable = true;
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
        areal->setAttrDict([NSMutableDictionary dictionaryWithDictionary:attr]);
        areal->initGeoMbr();
        shapes.insert(areal);
        
        selectable = true;
    }
    
    return self;
}

/// Add a hole to an existing areal feature
- (void)addHole:(MaplyCoordinate *)coords numCoords:(int)numCoords
{
    if (shapes.size() != 1)
        return;
    
    VectorArealRef areal = boost::dynamic_pointer_cast<VectorAreal>(*(shapes.begin()));
    if (areal)
    {
        VectorRing pts;
        for (unsigned int ii=0;ii<numCoords;ii++)
            pts.push_back(GeoCoord(coords[ii].x,coords[ii].y));
        areal->loops.push_back(pts);        
    }
}

// Look for areals that this point might be inside
- (bool)pointInAreal:(MaplyCoordinate)coord
{
    for (ShapeSet::iterator it = shapes.begin();it != shapes.end();++it)
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
    for (ShapeSet::iterator it = shapes.begin();it != shapes.end();++it)
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

- (bool)largestLoopCenter:(MaplyCoordinate *)center mbrLL:(MaplyCoordinate *)ll mbrUR:(MaplyCoordinate *)ur;
{
    // Find the loop with the larest area
    float bigArea = -1.0;
    const VectorRing *bigLoop = NULL;
    for (ShapeSet::iterator it = shapes.begin();it != shapes.end();++it)
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

- (NSArray *)splitVectors
{
    NSMutableArray *vecs = [NSMutableArray array];
    
    for (WhirlyKit::ShapeSet::iterator it = shapes.begin();
         it != shapes.end(); ++it)
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
    
    for (ShapeSet::iterator it = shapes.begin();it!=shapes.end();it++)
    {
        VectorLinearRef lin = boost::dynamic_pointer_cast<VectorLinear>(*it);
        if (lin)
        {
            std::vector<Point2f> outPts;
            SubdivideEdgesToSurface(lin->pts, outPts, false, &adapter, epsilon);
            lin->pts = outPts;
        } else {
            VectorArealRef ar = boost::dynamic_pointer_cast<VectorAreal>(*it);
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

@end

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
