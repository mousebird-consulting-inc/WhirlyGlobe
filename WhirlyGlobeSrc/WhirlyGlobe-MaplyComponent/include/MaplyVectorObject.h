/*
 *  MaplyVectorObject.h
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

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "MaplyCoordinate.h"
#import "MaplyCoordinateSystem.h"

@class MaplyBaseViewController;

/// Data type for the vector.  Multi means it contains multiple types
typedef NS_ENUM(NSInteger, MaplyVectorObjectType) {
	MaplyVectorNoneType,
	MaplyVectorPointType,
    MaplyVectorLinearType,
    MaplyVectorLinear3dType,
	MaplyVectorArealType,
	MaplyVectorMultiType,
};


/** @brief Maply Vector Object represents zero or more vector features.
    @details The Vector Object can hold several vector features of the same or different types.  It's meant to be a fairly opaque structure, often read from GeoJSON or Shapefiles.  It's less opaque than originally planned, however, and sports a number of specific methods.
    @details If you're doing real vector manipulation, it's best to do it somewhere else and then create one of these as needed for display.
    @details Vector Objects can be created directly or read from a MaplyVectorDatabase.  They are typically then displayed on top of a MaplyViewController or WhirlyGlobeViewController as vectors.
    @details Vector Objects vertices are always in geographic, with longitude = x and latitude = y.
  */
@interface MaplyVectorObject : NSObject

/** @brief User data object for selection
 @details When the user selects a feature and the developer gets it in their delegate, this is an object they can use to figure out what the vector object means to them.
 */
@property (nonatomic,strong) id __nullable userObject;

/// @brief Turn this off to make this vector invisible to selection.
/// @details On by default.
@property (nonatomic,assign) bool selectable;

/** @brief Return the attributes for the vector object.
    @details All vectors should have some set of attribution.  If there's more than one vector feature here, we'll return the attributes on the first one.
    @details The attribution is returned as an NSDictionary and, though you can modify it, you probably shouldn't.
  */
@property (nonatomic,readonly) NSMutableDictionary *__nonnull attributes;

/** @brief Parse vector data from geoJSON.
    @details Returns one object to represent the whole thing, which might include multiple different vectors.  This version uses the faster JSON parser.
    @details We assume the geoJSON is all in decimal degrees in WGS84.
  */
+ (MaplyVectorObject *__nullable)VectorObjectFromGeoJSON:(NSData *__nonnull)geoJSON;

/** @brief Parse vector data from geoJSON.
    @details Returns one object to represent the whole thing, which might include multiple different vectors.  This version uses slower JSON parser.
    @details We assume the geoJSON is all in decimal degrees in WGS84.
*/
+ (MaplyVectorObject *__nullable)VectorObjectFromGeoJSONApple:(NSData *__nonnull)geoJSON;

/** @brief Parse vector data from geoJSON.
    @details Returns one object to represent the whole thing, which might include multiple different vectors.  This version parses its data from an NSDictionary, which had to be parsed from JSON at some point.  Probably the slower path.
    @details We assume the geoJSON is all in decimal degrees in WGS84.
 */
+ (MaplyVectorObject *__nullable)VectorObjectFromGeoJSONDictionary:(NSDictionary *__nonnull)geoJSON;

/** @brief Read vector objects from the given cache file.
    @details MaplyVectorObject's can be written and read from a binary file.  We use this for caching data locally on the device.
    @param fileName Name of the binary vector file.
    @return The vector object(s) read from the file or nil on failure.
  */
+ (MaplyVectorObject *__nullable)VectorObjectFromFile:(NSString *__nonnull)fileName;

/** @brief Read vector objects from the given shapefile.
    @details This will read all the shapes in the given shapefile into memory and return them as one MaplyVectorObject.
    @param fileName The basename of the shape file.  Don't include the extension.
    @return The vector object(s) read from the file or nil on failure.
  */
+ (MaplyVectorObject *__nullable)VectorObjectFromShapeFile:(NSString *__nonnull)fileName;

/** @brief Parse vector objects from a JSON assembly.
    @details This version can deal with non-compliant assemblies returned by the experimental OSM server
  */
+ (NSDictionary *__nullable)VectorObjectsFromGeoJSONAssembly:(NSData *__nonnull)geoJSON;

/** @brief Initialize with a single data point and attribution.
 @details This version takes a single coordinate and the attributes to go with it.
 */
- (nonnull instancetype)initWithPoint:(MaplyCoordinate)coord attributes:(NSDictionary *__nullable)attr;

/** @brief Initialize with a single data point and attribution.
    @details This version takes a single coordinate and the attributes to go with it.
  */
- (nonnull instancetype)initWithPointRef:(MaplyCoordinate *__nonnull)coord attributes:(NSDictionary *__nullable)attr;

/** @brief Initialize with a linear feature.
 @details This version takes an array of coordinate pairs (as NSNumber) and the attribution.  With this it will make a linear feature.
 */
- (nonnull instancetype)initWithLineString:(NSArray *__nonnull)coords attributes:(NSDictionary *__nullable)attr;

/** @brief Initialize with a linear feature.
    @details This version takes an array of coordinates, the size of that array and the attribution.  With this it will make a linear feature.
  */
- (nonnull instancetype)initWithLineString:(MaplyCoordinate *__nonnull)coords numCoords:(int)numCoords attributes:(NSDictionary *__nullable)attr;

/** @brief Inintialize as an areal feature.
    @details This version takes an array of coordinates, the size of that array and the attribution.  With this it will make a single area feature with one (exterior) loop.  To add loops, call addHole:numCoords:
  */
- (nonnull instancetype)initWithAreal:(MaplyCoordinate *__nonnull)coords numCoords:(int)numCoords attributes:(NSDictionary *__nullable)attr;

/** @brief Initializes with vectors parsed from geoJSON.
	@details Returns one object to represent the whole thing, which might include multiple different vectors.  This version uses the faster JSON parser.
    @details We assume the geoJSON is all in decimal degrees in WGS84.
 */
- (nullable instancetype)initWithGeoJSON:(NSData *__nonnull)geoJSON;

/** @brief Initializes with vector parsed from geoJSON.
	@details Returns one object to represent the whole thing, which might include multiple different vectors.  This version uses slower JSON parser.
	@details We assume the geoJSON is all in decimal degrees in WGS84.
 */
- (nullable instancetype)initWithGeoJSONApple:(NSData *__nonnull)geoJSON;

/** @brief Initializes with vector parsed from geoJSON.
	@details Returns one object to represent the whole thing, which might include multiple different vectors.  This version parses its data from an NSDictionary, which had to be parsed from JSON at some point.  Probably the slower path.
	@details We assume the geoJSON is all in decimal degrees in WGS84.
 */
- (nullable instancetype)initWithGeoJSONDictionary:(NSDictionary *__nonnull)geoJSON;

/** @brief Initializes with vectors read from the given cache file.
	@details MaplyVectorObject's can be written and read from a binary file.  We use this for caching data locally on the device.
	@param fileName Name of the binary vector file.
	@return The vector object(s) read from the file or nil on failure.
 */
- (nullable instancetype)initWithFile:(NSString *__nonnull)fileName;

/** @brief Initializes with vectors read from the given shapefile.
	@details This will read all the shapes in the given shapefile into memory and return them as one MaplyVectorObject.
	@param fileName The basename of the shape file.  Don't include the extension.
	@return The vector object(s) read from the file or nil on failure.
 */
- (nullable instancetype)initWithShapeFile:(NSString *__nonnull)fileName;


/** @brief Write the vector object to the given file on the device.
    @details We support a binary format for caching vector data.  Typically you write these files on the device or in the simulator and then put them in a place you can easily find them when needed.
    @param fileName The file to read the vector data from.
    @return Returns true on succes, false on failure.
  */
- (bool)writeToFile:(NSString *__nonnull)fileName;

/** @brief Make a deep copy of the vector object and return it.
    @details This makes a complete copy of the vector object, with all features and nothing shared.
    @details Had to rename this because Apple's private method scanner is dumb.
  */
- (MaplyVectorObject *__nonnull)deepCopy2;

/** @brief Reproject from one coordinate system to another.
    @details This reprojects every single point in the points, linears, and areals (and mesh) from the source coordinate system to the destionation.
    @details Typically, you'll want Plate Carree for display, the destSystem is probably that.
    @details For various reasons (e.g. scale), this will probably not work right for you.
    @param srcSystem The source coordinate system.  The data is already in this sytem.
    @param destSystem The destination coordinate system.  The data will be in this system on return.
  */
- (void)reprojectFrom:(MaplyCoordinateSystem *__nonnull)srcSystem to:(MaplyCoordinateSystem *__nonnull)destSystem;

/** @brief Dump the feature(s) out as text
    @details This will write each feature out as text for debugging.
  */
- (NSString *__nonnull)log;

/** @brief Add a hole to an existing feature.
    @details This method is expecting to find exactly one areal feature.  If it finds one, it will add the given hole as a loop on the end of the list of loops.
  */
- (void)addHole:(MaplyCoordinate *__nonnull)coords numCoords:(int)numCoords;

/** @brief Returns the type of the vector feature.
    @details This method returns the type of the vector.  Since vector objects can contain multiple types of vectors at once, this is somewhat complicated.
 
|Type | Description |
|:----|:-----------:|
|MaplyVectorNoneType | There are no features in this object. |
|MaplyVectorPointType | There are only points (and multi-points) in the object. |
|MaplyVectorLinearType | There are only linear features in the object. |
|MaplyVectorLinear3dType | There are only linear features with Z values in the object. |
|MaplyVectorArealType | There are only areal features in the object. |
|MaplyVectorMultiType | There are multiple features of different types in the object. |
  */
- (MaplyVectorObjectType)vectorType;

/** @brief Run a point in polygon test on all the areal features within the object.
    @details We'll run a point in polygon test on all the areal features within the vector object.  If the point is within one of them, we return true, otherwise false.
  */
- (bool)pointInAreal:(MaplyCoordinate)coord;

/** @brief Test if any linear feature is within distance of coord
 */
- (bool)pointNearLinear:(MaplyCoordinate)coord distance:(float)maxDistance inViewController:(MaplyBaseViewController *__nonnull)vc;

/** @brief Calculate the center of the entire set of vectors in this object.
  */
- (MaplyCoordinate)center;

/** @brief Copy the vectors in the given vector object into this one.
  */
- (void)mergeVectorsFrom:(MaplyVectorObject *__nonnull)otherVec;

/** @brief For a linear feature, calculate the mid oint and rotation at that point.
    @details The vector object contains a number of half baked geometric queries, this being one of them.
    @details This finds the middle (as measured by distance) of a linear feature and then calculations an angle corresponding to the line segment that middle sits in.
    @details Why?  Think label road placement.
  */
- (bool)linearMiddle:(MaplyCoordinate *__nonnull)middle rot:(double *__nonnull)rot;

- (bool)linearMiddle:(MaplyCoordinate *__nullable)middle rot:(double *__nullable)rot displayCoordSys:(MaplyCoordinateSystem *__nonnull)coordSys;

/** @brief For a linear feature, calculate the mid point.
 @details This is a convenience method to be called without pointers (Swift)
 @details If you need both the mid point and the rotation, this method is less efficient than the method with pointers.
 @return kMaplyNullCoordinate in case of error

 */
- (MaplyCoordinate)linearMiddle:(MaplyCoordinateSystem *__nonnull)coordSys;

/** @brief For a linear feature, calculate the mid point and returns the rotation at that point.
 @details This is a convenience method to be called without pointers (Swift)
 @details If you need both the mid point and the rotation, this method is less efficient than the method with pointers.
 @return DBL_MIN in case of error
 */
- (double)linearMiddleRotation:(MaplyCoordinateSystem *__nonnull)coordSys;

/** @brief return the middle coordinate in a line feature
    @return kMaplyNullCoordinate in case of error
 */
- (MaplyCoordinate)middleCoordinate;

/** @brief return the middle coordinate in a line feature.
 */
- (bool)middleCoordinate:(MaplyCoordinate *__nonnull)middle;

/** @brief Calculate the center and extents of the largest loop in an areal feature.
    @details The vector object contains a number of half baked geometric queries, this being one of them.
    @details If this vector contains at least one areal feature, we'll determine which loop is the largest and return the center of that loop, as well as its bounding box.
    @details Why?  Think label placement on an areal feature.
    @return Returns false if there was no loop (i.e. probably isn't an areal)
  */
- (bool)largestLoopCenter:(MaplyCoordinate *__nullable)center mbrLL:(MaplyCoordinate *__nullable)ll mbrUR:(MaplyCoordinate *__nullable)ur;

/** @brief Calculate the centroid of the largest loop in the areal feature.
 @details The centroid is a better center for label placement than the middle of the largest loop as calculated by largestLoopCenter:mbrLL:mbrUR:
 @return Returns the centroid structure. If there was no loop (i.e. probably isn't an areal), the result will be kMaplyNullCoordinate
 */
- (MaplyCoordinate)centroid;

/** @brief Calculate the centroid of the largest loop in the areal feature.
    @details The centroid is a better center for label placement than the middle of the largest loop as calculated by largestLoopCenter:mbrLL:mbrUR:
    @return Returns false if there was no loop (probably wasn't an areal).
  */
- (bool)centroid:(MaplyCoordinate *__nonnull)centroid;

/** @brief Calculate the bounding box of all the features in this vector object.
 @return kMaplyNullBoundingBox in case of error
 */
- (MaplyBoundingBox)boundingBox;

/** @brief Calculate the bounding box of all the features in this vector object.
  */
- (bool)boundingBoxLL:(MaplyCoordinate *__nonnull)ll ur:(MaplyCoordinate *__nonnull)ur;

/** @brief Calculate the area of the outer loops.
    @details This returns the area of the outer loops of any areal features in the VectorObject.
  */
- (double)areaOfOuterLoops;

/** @brief Convert a feature to an NSArray of NSArrays of CLLocation points.
    @details This is intended for areal features.  It will convert those coordinates to CLLocation values and return them.  Obviously this is intended for things that need CLLocation values.
    @return Returns an NSArray of NSArray's which then contain CLLocation points.
  */
- (NSArray *__nullable)asCLLocationArrays;

/** @brief Return the data as an NSArray of NSNumbers.
    @details If this is a linear, we'll return the points as an NSArray of NSNumbers.
  */
- (NSArray *__nullable)asNumbers;

/** @brief Split up ths feature into individual features and return an array of them.
    @details A vector object can represent multiple features with no real rhyme or reason to it.  This method will make one vector object per feature, allowing you to operate on those individually.
    @return An NSArray of MaplyVectorObject.
  */
- (NSArray *__nonnull)splitVectors;

/** @brief Subdivide the edges in this feature to a given tolerance.
    @details This will break up long edges in a vector until they lie flat on a globe to a given epsilon.  The epislon is in display coordinates (radius = 1.0). This routine breaks this up along geographic boundaries.
  */
- (void)subdivideToGlobe:(float)epsilon;

/** @brief Subdivide the edges in this feature to a given tolerance, using great circle math.
    @details This will break up long edges in a vector until they lie flat on a globe to a given epsilon using a great circle route.  The epsilon is in display coordinates (radius = 1.0).
  */
- (void)subdivideToGlobeGreatCircle:(float)epsilon;

/** @brief Subdivide the edges in this feature to a given tolerance, using great circle math.
 @details This version samples a great circle to display on a flat map.
 */
- (void)subdivideToFlatGreatCircle:(float)epsilon;

/** @brief Tesselate the areal geometry in this vector object and return triangles.
    @details This will attempt to tesselate the areals (with holes) and turn them into triangles.  No attribution will be assigned to the new triangles, so be aware.  The tesselator is the GLU based one and does a decent job.  Odds are if there's something wrong it's in the input data.
  */
- (MaplyVectorObject *__nonnull)tesselate;

/** @brief Clip the given (presumably areal) feature(s) to a grid in radians of the given size.
    @details This will run through the loops in the input vectors and clip them against a grid.  The grid size is given in radians.
    @return New areal features broken up along the grid.
  */
- (MaplyVectorObject *__nullable)clipToGrid:(CGSize)gridSize;

/**
    @brief Clip the given (probably areal) features to the given bounding box.
    @details This will run through the loops of the areal features and clip them against a bounding box.
    @details The bounding box should be in the same coordinate system as the grid, probably radians.
    @return The new areal features will be clipped along the bounding box.
  */
- (MaplyVectorObject *__nullable)clipToMbr:(MaplyCoordinate)ll upperRight:(MaplyCoordinate)ur;

@end

typedef MaplyVectorObject WGVectorObject;

/** @brief The Maply Vector Database holds reference to a group of features that you can query.
    @details This object wraps more complex database-like objects that contain geometric features.  Primarily, that's just shapefiles.
    @details You can set this up and then query the database for features you'd like back.  There's an option to make a SQL query or you can just fetch all the vectors at once.
    @details The point of this object is to keep most features out of memory until needed.  However, a better way of doing that is probably using the MaplyPagingLayer.
  */
@interface MaplyVectorDatabase : NSObject

/** @brief Construct from a shapefile in the bundle
    @details Construct a MaplyVectorDatabase form a shapefile found in the app's bundle.  This will create a bounding box cache file and a sqlite database for the attributes to speed later lookups.
  */
+ (MaplyVectorDatabase *__nonnull) vectorDatabaseWithShape:(NSString *__nonnull)shapeName __deprecated;

/** @brief Construct from a shapefile in the bundle
    @details Construct a MaplyVectorDatabase form a shapefile found in the app's bundle.  This will create a bounding box cache file and a sqlite database for the attributes to speed later lookups.
 */
- (nonnull instancetype)initWithShape:(NSString *__nonnull)shapeName;

/** @brief Construct from a shapfile in the apps documents
    @details Construct a MaplyVectorDabase from a shapefile found in the app's bundle. This will create a bounding box cache file and a sqlite database for the attributes to speed later lookups.
 */
- (nonnull instancetype)initWithShapefileInDocuments:(NSString *__nonnull)shapeName;


/** @brief Return vectors that match the given SQL query
    @details Run a SQL query on the data, looking for vectors that match.  These will be returned in a single MaplyVectorObject, or nil if there are none.
  */
- (MaplyVectorObject *__nullable)fetchMatchingVectors:(NSString *__nonnull)sqlQuery;

/** @brief Search for all the areals that surround the given point (geographic)
    @details First this method does a bounding box check to eliminate areal features that won't overlap at all.  Then it runs a point in poly test on each feature that might.  Every areal feature that overlaps is returned in the MaplyVectorObject.
  */
- (MaplyVectorObject *__nullable)fetchArealsForPoint:(MaplyCoordinate)coord;

/** @brief Return all the vectors in the database.
    @details This method reads all the vectors in the database sequentially and returns them all in a MaplyVectorObject.  This is basically how you read a shapefile in WhirlyGlobe-Maply.
  */
- (MaplyVectorObject *__nullable)fetchAllVectors;

@end
