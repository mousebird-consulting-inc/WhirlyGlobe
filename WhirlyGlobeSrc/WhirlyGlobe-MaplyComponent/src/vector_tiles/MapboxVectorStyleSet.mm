/*
 *  MapboxVectorStyleSet.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/16/15.
 *  Copyright 2011-2015 mousebird consulting
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

#import "MapboxVectorStyleSet.h"
#import "MapboxVectorStyleBackground.h"
#import "MapboxVectorStyleFill.h"
#import "MapboxVectorStyleLine.h"
#import "MapboxVectorStyleRaster.h"
#import "MapboxVectorStyleSymbol.h"

@implementation MaplyMapboxVectorStyleSet
{
    NSMutableDictionary *layersByUUID;
}

- (instancetype)initWithJSON:(NSData *)styleJSON viewC:(MaplyBaseViewController *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    _viewC = viewC;
    NSError *error = nil;
    NSDictionary *styleDict = [NSJSONSerialization JSONObjectWithData:styleJSON options:NULL error:&error];
    if (!styleDict)
        return nil;
    
    _name = styleDict[@"name"];
    _version = [styleDict[@"version"] integerValue];
    _constants = styleDict[@"constants"];
    _spriteURL = styleDict[@"sprite"];
    NSArray *layerStyles = styleDict[@"layers"];
    NSMutableArray *layers = [NSMutableArray array];
    NSMutableDictionary *sourceLayers = [NSMutableDictionary dictionary];
    layersByUUID = [NSMutableDictionary dictionary];
    int which = 0;
    for (NSDictionary *layerStyle in layerStyles)
    {
		MaplyMapboxVectorStyleLayer *layer = [[MaplyMapboxVectorStyleLayer alloc] initWithStyleSet:self JSON:layerStyle drawPriority:(10*which + kMaplyVectorDrawPriorityDefault)];
        if (layer)
        {
            [layers addObject:layer];
            layersByUUID[layer.uuid] = layer;
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
    _layersBySource = sourceLayers;
    
    return self;
}

- (NSArray*)stylesForFeatureWithAttributes:(NSDictionary*)attributes
                                    onTile:(MaplyTileID)tileID
                                   inLayer:(NSString*)sourceLayer
                                     viewC:(MaplyBaseViewController *)viewC
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

- (MaplyVectorTileStyle*)styleForUUID:(NSString*)uuid viewC:(MaplyBaseViewController *)viewC
{
    return layersByUUID[uuid];
}

- (id)constantSubstitution:(id)thing forField:(NSString *)field
{
    // Look for a constant substitution
    if ([thing isKindOfClass:[NSString class]])
    {
        NSString *stringThing = thing;
        if ([stringThing length] == 0)
            return thing;
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

- (UIColor *)backgroundColor
{
    // Look for the background layer
    MapboxVectorLayerBackground *backLayer = nil;
    for (id layer in _layers)
        if ([layer isKindOfClass:[MapboxVectorLayerBackground class]])
        {
            backLayer = layer;
            break;
        }
    
    if (backLayer)
    {
        UIColor *color = [backLayer.paint.color maxColorWithStyleSet:self];
        if (color)
            return color;
    }
    
    return nil;
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

- (NSString *)stringValue:(NSString *)str defVal:(NSString *)defVal
{
    id thing = [self constantSubstitution:str forField:@""];
    
    if ([thing isKindOfClass:[NSString class]])
        return thing;
    if ([thing respondsToSelector:@selector(stringValue)])
        return [thing stringValue];
    
    return defVal;
}

- (NSString *)stringValue:(NSString *)name dict:(NSDictionary *)dict defVal:(NSString *)defVal
{
    id thing = dict[name];
    if (!thing)
        return defVal;

//    NSLog(@"Expected string for %@ but got something else",name);

    return [self stringValue:thing defVal:defVal];
}

- (NSArray *)arrayValue:(NSString *)name dict:(NSDictionary *)dict defVal:(NSArray *)defVal
{
    id thing = dict[name];
    if (!thing)
        return defVal;
    
    if ([thing isKindOfClass:[NSDictionary class]])
    {
        NSDictionary *dict = thing;
        NSString *type = dict[@"type"];
        if (![type isEqualToString:@"number"])
        {
            NSLog(@"Expecting number type for number (%@)",name);
            return nil;
        }
        thing = dict[@"value"];
    }
    
    thing = [self constantSubstitution:thing forField:name];
    
    if ([thing isKindOfClass:[NSArray class]])
        return thing;
    
    NSLog(@"Expected array for %@ but got something else",name);
    return defVal;
}

- (UIColor *)color:(UIColor *)color withOpacity:(double)opacity
{
    CGFloat red,green,blue,alpha;
    [color getRed:&red green:&green blue:&blue alpha:&alpha];
    return [UIColor colorWithRed:red*opacity green:green*opacity blue:blue*opacity alpha:alpha*opacity];
}

- (UIColor *)colorValue:(NSString *)name defVal:(UIColor *)defVal
{
    id thing = [self constantSubstitution:name forField:@""];
    
    if ([thing isKindOfClass:[NSDictionary class]])
    {
        NSDictionary *dict = thing;
        NSString *type = dict[@"type"];
        if (![type isEqualToString:@"color"])
        {
            NSLog(@"Expecting color type for color (%@)",name);
            return nil;
        }
        thing = dict[@"value"];
    }
    
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
        if ([str length] == 4)
        {
            red = (iVal >> 8) & 0xf;  red |= red << 4;
            green = (iVal >> 4) & 0xf;  green |= green << 4;
            blue = iVal & 0xf;  blue |= blue << 4;
        } else {
            red = (iVal >> 16) & 0xff;
            green = (iVal >> 8) & 0xff;
            blue = iVal & 0xff;
        }
        return [UIColor colorWithRed:(double)red/255.0 green:(double)green/255.0 blue:(double)blue/255.0 alpha:1.0];
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
        
        return [UIColor colorWithRed:red/255.0*alpha green:green/255.0*alpha blue:blue/255.0*alpha alpha:alpha];
    } else {
        // Try
    }
    
    NSLog(@"Didn't recognize format of color (%@)",name);
    return defVal;
}

- (UIColor *)colorValue:(NSString *)name dict:(NSDictionary *)dict defVal:(UIColor *)defVal
{
    id thing = dict[name];
    if (!thing)
        return defVal;
 
    return [self colorValue:thing defVal:defVal];
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

@implementation MaplyMapboxVectorStyleLayer

+ (instancetype)VectorStyleLayer:(MaplyMapboxVectorStyleSet *)styleSet JSON:(NSDictionary *)layerDict drawPriority:(int)drawPriority
{
	return [[MaplyMapboxVectorStyleLayer alloc] initWithStyleSet:styleSet JSON:layerDict drawPriority:drawPriority];
}

- (instancetype)initWithStyleSet:(MaplyMapboxVectorStyleSet *)styleSet JSON:(NSDictionary *)layerDict drawPriority:(int)drawPriority
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
	} else if ([type isEqualToString:@"raster"])
	{
		MapboxVectorLayerRaster *rasterLayer = [[MapboxVectorLayerRaster alloc] initWithStyleEntry:layerDict parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:styleSet.viewC];
		layer = rasterLayer;
	} else if ([type isEqualToString:@"background"])
	{
		MapboxVectorLayerBackground *backLayer = [[MapboxVectorLayerBackground alloc] initWithStyleEntry:layerDict parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:styleSet.viewC];
		layer = backLayer;
	}
	if (layerDict[@"interactive"])
	{
		layer.interactive = [layerDict[@"interactive"] boolValue];
	}
	if (layerDict[@"filter"])
	{
		layer.filter = [[MapboxVectorFilter alloc] initWithArray:[styleSet arrayValue:@"filter" dict:layerDict defVal:nil] styleSet:styleSet viewC:styleSet.viewC];
		if (!layer.filter)
		{
			NSLog(@"MapboxStyleSet: Failed to parse filter for layer %@",layer.ident);
		}
	}

	return layer;
}


- (instancetype)initWithStyleEntry:(NSDictionary *)layerDict parent:(MaplyMapboxVectorStyleLayer *)refLayer styleSet:(MaplyMapboxVectorStyleSet *)styleSet drawPriority:(int)drawPriority viewC:(MaplyBaseViewController *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    self.styleSet = styleSet;
    self.drawPriority = drawPriority;
    self.uuid = @(rand());
    
    _minzoom = -1;
    _maxzoom = -1;
    
    self.ident = layerDict[@"id"];
    self.source = [styleSet stringValue:@"source" dict:layerDict defVal:refLayer.source];
    self.sourceLayer = [styleSet stringValue:@"source-layer" dict:layerDict defVal:refLayer.sourceLayer];
    self.minzoom = [styleSet intValue:@"minzoom" dict:layerDict defVal:refLayer.minzoom];
    self.maxzoom = [styleSet intValue:@"maxzoom" dict:layerDict defVal:refLayer.maxzoom];
    
    return self;
}

- (NSArray *)buildObjects:(NSArray *)vecObjs forTile:(MaplyTileID)tileID viewC:(MaplyBaseViewController *)viewC
{
    return nil;
}

@end

@implementation MapboxVectorFilter

- (instancetype)initWithArray:(NSArray *)filterArray styleSet:(MaplyMapboxVectorStyleSet *)styleSet viewC:(MaplyBaseViewController *)viewC
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
                options:@[@"==",@"!=",@">",@">=",@"<",@"<=",@"in",@"!in",@"all",@"any",@"none"]
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

- (bool)testFeature:(NSDictionary *)attrs tile:(MaplyTileID)tileID viewC:(MaplyBaseViewController *)viewC
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
                if ([featAttrNum isKindOfClass:[NSNumber class]] && [attrNum isKindOfClass:[NSNumber class]])
                {
                    switch (_filterType)
                    {
                        case MBFilterEqual:
                            ret = [featAttrNum isEqualToNumber:attrNum];
                            break;
                        case MBFilterNotEqual:
                            ret = ![featAttrNum isEqualToNumber:attrNum];
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
        }
    }
    
    return ret;
}

@end

@implementation MaplyMapboxValue

- (instancetype)initWithValue:(id)value type:(MaplyMapboxValueType)dataType styleSet:(MaplyMapboxVectorStyleSet *)styleSet
{
    self = [super init];

    _type = dataType;
    switch (dataType)
    {
        case MaplyMapboxValueTypeNumber:
            if (styleSet)
                _value = [NSNumber numberWithDouble:[styleSet doubleValue:value defVal:0.0]];
            else {
                if ([value isKindOfClass:[NSNumber class]])
                {
                    _value = value;
                } else {
                    NSLog(@"Expecting NSNumber when initializing value.");
                    return nil;
                }
            }
            break;
        case MaplyMapboxValueTypeColor:
            if (styleSet)
                _value = [styleSet colorValue:value defVal:nil];
            else {
                if ([value isKindOfClass:[UIColor class]])
                    _value = value;
                else {
                    NSLog(@"Expecting UIColor when initializing value.");
                    return nil;
                }
            }
            break;
        case MaplyMapboxValueTypeString:
            if (styleSet)
                _value = [styleSet stringValue:value defVal:nil];
            else {
                if ([value isKindOfClass:[NSString class]])
                {
                    _value = value;
                } else {
                    NSLog(@"Expecting NSString when initializing value.");
                    return nil;
                }
            }
            break;
    }
    
    if (!_value)
        return nil;
    
    return self;
}

+ (MaplyMapboxValue *)interpolateFrom:(MaplyMapboxValue *)a to:(MaplyMapboxValue *)b t:(double)t
{
    if (a.type != b.type)
        return nil;
    
    switch (a.type)
    {
        case MaplyMapboxValueTypeNumber:
        {
            double aVal = [a.value doubleValue];
            double bVal = [b.value doubleValue];
            double res = (bVal-aVal)*t + aVal;
            return [[MaplyMapboxValue alloc] initWithValue:[NSNumber numberWithDouble:res] type:MaplyMapboxValueTypeNumber styleSet:nil];
        }
            break;
        case MaplyMapboxValueTypeColor:
        {
            UIColor *aCol = a.value;
            UIColor *bCol = b.value;
            CGFloat a[4],b[4],res[4];
            [aCol getRed:&a[0] green:&a[1] blue:&a[2] alpha:&a[3]];
            [bCol getRed:&b[0] green:&b[1] blue:&b[2] alpha:&b[3]];
            for (unsigned int ii=0;ii<4;ii++)
                res[ii] = (b[ii]-a[ii])*t + a[ii];
            UIColor *resColor = [UIColor colorWithRed:res[0] green:res[1] blue:res[2] alpha:res[3]];
            return [[MaplyMapboxValue alloc] initWithValue:resColor type:MaplyMapboxValueTypeColor styleSet:nil];
        }
            break;
        case MaplyMapboxValueTypeString:
            // Interpolation is weird here
            return a.value;
            break;
    }
    
    return nil;
}

@end

@implementation MaplyMapboxVectorFunctionStop
@end

@implementation MaplyMapboxVectorFunction

- (instancetype)initWithValueDict:(NSDictionary *)dict dataType:(MaplyMapboxValueType)dataType styleSet:(MaplyMapboxVectorStyleSet *)styleSet viewC:(MaplyBaseViewController *)viewC
{
    if (![dict isKindOfClass:[NSDictionary class]])
    {
        NSLog(@"Expected JSON object for function.");
        return nil;
    }
    NSNumber *base = dict[@"base"];
    NSArray *domain = dict[@"domain"];
    NSArray *range = dict[@"range"];
    NSString *type = dict[@"type"];

    if (![domain isKindOfClass:[NSArray class]] || ![range isKindOfClass:[NSArray class]] ||
        [domain count] != [range count])
    {
        NSLog(@"Expected JSON arrays of same length for domain and range of function.");
        return nil;
    }
    if (![base isKindOfClass:[NSNumber class]])
    {
        NSLog(@"Expected number for base of function.");
        return nil;
    }
    if (![type isKindOfClass:[NSString class]])
    {
        NSLog(@"Expected string for type of function.");
        return nil;
    }
    
    NSMutableArray *stops = [NSMutableArray array];
    int which = 0;
    for (NSNumber *inVal in domain)
    {
        id outVal = [range objectAtIndex:which];
        
        if (![inVal isKindOfClass:[NSNumber class]])
        {
            NSLog(@"Expecting numbers for domain of function.");
            return nil;
        }
        
        MaplyMapboxVectorFunctionStop *fStop = [[MaplyMapboxVectorFunctionStop alloc] init];
        fStop.inputVal = [inVal doubleValue];
        fStop.outputVal = outVal;
        [stops addObject:fStop];
        
        which++;
    }
    
    self = [super init];
    if (!self)
        return nil;
    
    _dataType = dataType;
    _base = [base doubleValue];
    _stops = stops;
    
    return self;
}

- (MaplyMapboxValue *)valueForInput:(double)inputVal type:(MaplyMapboxValueType)dataType styleSet:(MaplyMapboxVectorStyleSet *)styleSet
{
    if (dataType != _dataType)
        return nil;
    
    MaplyMapboxVectorFunctionStop *a = _stops[0],*b = nil;
    MaplyMapboxValue *aVal = [[MaplyMapboxValue alloc] initWithValue:a.outputVal type:dataType styleSet:styleSet];
    if (inputVal <= a.inputVal)
        return [[MaplyMapboxValue alloc] initWithValue:a.outputVal type:dataType styleSet:styleSet];
    for (int which = 1;which < _stops.count; which++)
    {
        b = _stops[which];
        MaplyMapboxValue *bVal = [[MaplyMapboxValue alloc] initWithValue:b.outputVal type:dataType styleSet:styleSet];
        if (a.inputVal <= inputVal && inputVal < b.inputVal)
        {
            double t = (inputVal-a.inputVal)/(b.inputVal-a.inputVal);
            double scaleT = pow(t,_base);

            return [MaplyMapboxValue interpolateFrom:aVal to:bVal t:scaleT];
        }
        a = b;
        aVal = bVal;
    }
    
    return [[MaplyMapboxValue alloc] initWithValue:b.outputVal type:dataType styleSet:styleSet];
}

- (MaplyMapboxValue *)minValueOfType:(MaplyMapboxValueType)dataType
{
    MaplyMapboxVectorFunctionStop *a = _stops[0];
    return [[MaplyMapboxValue alloc] initWithValue:a.outputVal type:_dataType styleSet:nil];
}

- (MaplyMapboxValue *)maxValueOfType:(MaplyMapboxValueType)dataType
{
    MaplyMapboxVectorFunctionStop *b = _stops[_stops.count-1];
    return [[MaplyMapboxValue alloc] initWithValue:b.outputVal type:_dataType styleSet:nil];
}

@end

@implementation MaplyMapboxValueWrapper
{
    MaplyMapboxValue *val;
    MaplyMapboxVectorFunction *func;
}

- (instancetype)initWithValue:(id)inValue dataType:(MaplyMapboxValueType)dataType styleSet:(MaplyMapboxVectorStyleSet *)styleSet
{
    self = [super init];

    id subValue = [styleSet constantSubstitution:inValue forField:@""];
    if (!subValue)
    {
        NSLog(@"Missing required field");
        return nil;
    }

    id value = subValue;
    if ([subValue isKindOfClass:[NSDictionary class]])
    {
        NSString *type = subValue[@"type"];
        value = subValue[@"value"];
        
        // Was a constant subsitution
        if (type && value)
        {
            switch (dataType)
            {
                case MaplyMapboxValueTypeNumber:
                    // Note: Opacity looks like a bug in the file
                    if (![type isEqualToString:@"number"] && ![type isEqualToString:@"opacity"])
                    {
                        NSLog(@"Mismatched data type in value.  Expecting number.");
                        return nil;
                    }
                    break;
                case MaplyMapboxValueTypeColor:
                    if (![type isEqualToString:@"color"])
                    {
                        NSLog(@"Mismatched data type in value.  Expecting color.");
                        return nil;
                    }
                    break;
                case MaplyMapboxValueTypeString:
                    if (![type isEqualToString:@"enum"])
                    {
                        NSLog(@"Mismatched data type in value.  Expecting string.");
                        return nil;
                    }
                    break;
            }
        } else {
            // Is just a straight function
            value = subValue;
        }
    }
    
    _dataType = dataType;
    if ([value isKindOfClass:[NSString class]])
    {
        if (dataType == MaplyMapboxValueTypeColor || dataType == MaplyMapboxValueTypeString)
        {
            val = [[MaplyMapboxValue alloc] initWithValue:value type:dataType styleSet:styleSet];
        } else {
            NSLog(@"Expecting string for color or string type in value.");
            return nil;
        }
    } else if ([value isKindOfClass:[NSNumber class]])
    {
        if (dataType == MaplyMapboxValueTypeNumber)
        {
            val = [[MaplyMapboxValue alloc] initWithValue:value type:MaplyMapboxValueTypeNumber styleSet:styleSet];
        } else {
            NSLog(@"Expecting number for value.");
            return nil;
        }
    } else if ([value isKindOfClass:[NSDictionary class]])
    {
        func = [[MaplyMapboxVectorFunction alloc] initWithValueDict:value dataType:dataType styleSet:styleSet viewC:nil];
    }
    
    return self;
}

- (instancetype)initWithDict:(NSDictionary *)dict name:(NSString *)attrName dataType:(MaplyMapboxValueType)dataType styleSet:(MaplyMapboxVectorStyleSet *)styleSet
{
    id thing = dict[attrName];
    if (!thing)
    {
        return nil;
    }
    
    return [self initWithValue:thing dataType:dataType styleSet:styleSet];
}

- (instancetype)initWithObject:(id)thing
{
    self = [super init];
    if ([thing isKindOfClass:[UIColor class]])
    {
        val = [[MaplyMapboxValue alloc] initWithValue:thing type:MaplyMapboxValueTypeColor styleSet:nil];
    } else if ([thing isKindOfClass:[NSString class]])
    {
        val = [[MaplyMapboxValue alloc] initWithValue:thing type:MaplyMapboxValueTypeString styleSet:nil];
        
    } else if ([thing isKindOfClass:[NSNumber class]])
    {
        val = [[MaplyMapboxValue alloc] initWithValue:thing type:MaplyMapboxValueTypeNumber styleSet:nil];
    } else
        return nil;
    
    return self;
}

- (double)numberForZoom:(int)zoom styleSet:(MaplyMapboxVectorStyleSet *)styleSet
{
    if (_dataType != MaplyMapboxValueTypeNumber)
        return 0.0;
    
    if (val)
        return [val.value doubleValue];
    else {
        return [[func valueForInput:zoom type:MaplyMapboxValueTypeNumber styleSet:styleSet].value doubleValue];
    }
}

- (double)maxNumberWithStyleSet:(MaplyMapboxVectorStyleSet *)styleSet
{
    if (_dataType != MaplyMapboxValueTypeNumber)
        return 0.0;

    if (val)
        return [val.value doubleValue];
    else {
        MaplyMapboxVectorFunctionStop *last = [func.stops lastObject];
        return [styleSet doubleValue:last.outputVal defVal:0.0];
    }
}

- (UIColor *)colorForZoom:(int)zoom styleSet:(MaplyMapboxVectorStyleSet *)styleSet
{
    if (_dataType != MaplyMapboxValueTypeColor)
        return nil;
    
    if (val)
        return val.value;
    else {
        return [func valueForInput:zoom type:MaplyMapboxValueTypeColor styleSet:styleSet].value;
    }
}

- (UIColor *)maxColorWithStyleSet:(MaplyMapboxVectorStyleSet *)styleSet
{
    if (_dataType != MaplyMapboxValueTypeColor)
        return nil;
    
    if (val)
        return val.value;
    else {
        MaplyMapboxVectorFunctionStop *last = [func.stops lastObject];
        return [styleSet colorValue:last.outputVal defVal:nil];
    }
}

- (NSString *)stringForZoom:(int)zoom styleSet:(MaplyMapboxVectorStyleSet *)styleSet
{
    if (_dataType != MaplyMapboxValueTypeString)
        return nil;
    
    if (val)
        return val.value;
    else {
        return [func valueForInput:zoom type:MaplyMapboxValueTypeString styleSet:styleSet].value;
    }
}

@end
