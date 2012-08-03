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

#import "WGVectorObject.h"
#import "WGVectorObject_private.h"
#import <WhirlyGlobe.h>

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation WGVectorObject

@synthesize shapes;

/// Parse vector data from geoJSON.  Returns one object to represent
//   the whole thing, which might include multiple different vectors.
+ (WGVectorObject *)VectorObjectFromGeoJSON:(NSData *)geoJSON
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

/// Construct with a single point
- (id)initWithPoint:(WGCoordinate *)coord attributes:(NSDictionary *)attr
{
    self = [super init];
    
    if (self)
    {
        VectorPointsRef pts = VectorPoints::createPoints();
        pts->pts.push_back(GeoCoord(coord->lon,coord->lat));
        pts->setAttrDict([NSMutableDictionary dictionaryWithDictionary:attr]);
        shapes.insert(pts);
    }
    
    return self;
}

/// Construct with a linear feature (e.g. line string)
- (id)initWithLineString:(WGCoordinate *)coords numCoords:(int)numCoords attributes:(NSDictionary *)attr
{
    self = [super init];
    
    if (self)
    {
        VectorLinearRef lin = VectorLinear::createLinear();
        for (unsigned int ii=0;ii<numCoords;ii++)
            lin->pts.push_back(GeoCoord(coords[ii].lon,coords[ii].lat));
        lin->setAttrDict([NSMutableDictionary dictionaryWithDictionary:attr]);
        shapes.insert(lin);
    }
    
    return self;
}

/// Construct as an areal with an exterior
- (id)initWithAreal:(WGCoordinate *)coords numCoords:(int)numCoords attributes:(NSDictionary *)attr
{
    self = [super init];
    
    if (self)
    {
        VectorArealRef areal = VectorAreal::createAreal();
        VectorRing pts;
        for (unsigned int ii=0;ii<numCoords;ii++)
            pts.push_back(GeoCoord(coords[ii].lat,coords[ii].lon));
        areal->loops.push_back(pts);
        areal->setAttrDict([NSMutableDictionary dictionaryWithDictionary:attr]);
        shapes.insert(areal);
    }
    
    return self;
}

/// Add a hole to an existing areal feature
- (void)addHole:(WGCoordinate *)coords numCoords:(int)numCoords
{
    if (shapes.size() != 1)
        return;
    
    VectorArealRef areal = boost::dynamic_pointer_cast<VectorAreal>(*(shapes.begin()));
    if (areal)
    {
        VectorRing pts;
        for (unsigned int ii=0;ii<numCoords;ii++)
            pts.push_back(GeoCoord(coords[ii].lat,coords[ii].lon));
        areal->loops.push_back(pts);        
    }
}

// Look for areals that this point might be inside
- (bool)pointInAreal:(WGCoordinate)coord
{
    for (ShapeSet::iterator it = shapes.begin();it != shapes.end();++it)
    {
        VectorArealRef areal = boost::dynamic_pointer_cast<VectorAreal>(*it);
        if (areal)
        {
            if (areal->pointInside(GeoCoord(coord.lon,coord.lat)))
                return true;
        }
    }
                
    return false;
}

// Calculate a center
- (WGCoordinate)center
{
    Mbr mbr;
    for (ShapeSet::iterator it = shapes.begin();it != shapes.end();++it)
    {
        GeoMbr geoMbr = (*it)->calcGeoMbr();
        mbr.addPoint(geoMbr.ll());
        mbr.addPoint(geoMbr.ur());
    }
    
    WGCoordinate ctr;
    ctr.lon = (mbr.ll().x() + mbr.ur().x())/2.0;
    ctr.lat = (mbr.ll().y() + mbr.ur().y())/2.0;

    return ctr;
}

- (WGCoordinate)largestLoopCenter
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

    WGCoordinate ctr;
    ctr.lon = 0;  ctr.lat = 0;
    if (bigLoop)
    {
        Mbr mbr;
        mbr.addPoints(*bigLoop);
        ctr.lon = (mbr.ll().x() + mbr.ur().x())/2.0;
        ctr.lat = (mbr.ll().y() + mbr.ur().y())/2.0;
    }

    return ctr;
}


@end
