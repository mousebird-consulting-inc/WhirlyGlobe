/*
 *  WGVectorObject.h
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

#import <Foundation/Foundation.h>
#import <MaplyCoordinate.h>

/** Maply Component Vector Object.
    This can represent one or more vector features parsed out of GeoJSON.
  */
@interface MaplyVectorObject : NSObject
{
    /// For user data
    NSObject *userObject;
    /// Turn this off to make this vector invisible to selection.
    /// On by default.
    bool selectable;
}

@property (nonatomic,strong) NSObject *userObject;
@property (nonatomic,assign) bool selectable;

/// Get the attributes.  If it's a multi-object this will just return the first
///  attribute dictionary.
@property (nonatomic,readonly) NSDictionary *attributes;

/// Parse vector data from geoJSON.  Returns one object to represent
///  the whole thing, which might include multiple different vectors.
/// We assume the geoJSON is all in decimal degrees in WGS84. 
+ (MaplyVectorObject *)VectorObjectFromGeoJSON:(NSData *)geoJSON;

/// Construct with a single point
- (id)initWithPoint:(MaplyCoordinate *)coord attributes:(NSDictionary *)attr;
/// Construct with a linear feature (e.g. line string)
- (id)initWithLineString:(MaplyCoordinate *)coords numCoords:(int)numCoords attributes:(NSDictionary *)attr;
/// Construct as an areal with an exterior
- (id)initWithAreal:(MaplyCoordinate *)coords numCoords:(int)numCoords attributes:(NSDictionary *)attr;

/// Add a hole to an existing areal feature
- (void)addHole:(MaplyCoordinate *)coords numCoords:(int)numCoords;

/// For areal features, check if the given point is inside
- (bool)pointInAreal:(MaplyCoordinate)coord;

/// Calculate and return the center of the whole object
- (MaplyCoordinate)center;

/// Calculate the center and extents of the largest loop.
/// Returns false if there was no loop
- (bool)largestLoopCenter:(MaplyCoordinate *)center mbrLL:(MaplyCoordinate *)ll mbrUR:(MaplyCoordinate *)ur;

/// Vector objects can encapsulate multiple objects since they're read from GeoJSON.
/// This splits any multiples into single objects.
- (NSArray *)splitVectors;

/// This will break up long edges in a vector until they lie flat on a globe to a given
///  epsilon.  The epislon is in display coordinates (radius = 1.0).
- (void)subdivideToGlobe:(float)epsilon;

@end

typedef MaplyVectorObject WGVectorObject;


/** Maply Vector Database.  This object encapsulates a simple database of vector features,
    possibly a Shapefile.
 */
@interface MaplyVectorDatabase : NSObject

/// Construct from a shapefile in the bundle
+ (MaplyVectorDatabase *) vectorDatabaseWithShape:(NSString *)shapeName;

+ (MaplyVectorDatabase *) vectorDatabaseWithShapePath:(NSString *)shapeFileName;


/// Return vectors that match the given SQL query
- (MaplyVectorObject *)fetchMatchingVectors:(NSString *)sqlQuery;

/// Search for all the areals that surround the given point (in geographic)
- (MaplyVectorObject *)fetchArealsForPoint:(MaplyCoordinate)coord;

/// Fetch all the vectors in the database
- (MaplyVectorObject *)fetchAllVectors;

@end
