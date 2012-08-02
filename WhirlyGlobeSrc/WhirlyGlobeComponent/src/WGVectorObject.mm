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

@end
