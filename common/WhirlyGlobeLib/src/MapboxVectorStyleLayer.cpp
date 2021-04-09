/* MapboxVectorStyleLayer.cpp
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

#import "MapboxVectorStyleLayer.h"
#import "MapboxVectorStyleSetC.h"
#import "MapboxVectorStyleBackground.h"
#import "MapboxVectorStyleSymbol.h"
#import "MapboxVectorStyleRaster.h"
#import "MapboxVectorStyleLine.h"
#import "MapboxVectorStyleFill.h"
#import "MapboxVectorStyleCircle.h"
#import "MapboxVectorFilter.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{

MapboxVectorStyleLayerRef MapboxVectorStyleLayer::VectorStyleLayer(PlatformThreadInfo *inst,
                                                                   MapboxVectorStyleSetImpl *styleSet,
                                                                   const DictionaryRef &layerDict,
                                                                   int drawPriority)
{
    MapboxVectorStyleLayerRef layer;
    MapboxVectorStyleLayerRef refLayer;
    
    if (layerDict->getType("ref") == DictTypeString) {
        std::string ref = layerDict->getString("ref");
        refLayer = styleSet->getLayer(ref);
        
        if (!refLayer)
            wkLogLevel(Warn,"Didn't find layer named %s",ref.c_str());
    }

    std::string type = layerDict->getString("type");
    if (type.empty() && refLayer && !refLayer->type.empty())
        type = refLayer->type;

    if (type.empty()) {
        wkLogLevel(Warn, "Expecting string type for layer");
        return nullptr;
    }
    
    if (type == "fill") {
        layer = std::make_shared<MapboxVectorLayerFill>(styleSet);
    } else if (type == "fill-extrusion") {
        wkLogLevel(Warn, "Treating fill-extrusion layer as fill");
        layer = std::make_shared<MapboxVectorLayerFill>(styleSet);
    } else if (type == "line") {
        layer = std::make_shared<MapboxVectorLayerLine>(styleSet);
    } else if (type == "symbol") {
        layer = std::make_shared<MapboxVectorLayerSymbol>(styleSet);
    } else if (type == "circle") {
        layer = std::make_shared<MapboxVectorLayerCircle>(styleSet);
    } else if (type == "raster") {
        layer = std::make_shared<MapboxVectorLayerRaster>(styleSet);
    } else if (type == "background") {
        layer = std::make_shared<MapboxVectorLayerBackground>(styleSet);
    } else if (type == "fill-extrusion") {
        wkLogLevel(Warn,"Skipping layer type %s",type.c_str());
        return nullptr;
    }
    if (!layer) {
        wkLogLevel(Warn,"Unknown layer type %s",type.c_str());
        return nullptr;
    }
    layer->type = type;
    if (!layer->parse(inst, layerDict, refLayer, drawPriority)) {
        wkLogLevel(Warn, "Failed to parse layer %s",layer->ident.c_str());
        return nullptr;
    }
    
    if (layerDict->getType("filter") == DictTypeArray) {
        layer->filter = std::make_shared<MapboxVectorFilter>();
        auto filterArray = layerDict->getArray("filter");
        layer->filter->parse(filterArray,styleSet);
    }

    layer->visible = styleSet->boolValue("visibility", layerDict->getDict("layout"), "visible", true);
    layer->selectable = styleSet->tileStyleSettings->selectable;
    layer->metadata = layerDict->getDict("metadata");
    layer->representation = layerDict->getString("X-Maply-Representation");

    return layer;
}

MapboxVectorStyleLayer::MapboxVectorStyleLayer(MapboxVectorStyleSetImpl *styleSet)
: visible(true), minzoom(0), maxzoom(0), drawPriority(0),
selectable(false), uuid(0), geomAdditiveVal(false), styleSet(styleSet)
{
}

MapboxVectorStyleLayer::~MapboxVectorStyleLayer()
{
}

bool MapboxVectorStyleLayer::parse(PlatformThreadInfo *inst,
                                   const DictionaryRef &styleEntry,
                                   const MapboxVectorStyleLayerRef &refLayer,
                                   int inDrawPriority)
{
    if (!styleEntry)
    {
        return false;
    }

    drawPriority = inDrawPriority;
    uuid = styleSet->generateID();

    ident = styleEntry->getString("id");
    source = styleSet->stringValue("source", styleEntry, refLayer ? refLayer->source : std::string());
    sourceLayer = styleSet->stringValue("source-layer", styleEntry, refLayer ? refLayer->sourceLayer : std::string());
    
    minzoom = styleSet->intValue("minzoom", styleEntry, 0);
    maxzoom = styleSet->intValue("maxzoom", styleEntry, 1000);
    
    category = styleSet->stringValue("wkcategory", styleEntry, std::string());
    
    return true;
}

long long MapboxVectorStyleLayer::getUuid(PlatformThreadInfo *inst)
{
    return uuid;
}

std::string MapboxVectorStyleLayer::getCategory(PlatformThreadInfo *inst)
{
    return category;
}

bool MapboxVectorStyleLayer::geomAdditive(PlatformThreadInfo *inst)
{
    return geomAdditiveVal;
}

void MapboxVectorStyleLayer::cleanup(PlatformThreadInfo *inst,ChangeSet &changes)
{
}

}
