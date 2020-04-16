/*
*  MapboxVectorStyleLayer.cpp
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

MapboxVectorStyleLayerRef MapboxVectorStyleLayer::VectorStyleLayer(MapboxVectorStyleSetImplRef styleSet,DictionaryRef layerDict,int drawPriority)
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
    if (type.empty()) {
        wkLogLevel(Warn, "Expecting string type for layer");
        return NULL;
    }
    
    if (type == "fill") {
        layer = MapboxVectorStyleLayerRef(new MapboxVectorLayerFill(styleSet));
    } else if (type == "line") {
        layer = MapboxVectorStyleLayerRef(new MapboxVectorLayerLine(styleSet));
    } else if (type == "symbol") {
        layer = MapboxVectorStyleLayerRef(new MapboxVectorLayerSymbol(styleSet));
    } else if (type == "circle") {
        layer = MapboxVectorStyleLayerRef(new MapboxVectorLayerCircle(styleSet));
    } else if (type == "raster") {
        layer = MapboxVectorStyleLayerRef(new MapboxVectorLayerRaster(styleSet));
    } else if (type == "background") {
        layer = MapboxVectorStyleLayerRef(new MapboxVectorLayerBackground(styleSet));
    }
    layer->parse(layerDict, refLayer, drawPriority);
    
    if (layerDict->getType("filter") == DictTypeArray) {
        layer->filter = MapboxVectorFilterRef(new MapboxVectorFilter(layerDict->getArray("filter"),styleSet));
    }
    
    layer->visible = styleSet->boolValue("visibility", layerDict->getDict("layout"), "visible", true);
    layer->selectable = styleSet->tileStyleSettings->selectable;
    
    layer->metadata = layerDict->getDict("metadata");
    
    return layer;
}

MapboxVectorStyleLayer::MapboxVectorStyleLayer(MapboxVectorStyleSetImplRef styleSet)
: visible(true), minzoom(0), maxzoom(0), drawPriority(0), drawPriorityPerLevel(0),
selectable(false), uuid(0), geomAdditiveVal(false), styleSet(styleSet)
{
}

MapboxVectorStyleLayer::~MapboxVectorStyleLayer()
{
}

bool MapboxVectorStyleLayer::parse(DictionaryRef styleEntry,
                                   MapboxVectorStyleLayerRef refLayer,
                                   int inDrawPriority)
{
    drawPriorityPerLevel = styleSet->tileStyleSettings->drawPriorityPerLevel;
    drawPriority = inDrawPriority;
    uuid = styleSet->generateID();
    
    ident = styleEntry->getInt("id");
    source = styleSet->stringValue("source", styleEntry, refLayer->source);
    sourceLayer = styleSet->stringValue("source-layer", styleEntry, refLayer->sourceLayer);
    
    minzoom = styleSet->intValue("minzoom", styleEntry, -1);
    maxzoom = styleSet->intValue("maxzoom", styleEntry, -1);
    
    category = styleSet->stringValue("wkcategory", styleEntry, "");
    
    return true;
}

long long MapboxVectorStyleLayer::getUuid()
{
    return styleSet->generateID();
}

const std::string &MapboxVectorStyleLayer::getCategory()
{
    return category;
}

bool MapboxVectorStyleLayer::geomAdditive()
{
    return geomAdditiveVal;
}

void MapboxVectorStyleLayer::cleanup(ChangeSet &changes)
{
}

}
