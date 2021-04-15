/*  MapboxVectorStyleLine.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/17/15.
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

#import "MapboxVectorStyleSetC.h"
#import "MapboxVectorStyleLayer.h"

namespace WhirlyKit
{

typedef enum {MBLineCapButt,MBLineCapRound,MBLineCapSquare} MapboxVectorLineCap;
typedef enum {MBLineJoinBevel,MBLineJoinRound,MBLineJoinMiter} MapboxVectorLineJoin;

/**
 Controls how the lines are laid out (geometry, largely).
 */
class MapboxVectorLineLayout
{
public:
    bool parse(PlatformThreadInfo *inst,MapboxVectorStyleSetImpl *styleSet,const DictionaryRef &styleEntry);

    MapboxVectorLineCap cap;
    MapboxVectorLineJoin join;
    double miterLimit;
    double roundLimit;
};

/**
 Controls how the vector line looks.
 */
class MapboxVectorLinePaint
{
public:
    bool parse(PlatformThreadInfo *inst,MapboxVectorStyleSetImpl *styleSet,const DictionaryRef &styleEntry);

    MapboxTransDoubleRef opacity;
    MapboxTransColorRef color;
    MapboxTransDoubleRef width;
    MapboxTransDoubleRef offset;
    std::string pattern;
    std::vector<double> lineDashArray;
};

/// @brief The line style
class MapboxVectorLayerLine : public MapboxVectorStyleLayer
{
public:
    MapboxVectorLayerLine(MapboxVectorStyleSetImpl *styleSet) : MapboxVectorStyleLayer(styleSet) { }

    virtual bool parse(PlatformThreadInfo *inst,
                       const DictionaryRef &styleEntry,
                       const MapboxVectorStyleLayerRef &refLayer,
                       int drawPriority) override;
    
    virtual void buildObjects(PlatformThreadInfo *inst,
                              const std::vector<VectorObjectRef> &vecObjs,
                              const VectorTileDataRef &tileInfo,
                              const Dictionary *desc,
                              const CancelFunction &cancelFn) override;
    
    virtual void cleanup(PlatformThreadInfo *inst,ChangeSet &changes) override;

    virtual RGBAColor getLegendColor(float zoom) const override {
        return paint.color ? paint.color->colorForZoom(zoom) : RGBAColor::clear();
    }

public:
    MapboxVectorLineLayout layout;
    MapboxVectorLinePaint paint;
    bool linearClipToBounds;
    bool dropGridLines;

    // If non-zero we'll subdivide the line along a globe to the given tolerance
    double subdivToGlobe;

    double lineScale;
    double totLen;
    double fade;
    SimpleIdentity filledLineTexID;

    std::string uuidField;      // UUID field for markers/labels (from style settings)
    std::string repUUIDField;   // UUID field for representations (from style layers)
};

}
