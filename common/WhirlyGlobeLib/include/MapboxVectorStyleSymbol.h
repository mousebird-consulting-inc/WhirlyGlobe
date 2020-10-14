/*
 *  MapboxVectorStyleSymbol.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/17/15.
 *  Copyright 2011-2019 mousebird consulting
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

#import "MapboxVectorStyleSetC.h"
#import "MapboxVectorStyleLayer.h"

namespace WhirlyKit
{

typedef enum {MBTextCenter,MBTextLeft,MBTextRight,MBTextTop,MBTextBottom,MBTextTopLeft,MBTextTopRight,MBTextBottomLeft,MBTextBottomRight} MapboxTextAnchor;
typedef enum {MBPlacePoint,MBPlaceLine} MapboxSymbolPlacement;
typedef enum {MBTextTransNone,MBTextTransUppercase,MBTextTransLowercase} MapboxTextTransform;

/** Controls the layout logic for Mapbox Symbols.
  */
class MapboxVectorSymbolLayout
{
public:
    bool parse(PlatformThreadInfo *inst,MapboxVectorStyleSetImpl *styleSet,DictionaryRef styleEntry);

    /// How we place the symbol (at a point, or along a line)
    MapboxSymbolPlacement placement;
    /// If set, turn the text uppercase
    MapboxTextTransform textTransform;
    /// @brief Font to use for display
    std::vector<std::string> textFontNames;
    /// @brief The maximum line width for wrapping
    MapboxTransDoubleRef textMaxWidth;
    /// If set, the immutable text size
    MapboxTransDoubleRef textSize;
    /// Text offset in ems
    MapboxTransDoubleRef textOffsetX;
    MapboxTransDoubleRef textOffsetY;
    /// Text scale from the global settings
    double globalTextScale;
    /// How the text is laid out in relation to it's attach point
    MapboxTextAnchor textAnchor;
    /// Whether it goes to the layout engine
    bool iconAllowOverlap,textAllowOverlap;
    /// Text justification
    bool textJustifySet;
    TextJustify textJustify;
    
    float layoutImportance;
        
    // Text can be expressed in a complex way
    MapboxRegexField textField;

    // Name of icon, if present, can be expressed the same way as text
    MapboxTransTextRef iconImageField;

    // Scale of the icon based on its original size
    MapboxTransDoubleRef iconSize;
};

// Symbol visuals
class MapboxVectorSymbolPaint
{
public:
    bool parse(PlatformThreadInfo *inst,MapboxVectorStyleSetImpl *styleSet,DictionaryRef styleEntry);

    // Default text color
    MapboxTransColorRef textColor;
    MapboxTransDoubleRef textOpacity;
    // If there's a halo, this is the color
    MapboxTransColorRef textHaloColor;
    // If there's a halo, it blurs out like so
    MapboxTransDoubleRef textHaloBlur;
    // If there's a halo, this is the size
    MapboxTransDoubleRef textHaloWidth;
};

/// @brief Icons and symbols
class MapboxVectorLayerSymbol : public MapboxVectorStyleLayer
{
public:
    MapboxVectorLayerSymbol(MapboxVectorStyleSetImpl *styleSet) : MapboxVectorStyleLayer(styleSet) { }

    virtual bool parse(PlatformThreadInfo *inst,
                       DictionaryRef styleEntry,
                       MapboxVectorStyleLayerRef refLayer,
                       int drawPriority) override;
    
    virtual void buildObjects(PlatformThreadInfo *inst,
                              std::vector<VectorObjectRef> &vecObjs,
                              VectorTileDataRef tileInfo) override;
    
    virtual void cleanup(PlatformThreadInfo *inst,ChangeSet &changes) override;

public:
    std::string breakUpText(PlatformThreadInfo *inst,const std::string &text,double textMaxWidth,LabelInfoRef labelInfo);
    SingleLabelRef setupLabel(PlatformThreadInfo *inst,const Point2f &pt,LabelInfoRef labelInfo,MutableDictionaryRef attrs,VectorTileDataRef tileInfo);
    Marker *setupMarker(PlatformThreadInfo *inst,const Point2f &pt,VectorObjectRef vecObj,MutableDictionaryRef attrs,ComponentObjectRef compObj,VectorTileDataRef tileInfo);
    
    MapboxVectorSymbolLayout layout;
    MapboxVectorSymbolPaint paint;

    /// If set, only one label with its text will be displayed.  Sorted out by the layout manager.
    bool uniqueLabel;
    std::string uuidField;
    bool useZoomLevels;
};

}
