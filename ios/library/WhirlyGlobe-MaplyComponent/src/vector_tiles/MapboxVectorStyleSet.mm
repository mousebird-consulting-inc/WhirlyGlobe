/*
 *  MapboxVectorStyleSet.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/16/15.
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

#import "private/MapboxVectorStyleSet_private.h"
#import "private/MapboxVectorStyleBackground_private.h"
#import "private/MapboxVectorStyleFill_private.h"
#import "private/MapboxVectorStyleLine_private.h"
#import "private/MapboxVectorStyleRaster_private.h"
#import "private/MapboxVectorStyleSymbol_private.h"
#import "private/MapboxVectorStyleCircle_private.h"
#import <map>

@implementation MapboxVectorStyleSet
{
    int currentID;
    NSMutableDictionary *layersByUUID;
}

- (id)initWithJSON:(NSData *)styleJSON settings:(MaplyVectorStyleSettings *)settings viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC filter:(bool (^)(NSMutableDictionary * __nonnull))filterBlock
{
    self = [super init];
    if (!self)
        return nil;
    
    _viewC = viewC;
    NSError *error = nil;
    _tileStyleSettings = settings;
    if (!_tileStyleSettings)
        _tileStyleSettings = [[MaplyVectorStyleSettings alloc] initWithScale:UIScreen.mainScreen.scale];
    NSDictionary *styleDict = [NSJSONSerialization JSONObjectWithData:styleJSON options:NULL error:&error];
    if (!styleDict)
        return nil;
    
    _name = styleDict[@"name"];
    _version = [styleDict[@"version"] integerValue];
    _constants = styleDict[@"constants"];
    _spriteURL = styleDict[@"sprite"];
    
    // Sources tell us where to get tiles
    NSDictionary *sourceStyles = styleDict[@"sources"];
    NSMutableArray *sources = [NSMutableArray array];
    for (NSString *sourceName in sourceStyles.allKeys) {
        NSDictionary *styleEntry = sourceStyles[sourceName];
        MaplyMapboxVectorStyleSource *source = [[MaplyMapboxVectorStyleSource alloc] initWithName:sourceName styleEntry:styleEntry styleSet:self viewC:_viewC];
        if (source)
            [sources addObject:source];
    }
    
    // Layers are where the action is
    NSArray *layerStyles = styleDict[@"layers"];
    NSMutableArray *layers = [NSMutableArray array];
    NSMutableDictionary *sourceLayers = [NSMutableDictionary dictionary];
    layersByUUID = [NSMutableDictionary dictionary];
    NSMutableDictionary *layersByName = [NSMutableDictionary dictionary];
    int which = 0;
    for (NSDictionary *layerStyleIter in layerStyles)
    {
        NSDictionary *layerStyle = layerStyleIter;
        if (filterBlock) {
            NSMutableDictionary *layerStyleMod = [NSMutableDictionary dictionaryWithDictionary:layerStyle];
            if (!(filterBlock(layerStyleMod)))
                continue;
            layerStyle = layerStyleMod;
        }
        MaplyMapboxVectorStyleLayer *layer = [MaplyMapboxVectorStyleLayer VectorStyleLayer:self JSON:layerStyle drawPriority:(1*which + settings.baseDrawPriority)];
        if (layer)
        {
            [layers addObject:layer];
            layersByUUID[@(layer.uuid)] = layer;
            layersByName[layer.ident] = layer;
            if (layer.sourceLayer)
            {
                NSMutableArray *sourceEntry = sourceLayers[layer.sourceLayer];
                if (!sourceEntry)
                    sourceEntry = [NSMutableArray array];
                [sourceEntry addObject:layer];
                sourceLayers[layer.sourceLayer] = sourceEntry;
            }
        }
        
        which++;
    }
    _layers = layers;
    _sources = sources;
    _layersBySource = sourceLayers;
    _layersByName = layersByName;
    
    return self;
}

@end

@implementation MaplyMapboxVectorStyleSource

- (id __nullable)initWithName:(NSString *)name styleEntry:(NSDictionary * __nonnull)styleEntry styleSet:(MapboxVectorStyleSet * __nonnull)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)viewC
{
    self = [super init];
    
    _name = name;
    
    NSString *typeStr = styleEntry[@"type"];
    if ([typeStr isEqualToString:@"vector"]) {
        _type = MapboxSourceVector;
    } else if ([typeStr isEqualToString:@"raster"]) {
        _type = MapboxSourceRaster;
    } else {
        NSLog(@"Unsupport source type %@",typeStr);
        return nil;
    }
    
    _url = styleEntry[@"url"];
    _tileSpec = styleEntry[@"tiles"];
    
    if (!_url && !_tileSpec) {
        NSLog(@"Expecting either URL or tileSpec in source %@",_name);
    }
    
    return self;
}

@end
