/*
 *  MaplyVectorPolygonStyle.mm
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

#import "MaplyVectorPolygonStyle.h"

// Filled polygons styles
@implementation MaplyVectorTileStylePolygon
{
    NSMutableArray *subStyles;
}

- (id)initWithStyleEntry:(NSDictionary *)styles settings:(MaplyVectorTileStyleSettings *)settings viewC:(MaplyBaseViewController *)viewC
{
    self = [super initWithStyleEntry:styles viewC:viewC];
    
    float alpha = 1.0;
    
    NSArray *stylesArray = styles[@"substyles"];
    subStyles = [NSMutableArray array];
    for (NSDictionary *styleEntry in stylesArray)
    {
        // Build up the vector description dictionary
        if (styleEntry[@"fill-opacity"])
        {
            alpha = [styleEntry[@"fill-opacity"] floatValue];
        } else if(styleEntry[@"opacity"]) {
            alpha = [styleEntry[@"opacity"] floatValue];
        }

        int drawPriority = 0;
        if (styleEntry[@"drawpriority"])
        {
            drawPriority = (int)[styleEntry[@"drawpriority"] integerValue];
        }
        NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:
                @{kMaplyFilled: @(YES),
                 kMaplyDrawPriority: @(drawPriority+kMaplyVectorDrawPriorityDefault),
                  kMaplyVecCentered: @(true),
                  kMaplySelectable: @(self.selectable)
                 }];
        
        if (styleEntry[@"fill"])
        {
            desc[kMaplyColor] = [MaplyVectorTiles ParseColor:styleEntry[@"fill"] alpha:alpha];
        }
        desc[kMaplySelectable] = @(settings.selectable);
        /*
        if(styleEntry[@"file"])
        {
            UIImage *imageFill = [UIImage imageNamed:styleEntry[@"file"]];
            desc[kMaplyVecTexture] = imageFill;
            desc[kMaplyVecTexScaleX] = @(100);
            desc[kMaplyVecTexScaleY] = @(100);
        }
        */
        [self resolveVisibility:styleEntry settings:settings desc:desc];
        [subStyles addObject:desc];
    }
    
    return self;
}

- (NSArray *)buildObjects:(NSArray *)vecObjs forTile:(MaplyTileID)tileID layer:(MaplyQuadPagingLayer *)layer viewC:(MaplyBaseViewController *)viewC;
{
    MaplyComponentObject *baseObj = nil;
    NSMutableArray *compObjs = [NSMutableArray array];
    for (NSDictionary *desc in subStyles)
    {
        MaplyComponentObject *compObj = nil;
        if (!baseObj)
        {
            // Tesselate everything here, rather than tying up the layer thread
            NSMutableArray *tessObjs = [NSMutableArray array];
            for (MaplyVectorObject *vec in vecObjs)
            {
                MaplyVectorObject *tessVec = [vec tesselate];
                if (tessVec)
                    [tessObjs addObject:tessVec];
            }
            
            baseObj = compObj = [viewC addVectors:tessObjs desc:desc mode:MaplyThreadCurrent];
        } else {
            compObj = [viewC instanceVectors:baseObj desc:desc mode:MaplyThreadCurrent];
        }
        if (compObj)
            [compObjs addObject:compObj];
    }
    
    return compObjs;
}

@end
