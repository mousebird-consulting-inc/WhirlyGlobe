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
    bool parse(MapboxVectorStyleSetImplRef styleSet,DictionaryRef styleEntry);

    /// How we place the symbol (at a point, or along a line)
    MapboxSymbolPlacement placement;
    /// If set, turn the text uppercase
    MapboxTextTransform textTransform;
    /// @brief Font to use for display
    std::string textFontName;
    /// @brief The maximum line width for wrapping
    MapboxTransDoubleRef textMaxWidth;
    /// If set, the immutable text size
    MapboxTransDoubleRef textSize;
    /// Text scale from the global settings
    double globalTextScale;
    /// How the text is laid out in relation to it's attach point
    MapboxTextAnchor textAnchor;
    
    // Used to track text data
    class TextChunk {
    public:
        // Set if this is a simple string
        std::string str;

        // Possible key names in the data. Tried in this order.
        // Not set if this is a simple string
        std::vector<std::string> keys;
    };

    float layoutImportance;
    std::vector<TextChunk> textChunks;
};

// Symbol visuals
class MapboxVectorSymbolPaint
{
public:
    bool parse(MapboxVectorStyleSetImplRef styleSet,DictionaryRef styleEntry);

    // Default text color
    MapboxTransColorRef textColor;
    MapboxTransDoubleRef textOpacity;
    // If there's a halo, this is the color
    RGBAColorRef textHaloColor;
    // If there's a halo, this is the size
    double textHaloWidth;
};

/// @brief Icons and symbols
class MapboxVectorLayerSymbol : public MapboxVectorStyleLayer
{
public:
    virtual bool parse(MapboxVectorStyleSetImplRef styleSet,
                       DictionaryRef styleEntry,
                       MapboxVectorStyleLayerRef refLayer,
                       int drawPriority);
    
    virtual void buildObjects(MapboxVectorStyleSetImplRef styleSet,std::vector<VectorObjectRef> &vecObjs,VectorTileDataRef tileInfo);
    
    virtual void cleanup(ChangeSet &changes);

protected:
    MapboxVectorSymbolLayout layout;
    MapboxVectorSymbolPaint paint;

    /// If set, only one label with its text will be displayed.  Sorted out by the layout manager.
    bool uniqueLabel;
    std::string uuidField;
    bool useZoomLevels;
};

}
