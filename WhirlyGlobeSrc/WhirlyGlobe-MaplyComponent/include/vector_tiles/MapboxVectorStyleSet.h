/*
 *  MapboxVectorStyleSet.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/16/15.
 *  Copyright 2011-2015 mousebird consulting
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
#import "MaplyVectorStyle.h"
#import "MaplyMapnikVectorTiles.h"

@class MapboxVectorFilter;
@class MaplyVectorFunctionStops;

/** @brief The Mapbox Vector Style Set parses Mapbox GL Style sheets and turns them into Maply compatible styles.
    @details A style delegate is required by the Mapnik parser to build geometry out of Mapnik vector tiles.  This style delegate can read a Mapbox GL Style sheet and produce compatible styles.
 */
@interface MaplyMapboxVectorStyleSet : NSObject <MaplyVectorStyleDelegate>

/// @brief Default settings and scale factor for Mapnik vector geometry.
@property (nonatomic, strong) MaplyVectorTileStyleSettings *tileStyleSettings;

/// @brief The view controller everything will be added to
@property (nonatomic, weak) MaplyBaseViewController *viewC;

/// @brief Style name
@property (nonatomic, strong) NSString *name;

/// @brief Version number from the style
@property (nonatomic) NSUInteger version;

/// @brief Constants from the Style sheet
@property (nonatomic, strong) NSDictionary *constants;

/// @brief Where we can fetch the sprites
@property (nonatomic, strong) NSString *spriteURL;

/// @brief Layers parsed from the style sheet
@property (nonatomic, strong) NSArray *layers;

/// @brief Layers sorted by their ID
@property (nonatomic, strong) NSDictionary *layersByName;

/// @brief Layers sorted by source layer name
@property (nonatomic, strong) NSDictionary *layersBySource;

/// @brief Initialize with the style JSON and the view controller
/// @details We'll parse the style JSON passed in and return nil on failure.
- (id)initWithJSON:(NSData *)styleJSON viewC:(MaplyBaseViewController *)viewC;

/// @brief Return an integer value for the given name, taking the constants into account.
- (int)intValue:(NSString *)name dict:(NSDictionary *)dict defVal:(int)defVal;

/// @brief Return a double value for the given name, taking the constants into account
- (double)doubleValue:(NSString *)name dict:(NSDictionary *)dict defVal:(double)devVal;

/// @brief Return a double value for the given name, taking the constants into account
- (double)doubleValue:(id)entry defVal:(double)defVal;

/// @brief Return a string for the given name, taking the constants into account
- (NSString *)stringValue:(NSString *)name defVal:(NSString *)defVal;

/// @brief Return a string for the given name, taking the constants into account
- (NSString *)stringValue:(NSString *)name dict:(NSDictionary *)dict defVal:(NSString *)defVal;

/// @brief Return a color, taking constants into account
- (UIColor *)colorValue:(NSString *)name defVal:(UIColor *)defVal;

/// @brief Return a color for the given name, taking the constants into account
- (UIColor *)colorValue:(NSString *)name dict:(NSDictionary *)dict defVal:(UIColor *)defVal;

/// @brief Return an array for the given name, taking the constants into account
- (NSArray *)arrayValue:(NSString *)name dict:(NSDictionary *)dict defVal:(NSArray *)defVal;

/// @brief Scale the color by the given opacity
- (UIColor *)color:(UIColor *)color withOpacity:(double)opacity;

/// @brief Return the integer corresponding to the name.  Basically parse the enumerated type.
- (NSUInteger)enumValue:(NSString *)name options:(NSArray *)options defVal:(NSUInteger)defVal;

/// @brief Check for and report an unsupported field
- (void)unsupportedCheck:(NSString *)field in:(NSString *)what styleEntry:(NSDictionary *)styleEntry;

/// @brief Check if the given thing is a constant and return its value if it is.  Otherwise just return it.
- (id)constantSubstitution:(id)thing forField:(NSString *)field;

/// @brief Figure out the background color
- (UIColor *)backgroundColor;

@end

/** @brief Layer definition from the Style Sheet.
    @details This is a single layer from the Mapbox style sheet.  It's also used to build visible objects.
  */
@interface MaplyMapboxVectorStyleLayer : MaplyVectorTileStyle
// Note: Need a better base class than MaplyVectorTileStyle

@property (nonatomic,weak) MaplyMapboxVectorStyleSet *styleSet;

/// @brief ID on the layer style entry
@property (nonatomic) NSString *ident;

/// @brief Source from layer defn
@property (nonatomic) NSString *source;

/// @brief Source layer from layer defn
@property (nonatomic) NSString *sourceLayer;

/// @brief Min/max zoom levels
@property (nonatomic) int minzoom,maxzoom;

/// @brief Interactive means you can tap on it
@property (nonatomic) bool interactive;

/// @brief Filter this layer uses to match up to data
@property (nonatomic) MapboxVectorFilter *filter;

/// @brief DrawPriority based on location in the style sheet
@property (nonatomic) int drawPriority;

/// @brief Initialize with the style sheet and the entry for this layer
+ (id)VectorStyleLayer:(MaplyMapboxVectorStyleSet *)styleSet JSON:(NSDictionary *)layerDict drawPriority:(int)drawPriority;

/// @brief Base class initialization.  Copies data out of the refLayer
- (id)initWithStyleEntry:(NSDictionary *)styleEntry parent:(MaplyMapboxVectorStyleLayer *)refLayer styleSet:(MaplyMapboxVectorStyleSet *)styleSet drawPriority:(int)drawPriority viewC:(MaplyBaseViewController *)viewC;

/// @brief Construct objects related to this style based on the input data.
- (NSArray *)buildObjects:(NSArray *)vecObjs forTile:(MaplyTileID)tileID viewC:(MaplyBaseViewController *)viewC;

@end

/// @brief Mapbox filter operator types
typedef enum {MBFilterEqual,MBFilterNotEqual,MBFilterGreaterThan,MBFilterGreaterThanEqual,MBFilterLessThan,MBFilterLessThanEqual,MBFilterIn,MBFilterNotIn,MBFilterAll,MBFilterAny,MBFilterNone} MapboxVectorFilterType;

/// @brief Mapbox geometry types
typedef enum {MBGeomPoint,MBGeomLineString,MBGeomPolygon,MBGeomNone} MapboxVectorGeometryType;

/// @brief Filter is used to match data in a layer to styles
@interface MapboxVectorFilter : NSObject

/// @brief The comparison type for this filter
@property (nonatomic) MapboxVectorFilterType filterType;

/// @brief Attribute name for all the types that take two arguments
@property (nonatomic) NSString *attrName;

/// @brief Set if we're comparing geometry type instead of an attribute
@property (nonatomic) MapboxVectorGeometryType geomType;

/// @brief Attribute value to compare for all the type that take two arguments
@property (nonatomic) id attrVal;

/// @brief Attribute values for the in and !in operators
@property (nonatomic) NSArray *attrVals;

/// @brief For All and Any these are the MapboxVectorFilters to evaluate
@property (nonatomic) NSArray *subFilters;

/// @brief Parse the filter info out of the style entry
- (id)initWithArray:(NSArray *)styleEntry styleSet:(MaplyMapboxVectorStyleSet *)styleSet viewC:(MaplyBaseViewController *)viewC;

/// @brief Test a feature's attributes against the filter
- (bool)testFeature:(NSDictionary *)attrs tile:(MaplyTileID)tileID viewC:(MaplyBaseViewController *)viewC;

@end

/// @brief Types for the value object
typedef enum {MaplyMapboxValueTypeNumber, MaplyMapboxValueTypeColor, MaplyMapboxValueTypeString} MaplyMapboxValueType;

/// @brief
@interface MaplyMapboxValue : NSObject

/// @brief Data type for value
@property (nonatomic) MaplyMapboxValueType type;

/// @brief NSNumber or UIColor at present
@property (nonatomic) id value;

/// @brief Initialize with data and a type we're expecting
- (id)initWithValue:(id)value type:(MaplyMapboxValueType)dataType styleSet:(MaplyMapboxVectorStyleSet *)styleSet;

/// @brief Returns a value interpolated between the two inputs
+ (MaplyMapboxValue *)interpolateFrom:(MaplyMapboxValue *)a to:(MaplyMapboxValue *)b t:(double)t;

@end

/// @brief A single input and output value
@interface MaplyMapboxVectorFunctionStop : NSObject

/// @brief Input data value
@property (nonatomic) double inputVal;

/// @brief Output data value.  Might be an NSNumber, UIColor, or
@property (nonatomic) id outputVal;

@end

/// @brief Type of the mapbox vector function
typedef enum {MapboxVectorFunctionExponential,MapboxVectorFunctionInterval} MapboxVectorFunctionType;

/// @brief Functions color a data value based on a property (probably zoom)
@interface MaplyMapboxVectorFunction : NSObject

/// @brief Function type (exponential or interval at the moment)
@property (nonatomic,readonly) MapboxVectorFunctionType type;

/// @brief Data type this returns
@property (nonatomic,readonly) MaplyMapboxValueType dataType;

/// @brief Array of function stops as they apply to value.  These are MaplyVectorFunctionStop objects.
@property (nonatomic,strong) NSArray *stops;

/// @brief Used in exponential calculation
@property (nonatomic,assign) double base;

/// @brief Parse out of a JSON object
- (id)initWithValueDict:(NSDictionary *)dict dataType:(MaplyMapboxValueType)dataType styleSet:(MaplyMapboxVectorStyleSet *)styleSet viewC:(MaplyBaseViewController *)viewC;

/// @brief Calculate a value given the input value.  Returns an NSNumber or UIColor
- (MaplyMapboxValue *)valueForInput:(double)inputVal type:(MaplyMapboxValueType)dataType styleSet:(MaplyMapboxVectorStyleSet *)styleSet;

/// @brief Returns the minimum value
- (MaplyMapboxValue *)minValueOfType:(MaplyMapboxValueType)dataType;

/// @brief Returns the maximum value
- (MaplyMapboxValue *)maxValueOfType:(MaplyMapboxValueType)dataType;

@end

/// @brief Represents either a value or a function to generate that value
@interface MaplyMapboxValueWrapper : NSObject

/// @brief Initialize with a field that could be a single value or a function and the data we're expecting it to be
- (id)initWithValue:(id)value dataType:(MaplyMapboxValueType)dataType styleSet:(MaplyMapboxVectorStyleSet *)styleSet;

/// @brief Initialize by reading the named field out of the dictionary and then parsing out either a value or a function
- (id)initWithDict:(NSDictionary *)dict name:(NSString *)attrName dataType:(MaplyMapboxValueType)dataType styleSet:(MaplyMapboxVectorStyleSet *)styleSet;

/// @brief Initialize with a real value of type NSNumber, UIColor, or NSString
- (id)initWithObject:(id)thing;

/// @brief Defined data type for this value or function
@property (nonatomic,readonly) MaplyMapboxValueType dataType;

/// @brief Set if this is a value rather than a function
@property (nonatomic,readonly) bool isValue;

/// @brief Return a simple number
- (double)numberForZoom:(int)zoom styleSet:(MaplyMapboxVectorStyleSet *)styleSet;

/// @brief Return the maximum possible number
- (double)maxNumberWithStyleSet:(MaplyMapboxVectorStyleSet *)styleSet;

/// @brief Return a color
- (UIColor *)colorForZoom:(int)zoom styleSet:(MaplyMapboxVectorStyleSet *)styleSet;

/// @brief Return the maximum possible color
- (UIColor *)maxColorWithStyleSet:(MaplyMapboxVectorStyleSet *)styleSet;

/// @brief Return a string
- (NSString *)stringForZoom:(int)zoom styleSet:(MaplyMapboxVectorStyleSet *)styleSet;

@end
