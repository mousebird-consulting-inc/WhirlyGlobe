/*
 *  MaplyVectorObject.h
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

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "math/MaplyCoordinate.h"
#import "math/MaplyCoordinateSystem.h"

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


/** 
    Maply Vector Object represents zero or more vector features.
    
    The Vector Object can hold several vector features of the same or different types.  It's meant to be a fairly opaque structure, often read from GeoJSON or Shapefiles.  It's less opaque than originally planned, however, and sports a number of specific methods.
    
    If you're doing real vector manipulation, it's best to do it somewhere else and then create one of these as needed for display.
    
    Vector Objects can be created directly or read from a MaplyVectorDatabase.  They are typically then displayed on top of a MaplyViewController or WhirlyGlobeViewController as vectors.
    
    Vector Objects vertices are always in geographic, with longitude = x and latitude = y.
  */
@interface MaplyVectorObject : NSObject

/// Turn this off to make this vector invisible to selection.
/// On by default.
@property (nonatomic,assign) bool selectable;

/** 
    Return the attributes for the vector object.
    
    All vectors should have some set of attribution.  If there's more than one vector feature here, we'll return the attributes on the first one.
    
    The attribution is returned as an NSDictionary and, though you can modify it, you probably shouldn't.
  */
@property (nonatomic,readonly) NSMutableDictionary *__nonnull attributes;

/** 
    Parse vector data from geoJSON.
    
    Returns one object to represent the whole thing, which might include multiple different vectors.  This version uses the faster JSON parser.
    
    We assume the geoJSON is all in decimal degrees in WGS84.
  */
+ (MaplyVectorObject *__nullable)VectorObjectFromGeoJSON:(NSData *__nonnull)geoJSON;

/** 
    Parse vector data from geoJSON.
    
    Returns one object to represent the whole thing, which might include multiple different vectors.  This version uses slower JSON parser.
    
    We assume the geoJSON is all in decimal degrees in WGS84.
*/
+ (MaplyVectorObject *__nullable)VectorObjectFromGeoJSONApple:(NSData *__nonnull)geoJSON;

/** 
    Parse vector data from geoJSON.
    
    Returns one object to represent the whole thing, which might include multiple different vectors.  This version parses its data from an NSDictionary, which had to be parsed from JSON at some point.  Probably the slower path.
    
    We assume the geoJSON is all in decimal degrees in WGS84.
 */
+ (MaplyVectorObject *__nullable)VectorObjectFromGeoJSONDictionary:(NSDictionary *__nonnull)geoJSON;

// Note: Reader turned off
#if 0
/** 
    Read vector objects from the given cache file.
    
    MaplyVectorObject's can be written and read from a binary file.  We use this for caching data locally on the device.
    
    @param fileName Name of the binary vector file.
    
    @return The vector object(s) read from the file or nil on failure.
  */
+ (MaplyVectorObject *__nullable)VectorObjectFromFile:(NSString *__nonnull)fileName;
#endif

/** 
    Read vector objects from the given shapefile.
    
    This will read all the shapes in the given shapefile into memory and return them as one MaplyVectorObject.
    
    @param fileName The basename of the shape file.  Don't include the extension.
    
    @return The vector object(s) read from the file or nil on failure.
  */
+ (MaplyVectorObject *__nullable)VectorObjectFromShapeFile:(NSString *__nonnull)fileName;

/** 
    Parse vector objects from a JSON assembly.
    
    This version can deal with non-compliant assemblies returned by the experimental OSM server
  */
+ (NSDictionary *__nullable)VectorObjectsFromGeoJSONAssembly:(NSData *__nonnull)geoJSON;

/** 
    Initialize with a single data point and attribution.
 
    This version takes a single coordinate and the attributes to go with it.
 */
- (nonnull instancetype)initWithPoint:(MaplyCoordinate)coord attributes:(NSDictionary *__nullable)attr;

/** 
    Initialize with a single data point and attribution.
    
    This version takes a single coordinate and the attributes to go with it.
  */
- (nonnull instancetype)initWithPointRef:(MaplyCoordinate *__nonnull)coord attributes:(NSDictionary *__nullable)attr;

/** 
    Initialize with a linear feature.
 
    This version takes an array of coordinate pairs (as NSNumber) and the attribution.  With this it will make a linear feature.
 */
- (nonnull instancetype)initWithLineString:(NSArray *__nonnull)coords attributes:(NSDictionary *__nullable)attr;

/** 
    Initialize with a linear feature.
    
    This version takes an array of coordinates, the size of that array and the attribution.  With this it will make a linear feature.
  */
- (nonnull instancetype)initWithLineString:(MaplyCoordinate *__nonnull)coords numCoords:(int)numCoords attributes:(NSDictionary *__nullable)attr;

/** 
    Inintialize as an areal feature.
    
    This version takes an array of coordinates, the size of that array and the attribution.  With this it will make a single area feature with one (exterior) loop.  To add loops, call addHole:numCoords:
  */
- (nonnull instancetype)initWithAreal:(MaplyCoordinate *__nonnull)coords numCoords:(int)numCoords attributes:(NSDictionary *__nullable)attr;

/**
  Inintialize as an areal feature.
  
  This version takes an array of coordinates (2 numbers per coordinate).  With this it will make a single area feature with one (exterior) loop.  To add loops, call addHole:numCoords:
*/
- (nonnull instancetype)initWithArealArray:(NSArray<NSNumber *> *__nonnull)coords attributes:(NSDictionary *__nullable)attr;

/** 
    Initializes with vectors parsed from geoJSON.
	
    Returns one object to represent the whole thing, which might include multiple different vectors.  This version uses the faster JSON parser.
    
    We assume the geoJSON is all in decimal degrees in WGS84.
 */
- (nullable instancetype)initWithGeoJSON:(NSData *__nonnull)geoJSON;

/** 
    Initializes with vector parsed from geoJSON.
	
    Returns one object to represent the whole thing, which might include multiple different vectors.  This version uses slower JSON parser.
	
    We assume the geoJSON is all in decimal degrees in WGS84.
 */
- (nullable instancetype)initWithGeoJSONApple:(NSData *__nonnull)geoJSON;

/** 
    Initializes with vector parsed from geoJSON.
	
    Returns one object to represent the whole thing, which might include multiple different vectors.  This version parses its data from an NSDictionary, which had to be parsed from JSON at some point.  Probably the slower path.
	
    We assume the geoJSON is all in decimal degrees in WGS84.
 */
- (nullable instancetype)initWithGeoJSONDictionary:(NSDictionary *__nonnull)geoJSON;

// Note: Turned off with reading
#if 0
/** 
    Initializes with vectors read from the given cache file.
	
    MaplyVectorObject's can be written and read from a binary file.  We use this for caching data locally on the device.
	
    @param fileName Name of the binary vector file.
	
    @return The vector object(s) read from the file or nil on failure.
 */
- (nullable instancetype)initWithFile:(NSString *__nonnull)fileName;
#endif

/** 
    Initializes with vectors read from the given shapefile.
	
    This will read all the shapes in the given shapefile into memory and return them as one MaplyVectorObject.
	
    @param fileName The basename of the shape file.  Don't include the extension.
	
    @return The vector object(s) read from the file or nil on failure.
 */
- (nullable instancetype)initWithShapeFile:(NSString *__nonnull)fileName;


// Note: Vector writing turned off
#if 0
/** 
    Write the vector object to the given file on the device.
    
    We support a binary format for caching vector data.  Typically you write these files on the device or in the simulator and then put them in a place you can easily find them when needed.
    
    @param fileName The file to read the vector data from.
    
    @return Returns true on succes, false on failure.
  */
- (bool)writeToFile:(NSString *__nonnull)fileName;
#endif

/** 
    Make a deep copy of the vector object and return it.
    
    This makes a complete copy of the vector object, with all features and nothing shared.
    
    Had to rename this because Apple's private method scanner is dumb.
  */
- (MaplyVectorObject *__nonnull)deepCopy2;

/** 
    Reproject from one coordinate system to another.
    
    This reprojects every single point in the points, linears, and areals (and mesh) from the source coordinate system to the destionation.
    
    Typically, you'll want Plate Carree for display, the destSystem is probably that.
    
    For various reasons (e.g. scale), this will probably not work right for you.
    
    @param srcSystem The source coordinate system.  The data is already in this sytem.
    
    @param destSystem The destination coordinate system.  The data will be in this system on return.
  */
- (void)reprojectFrom:(MaplyCoordinateSystem *__nonnull)srcSystem to:(MaplyCoordinateSystem *__nonnull)destSystem;

/** 
    Dump the feature(s) out as text
    
    This will write each feature out as text for debugging.
  */
- (NSString *__nonnull)log;

/** 
    Add a hole to an existing feature.
    
    This method is expecting to find exactly one areal feature.  If it finds one, it will add the given hole as a loop on the end of the list of loops.
  */
- (void)addHole:(MaplyCoordinate *__nonnull)coords numCoords:(int)numCoords;

/** 
    Returns the type of the vector feature.
    
    This method returns the type of the vector.  Since vector objects can contain multiple types of vectors at once, this is somewhat complicated.
 
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

/** 
    Run a point in polygon test on all the areal features within the object.
    
    We'll run a point in polygon test on all the areal features within the vector object.  If the point is within one of them, we return true, otherwise false.
  */
- (bool)pointInAreal:(MaplyCoordinate)coord;

/**
    Test if any linear feature is within distance of coord
 */
- (bool)pointNearLinear:(MaplyCoordinate)coord distance:(float)maxDistance inViewController:(MaplyBaseViewController *__nonnull)vc;

/** 
    Calculate the center of the entire set of vectors in this object.
  */
- (MaplyCoordinate)center;

/** 
    Copy the vectors in the given vector object into this one.
  */
- (void)mergeVectorsFrom:(MaplyVectorObject *__nonnull)otherVec;

/** 
    For a linear feature, calculate the mid oint and rotation at that point.
    
    The vector object contains a number of half baked geometric queries, this being one of them.
    
    This finds the middle (as measured by distance) of a linear feature and then calculations an angle corresponding to the line segment that middle sits in.
    
    Why?  Think label road placement.
  */
- (bool)linearMiddle:(MaplyCoordinate *__nonnull)middle rot:(double *__nonnull)rot;

- (bool)linearMiddle:(MaplyCoordinate *__nullable)middle rot:(double *__nullable)rot displayCoordSys:(MaplyCoordinateSystem *__nonnull)coordSys;

/** 
    For a linear feature, calculate the mid point.
 
    This is a convenience method to be called without pointers (Swift)
 
    If you need both the mid point and the rotation, this method is less efficient than the method with pointers.
 
    @return kMaplyNullCoordinate in case of error

 */
- (MaplyCoordinate)linearMiddle:(MaplyCoordinateSystem *__nonnull)coordSys;

/** 
    For a linear feature, calculate the mid point and returns the rotation at that point.
 
    This is a convenience method to be called without pointers (Swift)
 
    If you need both the mid point and the rotation, this method is less efficient than the method with pointers.
 
    @return DBL_MIN in case of error
 */
- (double)linearMiddleRotation:(MaplyCoordinateSystem *__nonnull)coordSys;

/** 
    return the middle coordinate in a line feature
    
    @return kMaplyNullCoordinate in case of error
 */
- (MaplyCoordinate)middleCoordinate;

/** 
    return the middle coordinate in a line feature.
 */
- (bool)middleCoordinate:(MaplyCoordinate *__nonnull)middle;

/** 
    Calculate the center and extents of the largest loop in an areal feature.
    
    The vector object contains a number of half baked geometric queries, this being one of them.
    
    If this vector contains at least one areal feature, we'll determine which loop is the largest and return the center of that loop, as well as its bounding box.
    
    Why?  Think label placement on an areal feature.
    
    @return Returns false if there was no loop (i.e. probably isn't an areal)
  */
- (bool)largestLoopCenter:(MaplyCoordinate *__nullable)center mbrLL:(MaplyCoordinate *__nullable)ll mbrUR:(MaplyCoordinate *__nullable)ur;

/** 
    Calculate the centroid of the largest loop in the areal feature.
 
    The centroid is a better center for label placement than the middle of the largest loop as calculated by largestLoopCenter:mbrLL:mbrUR:
 
    @return Returns the centroid structure. If there was no loop (i.e. probably isn't an areal), the result will be kMaplyNullCoordinate
 */
- (MaplyCoordinate)centroid;

/** 
    Calculate the centroid of the largest loop in the areal feature.
    
    The centroid is a better center for label placement than the middle of the largest loop as calculated by largestLoopCenter:mbrLL:mbrUR:
    
    @return Returns false if there was no loop (probably wasn't an areal).
  */
- (bool)centroid:(MaplyCoordinate *__nonnull)centroid;

/** 
    Calculate the bounding box of all the features in this vector object.
 
    @return kMaplyNullBoundingBox in case of error
 */
- (MaplyBoundingBox)boundingBox;

/** 
    Calculate the bounding box of all the features in this vector object.
  */
- (bool)boundingBoxLL:(MaplyCoordinate *__nonnull)ll ur:(MaplyCoordinate *__nonnull)ur;

/** 
    Calculate the area of the outer loops.
    
    This returns the area of the outer loops of any areal features in the VectorObject.
  */
- (double)areaOfOuterLoops;

/** 
    Convert any linear features into areal features.
    
    Convert linear features to areal features by closing each one individually.
  */
- (MaplyVectorObject * _Nonnull )linearsToAreals;

/**
    Convert any areal features into outlines.
  */
- (MaplyVectorObject * __nonnull)arealsToLinears;

/**
    Filter out edges created from clipping areal features on the server.
 
    In some very specific cases (OSM water) we get polygons that are obviously clipped
    along internal boundaries.  We can clear this up with some very, very specific logic.
 
    Input must be closed areals and output is linears.
  */
- (MaplyVectorObject *__nonnull)filterClippedEdges;

/** 
    Convert a feature to an NSArray of NSArrays of CLLocation points.
    
    This is intended for areal features.  It will convert those coordinates to CLLocation values and return them.  Obviously this is intended for things that need CLLocation values.
    
    @return Returns an NSArray of NSArray's which then contain CLLocation points.
  */
- (NSArray *__nullable)asCLLocationArrays;

/** 
    Return the data as an NSArray of NSNumbers.
    
    If this is a linear, we'll return the points as an NSArray of NSNumbers.
  */
- (NSArray *__nullable)asNumbers;

/** 
    Split up ths feature into individual features and return an array of them.
    
    A vector object can represent multiple features with no real rhyme or reason to it.  This method will make one vector object per feature, allowing you to operate on those individually.
    
    @return An NSArray of MaplyVectorObject.
  */
- (NSArray<MaplyVectorObject *> *__nonnull)splitVectors;

/** 
    Subdivide the edges in this feature to a given tolerance.
    
    This will break up long edges in a vector until they lie flat on a globe to a given epsilon.  The epislon is in display coordinates (radius = 1.0). This routine breaks this up along geographic boundaries.
  */
- (void)subdivideToGlobe:(float)epsilon;

/** 
    Subdivide the edges in this feature to a given tolerance, using great circle math.
    
    This will break up long edges in a vector until they lie flat on a globe to a given epsilon using a great circle route.  The epsilon is in display coordinates (radius = 1.0).
  */
- (void)subdivideToGlobeGreatCircle:(float)epsilon;

/** 
    Subdivide the edges in this feature to a given tolerance, using great circle math.
 
    This version samples a great circle to display on a flat map.
 */
- (void)subdivideToFlatGreatCircle:(float)epsilon;

/** 
    Tesselate the areal geometry in this vector object and return triangles.
    
    This will attempt to tesselate the areals (with holes) and turn them into triangles.  No attribution will be assigned to the new triangles, so be aware.  The tesselator is the GLU based one and does a decent job.  Odds are if there's something wrong it's in the input data.
  */
- (MaplyVectorObject *__nonnull)tesselate;

/** 
    Clip the given (presumably areal) feature(s) to a grid in radians of the given size.
    
    This will run through the loops in the input vectors and clip them against a grid.  The grid size is given in radians.
    
    @return New areal features broken up along the grid.
  */
- (MaplyVectorObject *__nullable)clipToGrid:(CGSize)gridSize;

/**
    
    Clip the given (probably areal) features to the given bounding box.
    
    This will run through the loops of the areal features and clip them against a bounding box.
    
    The bounding box should be in the same coordinate system as the grid, probably radians.
    
    @return The new areal features will be clipped along the bounding box.
  */
- (MaplyVectorObject *__nullable)clipToMbr:(MaplyCoordinate)ll upperRight:(MaplyCoordinate)ur;

@end

typedef MaplyVectorObject WGVectorObject;
