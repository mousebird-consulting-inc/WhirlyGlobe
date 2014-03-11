/*
 *  MaplyVectorStyle.mm
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

#import "MaplyVectorStyle.h"
#import "MaplyVectorLineStyle.h"
#import "MaplyVectorMarkerStyle.h"
#import "MaplyVectorPolygonStyle.h"
#import "MaplyVectorTextStyle.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

@implementation MaplyVectorTileStyleSettings

- (id)init
{
    self = [super init];
    _lineScale = 1.0;
    _textScale = 1.0;
    _markerScale = 1.0;
    _mapScaleScale = 1.0;
    
    return self;
}

@end

@implementation MaplyVectorTileStyle

+ (id)styleFromStyleEntry:(NSDictionary *)styleEntry settings:(MaplyVectorTileStyleSettings *)settings viewC:(MaplyBaseViewController *)viewC
{
    MaplyVectorTileStyle *tileStyle = nil;
    
    NSString *typeStr = styleEntry[@"type"];
    if ([typeStr isEqualToString:@"LineSymbolizer"])
    {
        tileStyle = [[MaplyVectorTileStyleLine alloc] initWithStyleEntry:styleEntry settings:settings viewC:viewC];
    } else if ([typeStr isEqualToString:@"PolygonSymbolizer"])
    {
        tileStyle = [[MaplyVectorTileStylePolygon alloc] initWithStyleEntry:styleEntry settings:settings viewC:viewC];
    } else if ([typeStr isEqualToString:@"TextSymbolizer"])
    {
        tileStyle = [[MaplyVectorTileStyleText alloc] initWithStyleEntry:styleEntry settings:settings viewC:viewC];
    } else if ([typeStr isEqualToString:@"MarkersSymbolizer"])
    {
        tileStyle = [[MaplyVectorTileStyleMarker alloc] initWithStyleEntry:styleEntry settings:settings viewC:viewC];
    } else {
        // Set up one that doesn't do anything
        NSLog(@"Unknown symbolizer type %@",typeStr);
        tileStyle = [[MaplyVectorTileStyle alloc] init];
    }
    
    return tileStyle;
}

- (id)initWithStyleEntry:(NSDictionary *)styleEntry viewC:(MaplyBaseViewController *)viewC
{
    self = [super init];
    _viewC = viewC;
    _uuid = styleEntry[@"uuid"];
    if ([styleEntry[@"tilegeom"] isEqualToString:@"add"])
        self.geomAdditive = true;
    
    return self;
}

- (void)resolveVisibility:(NSDictionary *)styleEntry settings:(MaplyVectorTileStyleSettings *)settings desc:(NSMutableDictionary *)desc
{
    float minVis = DrawVisibleInvalid;
    float maxVis = DrawVisibleInvalid;
    if (styleEntry[@"minscaledenom"])
    {
        float minScale = [styleEntry[@"minscaledenom"] floatValue];
        minVis = [self.viewC heightForMapScale:minScale] * settings.mapScaleScale;
        // Note: Debugging
        NSLog(@"minScale = %f, maxVis = %f",minScale,minVis);
    }
    if (styleEntry[@"maxscaledenom"])
    {
        float minScale = [styleEntry[@"maxscaledenom"] floatValue];
        maxVis = [self.viewC heightForMapScale:minScale] * settings.mapScaleScale;
        // Note: Debugging
        NSLog(@"minScale = %f, maxVis = %f",minScale,maxVis);
    }
    if (minVis != DrawVisibleInvalid)
    {
        desc[kMaplyMinVis] = @(minVis);
        if (maxVis != DrawVisibleInvalid)
            desc[kMaplyMaxVis] = @(maxVis);
        else
            desc[kMaplyMaxVis] = @(20.0);
    } else if (maxVis != DrawVisibleInvalid)
    {
        desc[kMaplyMinVis] = @(0.0);
        desc[kMaplyMaxVis] = @(maxVis);
    }
}

- (NSArray *)buildObjects:(NSArray *)vecObjs viewC:(MaplyBaseViewController *)viewC;
{
    return nil;
}

@end
