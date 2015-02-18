/*
 *  MapboxVectorStyleLine.mm
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

#import "MapboxVectorStyleLine.h"

@implementation MapboxVectorLineLayout

- (id)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MaplyMapboxVectorStyleSet *)styleSet viewC:(MaplyBaseViewController *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    [styleSet unsupportedCheck:@"visibility" in:@"line-layout" styleEntry:styleEntry];
    
    _cap = (MapboxVectorLineCap)[styleSet enumValue:styleEntry[@"line-cap"] options:@[@"butt",@"round",@"square"] defVal:MBLineCapButt];
    _join = (MapboxVectorLineJoin)[styleSet enumValue:styleEntry[@"line-join"] options:@[@"bevel",@"round",@"miter"] defVal:MBLineJoinMiter];
    _miterLimit = [styleSet doubleValue:@"line-miter-limit" dict:styleEntry defVal:2.0];
    _roundLimit = [styleSet doubleValue:@"line-round-limit" dict:styleEntry defVal:1.0];
    
    return self;
}

@end

@implementation MapboxVectorLinePaint

- (id)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MaplyMapboxVectorStyleSet *)styleSet viewC:(MaplyBaseViewController *)viewC
{
    self = [super init];
    if (!self)
        return nil;

    [styleSet unsupportedCheck:@"line-translate" in:@"line-paint" styleEntry:styleEntry];
    [styleSet unsupportedCheck:@"line-translate-anchor" in:@"line-paint" styleEntry:styleEntry];
    [styleSet unsupportedCheck:@"line-gap-width" in:@"line-paint" styleEntry:styleEntry];
    [styleSet unsupportedCheck:@"line-blur" in:@"line-paint" styleEntry:styleEntry];
    [styleSet unsupportedCheck:@"line-image" in:@"line-paint" styleEntry:styleEntry];
    [styleSet unsupportedCheck:@"line-dasharray" in:@"line-paint" styleEntry:styleEntry];

    id opEntry = styleEntry[@"line-opacity"];
    if (opEntry)
    {
        if ([opEntry isKindOfClass:[NSNumber class]])
            _opacity = [styleSet doubleValue:opEntry defVal:1.0];
        else
            _opacityFunc = [styleSet stopsValue:opEntry defVal:nil];
    } else
        _opacity = 1.0;
    _color = [styleSet colorValue:@"line-color" dict:styleEntry defVal:[UIColor blackColor]];
    id widthEntry = [styleSet constantSubstitution:styleEntry[@"line-width"] forField:@"line-width"];
    if (widthEntry)
    {
        if ([widthEntry isKindOfClass:[NSNumber class]])
            _width = [styleSet doubleValue:widthEntry defVal:1.0];
        else
            _widthFunc = [styleSet stopsValue:widthEntry defVal:nil];
    } else
        _width = 1.0;
    
    return self;
}

@end

@implementation MapboxVectorLayerLine
{
    NSMutableDictionary *lineDesc;
}

- (id)initWithStyleEntry:(NSDictionary *)styleEntry parent:(MaplyMapboxVectorStyleLayer *)refLayer styleSet:(MaplyMapboxVectorStyleSet *)styleSet drawPriority:(int)drawPriority viewC:(MaplyBaseViewController *)viewC
{
    self = [super initWithStyleEntry:styleEntry parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:viewC];
    if (!self)
        return nil;
    
    _layout = [[MapboxVectorLineLayout alloc] initWithStyleEntry:styleEntry[@"layout"] styleSet:styleSet viewC:viewC];
    _paint = [[MapboxVectorLinePaint alloc] initWithStyleEntry:styleEntry[@"paint"] styleSet:styleSet viewC:viewC];
    
    if (!_paint)
    {
        NSLog(@"Expecting paint in line layer");
        return nil;
    }
    
    lineDesc = [NSMutableDictionary dictionaryWithDictionary:
            @{kMaplyVecWidth: @(_paint.width),
              kMaplyColor: _paint.color,
              kMaplyDrawPriority: @(self.drawPriority),
              kMaplyFade: @0.0,
              kMaplyVecCentered: @YES,
              kMaplySelectable: @NO,
              kMaplyEnable: @NO
              }];
    
    return self;
}


- (NSArray *)buildObjects:(NSArray *)vecObjs forTile:(MaplyTileID)tileID  viewC:(MaplyBaseViewController *)viewC
{
    NSMutableArray *compObjs = [NSMutableArray array];
    
    NSDictionary *desc = lineDesc;
    bool include = true;
    if (_paint.widthFunc)
    {
        double width = [_paint.widthFunc valueForZoom:tileID.level];
        if (width > 0.0)
        {
            NSMutableDictionary *mutDesc = [NSMutableDictionary dictionaryWithDictionary:desc];
            mutDesc[kMaplyVecWidth] = @(width);
            desc = mutDesc;
        } else
            include = false;
    }
    if (_paint.opacityFunc && include)
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
        MaplyComponentObject *compObj = [viewC addWideVectors:vecObjs desc:desc mode:MaplyThreadCurrent];
        if (compObj)
            [compObjs addObject:compObj];
    }
    
    return compObjs;
}

@end
