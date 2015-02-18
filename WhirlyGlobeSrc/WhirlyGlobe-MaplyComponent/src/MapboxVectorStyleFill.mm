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

@implementation MapboxVectorFillPaint

- (id)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MaplyMapboxVectorStyleSet *)styleSet viewC:(MaplyBaseViewController *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    [styleSet unsupportedCheck:@"fill-antialias" in:@"paint_fill" styleEntry:styleEntry];
    [styleSet unsupportedCheck:@"fill-translate" in:@"paint_fill" styleEntry:styleEntry];
    [styleSet unsupportedCheck:@"fill-translate-anchor" in:@"paint_fill" styleEntry:styleEntry];
    [styleSet unsupportedCheck:@"fill-image" in:@"paint_fill" styleEntry:styleEntry];

    id fillEntry = styleEntry[@"fill-opacity"];
    if (fillEntry)
    {
        if ([fillEntry isKindOfClass:[NSNumber class]])
            _opacity = [styleSet doubleValue:fillEntry defVal:1.0];
        else
            _opacityFunc = [styleSet stopsValue:fillEntry defVal:nil];
    } else
        _opacity = 1.0;
    _color = [styleSet colorValue:@"fill-color" dict:styleEntry defVal:[UIColor blackColor]];
    _outlineColor = [styleSet colorValue:@"fill-outline-color" dict:styleEntry defVal:nil];
    
    return self;
}

@end

@implementation MapboxVectorLayerFill
{
    NSMutableDictionary *fillDesc,*outlineDesc;
}

- (id)initWithStyleEntry:(NSDictionary *)styleEntry parent:(MaplyMapboxVectorStyleLayer *)refLayer styleSet:(MaplyMapboxVectorStyleSet *)styleSet drawPriority:(int)drawPriority viewC:(MaplyBaseViewController *)viewC
{
    self = [super initWithStyleEntry:styleEntry parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:viewC];
    if (!self)
        return nil;
    
    if (styleEntry[@"layout"])
    {
        NSLog(@"Ignoring fill layout");
    }
    
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

- (NSArray *)buildObjects:(NSArray *)vecObjs forTile:(MaplyTileID)tileID viewC:(MaplyBaseViewController *)viewC
{
    NSMutableArray *compObjs = [NSMutableArray array];
    
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
            double opacity = [_paint.opacityFunc valueForZoom:tileID.level];
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
            double opacity = [_paint.opacityFunc valueForZoom:tileID.level];
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
