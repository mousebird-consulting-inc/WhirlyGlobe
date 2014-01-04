/*
 *  MaplyVectorTiles.m
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/3/14.
 *  Copyright 2011-2014 mousebird consulting
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

#import "MaplyVectorTiles.h"
#import <map>

@implementation MaplyVectorTileStyle

+ (id)styleFromStyleEntry:(NSDictionary *)styleEntry index:(int)index
{
    MaplyVectorTileStyle *tileStyle = nil;
    
    NSString *typeStr = styleEntry[@"type"];
    if ([typeStr isEqualToString:@"LineSymbolizer"])
    {
        tileStyle = [[MaplyVectorTileStyleLine alloc] initWithStyleEntry:styleEntry index:index];
    } else if ([typeStr isEqualToString:@"PolygonSymbolizer"])
    {
        tileStyle = [[MaplyVectorTileStylePolygon alloc] initWithStyleEntry:styleEntry index:index];
    } else if ([typeStr isEqualToString:@"TextSymbolizer"])
    {
        tileStyle = [[MaplyVectorTileStyleText alloc] initWithStyleEntry:styleEntry index:index];
    } else {
        // Set up one that doesn't do anything
        NSLog(@"Unknown symbolizer type %@",typeStr);
        tileStyle = [[MaplyVectorTileStyle alloc] init];
    }
    
    return tileStyle;
}

- (NSArray *)buildObjects:(NSArray *)vecObjs viewC:(MaplyBaseViewController *)viewC;
{
    return nil;
}

@end

// Line styles
@implementation MaplyVectorTileStyleLine
{
    NSDictionary *desc;
}

- (id)initWithStyleEntry:(NSDictionary *)styleEntry index:(int)index;
{
    self = [super init];
    
    float strokeWidth = 1.0;
    int red = 255,green = 255,blue = 255;
    float alpha = 1.0;

    // Build up the vector description dictionary
    if (styleEntry[@"stroke-width"])
        strokeWidth = [styleEntry[@"stroke-width"] floatValue];
    if (styleEntry[@"stroke-opacity"])
    {
        alpha = [styleEntry[@"stroke-opacity"] floatValue];
    }
    if (styleEntry[@"stroke"])
    {
        NSString *colorStr = styleEntry[@"stroke"];
        // parse the hex
        NSScanner *scanner = [NSScanner scannerWithString:colorStr];
        unsigned int colorVal;
        [scanner setScanLocation:1]; // bypass #
        [scanner scanHexInt:&colorVal];
        blue = colorVal & 0xFF;
        green = (colorVal >> 8) & 0xFF;
        red = (colorVal >> 16) & 0xFF;
    }
    desc = @{kMaplyVecWidth: @(strokeWidth),
             kMaplyColor: [UIColor colorWithRed:red/255.0*alpha green:green/255.0*alpha blue:blue/255.0*alpha alpha:alpha],
             kMaplyDrawPriority: @(index+kMaplyVectorDrawPriorityDefault)
            };
    
    return self;
}

- (NSArray *)buildObjects:(NSArray *)vecObjs viewC:(MaplyBaseViewController *)viewC;
{
    MaplyComponentObject *compObj = [viewC addVectors:vecObjs desc:desc];
    if (compObj)
        return @[compObj];
    
    return nil;
}

@end

// Filled polygons styles
@implementation MaplyVectorTileStylePolygon
{
    NSDictionary *desc;
}

- (id)initWithStyleEntry:(NSDictionary *)styleEntry index:(int)index;
{
    self = [super init];

    int red = 255,green = 255,blue = 255;
    float alpha = 1.0;

    // Build up the vector description dictionary
    if (styleEntry[@"fill-opacity"])
    {
        alpha = [styleEntry[@"fill-opacity"] floatValue];
    }
    if (styleEntry[@"fill"])
    {
        NSString *colorStr = styleEntry[@"fill"];
        // parse the hex
        NSScanner *scanner = [NSScanner scannerWithString:colorStr];
        unsigned int colorVal;
        [scanner setScanLocation:1]; // bypass #
        [scanner scanHexInt:&colorVal];
        blue = colorVal & 0xFF;
        green = (colorVal >> 8) & 0xFF;
        red = (colorVal >> 16) & 0xFF;
    }
    desc = @{kMaplyColor: [UIColor colorWithRed:red/255.0*alpha green:green/255.0*alpha blue:blue/255.0*alpha alpha:alpha],
             kMaplyFilled: @(YES),
             kMaplyDrawPriority: @(index+kMaplyVectorDrawPriorityDefault)
             };
    
    return self;
}

- (NSArray *)buildObjects:(NSArray *)vecObjs viewC:(MaplyBaseViewController *)viewC;
{
    MaplyComponentObject *compObj = [viewC addVectors:vecObjs desc:desc];
    if (compObj)
        return @[compObj];
    
    return nil;
}

@end

// Test placement styles
@implementation MaplyVectorTileStyleText

- (id)initWithStyleEntry:(NSDictionary *)styleEntry index:(int)index;
{
    self = [super init];
    return self;
}

- (NSArray *)buildObjects:(NSArray *)vecObjs viewC:(MaplyBaseViewController *)viewC;
{
    return nil;
}

@end


@implementation MaplyVectorTiles
{
    NSMutableArray *styleObjects;
}

- (id)initWithDirectory:(NSString *)tilesDir
{
    self = [super init];
    _tilesDir = tilesDir;
    
    // Look for the styles file
    NSData *styleData = [NSData dataWithContentsOfFile:[NSString stringWithFormat:@"%@/styles.json",tilesDir]];
    if (!styleData)
        return nil;
    
    // Convert to a dictionary
    NSError *error = nil;
    NSDictionary *styleDict = [NSJSONSerialization JSONObjectWithData:styleData options:NULL error:&error];
    if (error || ![styleDict isKindOfClass:[NSDictionary class]])
        return nil;
    
    NSDictionary *paramDict = styleDict[@"parameters"];
    if (![paramDict isKindOfClass:[NSDictionary class]])
        return nil;
    _minLevel = [paramDict[@"minLevel"] integerValue];
    _maxLevel = [paramDict[@"maxLevel"] integerValue];
    
    NSArray *layers = styleDict[@"layers"];
    if (![layers isKindOfClass:[NSArray class]])
        return nil;
    _layerNames = layers;
    
    NSArray *styles = styleDict[@"styles"];
    if (![styles isKindOfClass:[NSArray class]])
        return nil;
    _styles = styles;
    
    // Set up an entry for each style.  Don't fill them in yet.
    styleObjects = [NSMutableArray array];
    for (NSDictionary *styleEntry in _styles)
        [styleObjects addObject:[NSNull null]];
    
    return self;
}

- (int)minZoom
{
    return _minLevel;
}

- (int)maxZoom
{
    return _maxLevel;
}

// Return or create the object which will create the given style
- (MaplyVectorTileStyle *)getStyle:(int)which
{
    // Note: Is this too much locking?
    @synchronized(styleObjects)
    {
        id styleObj = styleObjects[which];
        if ([styleObj isKindOfClass:[NSNull class]])
        {
            styleObj = [MaplyVectorTileStyle styleFromStyleEntry:_styles[which] index:which];
            styleObjects[which] = styleObj;
        }
        
        return styleObj;
    }
}

typedef std::map<int,NSMutableArray *> VecsForStyles;

- (void)startFetchForTile:(MaplyTileID)tileID forLayer:(MaplyQuadPagingLayer *)layer
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
       ^{
           VecsForStyles vecsForStyles;
           
           // Work through the layers we might expect to find
           // We'll collect the vectors for each of the styles we encounter
           for (NSString *layerName in _layerNames)
           {
               // Note: Debugging
//               if (![layerName isEqualToString:@"polywater"])
//                   continue;
               
               NSString *fileName = [NSString stringWithFormat:@"%@/%d/%d/%d%@",_tilesDir,tileID.level,tileID.y,tileID.x,layerName];
               MaplyVectorObject *vecObj = [MaplyVectorObject VectorObjectFromShapeFile:fileName];
               if (vecObj)
               {
                   NSArray *vecObjs = [vecObj splitVectors];
                   for (MaplyVectorObject *thisVecObj in vecObjs)
                   {
                       NSDictionary *vecDict = thisVecObj.attributes;
                       // Look through the styles
                       for (unsigned int si=0;si<100;si++)
                       {
                           NSString *styleName = [NSString stringWithFormat:@"style%d",si];
                           NSString *styleVal = vecDict[styleName];
                           if (!styleVal || [styleVal length] == 0)
                               break;
                           int styleId = [styleVal integerValue];
                           
                           // Add the vector to the appropriate spot for the style
                           VecsForStyles::iterator it = vecsForStyles.find(styleId);
                           NSMutableArray *vecsForThisStyle = nil;
                           if (it == vecsForStyles.end())
                           {
                               vecsForThisStyle = [NSMutableArray array];
                               vecsForStyles[styleId] = vecsForThisStyle;
                           } else
                               vecsForThisStyle = it->second;
                           [vecsForThisStyle addObject:thisVecObj];
                       }
                   }
               }
           }

           // Work through the styles we found, adding everything together
           for (VecsForStyles::iterator it = vecsForStyles.begin();it != vecsForStyles.end(); ++it)
           {
               // Get the style object and then add the data
               MaplyVectorTileStyle *style = [self getStyle:it->first];
               NSArray *compObjs = [style buildObjects:it->second viewC:layer.viewC];
               if (compObjs)
                   [layer addData:compObjs forTile:tileID];
           }

           
           [layer tileDidLoad:tileID];
       });
}

@end