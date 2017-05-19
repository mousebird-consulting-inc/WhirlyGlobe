/*
 *  MaplyVectorPolygonStyle.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/3/14.
 *  Copyright 2011-2017 mousebird consulting
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

#import "MaplyVectorTilePolygonStyle.h"

// Filled polygons styles
@implementation MaplyVectorTileStylePolygon
{
    NSMutableArray *subStyles;
}

- (instancetype)initWithStyleEntry:(NSDictionary *)styles settings:(MaplyVectorStyleSettings *)settings viewC:(MaplyBaseViewController *)viewC
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
                  kMaplySelectable: @(self.selectable),
                  kMaplyEnable: @(NO)
                 }];
        
        if (styleEntry[@"fill"])
        {
            desc[kMaplyColor] = [MaplyVectorTiles ParseColor:styleEntry[@"fill"] alpha:alpha];
        }
        
        if (styleEntry[@"image"]) {
            UIImage *img = styleEntry[@"image"];
            MaplyTexture *texture = [viewC addTexture:img desc:@{kMaplyTexWrapX: @(YES),kMaplyTexWrapY: @(YES)} mode:MaplyThreadAny];
            desc[kMaplyVecTexture] = texture;
            desc[kMaplyVecTextureProjection] = kMaplyProjectionTangentPlane;
            
            float scaleX = 6400.0;
            float scaleY = -6400.0;
            if (styleEntry[@"width"])
                scaleX = 6400.0 / [styleEntry[@"width"] floatValue];
            if (styleEntry[@"height"])
                scaleY = -6400.0 / [styleEntry[@"height"] floatValue];
            
            desc[kMaplyVecTexScaleX] = @(scaleX);
            desc[kMaplyVecTexScaleY] = @(scaleY);
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

- (NSArray *)buildObjects:(NSArray *)vecObjs forTile:(MaplyTileID)tileID viewC:(MaplyBaseViewController *)viewC;
{
    MaplyComponentObject *baseObj = nil;
    NSMutableArray *compObjs = [NSMutableArray array];
    
    float ClipGridSize = 2.0/180.0*M_PI;
    
    for (NSDictionary *desc in subStyles)
    {
        MaplyComponentObject *compObj = nil;
        if (!baseObj)
        {
            // Tesselate everything here, rather than tying up the layer thread
            NSMutableArray *tessObjs = [NSMutableArray array];
            for (MaplyVectorObject *vec in vecObjs)
            {
                //MaplyVectorObject *tessVec = [vec tesselate];
                
                MaplyCoordinate center = [vec centroid];
                vec.attributes[kMaplyVecCenterX] = @(center.x);
                vec.attributes[kMaplyVecCenterY] = @(center.y);
                
                MaplyVectorObject *tessVec = [[vec clipToGrid:CGSizeMake(ClipGridSize, ClipGridSize)] tesselate];
                
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
