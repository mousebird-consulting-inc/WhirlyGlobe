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

namespace WhirlyKit
{

//@implementation MaplyMapboxVectorStyleLayer
//{
//    NSString *category;
//}
//
//+ (id)VectorStyleLayer:(MapboxVectorStyleSet *)styleSet JSON:(NSDictionary *)layerDict drawPriority:(int)drawPriority
//{
//    MaplyMapboxVectorStyleLayer *layer = nil;
//    MaplyMapboxVectorStyleLayer *refLayer = nil;
//
//    // Look for the layer with that name
//    NSString *refLayerName = layerDict[@"ref"];
//    if (refLayer)
//    {
//        if (![refLayerName isKindOfClass:[NSString class]])
//        {
//            NSLog(@"Was expecting string for ref in layer");
//            return nil;
//        }
//
//        refLayer = styleSet.layersByName[refLayerName];
//        if (!refLayer)
//        {
//            NSLog(@"Didn't find layer named (%@)",refLayerName);
//            return nil;
//        }
//    }
//
//    NSString *type = layerDict[@"type"];
//    if (type && ![type isKindOfClass:[NSString class]])
//    {
//        NSLog(@"Expecting string type for layer");
//        return nil;
//    }
//    if ([type isEqualToString:@"fill"])
//    {
//        MapboxVectorLayerFill *fillLayer = [[MapboxVectorLayerFill alloc] initWithStyleEntry:layerDict parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:styleSet.viewC];
//        layer = fillLayer;
//    } else if ([type isEqualToString:@"line"])
//    {
//        MapboxVectorLayerLine *lineLayer = [[MapboxVectorLayerLine alloc] initWithStyleEntry:layerDict parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:styleSet.viewC];
//        layer = lineLayer;
//    } else if ([type isEqualToString:@"symbol"])
//    {
//        MapboxVectorLayerSymbol *symbolLayer = [[MapboxVectorLayerSymbol alloc] initWithStyleEntry:layerDict parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:styleSet.viewC];
//        layer = symbolLayer;
//    } else if ([type isEqualToString:@"circle"])
//    {
//        MapboxVectorLayerCircle *circleLayer = [[MapboxVectorLayerCircle alloc] initWithStyleEntry:layerDict parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:styleSet.viewC];
//        layer = circleLayer;
//    } else if ([type isEqualToString:@"raster"])
//    {
//        MapboxVectorLayerRaster *rasterLayer = [[MapboxVectorLayerRaster alloc] initWithStyleEntry:layerDict parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:styleSet.viewC];
//        layer = rasterLayer;
//    } else if ([type isEqualToString:@"background"])
//    {
//        MapboxVectorLayerBackground *backLayer = [[MapboxVectorLayerBackground alloc] initWithStyleEntry:layerDict parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:styleSet.viewC];
//        layer = backLayer;
//    }
//    if (layerDict[@"filter"])
//    {
//        layer.filter = [[MapboxVectorFilter alloc] initWithArray:[styleSet arrayValue:@"filter" dict:layerDict defVal:nil] styleSet:styleSet viewC:styleSet.viewC];
//        if (!layer.filter)
//        {
//            NSLog(@"MapboxStyleSet: Failed to parse filter for layer %@",layerDict[@"id"]);
//        }
//    }
//
//    layer.visible = [styleSet boolValue:@"visibility" dict:layerDict[@"layout"] onValue:@"visible" defVal:true];
//    layer.selectable = styleSet.tileStyleSettings.selectable;
//
//    if (layerDict[@"metadata"])
//    {
//        NSDictionary *metadataDict = layerDict[@"metadata"];
//        if ([metadataDict isKindOfClass:[NSDictionary class]])
//            layer.metadata = metadataDict;
//    }
//
//    return layer;
//}
//
//- (id)initWithStyleEntry:(NSDictionary *)layerDict parent:(MaplyMapboxVectorStyleLayer *)refLayer styleSet:(MapboxVectorStyleSet *)styleSet drawPriority:(int)drawPriority viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
//{
//    self = [super init];
//    if (!self)
//        return nil;
//
//    self.styleSet = styleSet;
//    self.drawPriorityPerLevel = styleSet.tileStyleSettings.drawPriorityPerLevel;
//    self.drawPriority = drawPriority;
//    self.uuid = [styleSet generateID];
//
//    _minzoom = -1;
//    _maxzoom = -1;
//
//    self.ident = layerDict[@"id"];
//    self.source = [styleSet stringValue:@"source" dict:layerDict defVal:refLayer.source];
//    self.sourceLayer = [styleSet stringValue:@"source-layer" dict:layerDict defVal:refLayer.sourceLayer];
//    self.minzoom = [styleSet intValue:@"minzoom" dict:layerDict defVal:refLayer.minzoom];
//    self.maxzoom = [styleSet intValue:@"maxzoom" dict:layerDict defVal:refLayer.maxzoom];
//    category = [styleSet stringValue:@"wkcategory" dict:layerDict defVal:nil];
//
//    return self;
//}
//
//- (NSString *)getCategory
//{
//    return category;
//}
//
//- (void)buildObjects:(NSArray *)vecObjs forTile:(MaplyVectorTileData *)tileInfo viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
//{
//}
//
//@end


}
