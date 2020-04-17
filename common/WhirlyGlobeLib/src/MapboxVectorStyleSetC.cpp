/*
*  MapboxVectorStyleSetC.h
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

#import "MapboxVectorStyleSetC.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{

MapboxVectorStyleSetImpl::MapboxVectorStyleSetImpl(Scene *inScene)
{
    scene = inScene;
    vecManage = (VectorManager *)scene->getManager(kWKVectorManager);
    wideVecManage = (WideVectorManager *)scene->getManager(kWKWideVectorManager);
    markerManage = (MarkerManager *)scene->getManager(kWKMarkerManager);
    compManage = (ComponentManager *)scene->getManager(kWKComponentManager);
}

MaplyVectorFunctionStop::MaplyVectorFunctionStop()
: zoom(-1.0), val(0.0)
{
}

bool MaplyVectorFunctionStops::parse(const std::vector<DictionaryEntryRef> &dataArray,MapboxVectorStyleSetImplRef styleSet)
{
    if (dataArray.size() < 2)
    {
        wkLogLevel(Warn, "Expecting at least two arguments for function stops.");
        return false;
    }
    
    for (auto stop : dataArray) {
        if (stop->getType() != DictTypeArray) {
            std::vector<DictionaryEntryRef> stopEntries = stop->getArray();
            if (stopEntries.size() != 2) {
                wkLogLevel(Warn,"Expecting two arguments in each entry for a function stop.");
                return false;
            }
            
            MaplyVectorFunctionStop fStop;
            fStop.zoom = stopEntries[0]->getDouble();
            if (stopEntries[1]->getType() == DictTypeDouble) {
                fStop.val = stopEntries[1]->getDouble();
            } else {
                fStop.color = RGBAColorRef(new RGBAColor(stopEntries[1]->getColor()));
            }
            
            stops.push_back(fStop);
        }
    }

    base = 1.0;
    
    return true;
}

double MaplyVectorFunctionStops::valueForZoom(double zoom)
{
    MaplyVectorFunctionStop *a = &stops[0],*b = NULL;
    if (zoom <= a->zoom)
        return a->val;
    for (int which = 1;which < stops.size(); which++)
    {
        b = &stops[which];
        if (a->zoom <= zoom && zoom < b->zoom)
        {
            double ratio = 1.0;
            if (base == 1.0) {
                ratio = (zoom-a->zoom)/(b->zoom-a->zoom);
            } else {
                double soFar = zoom-a->zoom;
                ratio = (pow(base, soFar) - 1.0) / (pow(base,b->zoom-a->zoom) - 1.0);
            }
            return ratio * (b->val-a->val) + a->val;
        }
        a = b;
    }

    return b->val;
}

RGBAColorRef MaplyVectorFunctionStops::colorForZoom(int zoom)
{
    MaplyVectorFunctionStop *a = &stops[0],*b = NULL;
    if (zoom <= a->zoom)
        return a->color;
    for (int which = 1;which < stops.size(); which++)
    {
        b = &stops[which];
        if (a->zoom <= zoom && zoom < b->zoom)
        {
            double ratio = 1.0;
            if (base == 1.0) {
                ratio = (zoom-a->zoom)/(b->zoom-a->zoom);
            } else {
                double soFar = zoom-a->zoom;
                ratio = (pow(base, soFar) - 1.0) / (pow(base,b->zoom-a->zoom) - 1.0);
            }
            float ac[4],bc[4];
            a->color->asUnitFloats(ac);
            b->color->asUnitFloats(bc);
            float res[4];
            for (unsigned int ii=0;ii<4;ii++)
                res[ii] = ratio * (bc[ii]-ac[ii]) + ac[ii];
            return RGBAColorRef(new RGBAColor(RGBAColor::FromUnitFloats(res)));
        }
        a = b;
    }

    return b->color;
}

double MaplyVectorFunctionStops::minValue()
{
    double val = MAXFLOAT;

    for (auto stop : stops)
        val = std::min(val,stop.val);

    return val;
}

double MaplyVectorFunctionStops::maxValue()
{
    double val = -MAXFLOAT;

    for (auto stop : stops) {
        val = std::max(val,stop.val);
    }

    return val;
}

MapboxTransDouble::MapboxTransDouble(double value)
{
    val = value;
}

MapboxTransDouble::MapboxTransDouble(MaplyVectorFunctionStopsRef inStops)
{
    val = 0.0;
    stops = inStops;
}

double MapboxTransDouble::valForZoom(double zoom)
{
    if (stops) {
        return stops->valueForZoom(zoom);
    } else
        return val;
}

double MapboxTransDouble::minVal()
{
    if (stops) {
        return stops->minValue();
    } else
        return val;
}

double MapboxTransDouble::maxVal()
{
    if (stops) {
        return stops->maxValue();
    } else
        return val;
}

MapboxTransColor::MapboxTransColor(RGBAColorRef color)
: color(color), useAlphaOverride(false), alpha(1.0)
{
}

MapboxTransColor::MapboxTransColor(MaplyVectorFunctionStopsRef stops)
: useAlphaOverride(false), alpha(1.0), stops(stops)
{
}

void MapboxTransColor::setAlphaOverride(double alphaOverride)
{
    useAlphaOverride = true;
    alpha = alphaOverride;
}

RGBAColor MapboxTransColor::colorForZoom(double zoom)
{
    RGBAColor theColor = *(stops ? stops->colorForZoom(zoom) : color);

    if (useAlphaOverride) {
        theColor.a = alpha * 255.0;
    }

    return theColor;
}


//- (long long)generateID
//{
//    return currentID++;
//}
//
//- (NSArray*)stylesForFeatureWithAttributes:(NSDictionary*)attributes
//                                    onTile:(MaplyTileID)tileID
//                                   inLayer:(NSString*)sourceLayer
//                                     viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
//{
//    NSArray *layersToRun = _layersBySource[sourceLayer];
//    if (!layersToRun)
//        return nil;
//    NSMutableArray *passedLayers = [NSMutableArray array];
//    for (MaplyMapboxVectorStyleLayer *layer in layersToRun)
//    {
//        if (!layer.filter || [layer.filter testFeature:attributes tile:tileID viewC:viewC])
//            [passedLayers addObject:layer];
//    }
//
//    return passedLayers;
//}
//
//- (BOOL)layerShouldDisplay:(NSString*)sourceLayer tile:(MaplyTileID)tileID
//{
//    NSArray *layersToRun = _layersBySource[sourceLayer];
//
//    return (layersToRun.count != 0);
//}
//
//- (MaplyVectorTileStyle*)styleForUUID:(long long)uuid viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
//{
//    return layersByUUID[@(uuid)];
//}
//
//- (NSArray * _Nonnull)allStyles {
//    return [layersByUUID allValues];
//}
//
//- (UIColor *)backgroundColorForZoom:(double)zoom;
//{
//    MaplyMapboxVectorStyleLayer *layer = [_layersByName objectForKey:@"background"];
//    if ([layer isKindOfClass:[MapboxVectorLayerBackground class]]) {
//        MapboxVectorLayerBackground *backLayer = (MapboxVectorLayerBackground *)layer;
//        return [backLayer.paint.color colorForZoom:zoom];
//    }
//
//    return nil;
//}
//
//- (id)constantSubstitution:(id)thing forField:(NSString *)field
//{
//    // Look for a constant substitution
//    if ([thing isKindOfClass:[NSString class]])
//    {
//        NSString *stringThing = thing;
//        // Note: This just handles simple ones with full substitution
//        if ([stringThing characterAtIndex:0] == '@')
//        {
//            id constant = _constants[stringThing];
//            if (constant)
//                thing = constant;
//            else {
//                NSLog(@"Failed to substitute constant %@ for field %@",stringThing,field);
//                return thing;
//            }
//        }
//    }
//
//    return thing;
//}
//
//- (int)intValue:(NSString *)name dict:(NSDictionary *)dict defVal:(int)defVal
//{
//    id thing = dict[name];
//    if (!thing)
//        return defVal;
//
//    thing = [self constantSubstitution:thing forField:name];
//
//    if ([thing respondsToSelector:@selector(integerValue)])
//        return [thing integerValue];
//
//    NSLog(@"Expected integer for %@ but got something else",name);
//    return defVal;
//}
//
//- (double)doubleValue:(id)thing defVal:(double)defVal
//{
//    thing = [self constantSubstitution:thing forField:nil];
//
//    if ([thing respondsToSelector:@selector(doubleValue)])
//        return [thing doubleValue];
//
//    NSLog(@"Expected double but got something else (%@)",thing);
//    return defVal;
//}
//
//- (double)doubleValue:(NSString *)name dict:(NSDictionary *)dict defVal:(double)defVal
//{
//    id thing = dict[name];
//    if (!thing)
//        return defVal;
//
//    return [self doubleValue:thing defVal:defVal];
//}
//
//- (bool)boolValue:(NSString *)name dict:(NSDictionary *)dict onValue:(NSString *)onString defVal:(bool)defVal
//{
//    id thing = dict[name];
//    if (!thing)
//        return defVal;
//
//    if ([thing isKindOfClass:[NSString class]]) {
//        return [thing isEqualToString:onString];
//    } else
//        return defVal;
//}
//
//- (NSString *)stringValue:(NSString *)name dict:(NSDictionary *)dict defVal:(NSString *)defVal
//{
//    id thing = dict[name];
//    if (!thing)
//        return defVal;
//
//    thing = [self constantSubstitution:thing forField:name];
//
//    if ([thing isKindOfClass:[NSString class]])
//        return thing;
//    if ([thing respondsToSelector:@selector(stringValue)])
//        return [thing stringValue];
//
//    NSLog(@"Expected string for %@ but got something else",name);
//    return defVal;
//}
//
//- (NSArray *)arrayValue:(NSString *)name dict:(NSDictionary *)dict defVal:(NSArray *)defVal
//{
//    id thing = dict[name];
//    if (!thing)
//        return defVal;
//
//    thing = [self constantSubstitution:thing forField:name];
//
//    if ([thing isKindOfClass:[NSArray class]])
//        return thing;
//
//    NSLog(@"Expected array for %@ but got something else",name);
//    return defVal;
//}
//
//- (MaplyVectorFunctionStops *)stopsValue:(id)entry defVal:(id)defEntry
//{
//    entry = [self constantSubstitution:entry forField:nil];
//
//    NSNumber *base = nil;
//    if ([entry isKindOfClass:[NSDictionary class]])
//    {
//        base = ((NSDictionary *)entry)[@"base"];
//        entry = ((NSDictionary *)entry)[@"stops"];
//    }
//
//    if (!entry)
//    {
//        NSLog(@"Expecting key word 'stops' in entry %@",defEntry);
//        return defEntry;
//    }
//
//    MaplyVectorFunctionStops *stops = [[MaplyVectorFunctionStops alloc] initWithArray:entry styleSet:self viewC:self.viewC];
//    if (stops)
//    {
//        if ([base isKindOfClass:[NSNumber class]])
//            stops.base = [base doubleValue];
//        return stops;
//    }
//    return defEntry;
//}
//
//- (MapboxTransDouble *__nullable)transDouble:(NSString * __nonnull)name entry:(NSDictionary *__nonnull)entry defVal:(double)defVal
//{
//    // They pass in the whole dictionary and let us look the field up
//    id theEntry = entry[name];
//    if (!theEntry)
//        return [[MapboxTransDouble alloc] initWithDouble:defVal];
//    theEntry = [self constantSubstitution:theEntry forField:nil];
//
//    // This is probably stops
//    if ([theEntry isKindOfClass:[NSDictionary class]]) {
//        MaplyVectorFunctionStops *stops = [self stopsValue:theEntry defVal:name];
//        if (stops) {
//            return [[MapboxTransDouble alloc] initWithStops:stops];
//        } else {
//            NSLog(@"Expecting key word 'stops' in entry %@",name);
//        }
//    } else if ([theEntry isKindOfClass:[NSNumber class]]) {
//        return [[MapboxTransDouble alloc] initWithDouble:[theEntry doubleValue]];
//    } else {
//        NSLog(@"Unexpected type found in entry %@. Was expecting a double.",name);
//    }
//
//    return nil;
//}
//
//- (MapboxTransColor *__nullable)transColor:(NSString *__nonnull)name entry:(NSDictionary *__nonnull)entry defVal:(UIColor * __nullable)defVal
//{
//    // They pass in the whole dictionary and let us look the field up
//    id theEntry = entry[name];
//    if (!theEntry) {
//        if (defVal)
//            return [[MapboxTransColor alloc] initWithColor:defVal];
//        return nil;
//    }
//    theEntry = [self constantSubstitution:theEntry forField:nil];
//
//    // This is probably stops
//    if ([theEntry isKindOfClass:[NSDictionary class]]) {
//        MaplyVectorFunctionStops *stops = [self stopsValue:theEntry defVal:name];
//        if (stops) {
//            return [[MapboxTransColor alloc] initWithStops:stops];
//        } else {
//            NSLog(@"Expecting key word 'stops' in entry %@",name);
//        }
//    } else if ([theEntry isKindOfClass:[NSString class]]) {
//        UIColor *color = [self colorValue:name val:theEntry dict:nil defVal:defVal multiplyAlpha:false];
//        if (color)
//            return [[MapboxTransColor alloc] initWithColor:color];
//        else {
//            NSLog(@"Unexpected type found in entry %@. Was expecting a color.",name);
//        }
//    } else {
//        NSLog(@"Unexpected type found in entry %@. Was expecting a color.",name);
//    }
//
//    return nil;
//}
//
//- (UIColor *__nullable)resolveColor:(MapboxTransColor * __nullable)color opacity:(MapboxTransDouble * __nullable)opacity forZoom:(double)zoom mode:(MBResolveColorType)resolveMode
//{
//    // No color means no color
//    if (!color)
//        return nil;
//
//    UIColor *thisColor = [color colorForZoom:zoom];
//
//    // No opacity, means full opacity
//    if (!opacity)
//        return thisColor;
//
//    double thisOpacity = [opacity valForZoom:zoom];
//
//    CGFloat red,green,blue,alpha;
//    [thisColor getRed:&red green:&green blue:&blue alpha:&alpha];
//    switch (resolveMode)
//    {
//        case MBResolveColorOpacityMultiply:
//            return [UIColor colorWithRed:red*thisOpacity green:green*thisOpacity blue:blue*thisOpacity alpha:alpha*thisOpacity];
//            break;
//        case MBResolveColorOpacityReplaceAlpha:
//            return [UIColor colorWithRed:red green:green blue:blue alpha:thisOpacity];
//            break;
//    }
//}
//
//- (UIColor *)color:(UIColor *)color withOpacity:(double)opacity
//{
//    CGFloat red,green,blue,alpha;
//    [color getRed:&red green:&green blue:&blue alpha:&alpha];
//    return [UIColor colorWithRed:red*opacity green:green*opacity blue:blue*opacity alpha:alpha*opacity];
//}
//
//- (UIColor *)colorValue:(NSString *)name val:(id)val dict:(NSDictionary *)dict defVal:(UIColor *)defVal multiplyAlpha:(bool)multiplyAlpha
//{
//    id thing = nil;
//    if (dict)
//        thing = dict[name];
//    else
//        thing = val;
//    if (!thing)
//        return defVal;
//
//    thing = [self constantSubstitution:thing forField:name];
//
//    if (![thing isKindOfClass:[NSString class]])
//    {
//        NSLog(@"Expecting a string for color (%@)",name);
//        return defVal;
//    }
//
//    NSString *str = thing;
//    if ([str length] == 0)
//    {
//        NSLog(@"Expecting non-empty string for color (%@)",name);
//        return defVal;
//    }
//    // Hex string
//    if ([str characterAtIndex:0] == '#')
//    {
//        // Hex string
//        NSScanner *scanner = [NSScanner scannerWithString:str];
//        [scanner setScanLocation:1];
//        unsigned int iVal;
//        if (![scanner scanHexInt:&iVal])
//        {
//            NSLog(@"Invalid hex value (%@) in color (%@)",str,name);
//            return defVal;
//        }
//
//        int red,green,blue;
//        int alpha = 255;
//        if ([str length] == 4)
//        {
//            red = (iVal >> 8) & 0xf;  red |= red << 4;
//            green = (iVal >> 4) & 0xf;  green |= green << 4;
//            blue = iVal & 0xf;  blue |= blue << 4;
//        } else if ([str length] > 7) {
//            red = (iVal >> 24) & 0xff;
//            green = (iVal >> 16) & 0xff;
//            blue = (iVal >> 8) & 0xff;
//            alpha = iVal & 0xff;
//        } else {
//            red = (iVal >> 16) & 0xff;
//            green = (iVal >> 8) & 0xff;
//            blue = iVal & 0xff;
//        }
//        return [UIColor colorWithRed:(double)red/255.0 green:(double)green/255.0 blue:(double)blue/255.0 alpha:alpha/255.0];
//    } else if ([str rangeOfString:@"rgb("].location == 0)
//    {
//        NSScanner *scanner = [NSScanner scannerWithString:str];
//        NSMutableCharacterSet *skipSet = [[NSMutableCharacterSet alloc] init];
//        [skipSet addCharactersInString:@"(), "];
//        [scanner setCharactersToBeSkipped:skipSet];
//        [scanner setScanLocation:4];
//        int red,green,blue;
//        [scanner scanInt:&red];
//        [scanner scanInt:&green];
//        [scanner scanInt:&blue];
//
//        return [UIColor colorWithRed:red/255.0 green:green/255.0 blue:blue/255.0 alpha:1.0];
//    } else if ([str rangeOfString:@"rgba("].location == 0)
//    {
//        NSScanner *scanner = [NSScanner scannerWithString:str];
//        NSMutableCharacterSet *skipSet = [[NSMutableCharacterSet alloc] init];
//        [skipSet addCharactersInString:@"(), "];
//        [scanner setCharactersToBeSkipped:skipSet];
//        [scanner setScanLocation:5];
//        int red,green,blue;
//        [scanner scanInt:&red];
//        [scanner scanInt:&green];
//        [scanner scanInt:&blue];
//        float alpha;
//        [scanner scanFloat:&alpha];
//
//        if (multiplyAlpha)
//            return [UIColor colorWithRed:red/255.0*alpha green:green/255.0*alpha blue:blue/255.0*alpha alpha:alpha];
//        else
//            return [UIColor colorWithRed:red/255.0 green:green/255.0 blue:blue/255.0 alpha:alpha];
//    } else if ([str rangeOfString:@"hsl("].location == 0)
//    {
//        NSScanner *scanner = [NSScanner scannerWithString:str];
//        NSMutableCharacterSet *skipSet = [[NSMutableCharacterSet alloc] init];
//        [skipSet addCharactersInString:@"(),% "];
//        [scanner setCharactersToBeSkipped:skipSet];
//        [scanner setScanLocation:4];
//        int hue,sat,light;
//        [scanner scanInt:&hue];
//        [scanner scanInt:&sat];
//        [scanner scanInt:&light];
//        float newLight = light / 100.0;
//        float newSat = sat / 100.0;
//        newSat = newSat * (newLight < 0.5 ? newLight : 1.0-newLight);
//
//        return [UIColor colorWithHue:hue/360.0 saturation:2.0*newSat/(newLight+newSat) brightness:newLight+newSat alpha:1.0];
//    } else if ([str rangeOfString:@"hsla("].location == 0)
//    {
//        NSScanner *scanner = [NSScanner scannerWithString:str];
//        NSMutableCharacterSet *skipSet = [[NSMutableCharacterSet alloc] init];
//        [skipSet addCharactersInString:@"(),% "];
//        [scanner setCharactersToBeSkipped:skipSet];
//        [scanner setScanLocation:4];
//        int hue,sat,light;
//        float alpha;
//        [scanner scanInt:&hue];
//        [scanner scanInt:&sat];
//        [scanner scanInt:&light];
//        [scanner scanFloat:&alpha];
//        float newLight = light / 100.0;
//        float newSat = sat / 100.0;
//        newSat = newSat * (newLight < 0.5 ? newLight : 1.0-newLight);
//
//        return [UIColor colorWithHue:hue/360.0 saturation:2.0*newSat/(newLight+newSat) brightness:newLight+newSat alpha:alpha];
//    }
//
//
//    NSLog(@"Didn't recognize format of color (%@)",name);
//    return defVal;
//}
//
//- (NSUInteger)enumValue:(NSString *)name options:(NSArray *)options defVal:(NSUInteger)defVal
//{
//    if (!name)
//        return defVal;
//
//    if (![name isKindOfClass:[NSString class]])
//    {
//        NSLog(@"Expecting string for enumerated type.");
//        return defVal;
//    }
//
//    int which = 0;
//    for (NSString *val in options)
//    {
//        if ([val isEqualToString:name])
//            return which;
//        which++;
//    }
//
//    NSLog(@"Found unexpected value (%@) in enumerated type",name);
//    return defVal;
//}
//
//- (void)unsupportedCheck:(NSString *)field in:(NSString *)what styleEntry:(NSDictionary *)styleEntry
//{
//    if (styleEntry[field])
//        NSLog(@"Found unsupported field (%@) for (%@)",field,what);
//}


}
