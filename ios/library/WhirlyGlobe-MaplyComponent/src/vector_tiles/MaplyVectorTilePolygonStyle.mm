/*  MaplyVectorPolygonStyle.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/3/14.
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

#import "control/WhirlyGlobeViewController.h"
#import "vector_styles/MaplyVectorTilePolygonStyle.h"
#import "vector_tiles/MapboxVectorTiles.h"
#import "NSDictionary+Stuff.h"

// Filled polygons styles
@implementation MaplyVectorTileStylePolygon
{
    NSMutableArray *subStyles;
}

- (instancetype)initWithStyleEntry:(NSDictionary *)styles settings:(MaplyVectorStyleSettings *)settings viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
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
            desc[kMaplyColor] = [MaplyVectorTileStyle ParseColor:styleEntry[@"fill"] alpha:alpha];
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

- (void)buildObjects:(NSArray *)vecObjs
             forTile:(MaplyVectorTileData *)tileInfo
               viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
                desc:(NSDictionary * _Nullable)extraDesc
{
    [self buildObjects:vecObjs forTile:tileInfo viewC:viewC desc:extraDesc cancelFn:nil];
}

/// Construct objects related to this style based on the input data.
- (void)buildObjects:(NSArray * _Nonnull)vecObjs
             forTile:(MaplyVectorTileData * __nonnull)tileData
               viewC:(NSObject<MaplyRenderControllerProtocol> * _Nonnull)viewC
                desc:(NSDictionary * _Nullable)extraDesc
            cancelFn:(bool(^__nullable)(void))cancelFn
{
    MaplyComponentObject *baseObj = nil;
    NSMutableArray *compObjs = [NSMutableArray array];

    const float ClipGridSize = 2.0/180.0*M_PI;

    for (__strong NSDictionary *desc in subStyles)
    {
        MaplyComponentObject *compObj = nil;
        if (!baseObj)
        {
            if (extraDesc)
            {
                desc = [desc dictionaryByMergingWith:extraDesc];
            }
            
            // Tesselate everything here, rather than tying up the layer thread
            NSMutableArray *tessObjs = [NSMutableArray array];
            for (MaplyVectorObject *vec in vecObjs)
            {
                //MaplyVectorObject *tessVec = [vec tesselate];
                
                MaplyCoordinate center = [vec centroid];
                vec.attributes[kMaplyVecCenterX] = @(center.x);
                vec.attributes[kMaplyVecCenterY] = @(center.y);
                
                MaplyVectorObject *tessVec = nil;
                if ([viewC isKindOfClass:[WhirlyGlobeViewController class]])
                    tessVec = [[vec clipToGrid:CGSizeMake(ClipGridSize, ClipGridSize)] tesselate];
                else
                    tessVec = [vec tesselate];
                
                if (tessVec)
                {
                    [tessObjs addObject:tessVec];
                }
            }
            
            baseObj = compObj = [viewC addVectors:tessObjs desc:desc mode:MaplyThreadCurrent];
        }
        else
        {
            compObj = [viewC instanceVectors:baseObj desc:desc mode:MaplyThreadCurrent];
        }
        if (compObj)
        {
            [compObjs addObject:compObj];
        }
    }
    
    [tileData addComponentObjects:compObjs];
}

@end
