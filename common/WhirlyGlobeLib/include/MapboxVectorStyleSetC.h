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
#import "MapboxVectorStyleSpritesImpl.h"
#import <set>

namespace WhirlyKit
{

class MapboxVectorStyleLayer;
typedef std::shared_ptr<MapboxVectorStyleLayer> MapboxVectorStyleLayerRef;

class MapboxVectorStyleSetImpl;
typedef std::shared_ptr<MapboxVectorStyleSetImpl> MapboxVectorStyleSetImplRef;

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
};

// Collection of function stops
class MaplyVectorFunctionStops
{
public:
    bool parse(DictionaryRef entry,MapboxVectorStyleSetImpl *styleSet);

    /// @brief Calculate a value given the zoom level
    double valueForZoom(double zoom);

    /// @brief This version returns colors, assuming we're dealing with colors
    RGBAColorRef colorForZoom(int zoom);

    /// @brief Returns the minimum value
    double minValue();

    /// @brief Returns the maximum value
    double maxValue();

protected:
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

    // Return a color for the given zoom level
    RGBAColor colorForZoom(double zoom);
    
protected:
    RGBAColorRef color;
    bool useAlphaOverride;
    double alpha;
    MaplyVectorFunctionStopsRef stops;
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
class MapboxVectorStyleSetImpl : public VectorStyleDelegateImpl
{
public:
    MapboxVectorStyleSetImpl(Scene *scene,CoordSystem *coordSys,VectorStyleSettingsImplRef settings);
    virtual ~MapboxVectorStyleSetImpl();
    
    // Parse the entire style sheet.  False on failure
    bool parse(PlatformThreadInfo *inst,DictionaryRef dict);
        
    /// @brief Default settings and scale factor for Mapnik vector geometry.
    VectorStyleSettingsImplRef tileStyleSettings;

    /// @brief Generates a unique ID for a style
    long long generateID();

    /// @brief Return an integer value for the given name, taking the constants into account.
    int intValue(const std::string &name,DictionaryRef dict,int defVal);

    /// @brief Return a double value for the given name, taking the constants into account
    double doubleValue(DictionaryEntryRef entry,double defVal);

    /// @brief Return a double value for the given name, taking the constants into account
    double doubleValue(const std::string &name,DictionaryRef dict,double defVal);
        
    /// @brief Return a bool for the given name.  True if it matches the onString.  Default if it's missing
    bool boolValue(const std::string &name,DictionaryRef dict,const std::string &onString,bool defVal);

    /// @brief Return a string for the given name, taking the constants into account
    std::string stringValue(const std::string &name,DictionaryRef dict,const std::string &defVal);

    /// @brief Return an array for the given name, taking the constants into account
    std::vector<DictionaryEntryRef> arrayValue(const std::string &name,DictionaryRef dict);

    /// @brief Return a color for the given name, taking the constants into account
    RGBAColorRef colorValue(const std::string &name,DictionaryEntryRef val,DictionaryRef dict,RGBAColorRef defVal,bool multiplyAlpha);
    RGBAColorRef colorValue(const std::string &name,DictionaryEntryRef val,DictionaryRef dict,RGBAColor defVal,bool multiplyAlpha);

    /// @brief Return the integer corresponding to the name.  Basically parse the enumerated type
    int enumValue(DictionaryEntryRef entry,const char *options[],int defVal);

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

    /// @brief Check for and report an unsupported field
    void unsupportedCheck(const char *field,const char *what,DictionaryRef styleEntry);
    
    /// Fetch a layer by name
    virtual MapboxVectorStyleLayerRef getLayer(const std::string &name);
    
    /// Fish out the background color for a given zoom level
    RGBAColorRef backgroundColor(double zoom);
    
    /** VectorStyleDelegateImpl **/
    
    /// Return the styles that apply to the given feature (attributes).
    virtual std::vector<VectorStyleImplRef> stylesForFeature(DictionaryRef attrs,
                                                             const QuadTreeIdentifier &tileID,
                                                             const std::string &layerName);
    
    /// Return true if the given layer is meant to display for the given tile (zoom level)
    virtual bool layerShouldDisplay(const std::string &name,
                                    const QuadTreeNew::Node &tileID);

    /// Return the style associated with the given UUID.
    virtual VectorStyleImplRef styleForUUID(long long uuid);

    // Return a list of all the styles in no particular order.  Needed for categories and indexing
    virtual std::vector<VectorStyleImplRef> allStyles();

    
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
    virtual LabelInfoRef makeLabelInfo(PlatformThreadInfo *inst,const std::string &fontName,float fontSize) = 0;
    
    /// Create a local platform label (fonts are local, and other stuff)
    virtual SingleLabelRef makeSingleLabel(PlatformThreadInfo *inst,const std::string &text) = 0;

    /// Tie a selection ID to the given vector object
    virtual void addSelectionObject(SimpleIdentity selectID,VectorObjectRef vecObj,ComponentObjectRef compObj) = 0;

    /// Return the width of the given line of text
    virtual double calculateTextWidth(PlatformThreadInfo *inInst,LabelInfoRef labelInfo,const std::string &testStr) = 0;
    
    /// Create a local platform component object
    virtual ComponentObjectRef makeComponentObject(PlatformThreadInfo *inst) = 0;

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
    std::map<std::string, MapboxVectorStyleLayerRef> layersByName;
    
    /// Layers sorted by UUID
    std::map<long long, MapboxVectorStyleLayerRef> layersByUUID;

    /// @brief Layers sorted by source layer name
    std::map<std::string, std::vector<MapboxVectorStyleLayerRef> > layersBySource;

    VectorManager *vecManage;
    WideVectorManager *wideVecManage;
    MarkerManager *markerManage;
    LabelManager *labelManage;
    ComponentManager *compManage;
    
    // ID's for the various programs
    SimpleIdentity screenMarkerProgramID;
    SimpleIdentity vectorArealProgramID;
    SimpleIdentity vectorLinearProgramID;
    SimpleIdentity wideVectorProgramID;
    
    long long currentID;
};
typedef std::shared_ptr<MapboxVectorStyleSetImpl> MapboxVectorStyleSetImplRef;

}
