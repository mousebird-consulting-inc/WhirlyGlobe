/*  MapboxVectorStyleSymbol.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/17/15.
 *  Copyright 2011-2022 mousebird consulting
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

#import "MapboxVectorStyleSetC.h"
#import "MapboxVectorStyleLayer.h"

namespace WhirlyKit
{

typedef enum {MBTextCenter,MBTextLeft,MBTextRight,MBTextTop,MBTextBottom,MBTextTopLeft,MBTextTopRight,MBTextBottomLeft,MBTextBottomRight} MapboxTextAnchor;
typedef enum {MBPlacePoint,MBPlaceLine} MapboxSymbolPlacement;
typedef enum {MBTextTransNone,MBTextTransUppercase,MBTextTransLowercase} MapboxTextTransform;

/** Controls the layout logic for Mapbox Symbols.
  */
struct MapboxVectorSymbolLayout
{
    MapboxVectorSymbolLayout() = default;
    MapboxVectorSymbolLayout(const MapboxVectorSymbolLayout&) = default;

    bool parse(PlatformThreadInfo *inst,
               MapboxVectorStyleSetImpl *styleSet,
               const DictionaryRef &styleEntry);

    /// How we place the symbol (at a point, or along a line)
    MapboxSymbolPlacement placement = MBPlacePoint;
    /// If set, turn the text uppercase
    MapboxTextTransform textTransform = MBTextTransNone;
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
    double globalTextScale = 0.0;
    /// How the text is laid out in relation to it's attach point
    MapboxTextAnchor textAnchor = MBTextCenter;
    MapboxTextAnchor iconAnchor = (MapboxTextAnchor)-1;
    /// Whether it goes to the layout engine
    bool iconAllowOverlap = false;
    bool textAllowOverlap = false;
    /// Text justification
    bool textJustifySet = false;
    TextJustify textJustify = WhirlyKitTextCenter;
    
    float layoutImportance = 0.0f;
        
    // Text can be expressed in a complex way
    MapboxTransTextRef textField;

    // Name of icon, if present, can be expressed the same way as text
    MapboxTransTextRef iconImageField;

    // Scale of the icon based on its original size
    MapboxTransDoubleRef iconSize;
};

// Symbol visuals
struct MapboxVectorSymbolPaint
{
    MapboxVectorSymbolPaint() = default;
    MapboxVectorSymbolPaint(const MapboxVectorSymbolPaint&) = default;

    bool parse(PlatformThreadInfo *inst,
               MapboxVectorStyleSetImpl *styleSet,
               const DictionaryRef &styleEntry);

    // Default text color
    MapboxTransColorRef textColor;
    MapboxTransDoubleRef textOpacity;
    // If there's a halo, this is the color
    MapboxTransColorRef textHaloColor;
    // If there's a halo, it blurs out like so
    MapboxTransDoubleRef textHaloBlur;
    // If there's a halo, this is the size
    MapboxTransDoubleRef textHaloWidth;
    MapboxTransDoubleRef iconOpacity;
};

/// @brief Icons and symbols
class MapboxVectorLayerSymbol : public MapboxVectorStyleLayer
{
public:
    MapboxVectorLayerSymbol(MapboxVectorStyleSetImpl *styleSet) : MapboxVectorStyleLayer(styleSet) { }

    virtual bool parse(PlatformThreadInfo *inst,
                       const DictionaryRef &styleEntry,
                       const MapboxVectorStyleLayerRef &refLayer,
                       int drawPriority) override;
    
    virtual void buildObjects(PlatformThreadInfo *inst,
                              const std::vector<VectorObjectRef> &vecObjs,
                              const VectorTileDataRef &tileInfo,
                              const Dictionary *desc,
                              const CancelFunction &cancelFn) override;

    virtual MapboxVectorStyleLayerRef clone() const override;
    virtual MapboxVectorStyleLayer& copy(const MapboxVectorStyleLayer&) override;

    virtual void cleanup(PlatformThreadInfo *inst,ChangeSet &changes) override { }

    virtual std::string getLegendText(float zoom) const override {
        if (layout.iconImageField) {
            const auto text = layout.iconImageField->textForZoom(zoom);
            if (text.valid && !text.chunks.empty()) {
                return text.chunks[0].str;
            }
        }
        return std::string();
    }
    virtual RGBAColor getLegendColor(float zoom) const override {
        return paint.textColor ? paint.textColor->colorForZoom(zoom) : RGBAColor::clear();
    }

    std::string breakUpText(PlatformThreadInfo *,
                            const std::string &text,
                            double textMaxWidth,
                            const LabelInfoRef &);
    SingleLabelRef setupLabel(PlatformThreadInfo *,
                              const Point2f &pt,
                              const LabelInfoRef &,
                              const MutableDictionaryRef &attrs,
                              const VectorTileDataRef &tileInfo,
                              bool mergedIcon);
    std::unique_ptr<Marker> setupMarker(PlatformThreadInfo *,
                        const Point2f &pt,
                        const MutableDictionaryRef &attrs,
                        const VectorTileDataRef &tileInfo);

protected:
    // N.B.: This does not copy the base members
    MapboxVectorLayerSymbol& operator=(const MapboxVectorLayerSymbol&) = default;

public:
    MapboxVectorSymbolLayout layout;
    MapboxVectorSymbolPaint paint;

    /// If set, only one label with its text will be displayed.  Sorted out by the layout manager.
    bool uniqueLabel = false;
    bool useZoomLevels = false;
};

}
