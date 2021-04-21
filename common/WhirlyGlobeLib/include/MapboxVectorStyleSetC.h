/* MapboxVectorStyleSetC.h
*  WhirlyGlobeLib
*
*  Created by Steve Gifford on 4/8/20.
*  Copyright 2011-2021 mousebird consulting
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
*/

#import "Scene.h"
#import "VectorManager.h"
#import "WideVectorManager.h"
#import "MarkerManager.h"
#import "ComponentManager.h"
#import "MapboxVectorTileParser.h"
#import "MaplyVectorStyleC.h"
#import "MapboxVectorStyleSpritesImpl.h"
#import <set>

namespace WhirlyKit
{

class MapboxVectorStyleLayer;
typedef std::shared_ptr<MapboxVectorStyleLayer> MapboxVectorStyleLayerRef;

class MapboxVectorStyleSetImpl;
typedef std::shared_ptr<MapboxVectorStyleSetImpl> MapboxVectorStyleSetImplRef;

// Used to track text data
class MapboxTextChunk {
public:
    // Set if this is a simple string
    std::string str;

    // Possible key names in the data. Tried in this order.
    // Not set if this is a simple string
    std::vector<std::string> keys;
};

// Encapsulates a regular expression field.  Could be a string, could be more complex
class MapboxRegexField {
public:
    MapboxRegexField() : valid(false) { }

    // Simpler version that just takes the text string
    bool parse(const std::string &textVal);
    // Parse the regex text field out of a field name string
    bool parse(const std::string &fieldName,MapboxVectorStyleSetImpl *styleSet,const DictionaryRef &styleEntry);
    
    // Build the field based on the attributes
    std::string build(const DictionaryRef &attrs);
    
    std::vector<MapboxTextChunk> chunks;
    
    bool valid;
};

// One value correlated with a zoom level
class MaplyVectorFunctionStop
{
public:
    MaplyVectorFunctionStop();
    
    /// @brief Zoom level this applies to
    double zoom;

    /// @brief Value at that zoom level
    double val;

    /// @brief Could also just be a color
    RGBAColorRef color;
    
    /// Might be a string (reg-ex function thing)
    MapboxRegexField textField;
};

// Collection of function stops
class MaplyVectorFunctionStops
{
public:
    bool parse(const DictionaryRef &entry,MapboxVectorStyleSetImpl *styleSet,bool isText);

    /// @brief Calculate a value given the zoom level
    double valueForZoom(double zoom);

    /// @brief This version returns colors, assuming we're dealing with colors
    RGBAColorRef colorForZoom(double zoom);
    
    /// Return the text for a given zoom level
    MapboxRegexField textForZoom(double zoom);

    /// @brief Returns the minimum value
    double minValue();

    /// @brief Returns the maximum value
    double maxValue();

public:
    std::vector<MaplyVectorFunctionStop> stops;
    
    /// @brief Used in exponential calculation
    double base;
};
typedef std::shared_ptr<MaplyVectorFunctionStops> MaplyVectorFunctionStopsRef;

// A transitionable double value
// This may be stops or a single value
class MapboxTransDouble
{
public:
    MapboxTransDouble(double value);
    MapboxTransDouble(MaplyVectorFunctionStopsRef stops);
    
    // Return the value for a given level
    double valForZoom(double zoom);
    
    // True if this is an expression, rather than a constant
    bool isExpression();

    // Build the expression, if this has stops
    FloatExpressionInfoRef expression();

    // Minimum possible value
    double minVal();

    // Maximum possible value
    double maxVal();
    
protected:
    double val;
    MaplyVectorFunctionStopsRef stops;
};
typedef std::shared_ptr<MapboxTransDouble> MapboxTransDoubleRef;

// Transitionable color
// Might be stops, might be a single value
class MapboxTransColor
{
public:
    MapboxTransColor(RGBAColorRef color);
    MapboxTransColor(MaplyVectorFunctionStopsRef stops);
    
    // If set, we're using the alpha to indicate some other value, so just pass it through
    void setAlphaOverride(double alpha);
    bool hasAlphaOverride() { return useAlphaOverride; }

    // Return a color for the given zoom level
    RGBAColor colorForZoom(double zoom);

    // Check if we've got an expression or it's going to be a constant
    bool isExpression() const;

    // Build the expression, if this has stops
    ColorExpressionInfoRef expression();
    
protected:
    RGBAColorRef color;
    bool useAlphaOverride;
    double alpha;
    MaplyVectorFunctionStopsRef stops;
};
typedef std::shared_ptr<MapboxTransColor> MapboxTransColorRef;

// Transitional text
// Picks a text value at a particular level
class MapboxTransText
{
public:
    MapboxTransText(const std::string &text);
    MapboxTransText(MaplyVectorFunctionStopsRef stops);
    
    // Return text for the given zoom level
    MapboxRegexField textForZoom(double zoom);
    
protected:
    MapboxRegexField textField;
    MaplyVectorFunctionStopsRef stops;
};
typedef std::shared_ptr<MapboxTransText> MapboxTransTextRef;

// Used for combining color and opacity
typedef enum {
    MBResolveColorOpacityReplaceAlpha,
    MBResolveColorOpacityMultiply,
    MBResolveColorOpacityComposeAlpha,
} MBResolveColorType;

/**
  Holds the low level implementation for Mapbox Style Sheet parsing and object construction.
 */
class MapboxVectorStyleSetImpl : public VectorStyleDelegateImpl
{
public:
    MapboxVectorStyleSetImpl(Scene *scene,CoordSystem *coordSys,VectorStyleSettingsImplRef settings);
    virtual ~MapboxVectorStyleSetImpl() = default;
    
    // Parse the entire style sheet.  False on failure
    virtual bool parse(PlatformThreadInfo *inst,const DictionaryRef &dict);

    /// @brief Default settings and scale factor for Mapnik vector geometry.
    VectorStyleSettingsImplRef tileStyleSettings;

    /// @brief Generates a unique ID for a style
    long long generateID();

    /// @brief Return an integer value for the given name, taking the constants into account.
    int intValue(const std::string &name,const DictionaryRef &dict,int defVal);

    /// @brief Return a double value for the given name, taking the constants into account
    double doubleValue(const DictionaryEntryRef &entry,double defVal);

    /// @brief Return a double value for the given name, taking the constants into account
    double doubleValue(const std::string &name,const DictionaryRef &dict,double defVal);
        
    /// @brief Return a bool for the given name.  True if it matches the onString.  Default if it's missing
    bool boolValue(const std::string &name,const DictionaryRef &dict,const std::string &onString,bool defVal);

    /// @brief Return a string for the given name, taking the constants into account
    std::string stringValue(const std::string &name,const DictionaryRef &dict,const std::string &defVal);

    /// @brief Return an array for the given name, taking the constants into account
    std::vector<DictionaryEntryRef> arrayValue(const std::string &name,const DictionaryRef &dict);

    /// @brief Return a color for the given name, taking the constants into account
    RGBAColorRef colorValue(const std::string &name,const DictionaryEntryRef &val,const DictionaryRef &dict,const RGBAColorRef &defVal,bool multiplyAlpha);
    RGBAColorRef colorValue(const std::string &name,const DictionaryEntryRef &val,const DictionaryRef &dict,const RGBAColor &defVal,bool multiplyAlpha);

    /// @brief Return the integer corresponding to the name.  Basically parse the enumerated type
    int enumValue(const DictionaryEntryRef &entry, const char * const options[],int defVal);

    /// Builds a transitionable double object from a style entry and returns that
    MapboxTransDoubleRef transDouble(const DictionaryEntryRef &entry,double defVal);
    
    /// Builds a transitionable double object from a style entry lookup and returns that
    MapboxTransDoubleRef transDouble(const std::string &name,const DictionaryRef &entry,double defVal);

    /// Builds a transitionable color object and returns that
    MapboxTransColorRef transColor(const std::string &name,const DictionaryRef &entry,const RGBAColor *);
    MapboxTransColorRef transColor(const std::string &name,const DictionaryRef &entry,const RGBAColor &);
    
    /// Builds a transitional text object
    MapboxTransTextRef transText(const std::string &name,const DictionaryRef &entry,const std::string &str);

    /// Resolve transitionable color and opacity into a single color for the zoom
    /// If this returns nil, then the object shouldn't appear
    RGBAColorRef resolveColor(const MapboxTransColorRef &color,const MapboxTransDoubleRef &opacity,double zoom,MBResolveColorType resolveMode);

    /// @brief Scale the color by the given opacity
    static RGBAColor color(RGBAColor color,double opacity);

    /// @brief Check for and report an unsupported field
    void unsupportedCheck(const char *field,const char *what,const DictionaryRef &styleEntry);
    
    /// Fetch a layer by name
    virtual MapboxVectorStyleLayerRef getLayer(const std::string &name);

    /// Set the zoom slot if we've got continuous zoom going on
    virtual int getZoomSlot() const override { return zoomSlot; }

    /// Set the zoom slot if we've got continuous zoom going on
    virtual void setZoomSlot(int slot) override { zoomSlot = slot; }

    /// Get the background style, if any
    VectorStyleImplRef backgroundStyle(PlatformThreadInfo *inst) const override;

    /// Fish out the background color for a given zoom level
    RGBAColorRef backgroundColor(PlatformThreadInfo *inst,double zoom) override;
    
    /** VectorStyleDelegateImpl **/
    
    /// Return the styles that apply to the given feature (attributes).
    virtual std::vector<VectorStyleImplRef> stylesForFeature(PlatformThreadInfo *inst,
                                                             const Dictionary &attrs,
                                                             const QuadTreeIdentifier &tileID,
                                                             const std::string &layerName) override;
    
    /// Return true if the given layer is meant to display for the given tile (zoom level)
    virtual bool layerShouldDisplay(PlatformThreadInfo *inst,
                                    const std::string &name,
                                    const QuadTreeNew::Node &tileID) override;

    /// Return the style associated with the given UUID.
    virtual VectorStyleImplRef styleForUUID(PlatformThreadInfo *inst,long long uuid) override;

    // Return a list of all the styles in no particular order.  Needed for categories and indexing
    virtual std::vector<VectorStyleImplRef> allStyles(PlatformThreadInfo *inst) override;

    
    /** Platform specific implementation **/
    
    /// Local platform implementation for generating a circle and adding it as a texture
    virtual SimpleIdentity makeCircleTexture(PlatformThreadInfo *inst,
                                             double radius,
                                             const RGBAColor &fillColor,
                                             const RGBAColor &strokeColor,
                                             float strokeWidth,Point2f *circleSize) = 0;
    
    /// Local platform implementation for generating a repeating line texture
    virtual SimpleIdentity makeLineTexture(PlatformThreadInfo *inst,const std::vector<double> &dashComponents) = 0;
    
    /// Create a local platform LabelInfo (since fonts are local)
    virtual LabelInfoRef makeLabelInfo(PlatformThreadInfo *inst,const std::vector<std::string> &fontName,float fontSize) = 0;
    
    /// Create a local platform label (fonts are local, and other stuff)
    virtual SingleLabelRef makeSingleLabel(PlatformThreadInfo *inst,const std::string &text) = 0;

    /// Tie a selection ID to the given vector object
    virtual void addSelectionObject(SimpleIdentity selectID,const VectorObjectRef &vecObj,const ComponentObjectRef &compObj) = 0;

    /// Return the width of the given line of text
    virtual double calculateTextWidth(PlatformThreadInfo *inInst,const LabelInfoRef &labelInfo,const std::string &testStr) = 0;
    
    /// Add a sprite sheet for use by the layers
    virtual void addSprites(MapboxVectorStyleSpritesRef newSprites);

    /// Create a local platform component object
    virtual ComponentObjectRef makeComponentObject(PlatformThreadInfo *inst, const Dictionary *desc = nullptr) = 0;

public:
    Scene *scene;
    CoordSystem *coordSys;
    MapboxVectorStyleSpritesRef sprites;

    /// @brief Style name
    std::string name;

    /// @brief Version number from the style
    int version;

    /// @brief Layers parsed from the style sheet
    std::vector<MapboxVectorStyleLayerRef> layers;

    /// @brief Layers sorted by their ID
    std::unordered_map<std::string, MapboxVectorStyleLayerRef> layersByName;

    /// Layers sorted by UUID
    std::unordered_map<long long, MapboxVectorStyleLayerRef> layersByUUID;

    /// @brief Layers sorted by source layer name
    std::unordered_multimap<std::string, MapboxVectorStyleLayerRef> layersBySource;

    VectorManagerRef vecManage;
    WideVectorManagerRef wideVecManage;
    MarkerManagerRef markerManage;
    LabelManagerRef labelManage;
    ComponentManagerRef compManage;
    
    // ID's for the various programs
    SimpleIdentity screenMarkerProgramID;
    SimpleIdentity vectorArealProgramID;
    SimpleIdentity vectorLinearProgramID;
    SimpleIdentity wideVectorProgramID;
    
    int zoomSlot;
    long long currentID;
};
typedef std::shared_ptr<MapboxVectorStyleSetImpl> MapboxVectorStyleSetImplRef;

}
