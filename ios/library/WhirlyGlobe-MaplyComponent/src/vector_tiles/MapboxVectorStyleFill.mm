/*
 *  MapboxVectorStyleFill.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/17/15.
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

#import "vector_styles/MapboxVectorStyleFill.h"

@implementation MapboxVectorFillLayout

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super init];
    if (!self)
        return nil;
        
    return self;
}

@end

@implementation MapboxVectorFillPaint

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    [styleSet unsupportedCheck:@"fill-antialias" in:@"paint_fill" styleEntry:styleEntry];
    [styleSet unsupportedCheck:@"fill-translate" in:@"paint_fill" styleEntry:styleEntry];
    [styleSet unsupportedCheck:@"fill-translate-anchor" in:@"paint_fill" styleEntry:styleEntry];
    [styleSet unsupportedCheck:@"fill-image" in:@"paint_fill" styleEntry:styleEntry];

    _opacity = [styleSet transDouble:@"fill-opacity" entry:styleEntry defVal:1.0];
    _color = [styleSet transColor:@"fill-color" entry:styleEntry defVal:nil];
    _outlineColor = [styleSet transColor:@"fill-outline-color" entry:styleEntry defVal:nil];
    
    return self;
}

@end

@implementation MapboxVectorLayerFill
{
    NSString *arealShaderName;
}

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry parent:(MaplyMapboxVectorStyleLayer *)refLayer styleSet:(MapboxVectorStyleSet *)styleSet drawPriority:(int)drawPriority viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super initWithStyleEntry:styleEntry parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:viewC];
    if (!self)
        return nil;
    
    _layout = [[MapboxVectorFillLayout alloc] initWithStyleEntry:styleEntry[@"layout"] styleSet:styleSet viewC:viewC];
    _paint = [[MapboxVectorFillPaint alloc] initWithStyleEntry:styleEntry[@"paint"] styleSet:styleSet viewC:viewC];
    if (!_paint)
    {
        NSLog(@"Expecting paint in fill layer");
        return nil;
    }
    
    if (styleSet.tileStyleSettings.arealShaderName)
        arealShaderName = styleSet.tileStyleSettings.arealShaderName;

    return self;
}

- (void)buildObjects:(NSArray *)vecObjs forTile:(MaplyVectorTileData *)tileInfo viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    if (!self.visible) {
        return;
    }

    NSMutableArray *compObjs = [NSMutableArray array];
    
    // Filled polygons
    if (_paint.color)
    {
        NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:
                     @{kMaplyFilled: @(YES),
                       kMaplyVecCentered: @(true),
                       kMaplySelectable: @(self.selectable),
                       kMaplyEnable: @(NO)
                      }];
        if (arealShaderName)
            desc[kMaplyShader] = arealShaderName;

        NSMutableArray *tessVecObjs = [NSMutableArray array];
        for (MaplyVectorObject *vecObj in vecObjs)
        {
            MaplyVectorObject *tessVecObj = [vecObj tesselate];
            if (tessVecObj)
                [tessVecObjs addObject:tessVecObj];
        }
        
        bool include = true;
        UIColor *color = [self.styleSet resolveColor:_paint.color opacity:_paint.opacity forZoom:tileInfo.tileID.level mode:MBResolveColorOpacityMultiply];
        if (color) {
            // For fill we want the alpha multiplied through
            // TODO: When we do this, it's a mess.  Need to track colors more exactly (some have alpha, some don't)
//            CGFloat red,green,blue,alpha;
//            [color getRed:&red green:&green blue:&blue alpha:&alpha];
//            color = [UIColor colorWithRed:red*alpha green:green*alpha blue:blue*alpha alpha:alpha];
            desc[kMaplyColor] = color;
            include = true;
        }
        
        
        if (self.drawPriorityPerLevel > 0) {
            desc[kMaplyDrawPriority] = @(self.drawPriority + tileInfo.tileID.level * self.drawPriorityPerLevel);
        } else {
            desc[kMaplyDrawPriority] = @(self.drawPriority);
        }
                
        if (include)
        {
            MaplyComponentObject *compObj = [viewC addVectors:tessVecObjs desc:desc mode:MaplyThreadCurrent];
            if (compObj)
                [compObjs addObject:compObj];
        }
    }
    
    // Outline
    if (_paint.outlineColor)
    {
        NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:
                    @{kMaplyFilled: @(NO),
                      kMaplyVecCentered: @(true),
                      kMaplySelectable: @(self.selectable),
                      kMaplyEnable: @(NO)
                      }];

        bool include = true;
        UIColor *color = [self.styleSet resolveColor:_paint.outlineColor opacity:nil forZoom:tileInfo.tileID.level mode:MBResolveColorOpacityMultiply];
        if (color) {
            desc[kMaplyColor] = color;
            include = true;
        }

        if (self.drawPriorityPerLevel > 0) {
            desc[kMaplyDrawPriority] = @(self.drawPriority + tileInfo.tileID.level * self.drawPriorityPerLevel+1);
        } else {
            desc[kMaplyDrawPriority] = @(self.drawPriority+1);
        }

        if (include)
        {
            MaplyComponentObject *compObj = [viewC addVectors:vecObjs desc:desc mode:MaplyThreadCurrent];
            if (compObj)
                [compObjs addObject:compObj];
        }
    }
    
    [tileInfo addComponentObjects:compObjs];
}

@end
