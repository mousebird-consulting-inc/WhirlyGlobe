/*
*  MapboxVectorStyleLayer.h
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

#import "MapboxVectorFilter.h"
#import "VectorObject.h"
#import "MaplyVectorStyleC.h"

namespace WhirlyKit
{

class MapboxVectorStyleLayer;
typedef std::shared_ptr<MapboxVectorStyleLayer> MapboxVectorStyleLayerRef;

/** @brief Layer definition from the Style Sheet.
    @details This is a single layer from the Mapbox style sheet.  It's also used to build visible objects.
  */
class MapboxVectorStyleLayer : public VectorStyleImpl
{
public:

    /// @brief Initialize with the style sheet and the entry for this layer
    static MapboxVectorStyleLayerRef VectorStyleLayer(PlatformThreadInfo *inst,
                                                      MapboxVectorStyleSetImpl *styleSet,
                                                      DictionaryRef layerDict,
                                                      int drawPriority);

    /// @brief Base class initialization.  Copies data out of the refLayer
    MapboxVectorStyleLayer(MapboxVectorStyleSetImpl *styleSet);
    virtual ~MapboxVectorStyleLayer();

    // Parse the layer entry out of the style sheet
    virtual bool parse(PlatformThreadInfo *inst,
                       DictionaryRef styleEntry,
                       MapboxVectorStyleLayerRef parentLayer,
                       int drawPriority);

    /// Unique Identifier for this style
    virtual long long getUuid();
    
    /// Category used for sorting
    virtual std::string getCategory();
    
    // Note: This no longer really holds
    /// Set if this geometry is additive (e.g. sticks around) rather than replacement
    virtual bool geomAdditive();

    /// Construct objects related to this style based on the input data.
    virtual void buildObjects(PlatformThreadInfo *inst,
                              std::vector<VectorObjectRef> &vecObjs,
                              VectorTileDataRef tileInfo) = 0;
    
    /// Clean up any objects (textures, probably)
    virtual void cleanup(PlatformThreadInfo *inst,ChangeSet &changes);

    MapboxVectorStyleSetImpl *styleSet;

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
    
    /// Category value, if set
    std::string category;
};


}
