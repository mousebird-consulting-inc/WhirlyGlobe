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

#import "vector_styles/MapboxVectorStyleSet.h"
#import "vector_styles/MapboxVectorStyleBackground.h"
#import "vector_styles/MapboxVectorStyleFill.h"
#import "vector_styles/MapboxVectorStyleLine.h"
#import "vector_styles/MapboxVectorStyleRaster.h"
#import "vector_styles/MapboxVectorStyleSymbol.h"
#import "vector_styles/MapboxVectorStyleCircle.h"
#import <map>

/// @brief A single zoom and value
@interface MaplyVectorFunctionStop : NSObject

/// @brief Zoom level this applies to
@property (nonatomic) double zoom;

/// @brief Value at that zoom level
@property (nonatomic) double val;

/// @brief Could also just be a color
@property (nonatomic,nullable,strong) UIColor *color;

@end

/// @brief These are used to control simple values based on zoom level
@interface MaplyVectorFunctionStops : NSObject

/// @brief Array of function stops as they apply to value.  These are MaplyVectorFunctionStop objects.
@property (nonatomic,strong,nullable) NSArray *stops;

/// @brief Used in exponential calculation
@property (nonatomic,assign) double base;

/// @brief Parse out of a JSON array
- (id _Nullable)initWithArray:(NSArray * __nonnull)dataArray styleSet:(MapboxVectorStyleSet * __nonnull)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)viewC;

/// @brief Calculate a value given the zoom level
- (double)valueForZoom:(double)zoom;

/// @brief This version returns colors, assuming we're dealing with colors
- (UIColor * _Nonnull)colorForZoom:(int)zoom;

/// @brief Returns the minimum value
- (double)minValue;

/// @brief Returns the maximum value
- (double)maxValue;

@end

@implementation MapboxTransDouble
{
@public
    double val;
    MaplyVectorFunctionStops *stops;
}

- (id)initWithDouble:(double)value
{
    self = [super init];
    val = value;
    
    return self;
}

- (id)initWithStops:(MaplyVectorFunctionStops *)inStops
{
    self = [super init];
    stops = inStops;
    
    return self;
}

- (double)valForZoom:(double)zoom
{
    if (stops) {
        return [stops valueForZoom:zoom];
    } else
        return val;
}

- (double)minVal
{
    if (stops) {
        return [stops minValue];
    } else
        return val;
}

- (double)maxVal
{
    if (stops) {
        return [stops maxValue];
    } else
        return val;
}

@end

@implementation MapboxTransColor
{
@public
    UIColor *color;
    MaplyVectorFunctionStops *stops;
}

- (id)initWithColor:(UIColor *)inColor
{
    self = [super init];
    color = inColor;
    
    return self;
}

- (id)initWithStops:(MaplyVectorFunctionStops *)inStops
{
    self = [super init];
    stops = inStops;
    
    return self;
}
 
- (UIColor *)colorForZoom:(double)zoom
{
    if (stops) {
        return [stops colorForZoom:(int)zoom];
    } else
        return color;
}

@end

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

- (long long)generateID
{
    return currentID++;
}

- (NSArray*)stylesForFeatureWithAttributes:(NSDictionary*)attributes
                                    onTile:(MaplyTileID)tileID
                                   inLayer:(NSString*)sourceLayer
                                     viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    NSArray *layersToRun = _layersBySource[sourceLayer];
    if (!layersToRun)
        return nil;
    NSMutableArray *passedLayers = [NSMutableArray array];
    for (MaplyMapboxVectorStyleLayer *layer in layersToRun)
    {
        if (!layer.filter || [layer.filter testFeature:attributes tile:tileID viewC:viewC])
            [passedLayers addObject:layer];
    }
    
    return passedLayers;
}

- (BOOL)layerShouldDisplay:(NSString*)sourceLayer tile:(MaplyTileID)tileID
{
    NSArray *layersToRun = _layersBySource[sourceLayer];
    
    return (layersToRun.count != 0);
}

- (MaplyVectorTileStyle*)styleForUUID:(long long)uuid viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    return layersByUUID[@(uuid)];
}

- (NSArray * _Nonnull)allStyles {
    return [layersByUUID allValues];
}


- (id)constantSubstitution:(id)thing forField:(NSString *)field
{
    // Look for a constant substitution
    if ([thing isKindOfClass:[NSString class]])
    {
        NSString *stringThing = thing;
        // Note: This just handles simple ones with full substitution
        if ([stringThing characterAtIndex:0] == '@')
        {
            id constant = _constants[stringThing];
            if (constant)
                thing = constant;
            else {
                NSLog(@"Failed to substitute constant %@ for field %@",stringThing,field);
                return thing;
            }
        }
    }
    
    return thing;
}

- (int)intValue:(NSString *)name dict:(NSDictionary *)dict defVal:(int)defVal
{
    id thing = dict[name];
    if (!thing)
        return defVal;
    
    thing = [self constantSubstitution:thing forField:name];
    
    if ([thing respondsToSelector:@selector(integerValue)])
        return [thing integerValue];
    
    NSLog(@"Expected integer for %@ but got something else",name);
    return defVal;
}

- (double)doubleValue:(id)thing defVal:(double)defVal
{
    thing = [self constantSubstitution:thing forField:nil];
    
    if ([thing respondsToSelector:@selector(doubleValue)])
        return [thing doubleValue];
    
    NSLog(@"Expected double but got something else (%@)",thing);
    return defVal;
}

- (double)doubleValue:(NSString *)name dict:(NSDictionary *)dict defVal:(double)defVal
{
    id thing = dict[name];
    if (!thing)
        return defVal;
    
    return [self doubleValue:thing defVal:defVal];
}
    
- (bool)boolValue:(NSString *)name dict:(NSDictionary *)dict onValue:(NSString *)onString defVal:(bool)defVal
{
    id thing = dict[name];
    if (!thing)
        return defVal;
    
    if ([thing isKindOfClass:[NSString class]]) {
        return [thing isEqualToString:onString];
    } else
        return defVal;
}

- (NSString *)stringValue:(NSString *)name dict:(NSDictionary *)dict defVal:(NSString *)defVal
{
    id thing = dict[name];
    if (!thing)
        return defVal;
    
    thing = [self constantSubstitution:thing forField:name];
    
    if ([thing isKindOfClass:[NSString class]])
        return thing;
    if ([thing respondsToSelector:@selector(stringValue)])
        return [thing stringValue];
    
    NSLog(@"Expected string for %@ but got something else",name);
    return defVal;
}

- (NSArray *)arrayValue:(NSString *)name dict:(NSDictionary *)dict defVal:(NSArray *)defVal
{
    id thing = dict[name];
    if (!thing)
        return defVal;
    
    thing = [self constantSubstitution:thing forField:name];
    
    if ([thing isKindOfClass:[NSArray class]])
        return thing;
    
    NSLog(@"Expected array for %@ but got something else",name);
    return defVal;
}

- (MaplyVectorFunctionStops *)stopsValue:(id)entry defVal:(id)defEntry
{
    entry = [self constantSubstitution:entry forField:nil];
    
    NSNumber *base = nil;
    if ([entry isKindOfClass:[NSDictionary class]])
    {
        base = ((NSDictionary *)entry)[@"base"];
        entry = ((NSDictionary *)entry)[@"stops"];
    }
    
    if (!entry)
    {
        NSLog(@"Expecting key word 'stops' in entry %@",defEntry);
        return defEntry;
    }
    
    MaplyVectorFunctionStops *stops = [[MaplyVectorFunctionStops alloc] initWithArray:entry styleSet:self viewC:self.viewC];
    if (stops)
    {
        if ([base isKindOfClass:[NSNumber class]])
            stops.base = [base doubleValue];
        return stops;
    }
    return defEntry;
}

- (MapboxTransDouble *__nullable)transDouble:(NSString * __nonnull)name entry:(NSDictionary *__nonnull)entry defVal:(double)defVal
{
    // They pass in the whole dictionary and let us look the field up
    id theEntry = entry[name];
    if (!theEntry)
        return [[MapboxTransDouble alloc] initWithDouble:defVal];
    theEntry = [self constantSubstitution:theEntry forField:nil];

    // This is probably stops
    if ([theEntry isKindOfClass:[NSDictionary class]]) {
        MaplyVectorFunctionStops *stops = [self stopsValue:theEntry defVal:name];
        if (stops) {
            return [[MapboxTransDouble alloc] initWithStops:stops];
        } else {
            NSLog(@"Expecting key word 'stops' in entry %@",name);
        }
    } else if ([theEntry isKindOfClass:[NSNumber class]]) {
        return [[MapboxTransDouble alloc] initWithDouble:[theEntry doubleValue]];
    } else {
        NSLog(@"Unexpected type found in entry %@. Was expecting a double.",name);
    }
    
    return nil;
}

- (MapboxTransColor *__nullable)transColor:(NSString *__nonnull)name entry:(NSDictionary *__nonnull)entry defVal:(UIColor * __nullable)defVal
{
    // They pass in the whole dictionary and let us look the field up
    id theEntry = entry[name];
    if (!theEntry) {
        if (defVal)
            return [[MapboxTransColor alloc] initWithColor:defVal];
        return nil;
    }
    theEntry = [self constantSubstitution:theEntry forField:nil];

    // This is probably stops
    if ([theEntry isKindOfClass:[NSDictionary class]]) {
        MaplyVectorFunctionStops *stops = [self stopsValue:theEntry defVal:name];
        if (stops) {
            return [[MapboxTransColor alloc] initWithStops:stops];
        } else {
            NSLog(@"Expecting key word 'stops' in entry %@",name);
        }
    } else if ([theEntry isKindOfClass:[NSString class]]) {
        UIColor *color = [self colorValue:name val:theEntry dict:nil defVal:defVal multiplyAlpha:false];
        if (color)
            return [[MapboxTransColor alloc] initWithColor:color];
        else {
            NSLog(@"Unexpected type found in entry %@. Was expecting a color.",name);
        }
    } else {
        NSLog(@"Unexpected type found in entry %@. Was expecting a color.",name);
    }
    
    return nil;
}

- (UIColor *__nullable)resolveColor:(MapboxTransColor * __nullable)color opacity:(MapboxTransDouble * __nullable)opacity forZoom:(double)zoom mode:(MBResolveColorType)resolveMode
{
    // No color means no color
    if (!color)
        return nil;
    
    UIColor *thisColor = [color colorForZoom:zoom];

    // No opacity, means full opacity
    if (!opacity)
        return thisColor;
    
    double thisOpacity = [opacity valForZoom:zoom];
    
    CGFloat red,green,blue,alpha;
    [thisColor getRed:&red green:&green blue:&blue alpha:&alpha];
    switch (resolveMode)
    {
        case MBResolveColorOpacityMultiply:
            return [UIColor colorWithRed:red*thisOpacity green:green*thisOpacity blue:blue*thisOpacity alpha:alpha*thisOpacity];
            break;
        case MBResolveColorOpacityReplaceAlpha:
            return [UIColor colorWithRed:red green:green blue:blue alpha:thisOpacity];
            break;
    }
}

- (UIColor *)color:(UIColor *)color withOpacity:(double)opacity
{
    CGFloat red,green,blue,alpha;
    [color getRed:&red green:&green blue:&blue alpha:&alpha];
    return [UIColor colorWithRed:red*opacity green:green*opacity blue:blue*opacity alpha:alpha*opacity];
}

- (UIColor *)colorValue:(NSString *)name val:(id)val dict:(NSDictionary *)dict defVal:(UIColor *)defVal multiplyAlpha:(bool)multiplyAlpha
{
    id thing = nil;
    if (dict)
        thing = dict[name];
    else
        thing = val;
    if (!thing)
        return defVal;
    
    thing = [self constantSubstitution:thing forField:name];
    
    if (![thing isKindOfClass:[NSString class]])
    {
        NSLog(@"Expecting a string for color (%@)",name);
        return defVal;
    }
    
    NSString *str = thing;
    if ([str length] == 0)
    {
        NSLog(@"Expecting non-empty string for color (%@)",name);
        return defVal;
    }
    // Hex string
    if ([str characterAtIndex:0] == '#')
    {
        // Hex string
        NSScanner *scanner = [NSScanner scannerWithString:str];
        [scanner setScanLocation:1];
        unsigned int iVal;
        if (![scanner scanHexInt:&iVal])
        {
            NSLog(@"Invalid hex value (%@) in color (%@)",str,name);
            return defVal;
        }

        int red,green,blue;
        int alpha = 255;
        if ([str length] == 4)
        {
            red = (iVal >> 8) & 0xf;  red |= red << 4;
            green = (iVal >> 4) & 0xf;  green |= green << 4;
            blue = iVal & 0xf;  blue |= blue << 4;
        } else if ([str length] > 7) {
            red = (iVal >> 24) & 0xff;
            green = (iVal >> 16) & 0xff;
            blue = (iVal >> 8) & 0xff;
            alpha = iVal & 0xff;
        } else {
            red = (iVal >> 16) & 0xff;
            green = (iVal >> 8) & 0xff;
            blue = iVal & 0xff;
        }
        return [UIColor colorWithRed:(double)red/255.0 green:(double)green/255.0 blue:(double)blue/255.0 alpha:alpha/255.0];
    } else if ([str rangeOfString:@"rgb("].location == 0)
    {
        NSScanner *scanner = [NSScanner scannerWithString:str];
        NSMutableCharacterSet *skipSet = [[NSMutableCharacterSet alloc] init];
        [skipSet addCharactersInString:@"(), "];
        [scanner setCharactersToBeSkipped:skipSet];
        [scanner setScanLocation:4];
        int red,green,blue;
        [scanner scanInt:&red];
        [scanner scanInt:&green];
        [scanner scanInt:&blue];
        
        return [UIColor colorWithRed:red/255.0 green:green/255.0 blue:blue/255.0 alpha:1.0];
    } else if ([str rangeOfString:@"rgba("].location == 0)
    {
        NSScanner *scanner = [NSScanner scannerWithString:str];
        NSMutableCharacterSet *skipSet = [[NSMutableCharacterSet alloc] init];
        [skipSet addCharactersInString:@"(), "];
        [scanner setCharactersToBeSkipped:skipSet];
        [scanner setScanLocation:5];
        int red,green,blue;
        [scanner scanInt:&red];
        [scanner scanInt:&green];
        [scanner scanInt:&blue];
        float alpha;
        [scanner scanFloat:&alpha];

        if (multiplyAlpha)
            return [UIColor colorWithRed:red/255.0*alpha green:green/255.0*alpha blue:blue/255.0*alpha alpha:alpha];
        else
            return [UIColor colorWithRed:red/255.0 green:green/255.0 blue:blue/255.0 alpha:alpha];
    } else if ([str rangeOfString:@"hsl("].location == 0)
    {
        NSScanner *scanner = [NSScanner scannerWithString:str];
        NSMutableCharacterSet *skipSet = [[NSMutableCharacterSet alloc] init];
        [skipSet addCharactersInString:@"(),% "];
        [scanner setCharactersToBeSkipped:skipSet];
        [scanner setScanLocation:4];
        int hue,sat,light;
        [scanner scanInt:&hue];
        [scanner scanInt:&sat];
        [scanner scanInt:&light];
        float newLight = light / 100.0;
        float newSat = sat / 100.0;
        newSat = newSat * (newLight < 0.5 ? newLight : 1.0-newLight);
        
        return [UIColor colorWithHue:hue/360.0 saturation:2.0*newSat/(newLight+newSat) brightness:newLight+newSat alpha:1.0];
    } else if ([str rangeOfString:@"hsla("].location == 0)
    {
        NSScanner *scanner = [NSScanner scannerWithString:str];
        NSMutableCharacterSet *skipSet = [[NSMutableCharacterSet alloc] init];
        [skipSet addCharactersInString:@"(),% "];
        [scanner setCharactersToBeSkipped:skipSet];
        [scanner setScanLocation:4];
        int hue,sat,light;
        float alpha;
        [scanner scanInt:&hue];
        [scanner scanInt:&sat];
        [scanner scanInt:&light];
        [scanner scanFloat:&alpha];
        float newLight = light / 100.0;
        float newSat = sat / 100.0;
        newSat = newSat * (newLight < 0.5 ? newLight : 1.0-newLight);
        
        return [UIColor colorWithHue:hue/360.0 saturation:2.0*newSat/(newLight+newSat) brightness:newLight+newSat alpha:alpha];
    }
    
    
    NSLog(@"Didn't recognize format of color (%@)",name);
    return defVal;
}

- (NSUInteger)enumValue:(NSString *)name options:(NSArray *)options defVal:(NSUInteger)defVal
{
    if (!name)
        return defVal;
    
    if (![name isKindOfClass:[NSString class]])
    {
        NSLog(@"Expecting string for enumerated type.");
        return defVal;
    }

    int which = 0;
    for (NSString *val in options)
    {
        if ([val isEqualToString:name])
            return which;
        which++;
    }
    
    NSLog(@"Found unexpected value (%@) in enumerated type",name);
    return defVal;
}

- (void)unsupportedCheck:(NSString *)field in:(NSString *)what styleEntry:(NSDictionary *)styleEntry
{
    if (styleEntry[field])
        NSLog(@"Found unsupported field (%@) for (%@)",field,what);
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

@implementation MaplyMapboxVectorStyleLayer
{
    NSString *category;
}

+ (id)VectorStyleLayer:(MapboxVectorStyleSet *)styleSet JSON:(NSDictionary *)layerDict drawPriority:(int)drawPriority
{
    MaplyMapboxVectorStyleLayer *layer = nil;
    MaplyMapboxVectorStyleLayer *refLayer = nil;
    
    // Look for the layer with that name
    NSString *refLayerName = layerDict[@"ref"];
    if (refLayer)
    {
        if (![refLayerName isKindOfClass:[NSString class]])
        {
            NSLog(@"Was expecting string for ref in layer");
            return nil;
        }
        
        refLayer = styleSet.layersByName[refLayerName];
        if (!refLayer)
        {
            NSLog(@"Didn't find layer named (%@)",refLayerName);
            return nil;
        }
    }

    NSString *type = layerDict[@"type"];
    if (type && ![type isKindOfClass:[NSString class]])
    {
        NSLog(@"Expecting string type for layer");
        return nil;
    }
    if ([type isEqualToString:@"fill"])
    {
        MapboxVectorLayerFill *fillLayer = [[MapboxVectorLayerFill alloc] initWithStyleEntry:layerDict parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:styleSet.viewC];
        layer = fillLayer;
    } else if ([type isEqualToString:@"line"])
    {
        MapboxVectorLayerLine *lineLayer = [[MapboxVectorLayerLine alloc] initWithStyleEntry:layerDict parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:styleSet.viewC];
        layer = lineLayer;
    } else if ([type isEqualToString:@"symbol"])
    {
        MapboxVectorLayerSymbol *symbolLayer = [[MapboxVectorLayerSymbol alloc] initWithStyleEntry:layerDict parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:styleSet.viewC];
        layer = symbolLayer;
    } else if ([type isEqualToString:@"circle"])
    {
        MapboxVectorLayerCircle *circleLayer = [[MapboxVectorLayerCircle alloc] initWithStyleEntry:layerDict parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:styleSet.viewC];
        layer = circleLayer;
    } else if ([type isEqualToString:@"raster"])
    {
        MapboxVectorLayerRaster *rasterLayer = [[MapboxVectorLayerRaster alloc] initWithStyleEntry:layerDict parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:styleSet.viewC];
        layer = rasterLayer;
    } else if ([type isEqualToString:@"background"])
    {
        MapboxVectorLayerBackground *backLayer = [[MapboxVectorLayerBackground alloc] initWithStyleEntry:layerDict parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:styleSet.viewC];
        layer = backLayer;
    }
    if (layerDict[@"filter"])
    {
        layer.filter = [[MapboxVectorFilter alloc] initWithArray:[styleSet arrayValue:@"filter" dict:layerDict defVal:nil] styleSet:styleSet viewC:styleSet.viewC];
        if (!layer.filter)
        {
            NSLog(@"MapboxStyleSet: Failed to parse filter for layer %@",layerDict[@"id"]);
        }
    }
    
    layer.visible = [styleSet boolValue:@"visibility" dict:layerDict[@"layout"] onValue:@"visible" defVal:true];
    layer.selectable = styleSet.tileStyleSettings.selectable;
    
    if (layerDict[@"metadata"])
    {
        NSDictionary *metadataDict = layerDict[@"metadata"];
        if ([metadataDict isKindOfClass:[NSDictionary class]])
            layer.metadata = metadataDict;
    }
    
    return layer;
}

- (id)initWithStyleEntry:(NSDictionary *)layerDict parent:(MaplyMapboxVectorStyleLayer *)refLayer styleSet:(MapboxVectorStyleSet *)styleSet drawPriority:(int)drawPriority viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    self.styleSet = styleSet;
    self.drawPriorityPerLevel = styleSet.tileStyleSettings.drawPriorityPerLevel;
    self.drawPriority = drawPriority;
    self.uuid = [styleSet generateID];
    
    _minzoom = -1;
    _maxzoom = -1;
    
    self.ident = layerDict[@"id"];
    self.source = [styleSet stringValue:@"source" dict:layerDict defVal:refLayer.source];
    self.sourceLayer = [styleSet stringValue:@"source-layer" dict:layerDict defVal:refLayer.sourceLayer];
    self.minzoom = [styleSet intValue:@"minzoom" dict:layerDict defVal:refLayer.minzoom];
    self.maxzoom = [styleSet intValue:@"maxzoom" dict:layerDict defVal:refLayer.maxzoom];
    category = [styleSet stringValue:@"wkcategory" dict:layerDict defVal:nil];
    
    return self;
}

- (NSString *)getCategory
{
    return category;
}

- (void)buildObjects:(NSArray *)vecObjs forTile:(MaplyVectorTileData *)tileInfo viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
}

@end

@implementation MapboxVectorFilter

- (id)initWithArray:(NSArray *)filterArray styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    if (![filterArray isKindOfClass:[NSArray class]])
    {
        NSLog(@"Expecting array for filter");
        return nil;
    }
    if ([filterArray count] < 1)
    {
        NSLog(@"Expecting at least one entry in filter");
        return nil;
    }
    
    self = [super init];
    if (!self)
        return nil;
    
    _geomType = MBGeomNone;
    
    _filterType = (MapboxVectorFilterType)[styleSet enumValue:[filterArray objectAtIndex:0]
                options:@[@"==",@"!=",@">",@">=",@"<",@"<=",@"in",@"!in",@"has",@"!has",@"all",@"any",@"none"]
                 defVal:MBFilterNone];
    
    // Filter with two arguments
    if (_filterType == MBFilterNone)
    {
        // That's easy
    } else if (_filterType <= MBFilterLessThanEqual)
    {
        // Filters with two arguments
        if ([filterArray count] < 3)
        {
            NSLog(@"Expecting three arugments for filter of type (%@)",[filterArray objectAtIndex:0]);
            return nil;
        }
        
        // Attribute name can be name or geometry type
        _attrName = [filterArray objectAtIndex:1];
        if ([_attrName isEqualToString:@"$type"])
        {
            _geomType = (MapboxVectorGeometryType)[styleSet enumValue:[filterArray objectAtIndex:2] options:@[@"Point",@"LineString",@"Polygon"] defVal:MBGeomNone];
            if (_geomType == MBGeomNone)
            {
                NSLog(@"Unrecognized geometry type (%@) in filter",_attrName);
                return nil;
            }
        }
        
        _attrVal = [styleSet constantSubstitution:[filterArray objectAtIndex:2] forField:@"Filter attribute value"];
        if (!_attrVal)
            return nil;
    } else if (_filterType <= MBFilterNotIn)
    {
        // Filters with inclusion
        NSMutableArray *inclVals = [NSMutableArray array];
        if ([filterArray count] < 3)
        {
            NSLog(@"Expecting at least three arguments for filter of type (%@)",[filterArray objectAtIndex:0]);
            return nil;
        }
        _attrName = [filterArray objectAtIndex:1];
        for (unsigned int ii=2;ii<[filterArray count];ii++)
        {
            id val = [filterArray objectAtIndex:ii];
            val = [styleSet constantSubstitution:val forField:@"Filter attribute value"];
            if (!val)
                return nil;
            [inclVals addObject:val];
        }
        _attrVals = inclVals;
    } else if (_filterType <= MBFilterNotHas)
    {
        // Filters with existence
        if ([filterArray count] < 2)
        {
            NSLog(@"Expecting at least two arguments for filter of type (%@)",[filterArray objectAtIndex:0]);
            return nil;
        }
        _attrName = [filterArray objectAtIndex:1];
    } else if (_filterType == MBFilterAll || _filterType == MBFilterAny)
    {
        // Any and all have subfilters
        NSMutableArray *subFilters = [NSMutableArray array];
        for (unsigned int ii=1;ii<[filterArray count];ii++)
        {
            id val = [filterArray objectAtIndex:ii];
            MapboxVectorFilter *subFilter = [[MapboxVectorFilter alloc] initWithArray:val styleSet:styleSet viewC:viewC];
            if (!subFilter)
                return nil;
            [subFilters addObject:subFilter];
        }
        _subFilters = subFilters;
    }
    
    return self;
}

- (bool)testFeature:(NSDictionary *)attrs tile:(MaplyTileID)tileID viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    bool ret = true;
    
    // Compare geometry type
    if (_geomType != MBGeomNone)
    {
        int attrGeomType = [attrs[@"geometry_type"] integerValue] - 1;
        switch (_filterType)
        {
            case MBFilterEqual:
                ret = attrGeomType == _geomType;
                break;
            case MBFilterNotEqual:
                ret = attrGeomType != _geomType;
                break;
            default:
                break;
        }
    } else if (_filterType == MBFilterAll || _filterType == MBFilterAny)
    {
        // Run each of the rules as either AND or OR
        if (_filterType == MBFilterAll)
        {
            for (MapboxVectorFilter *filter in _subFilters)
            {
                ret &= [filter testFeature:attrs tile:tileID viewC:viewC];
                if (!ret)
                    break;
            }
        } else if (_filterType == MBFilterAny)
        {
            ret = false;
            for (MapboxVectorFilter *filter in _subFilters)
            {
                ret |= [filter testFeature:attrs tile:tileID viewC:viewC];
                if (ret)
                    break;
            }
        } else
            ret = false;
    } else if (_filterType == MBFilterIn || _filterType == MBFilterNotIn)
    {
        // Check for attribute value membership
        bool isIn = false;
        
        // Note: Not dealing with differing types well
        id featAttrVal = attrs[_attrName];
        if (featAttrVal)
        {
            for (id match in _attrVals)
            {
                if ([match isEqual:featAttrVal])
                {
                    isIn = true;
                    break;
                }
            }
        }
        
        ret = (_filterType == MBFilterIn ? isIn : !isIn);
    } else if (_filterType == MBFilterHas || _filterType == MBFilterNotHas)
    {
        // Check for attribute existence
        bool canHas = false;
        
        id featAttrVal = attrs[_attrName];
        if (featAttrVal)
            canHas = true;
        
        ret = (_filterType == MBFilterHas ? canHas : !canHas);
    } else {
        // Equality related operators
        id featAttrVal = attrs[_attrName];
        if (featAttrVal)
        {
            if ([featAttrVal isKindOfClass:[NSString class]])
            {
                switch (_filterType)
                {
                    case MBFilterEqual:
                        ret = [featAttrVal isEqualToString:_attrVal];
                        break;
                    case MBFilterNotEqual:
                        ret = ![featAttrVal isEqualToString:_attrVal];
                        break;
                    default:
                        // Note: Not expecting other comparisons to strings
                        break;
                }
            } else {
                NSNumber *featAttrNum = (NSNumber *)featAttrVal;
                NSNumber *attrNum = (NSNumber *)_attrVal;
                if ([featAttrNum isKindOfClass:[NSNumber class]])
                {
                    switch (_filterType)
                    {
                        case MBFilterEqual:
                            ret = [featAttrNum doubleValue] == [attrNum doubleValue];
                            break;
                        case MBFilterNotEqual:
                            ret = [featAttrNum doubleValue] != [attrNum doubleValue];
                            break;
                        case MBFilterGreaterThan:
                            ret = [featAttrNum doubleValue] > [attrNum doubleValue];
                            break;
                        case MBFilterGreaterThanEqual:
                            ret = [featAttrNum doubleValue] >= [attrNum doubleValue];
                            break;
                        case MBFilterLessThan:
                            ret = [featAttrNum doubleValue] < [attrNum doubleValue];
                            break;
                        case MBFilterLessThanEqual:
                            ret = [featAttrNum doubleValue] <= [attrNum doubleValue];
                            break;
                        default:
                            break;
                    }
                } else {
                    NSLog(@"MapboxVectorFilter: Found numeric comparison that doesn't use numbers.");
                }
            }
        } else {
            // No attribute means no pass
            ret = false;

            // A missing value and != is valid
            if (_filterType == MBFilterNotEqual)
                ret = true;
        }
    }
    
    return ret;
}

@end

@implementation MaplyVectorFunctionStop
@end

@implementation MaplyVectorFunctionStops

- (id)initWithArray:(NSArray *)dataArray styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    if (![dataArray isKindOfClass:[NSArray class]])
    {
        NSLog(@"Expected JSON array for function stops.");
        return nil;
    }
    if ([dataArray count] < 2)
    {
        NSLog(@"Expecting at least two arguments for function stops.");
        return nil;
    }
    
    NSMutableArray *stops = [NSMutableArray array];
    for (NSArray *stop in dataArray)
    {
        if (![stop isKindOfClass:[NSArray class]] || [stop count] != 2)
        {
            NSLog(@"Expecting two arguments in each entry for a function stop.");
            return nil;
        }
        
        MaplyVectorFunctionStop *fStop = [[MaplyVectorFunctionStop alloc] init];
        fStop.zoom = [[stop objectAtIndex:0] doubleValue];
        id valObj = [stop objectAtIndex:1];
        if ([valObj isKindOfClass:[NSNumber class]])
            fStop.val = [valObj doubleValue];
        else {
            // Maybe a color?
            fStop.color = [styleSet colorValue:nil val:valObj dict:nil defVal:nil multiplyAlpha:true];
        }
        [stops addObject:fStop];
    }
    
    self = [super init];
    if (!self)
        return nil;
    
    _base = 1.0;
    _stops = stops;
    
    return self;
}

- (double)valueForZoom:(double)zoom
{
    MaplyVectorFunctionStop *a = _stops[0],*b = nil;
    if (zoom <= a.zoom)
        return a.val;
    for (int which = 1;which < _stops.count; which++)
    {
        b = _stops[which];
        if (a.zoom <= zoom && zoom < b.zoom)
        {
            double ratio = 1.0;
            if (_base == 1.0) {
                ratio = (zoom-a.zoom)/(b.zoom-a.zoom);
            } else {
                double soFar = zoom-a.zoom;
                ratio = (pow(_base, soFar) - 1.0) / (pow(_base,b.zoom-a.zoom) - 1.0);
            }
            return ratio * (b.val-a.val) + a.val;
        }
        a = b;
    }
    
    return b.val;
}

- (UIColor *)colorForZoom:(int)zoom
{
    MaplyVectorFunctionStop *a = _stops[0],*b = nil;
    if (zoom <= a.zoom)
        return a.color;
    for (int which = 1;which < _stops.count; which++)
    {
        b = _stops[which];
        if (a.zoom <= zoom && zoom < b.zoom)
        {
            double ratio = 1.0;
            if (_base == 1.0) {
                ratio = (zoom-a.zoom)/(b.zoom-a.zoom);
            } else {
                double soFar = zoom-a.zoom;
                ratio = (pow(_base, soFar) - 1.0) / (pow(_base,b.zoom-a.zoom) - 1.0);
            }
            CGFloat ac[4],bc[4];
            [a.color getRed:&ac[0] green:&ac[1] blue:&ac[2] alpha:&ac[3]];
            [b.color getRed:&bc[0] green:&bc[1] blue:&bc[2] alpha:&bc[3]];
            CGFloat res[4];
            for (unsigned int ii=0;ii<4;ii++)
                res[ii] = ratio * (bc[ii]-ac[ii]) + ac[ii];
            return [UIColor colorWithRed:res[0] green:res[1] blue:res[2] alpha:res[3]];
        }
        a = b;
    }
    
    return b.color;
}

- (double)minValue
{
    double val = MAXFLOAT;
    
    for (MaplyVectorFunctionStop *stop in _stops) {
        val = MIN(val,stop.val);
    }
    
    return val;
}

- (double)maxValue
{
    double val = -MAXFLOAT;
    
    for (MaplyVectorFunctionStop *stop in _stops) {
        val = MAX(val,stop.val);
    }
    
    return val;
}


@end
