/*
 *  MapboxVectorStyleBackground.h
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

/**
  This class corresponds to the paint portion of the Mapbox Vector Style definition
    of the background.  You get one of these from parsing a Style, don't generate one.
 */
class MapboxVectorBackgroundPaint
{
public:
    bool parse(PlatformThreadInfo *inst,MapboxVectorStyleSetImpl *styleSet,DictionaryRef styleEntry);

    MapboxTransColorRef color;
    MapboxTransDoubleRef opacity;
};

/**
 This is the layer corresponding to the background in a Mapbox Vector Style definition.
 You don't create these.  They come from a Style sheet.
 */
class MapboxVectorLayerBackground : public MapboxVectorStyleLayer
{
public:
    MapboxVectorLayerBackground(MapboxVectorStyleSetImpl *styleSet) : MapboxVectorStyleLayer(styleSet) { }

    virtual bool parse(PlatformThreadInfo *inst,
                       DictionaryRef styleEntry,
                       MapboxVectorStyleLayerRef refLayer,
                       int drawPriority);
    
    virtual void buildObjects(PlatformThreadInfo *inst,
                              std::vector<VectorObjectRef> &vecObjs,
                              VectorTileDataRef tileInfo);

public:
    /// Controls how the background looks.
    MapboxVectorBackgroundPaint paint;
};
typedef std::shared_ptr<MapboxVectorLayerBackground> MapboxVectorLayerBackgroundRef;

}
