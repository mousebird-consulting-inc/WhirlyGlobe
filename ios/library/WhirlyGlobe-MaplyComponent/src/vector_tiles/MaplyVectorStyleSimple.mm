/*  MaplyVectorStyleSimple.m
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 3/15/16.
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

#import "vector_styles/MaplyVectorStyleSimple.h"
#import "visual_objects/MaplyScreenLabel.h"
#import "NSDictionary+Stuff.h"

@implementation MaplyVectorStyleSimpleGenerator
{
    NSMutableDictionary *stylesByUUID;
    NSMutableDictionary *stylesByLayerName;
    MaplyVectorStyleSimplePolygon *polyStyle;
    MaplyVectorStyleSimplePoint *pointStyle;
    MaplyVectorStyleSimpleLinear *linStyle;
    int uuidCount;
}

- (id)initWithViewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    stylesByUUID = [NSMutableDictionary dictionary];
    stylesByLayerName = [NSMutableDictionary dictionary];
    
    return self;
}

- (nullable NSArray *)stylesForFeatureWithAttributes:(NSDictionary *__nonnull)attributes
                                              onTile:(MaplyTileID)tileID
                                             inLayer:(NSString *__nonnull)layer
                                               viewC:(NSObject<MaplyRenderControllerProtocol> *__nonnull)viewC
{
    MaplyVectorStyleSimple *style = nil;
    
    // Look for existing layer
    @synchronized (self) {
        style = stylesByLayerName[layer];
        if (style)
            return @[style];
    }
    const int layer_order = (int)[attributes[@"layer_order"] integerValue];
    
    const int geomType = (int)[attributes[@"geometry_type"] integerValue];
    switch (geomType)
    {
        case GeomTypePoint:
            style = [[MaplyVectorStyleSimplePoint alloc] initWithGen:self viewC:viewC];
            style.drawPriority = kMaplyLabelDrawPriorityDefault+layer_order;
            break;
        case GeomTypeLineString:
            style = [[MaplyVectorStyleSimpleLinear alloc] initWithGen:self viewC:viewC];
            style.drawPriority = kMaplyVectorDrawPriorityDefault+1000+layer_order;
            break;
        case GeomTypePolygon:
            style = [[MaplyVectorStyleSimplePolygon alloc] initWithGen:self viewC:viewC];
            style.drawPriority = kMaplyVectorDrawPriorityDefault+layer_order;
            break;
        default:
            break;
    }
    
    @synchronized (self) {
        if (style) {
            stylesByUUID[@(style.uuid)] = style;
            stylesByLayerName[layer] = style;
        }
    }

    return style ? @[style] : @[];
}

// We'll display all layers
- (BOOL)layerShouldDisplay:(NSString *__nonnull)layer tile:(MaplyTileID)tileID
{
    return true;
}

- (nullable MaplyVectorTileStyle *)styleForUUID:(long long)uuid viewC:(NSObject<MaplyRenderControllerProtocol> *__nonnull)viewC
{
    return stylesByUUID[@(uuid)];
}

- (nullable NSObject<MaplyVectorStyle> *)backgroundStyleViewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    return nil;
}

- (NSArray * _Nonnull)allStyles
{
    return [stylesByUUID allValues];
}


- (long long)generateID
{
    uuidCount = uuidCount + 1;
    return uuidCount;
}

@end

@implementation MaplyVectorStyleSimple

- (id)initWithGen:(MaplyVectorStyleSimpleGenerator *)gen viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super init];
    _viewC = viewC;

    _uuid = [gen generateID];
    
    return self;
}

- (NSString *)getCategory
{
    return nil;
}

- (void)buildObjects:(NSArray * _Nonnull)vecObjs
             forTile:(MaplyVectorTileData *)tileInfo
               viewC:(NSObject<MaplyRenderControllerProtocol> * _Nonnull)viewC
                desc:(NSDictionary * _Nullable)desc
{
}

/// Construct objects related to this style based on the input data.
- (void)buildObjects:(NSArray * _Nonnull)vecObjs
             forTile:(MaplyVectorTileData * __nonnull)tileData
               viewC:(NSObject<MaplyRenderControllerProtocol> * _Nonnull)viewC
                desc:(NSDictionary * _Nullable)desc
            cancelFn:(bool(^__nullable)(void))cancelFn
{
}

@end

@implementation MaplyVectorStyleSimplePolygon

- (id)initWithGen:(MaplyVectorStyleSimpleGenerator *)gen viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super initWithGen:gen viewC:viewC];
    float red = drand48()/2.0;
    float green = drand48()/2.0;
    float blue = 0.0;
    _color = [UIColor colorWithRed:red+0.5 green:green+0.5 blue:blue+0.5 alpha:0.5];

    return self;
}

- (void)buildObjects:(NSArray * _Nonnull)vecObjs
             forTile:(MaplyVectorTileData *)tileInfo
               viewC:(NSObject<MaplyRenderControllerProtocol> * _Nonnull)viewC
                desc:(NSDictionary * _Nullable)extraDesc
{
    [self buildObjects:vecObjs forTile:tileInfo viewC:viewC desc:extraDesc cancelFn:nil];
}

/// Construct objects related to this style based on the input data.
- (void)buildObjects:(NSArray * _Nonnull)vecObjs
             forTile:(MaplyVectorTileData * __nonnull)tileInfo
               viewC:(NSObject<MaplyRenderControllerProtocol> * _Nonnull)viewC
                desc:(NSDictionary * _Nullable)extraDesc
            cancelFn:(bool(^__nullable)(void))cancelFn
{
    NSMutableArray *tessObjs = [NSMutableArray array];
    for (MaplyVectorObject *vecObj in vecObjs)
    {
        MaplyVectorObject *tessObj = [vecObj tesselate];
        if (tessObj)
        {
            [tessObjs addObject:tessObj];
        }
    }

    NSDictionary *desc = @{
        kMaplyColor: _color,
        kMaplyFilled: @(YES),
        kMaplyDrawPriority: @(self.drawPriority)
    };
    desc = [desc dictionaryByMergingWith:extraDesc];

    MaplyComponentObject *compObj = [super.viewC addVectors:tessObjs desc:desc mode:MaplyThreadCurrent];
    if (compObj)
    {
        [tileInfo addComponentObject:compObj];
    }
}

@end

@implementation MaplyVectorStyleSimplePoint

- (id)initWithGen:(MaplyVectorStyleSimpleGenerator *)gen viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super initWithGen:gen viewC:viewC];
    _font = [UIFont systemFontOfSize:24.0];
    
    return self;
}

- (void)buildObjects:(NSArray * _Nonnull)vecObjs
             forTile:(MaplyVectorTileData *)tileInfo
               viewC:(NSObject<MaplyRenderControllerProtocol> * _Nonnull)viewC
                desc:(NSDictionary * _Nullable)extraDesc
{
    [self buildObjects:vecObjs forTile:tileInfo viewC:viewC desc:extraDesc cancelFn:nil];
}

/// Construct objects related to this style based on the input data.
- (void)buildObjects:(NSArray * _Nonnull)vecObjs
             forTile:(MaplyVectorTileData * __nonnull)tileInfo
               viewC:(NSObject<MaplyRenderControllerProtocol> * _Nonnull)viewC
                desc:(NSDictionary * _Nullable)extraDesc
            cancelFn:(bool(^__nullable)(void))cancelFn
{
    NSMutableArray *labels = [NSMutableArray array];
    
    for (MaplyVectorObject *vecObj in vecObjs)
    {
        NSArray *points = [vecObj splitVectors];
        for (MaplyVectorObject *point in points)
        {
            NSString *name = point.attributes[@"name"];
            MaplyCoordinate center = [point center];
            
            MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
            label.text = (name ? name : @".");
            label.loc = center;
            
            [labels addObject:label];
        }
    }
    
    if (labels.count == 0)
        return;
    
    NSDictionary *desc = @{
        kMaplyTextColor: [UIColor blackColor],
        kMaplyFont: _font
    };
    desc = [desc dictionaryByMergingWith:extraDesc];

    MaplyComponentObject *compObj = [self.viewC addScreenLabels:labels
                                                           desc:desc
                                                           mode:MaplyThreadCurrent];
    
    if (compObj)
        [tileInfo addComponentObject:compObj];
}

@end

@implementation MaplyVectorStyleSimpleLinear

- (id)initWithGen:(MaplyVectorStyleSimpleGenerator *)gen viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super initWithGen:gen viewC:viewC];
    float red = drand48()/2.0;
    float green = drand48()/2.0;
    float blue = drand48()/2.0;
    _color = [UIColor colorWithRed:red+0.5 green:green+0.5 blue:blue+0.5 alpha:1.0];
    
    return self;
}

- (void)buildObjects:(NSArray * _Nonnull)vecObjs
             forTile:(MaplyVectorTileData *)tileInfo
               viewC:(NSObject<MaplyRenderControllerProtocol> * _Nonnull)viewC
                desc:(NSDictionary * _Nullable)extraDesc
{
    [self buildObjects:vecObjs forTile:tileInfo viewC:viewC desc:extraDesc cancelFn:nil];
}

/// Construct objects related to this style based on the input data.
- (void)buildObjects:(NSArray * _Nonnull)vecObjs
             forTile:(MaplyVectorTileData * __nonnull)tileInfo
               viewC:(NSObject<MaplyRenderControllerProtocol> * _Nonnull)viewC
                desc:(NSDictionary * _Nullable)extraDesc
            cancelFn:(bool(^__nullable)(void))cancelFn
{
    NSDictionary *desc = @{
        kMaplyColor: _color,
        kMaplyDrawPriority: @(self.drawPriority),
        kMaplyFilled: @(NO),
        kMaplyVecWidth: @(4.0)
    };
    desc = [desc dictionaryByMergingWith:extraDesc];

    MaplyComponentObject *compObj = [super.viewC addVectors:vecObjs desc:desc mode:MaplyThreadCurrent];
    if (compObj)
    {
        [tileInfo addComponentObject:compObj];
    }
}



@end
