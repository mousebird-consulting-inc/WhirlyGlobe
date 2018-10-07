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

#import "MapboxVectorStyleFill.h"

@implementation MapboxVectorFillLayout

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    _visible = [styleSet boolValue:@"visibility" dict:styleEntry onValue:@"visible" defVal:true];
    
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

    _opacityBase = 1.0;
    id fillEntry = styleEntry[@"fill-opacity"];
    if (fillEntry)
    {
        if ([fillEntry isKindOfClass:[NSNumber class]])
            _opacity = [styleSet doubleValue:fillEntry defVal:1.0];
        else {
            _opacityFunc = [styleSet stopsValue:fillEntry defVal:nil];
        }
    } else
        _opacity = 1.0;
    _color = [styleSet colorValue:@"fill-color" val:nil dict:styleEntry defVal:[UIColor blackColor] multiplyAlpha:true];
    _outlineColor = [styleSet colorValue:@"fill-outline-color" val:nil dict:styleEntry defVal:nil multiplyAlpha:true];
    
    return self;
}

@end

@implementation MapboxVectorLayerFill
{
    NSMutableDictionary *fillDesc,*outlineDesc;
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
    
    if (_paint.color)
    {
        fillDesc = [NSMutableDictionary dictionaryWithDictionary:
                     @{kMaplyFilled: @(YES),
                       kMaplyDrawPriority: @(self.drawPriority),
                       kMaplyVecCentered: @(true),
                       kMaplyColor: [styleSet color:_paint.color withOpacity:_paint.opacity],
                       kMaplySelectable: @(false),
                       kMaplyEnable: @(NO)
                      }];
        if (styleSet.tileStyleSettings.arealShaderName)
            fillDesc[kMaplyShader] = styleSet.tileStyleSettings.arealShaderName;
    }
    
    if (_paint.outlineColor)
    {
        outlineDesc = [NSMutableDictionary dictionaryWithDictionary:
                    @{kMaplyFilled: @(NO),
                      kMaplyDrawPriority: @(self.drawPriority+1),
                      kMaplyVecCentered: @(true),
                      kMaplyColor: [styleSet color:_paint.outlineColor withOpacity:_paint.opacity],
                      kMaplySelectable: @(false),
                      kMaplyEnable: @(NO)
                      }];
    }
    
    return self;
}

- (NSArray *)buildObjects:(NSArray *)vecObjs forTile:(MaplyVectorTileInfo *)tileInfo viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    NSMutableArray *compObjs = [NSMutableArray array];
    
    // Note: Would be better to do this earlier
    if (!_layout.visible)
        return compObjs;

    // Filled polygons
    if (fillDesc)
    {
        NSMutableArray *tessVecObjs = [NSMutableArray array];
        for (MaplyVectorObject *vecObj in vecObjs)
        {
            MaplyVectorObject *tessVecObj = [vecObj tesselate];
            if (tessVecObj)
                [tessVecObjs addObject:tessVecObj];
        }
        
        NSDictionary *desc = fillDesc;
        bool include = true;
        if (_paint.opacityFunc)
        {
            double opacity = [_paint.opacityFunc valueForZoom:tileInfo.tileID.level];
            if (opacity > 0.0)
            {
                UIColor *color = [self.styleSet color:_paint.color withOpacity:opacity];
                NSMutableDictionary *mutDesc = [NSMutableDictionary dictionaryWithDictionary:desc];
                mutDesc[kMaplyColor] = color;
                desc = mutDesc;
            } else
                include = false;
        }
        
        if (include)
        {
            MaplyComponentObject *compObj = [viewC addVectors:tessVecObjs desc:desc mode:MaplyThreadCurrent];
            if (compObj)
                [compObjs addObject:compObj];
        }
    }
    
    // Outline
    if (outlineDesc)
    {
        NSDictionary *desc = outlineDesc;
        bool include = true;
        // Note: Does opacity apply here?
        if (_paint.opacityFunc)
        {
            double opacity = [_paint.opacityFunc valueForZoom:tileInfo.tileID.level];
            if (opacity > 0.0)
            {
                UIColor *color = [self.styleSet color:_paint.outlineColor withOpacity:opacity];
                NSMutableDictionary *mutDesc = [NSMutableDictionary dictionaryWithDictionary:desc];
                mutDesc[kMaplyColor] = color;
                desc = mutDesc;
            } else
                include = false;
        }

        if (include)
        {
            MaplyComponentObject *compObj = [viewC addVectors:vecObjs desc:desc mode:MaplyThreadCurrent];
            if (compObj)
                [compObjs addObject:compObj];
        }
    }
    
    return compObjs;
}

@end
