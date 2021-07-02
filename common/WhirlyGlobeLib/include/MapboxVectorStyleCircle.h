/* MapboxVectorStyleCircle.h
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

/**
 This is the paint portion of the MapboxVectorLayer Circle.
 
 You control the look of any rendered circles with this.  It would typically be produced in parsing a Mapbox Vector Style.
 */
class MapboxVectorCirclePaint
{
public:
    bool parse(PlatformThreadInfo *inst,MapboxVectorStyleSetImpl *styleSet,DictionaryRef styleEntry);

    /// Radius, in pixels, of the circle to be produced
    MapboxTransDoubleRef radius;

    /// Filled color of the circles
    RGBAColorRef fillColor;
    /// Filled opacity of the circles
    MapboxTransDoubleRef opacity;
    /// Stroke width, in pixels, around the outside of the circles
    MapboxTransDoubleRef strokeWidth;
    /// Color of the stroke around the outside of the circles
    RGBAColorRef strokeColor;
    /// Opacity o the stroke around the outside of the circles
    MapboxTransDoubleRef strokeOpacity;
};

/// Circles for individual points
class MapboxVectorLayerCircle : public MapboxVectorStyleLayer
{
public:
    MapboxVectorLayerCircle(MapboxVectorStyleSetImpl *styleSet) : MapboxVectorStyleLayer(styleSet) { }

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
        if (paint.fillColor) return *paint.fillColor;
        if (paint.strokeColor) return *paint.strokeColor;
        return RGBAColor::clear();
    }

public:
    MapboxVectorCirclePaint paint;
    
    SimpleIdentity circleTexID;
    Point2f circleSize;
    float importance;
    std::string uuidField;      // UUID field for markers/labels (from style settings)
    std::string repUUIDField;   // UUID field for representations (from style layers)
};

}
