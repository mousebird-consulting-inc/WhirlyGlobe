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

@implementation MaplyVectorTiles
{
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

- (void)startFetchForTile:(MaplyTileID)tileID forLayer:(MaplyQuadPagingLayer *)layer
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
       ^{
           // Work through the layers we might expect to find
           for (NSString *layerName in _layerNames)
           {
               NSString *fileName = [NSString stringWithFormat:@"%@/%d/%d/%d%@",_tilesDir,tileID.level,tileID.y,tileID.x,layerName];
               MaplyVectorObject *vecObj = [MaplyVectorObject VectorObjectFromShapeFile:fileName];
               if (vecObj)
               {
                   MaplyComponentObject *compObj = [layer.viewC addVectors:@[vecObj] desc:nil];
                   [layer addData:@[compObj] forTile:tileID];
               }
           }
           
           [layer tileDidLoad:tileID];
       });
}

@end