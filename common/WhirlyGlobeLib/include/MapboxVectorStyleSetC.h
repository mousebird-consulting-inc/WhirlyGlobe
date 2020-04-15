/*
*  MapboxVectorStyleSetC.h
*  WhirlyGlobeLib
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

#import "Scene.h"
#import "VectorManager.h"
#import "WideVectorManager.h"
#import "MarkerManager.h"
#import "ComponentManager.h"
#import "MapboxVectorTileParser.h"
#import "MaplyVectorStyleC.h"
#import <set>

namespace WhirlyKit
{

class MapboxVectorStyleLayer;
typedef std::shared_ptr<MapboxVectorStyleLayer> MapboxVectorStyleLayerRef;

// A transitionable double value
// This may be stops or a single value
class MapboxTransDouble
{
public:
    // Return the value for a given level
    double valForZoom(double zoom);

    // Minimum possible value
    double minVal();

    // Maximum possible value
    double maxVal();
    
protected:
};
typedef std::shared_ptr<MapboxTransDouble> MapboxTransDoubleRef;

// Transitionable color
// Might be stops, might be a single value
class MapboxTransColor
{
public:
    // If set, we're using the alpha to indicate some other value, so just pass it through
    void setAlphaOverride(double alpha);

    // Return a color for the given zoom level
    RGBAColor colorForZoom(double zoom);
    
protected:
};
typedef std::shared_ptr<MapboxTransColor> MapboxTransColorRef;

// Used for combining color and opacity
typedef enum {
    MBResolveColorOpacityReplaceAlpha,
    MBResolveColorOpacityMultiply
} MBResolveColorType;

/**
  Holds the low level implementation for Mapbox Style Sheet parsing and object construction.
 */
class MapboxVectorStyleSetImpl
{
public:
    MapboxVectorStyleSetImpl(Scene *scene);
        
    /// @brief Default settings and scale factor for Mapnik vector geometry.
    VectorStyleSettingsImpl *tileStyleSettings;

    /// @brief Style name
    std::string name;

    /// @brief Version number from the style
    int version;

    /// @brief Constants from the Style sheet
    DictionaryRef constants;

    /// @brief Layers parsed from the style sheet
    std::vector<MapboxVectorStyleLayerRef> layers;

    /// @brief Layers sorted by their ID
    std::map<std::string, MapboxVectorStyleLayerRef> layersByName;

    /// @brief Layers sorted by source layer name
    std::map<std::string, MapboxVectorStyleLayerRef> layersBySource;

    /// @brief Generates a unique ID for a style
    long long generateID();

    /// @brief Return an integer value for the given name, taking the constants into account.
    int intValue(const std::string &name,DictionaryRef dict,int defVal);

    /// @brief Return a double value for the given name, taking the constants into account
    double doubleValue(const std::string &name,DictionaryRef dict,double defVal);

    /// @brief Return a double value for the given name, taking the constants into account
    double doubleValue(DictionaryEntryRef entry,double defVal);
        
    /// @brief Return a bool for the given name.  True if it matches the onString.  Default if it's missing
    bool boolValue(const std::string &name,DictionaryRef dict,const std::string &onString,bool defVal);

    /// @brief Return a string for the given name, taking the constants into account
    std::string stringValue(const std::string &name,DictionaryRef dict,const std::string &defVal);

    /// @brief Return a color for the given name, taking the constants into account
    RGBAColorRef colorValue(const std::string &name,DictionaryEntryRef val,DictionaryRef dict,const RGBAColor &defVal,bool multiplyAlpha);

    /// @brief Return an array for the given name, taking the constants into account
    std::vector<DictionaryRef> arrayValue(const std::string &name,DictionaryRef dict);

    /// Builds a transitionable double object and returns that
    MapboxTransDoubleRef transDouble(const std::string &name,DictionaryRef entry,double defVal);

    /// Builds a transitionable color object and returns that
    MapboxTransColorRef transColor(const std::string &name,DictionaryRef entry,const RGBAColor *);
    MapboxTransColorRef transColor(const std::string &name,DictionaryRef entry,const RGBAColor &);

    /// Resolve transitionable color and opacity into a single color for the zoom
    /// If this returns nil, then the object shouldn't appear
    RGBAColorRef resolveColor(MapboxTransColorRef color,MapboxTransDoubleRef opacity,double zoom,MBResolveColorType resolveMode);

    /// @brief Scale the color by the given opacity
    RGBAColor color(RGBAColor color,double opacity);

    /// @brief Return the integer corresponding to the name.  Basically parse the enumerated type.
//    - (NSUInteger)enumValue:(NSString * __nonnull)name options:(NSArray * __nonnull)options defVal:(NSUInteger)defVal;

    /// @brief Check for and report an unsupported field
    void unsupportedCheck(const std::string &field,const std::string &what,DictionaryRef styleEntry);

    /// @brief Check if the given thing is a constant and return its value if it is.  Otherwise just return it.
//    - (id __nullable)constantSubstitution:(id __nonnull)thing forField:(NSString * __nullable)field;
    
    /// Local platform implementation for generating a circle and adding it as a texture
    virtual SimpleIdentity makeCircleTexture(double radius,const RGBAColor &fillColor,const RGBAColor &strokeColor,Point2f *circleSize) = 0;

public:
    Scene *scene;

    VectorManager *vecManage;
    WideVectorManager *wideVecManage;
    MarkerManager *markerManage;
    ComponentManager *compManage;
    
    // ID's for the various programs
    SimpleIdentity screenMarkerProgramID;
    SimpleIdentity vectorArealProgramID;
    SimpleIdentity vectorLinearProgramID;
};
typedef std::shared_ptr<MapboxVectorStyleSetImpl> MapboxVectorStyleSetImplRef;

/// @brief Mapbox filter operator types
typedef enum {MBFilterEqual,MBFilterNotEqual,MBFilterGreaterThan,MBFilterGreaterThanEqual,MBFilterLessThan,MBFilterLessThanEqual,MBFilterIn,MBFilterNotIn,MBFilterHas,MBFilterNotHas,MBFilterAll,MBFilterAny,MBFilterNone} MapboxVectorFilterType;

/// @brief Mapbox geometry types
typedef enum {MBGeomPoint,MBGeomLineString,MBGeomPolygon,MBGeomNone} MapboxVectorGeometryType;

class MapboxVectorFilter;
typedef std::shared_ptr<MapboxVectorFilter> MapboxVectorFilterRef;

/// @brief Filter is used to match data in a layer to styles
class MapboxVectorFilter
{
public:
    /// @brief Parse the filter info out of the style entry
    MapboxVectorFilter(const std::vector<DictionaryRef> &styleEntry,MapboxVectorStyleSetImplRef styleSet);

    /// @brief Test a feature's attributes against the filter
    bool testFeature(DictionaryRef attrs,const QuadTreeIdentifier &tileID);

    /// @brief The comparison type for this filter
    MapboxVectorFilterType filterType;

    /// @brief Attribute name for all the types that take two arguments
    std::string attrName;

    /// @brief Set if we're comparing geometry type instead of an attribute
    MapboxVectorGeometryType geomType;

    /// @brief Attribute value to compare for all the type that take two arguments
//    id attrVal;

    /// @brief Attribute values for the in and !in operators
//    NSArray *attrVals;

    /// @brief For All and Any these are the MapboxVectorFilters to evaluate
    std::vector<MapboxVectorFilterRef> subFilters;
};

/** @brief Layer definition from the Style Sheet.
    @details This is a single layer from the Mapbox style sheet.  It's also used to build visible objects.
  */
class MapboxVectorStyleLayer : public VectorStyleImpl
{
public:

    /// @brief Initialize with the style sheet and the entry for this layer
    static MapboxVectorStyleLayerRef VectorStyleLayer(MapboxVectorStyleSetImplRef styleSet,DictionaryRef layerDict,int drawPriority);

    /// @brief Base class initialization.  Copies data out of the refLayer
    MapboxVectorStyleLayer();
    virtual ~MapboxVectorStyleLayer();

    // Parse the layer entry out of the style sheet
    virtual bool parse(MapboxVectorStyleSetImplRef styleSet,
                       DictionaryRef styleEntry,
                       MapboxVectorStyleLayerRef parentLayer,
                       int drawPriority);

    /// Unique Identifier for this style
    virtual long long getUuid();
    
    /// Category used for sorting
    virtual const std::string &getCategory();
    
    // Note: This no longer really holds
    /// Set if this geometry is additive (e.g. sticks around) rather than replacement
    virtual bool geomAdditive();

    /// Construct objects related to this style based on the input data.
    virtual void buildObjects(std::vector<VectorObjectRef> &vecObjs,VectorTileDataRef tileInfo) = 0;
    
    /// Clean up any objects (textures, probably)
    virtual void cleanup(ChangeSet &changes);

    MapboxVectorStyleSetImplRef styleSet;

    /// Set if we actually use this layer.  Copied from the layout
    bool visible;

    /// @brief ID on the layer style entry
    std::string ident;

    /// @brief Source from layer defn
    std::string source;

    /// @brief Source layer from layer defn
    std::string sourceLayer;

    /// @brief Min/max zoom levels
    int minzoom,maxzoom;

    /// @brief Filter this layer uses to match up to data
    MapboxVectorFilterRef filter;

    /// @brief DrawPriority based on location in the style sheet
    int drawPriority;

    /// If non-zero we set the draw priority differently per level
    int drawPriorityPerLevel;

    // If set, the features produced will be selectable (if they can be)
    // Inherited from the settings
    bool selectable;

    /// @brief Unique Identifier for this style
    long long uuid;

    /// @brief Set if this geometry is additive (e.g. sticks around) rather than replacement
    bool geomAdditiveVal;

    /// @brief metadata tag from the JSON file
    DictionaryRef metadata;
};
typedef std::shared_ptr<MapboxVectorStyleLayer> MapboxVectorStyleLayerRef;

}
