/*
*  MapboxVectorStyleSet_private.h
*  WhirlyGlobe-MaplyComponent
*
*  Created by Steve Gifford on 4/8/20.
*  Copyright 2011-2020 mousebird consulting
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

#import "MapboxVectorStyleSet.h"
#import "MapboxVectorStyleSetC.h"

@interface MapboxVectorStyleSet()
{
    WhirlyKit::MapboxVectorStyleSetImplRef impl;
}

/// @brief Default settings and scale factor for Mapnik vector geometry.
@property (nonatomic, strong, nonnull) MaplyVectorStyleSettings *tileStyleSettings;

/// @brief The view controller everything will be added to
@property (nonatomic, weak, nullable) NSObject<MaplyRenderControllerProtocol> *viewC;

/// @brief Style name
@property (nonatomic, strong, nullable) NSString *name;

/// @brief Version number from the style
@property (nonatomic) NSUInteger version;

/// @brief Constants from the Style sheet
@property (nonatomic, strong, nullable) NSDictionary *constants;

/// @brief Layers parsed from the style sheet
@property (nonatomic, strong, nullable) NSArray *layers;

/// @brief Layers sorted by their ID
@property (nonatomic, strong, nullable) NSDictionary *layersByName;

/// @brief Layers sorted by source layer name
@property (nonatomic, strong, nullable) NSDictionary *layersBySource;

/// @brief Generates a unique ID for a style
- (long long)generateID;

/// @brief Return an integer value for the given name, taking the constants into account.
- (int)intValue:(NSString * __nonnull)name dict:(NSDictionary * __nullable)dict defVal:(int)defVal;

/// @brief Return a double value for the given name, taking the constants into account
- (double)doubleValue:(NSString * __nonnull)name dict:(NSDictionary * __nullable)dict defVal:(double)devVal;

/// @brief Return a double value for the given name, taking the constants into account
- (double)doubleValue:(id __nonnull)entry defVal:(double)defVal;
    
/// @brief Return a bool for the given name.  True if it matches the onString.  Default if it's missing
- (bool)boolValue:(NSString * __nonnull)name dict:(NSDictionary * __nullable)dict onValue:(NSString * __nonnull)onString defVal:(bool)defVal;

/// @brief Return a string for the given name, taking the constants into account
- (NSString *_Nullable)stringValue:(NSString * __nullable)name dict:(NSDictionary * __nullable)dict defVal:(NSString * __nullable)defVal;

/// @brief Return a color for the given name, taking the constants into account
- (UIColor *_Nullable)colorValue:(NSString * __nullable)name val:(id __nullable )val dict:(NSDictionary *__nullable)dict defVal:(UIColor * __nullable)defVal multiplyAlpha:(bool)multiplyAlpha;

/// @brief Return an array for the given name, taking the constants into account
- (NSArray *_Nullable)arrayValue:(NSString * __nonnull)name dict:(NSDictionary * __nullable)dict defVal:(NSArray * __nullable)defVal;

/// Builds a transitionable double object and returns that
- (MapboxTransDouble *__nullable)transDouble:(NSString * __nonnull)name entry:(NSDictionary *__nonnull)entry defVal:(double)defVal;

/// Builds a transitionable color object and returns that
- (MapboxTransColor *__nullable)transColor:(NSString *__nonnull)name entry:(NSDictionary *__nonnull)entry defVal:(UIColor * __nullable)defVal;

/// Resolve transitionable color and opacity into a single color for the zoom
/// If this returns nil, then the object shouldn't appear
- (UIColor *__nullable)resolveColor:(MapboxTransColor * __nullable)color opacity:(MapboxTransDouble * __nullable)opacity forZoom:(double)zoom mode:(MBResolveColorType)resolveMode;

/// @brief Scale the color by the given opacity
- (UIColor * __nullable)color:(UIColor * __nonnull)color withOpacity:(double)opacity;

/// @brief Return the integer corresponding to the name.  Basically parse the enumerated type.
- (NSUInteger)enumValue:(NSString * __nonnull)name options:(NSArray * __nonnull)options defVal:(NSUInteger)defVal;

/// @brief Check for and report an unsupported field
- (void)unsupportedCheck:(NSString * __nonnull)field in:(NSString * __nonnull)what styleEntry:(NSDictionary * __nonnull)styleEntry;

/// @brief Check if the given thing is a constant and return its value if it is.  Otherwise just return it.
- (id __nullable)constantSubstitution:(id __nonnull)thing forField:(NSString * __nullable)field;

@end

namespace WhirlyKit
{

/**
  Base class for Mapbox Vector Styles.
 */
class MapboxVectorStyleLayerImpl : public VectorStyleImpl
{
public:
    
    /// Unique Identifier for this style
    virtual long long getUuid();

    /// Category used for sorting
    virtual const std::string &getCategory();

    /// Set if this geometry is additive (e.g. sticks around) rather than replacement
    virtual bool geomAdditive();

    /// Construct objects related to this style based on the input data.
    virtual void buildObject(std::vector<VectorObjectRef> &vecObjs,VectorTileDataRef tileInfo,MapboxVectorStyleSetImplRef impl);
    
protected:
    MapboxVectorStyleSetImplRef style;
};

}

/** @brief Layer definition from the Style Sheet.
    @details This is a single layer from the Mapbox style sheet.  It's also used to build visible objects.
  */
@interface MaplyMapboxVectorStyleLayer : NSObject<MaplyVectorStyle>
// Note: Need a better base class than MaplyVectorTileStyle

@property (nonatomic,weak,nullable) MapboxVectorStyleSet *styleSet;

/// Set if we actually use this layer.  Copied from the layout
@property (nonatomic) bool visible;

/// @brief ID on the layer style entry
@property (nonatomic,nullable,strong) NSString *ident;

/// @brief Source from layer defn
@property (nonatomic,nullable,strong) NSString *source;

/// @brief Source layer from layer defn
@property (nonatomic,nullable,strong) NSString *sourceLayer;

/// @brief Min/max zoom levels
@property (nonatomic) int minzoom,maxzoom;

/// @brief Filter this layer uses to match up to data
@property (nonatomic,nullable,strong) MapboxVectorFilter *filter;

/// @brief DrawPriority based on location in the style sheet
@property (nonatomic) int drawPriority;

/// If non-zero we set the draw priority differently per level
@property (nonatomic) int drawPriorityPerLevel;

// If set, the features produced will be selectable (if they can be)
// Inherited from the settings
@property (nonatomic) bool selectable;

/// @brief Initialize with the style sheet and the entry for this layer
+ (id __nullable)VectorStyleLayer:(MapboxVectorStyleSet * __nonnull)styleSet JSON:(NSDictionary * __nonnull)layerDict drawPriority:(int)drawPriority;

/// @brief Base class initialization.  Copies data out of the refLayer
- (id __nullable)initWithStyleEntry:(NSDictionary * __nonnull)styleEntry parent:(MaplyMapboxVectorStyleLayer * __nonnull)refLayer styleSet:(MapboxVectorStyleSet * __nonnull)styleSet drawPriority:(int)drawPriority viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)viewC;

/// @brief Unique Identifier for this style
@property(nonatomic) long long uuid;

/// @brief Set if this geometry is additive (e.g. sticks around) rather than replacement
@property(nonatomic) bool geomAdditive;

/// @brief metadata tag from the JSON file
@property(nonatomic,nullable) NSDictionary *metadata;

@end

/// @brief Mapbox filter operator types
typedef enum {MBFilterEqual,MBFilterNotEqual,MBFilterGreaterThan,MBFilterGreaterThanEqual,MBFilterLessThan,MBFilterLessThanEqual,MBFilterIn,MBFilterNotIn,MBFilterHas,MBFilterNotHas,MBFilterAll,MBFilterAny,MBFilterNone} MapboxVectorFilterType;

/// @brief Mapbox geometry types
typedef enum {MBGeomPoint,MBGeomLineString,MBGeomPolygon,MBGeomNone} MapboxVectorGeometryType;

/// @brief Filter is used to match data in a layer to styles
@interface MapboxVectorFilter : NSObject

/// @brief The comparison type for this filter
@property (nonatomic) MapboxVectorFilterType filterType;

/// @brief Attribute name for all the types that take two arguments
@property (nonatomic,nullable,strong) NSString *attrName;

/// @brief Set if we're comparing geometry type instead of an attribute
@property (nonatomic) MapboxVectorGeometryType geomType;

/// @brief Attribute value to compare for all the type that take two arguments
@property (nonatomic,nullable,strong) id attrVal;

/// @brief Attribute values for the in and !in operators
@property (nonatomic,nullable,strong) NSArray *attrVals;

/// @brief For All and Any these are the MapboxVectorFilters to evaluate
@property (nonatomic,nullable,strong) NSArray *subFilters;

/// @brief Parse the filter info out of the style entry
- (id _Nullable)initWithArray:(NSArray * __nonnull)styleEntry styleSet:(MapboxVectorStyleSet * __nonnull)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)viewC;

/// @brief Test a feature's attributes against the filter
- (bool)testFeature:(NSDictionary * __nonnull)attrs tile:(MaplyTileID)tileID viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)viewC;

@end

// A transitionable double value
// This may be stops or a single value
@interface MapboxTransDouble : NSObject

// Return the value for a given level
- (double)valForZoom:(double)zoom;

// Minimum possible value
- (double)minVal;

// Maximum possible value
- (double)maxVal;

@end

// Transitionable color
// Might be stops, might be a single value
@interface MapboxTransColor : NSObject

// If set, we're using the alpha to indicate some other value, so just pass it through
- (void)setAlphaOverride:(double)alpha;

// Return a color for the given zoom level
- (UIColor * __nonnull)colorForZoom:(double)zoom;

@end
