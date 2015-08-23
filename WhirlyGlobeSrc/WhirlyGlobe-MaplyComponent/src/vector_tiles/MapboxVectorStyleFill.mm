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

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MaplyMapboxVectorStyleSet *)styleSet viewC:(MaplyBaseViewController *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    [styleSet unsupportedCheck:@"fill-antialias" in:@"paint_fill" styleEntry:styleEntry];
    [styleSet unsupportedCheck:@"fill-translate" in:@"paint_fill" styleEntry:styleEntry];
    [styleSet unsupportedCheck:@"fill-translate-anchor" in:@"paint_fill" styleEntry:styleEntry];
    [styleSet unsupportedCheck:@"fill-image" in:@"paint_fill" styleEntry:styleEntry];

    _opacity = [[MaplyMapboxValueWrapper alloc] initWithDict:styleEntry name:@"fill-opacity" dataType:MaplyMapboxValueTypeNumber styleSet:styleSet];
    if (!_opacity)
        _opacity = [[MaplyMapboxValueWrapper alloc] initWithObject:@(1.0)];
    _color = [[MaplyMapboxValueWrapper alloc] initWithDict:styleEntry name:@"fill-color" dataType:MaplyMapboxValueTypeColor styleSet:styleSet];
    if (!_color)
        _color = [[MaplyMapboxValueWrapper alloc] initWithObject:[UIColor whiteColor]];
    _outlineColor = [[MaplyMapboxValueWrapper alloc] initWithDict:styleEntry name:@"fill-outline-color" dataType:MaplyMapboxValueTypeColor styleSet:styleSet];
    
    return self;
}

@end

@implementation MapboxVectorLayerFill
{
    NSMutableDictionary *fillDesc,*outlineDesc;
}

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry parent:(MaplyMapboxVectorStyleLayer *)refLayer styleSet:(MaplyMapboxVectorStyleSet *)styleSet drawPriority:(int)drawPriority viewC:(MaplyBaseViewController *)viewC
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
    
    fillDesc = [NSMutableDictionary dictionaryWithDictionary:
                 @{kMaplyFilled: @(YES),
                   kMaplyDrawPriority: @(self.drawPriority),
                   kMaplyVecCentered: @(true),
                   kMaplySelectable: @(false),
                   kMaplyEnable: @(NO)
                  }];
    
    if (_paint.outlineColor)
    {
        outlineDesc = [NSMutableDictionary dictionaryWithDictionary:
                    @{kMaplyFilled: @(NO),
                      kMaplyDrawPriority: @(self.drawPriority+1),
                      kMaplyVecCentered: @(true),
                      kMaplySelectable: @(false),
                      kMaplyEnable: @(NO)
                      }];
    }
    
    return self;
}

- (NSArray *)buildObjects:(NSArray *)vecObjs forTile:(MaplyTileID)tileID viewC:(MaplyBaseViewController *)viewC
{
    if (tileID.level < self.minzoom || tileID.level > self.maxzoom)
        return nil;

    NSMutableArray *compObjs = [NSMutableArray array];
    
    double opacity = [_paint.opacity numberForZoom:tileID.level styleSet:self.styleSet];

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
        
        bool include = true;
        if (opacity <= 0.0)
            include = false;
        
        if (include)
        {
            NSMutableDictionary *mutDesc = [NSMutableDictionary dictionaryWithDictionary:fillDesc];
            mutDesc[kMaplyColor] = [self.styleSet color:[_paint.color colorForZoom:tileID.level styleSet:self.styleSet] withOpacity:opacity ];

            MaplyComponentObject *compObj = [viewC addVectors:tessVecObjs desc:mutDesc mode:MaplyThreadCurrent];
            if (compObj)
                [compObjs addObject:compObj];
        }
    }
    
    // Outline
    if (outlineDesc)
    {
        bool include = true;
        
        if (opacity <= 0.0)
            include = false;
        
        if (include)
        {
            NSMutableDictionary *mutDesc = [NSMutableDictionary dictionaryWithDictionary:outlineDesc];
            mutDesc[kMaplyColor] = [self.styleSet color:[_paint.outlineColor colorForZoom:tileID.level styleSet:self.styleSet] withOpacity:opacity];
            
            MaplyComponentObject *compObj = [viewC addVectors:vecObjs desc:mutDesc mode:MaplyThreadCurrent];
            if (compObj)
                [compObjs addObject:compObj];
        }
    }
        
    return compObjs;
}

@end
