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
struct MapboxVectorLineLayout
{
    MapboxVectorLineLayout() = default;
    MapboxVectorLineLayout(const MapboxVectorLineLayout&) = default;

    bool parse(PlatformThreadInfo *inst,MapboxVectorStyleSetImpl *styleSet,const DictionaryRef &styleEntry);

    MapboxVectorLineCap cap = MBLineCapButt;
    MapboxVectorLineJoin join = MBLineJoinBevel;
    double miterLimit = 0.0;
    double roundLimit = 0.0;
};

/**
 Controls how the vector line looks.
 */
struct MapboxVectorLinePaint
{
    MapboxVectorLinePaint() = default;
    MapboxVectorLinePaint(const MapboxVectorLinePaint&) = default;

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

    virtual MapboxVectorStyleLayerRef clone() const override;
    virtual MapboxVectorStyleLayer& copy(const MapboxVectorStyleLayer&) override;

    virtual void buildObjects(PlatformThreadInfo *inst,
                              const std::vector<VectorObjectRef> &vecObjs,
                              const VectorTileDataRef &tileInfo,
                              const Dictionary *desc,
                              const CancelFunction &cancelFn) override;
    
    virtual void cleanup(PlatformThreadInfo *inst,ChangeSet &changes) override { }

    virtual RGBAColor getLegendColor(float zoom) const override {
        return paint.color ? paint.color->colorForZoom(zoom) : RGBAColor::clear();
    }

protected:
    // N.B.: This does not copy the base members
    MapboxVectorLayerLine& operator=(const MapboxVectorLayerLine&) = default;

public:
    MapboxVectorLineLayout layout;
    MapboxVectorLinePaint paint;
    bool linearClipToBounds = false;
    bool dropGridLines = false;

    // If non-zero we'll subdivide the line along a globe to the given tolerance
    double subdivToGlobe = 0.0;

    double lineScale = 0.0;
    double totLen = 0.0;
    double fade = 0.0;
    SimpleIdentity filledLineTexID = EmptyIdentity;
};

}
