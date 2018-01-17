/*
 *  MapboxVectorStyleSymbol.h
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

#import "MapboxVectorStyleSymbol.h"
#import "MaplyScreenLabel.h"

@implementation MapboxVectorSymbolLayout

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MaplyMapboxVectorStyleSet *)styleSet viewC:(MaplyBaseViewController *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    _textField = [styleSet stringValue:@"text-field" dict:styleEntry defVal:nil];
    _textMaxSize = [styleSet doubleValue:@"text-max-size" dict:styleEntry defVal:128.0];
    // Note: Missing a lot of these
    
    return self;
}

@end

@implementation MapboxVectorSymbolPaint

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MaplyMapboxVectorStyleSet *)styleSet viewC:(MaplyBaseViewController *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    _textColor = [styleSet colorValue:@"text-color" dict:styleEntry defVal:[UIColor whiteColor]];
    _textHaloColor = [styleSet colorValue:@"text-halo-color" dict:styleEntry defVal:nil];
    id sizeEntry = styleEntry[@"text-size"];
    if (sizeEntry)
    {
        if ([sizeEntry isKindOfClass:[NSNumber class]])
            _textSize = [styleSet doubleValue:sizeEntry defVal:1.0];
        else
            _textSizeFunc = [styleSet stopsValue:sizeEntry defVal:nil];
    } else
        _textSize = 24.0;

    return self;
}

@end

@implementation MapboxVectorLayerSymbol
{
    NSMutableDictionary *symbolDesc;
}

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry parent:(MaplyMapboxVectorStyleLayer *)refLayer styleSet:(MaplyMapboxVectorStyleSet *)styleSet drawPriority:(int)drawPriority viewC:(MaplyBaseViewController *)viewC
{
    self = [super initWithStyleEntry:styleEntry parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:viewC];
    if (!self)
        return nil;
    
    _layout = [[MapboxVectorSymbolLayout alloc] initWithStyleEntry:styleEntry[@"layout"] styleSet:styleSet viewC:viewC];
    _paint = [[MapboxVectorSymbolPaint alloc] initWithStyleEntry:styleEntry[@"paint"] styleSet:styleSet viewC:viewC];

    if (!_layout)
    {
        NSLog(@"Expecting layout in symbol layer.");
        return nil;
    }
    if (!_paint)
    {
        NSLog(@"Expecting paint in symbol layer.");
        return nil;
    }
    
    // Note: Need to look up the font
    UIFont *font = [UIFont systemFontOfSize:_paint.textSize];
    
    symbolDesc = [NSMutableDictionary dictionaryWithDictionary:
                  @{kMaplyTextColor: _paint.textColor,
                    kMaplyFont: font,
                    kMaplyFade: @(0.0),
                    kMaplyEnable: @(NO)
                    }];
    if (_paint.textHaloColor)
    {
        symbolDesc[kMaplyTextOutlineColor] = _paint.textHaloColor;
        // Note: Pick this up from the spec
        symbolDesc[kMaplyTextOutlineSize] = @(2.0);
    }
    
    return self;
}

- (NSArray *)buildObjects:(NSArray *)vecObjs forTile:(MaplyTileID)tileID viewC:(MaplyBaseViewController *)viewC
{
    NSMutableArray *compObjs = [NSMutableArray array];
    
    NSDictionary *desc = symbolDesc;
    if (_paint.textSizeFunc)
    {
        double textSize = [_paint.textSizeFunc valueForZoom:tileID.level];
        if (textSize > _layout.textMaxSize)
            textSize = _layout.textMaxSize;
        UIFont *font = [UIFont systemFontOfSize:textSize];
        NSMutableDictionary *mutDesc = [NSMutableDictionary dictionaryWithDictionary:desc];
        mutDesc[kMaplyFont] = font;
        desc = mutDesc;
    } else {
        // Note: Providing a reasonable default
        UIFont *font = [UIFont systemFontOfSize:16.0];
        NSMutableDictionary *mutDesc = [NSMutableDictionary dictionaryWithDictionary:desc];
        mutDesc[kMaplyFont] = font;
        desc = mutDesc;
    }
    
    NSMutableArray *labels = [NSMutableArray array];
    for (MaplyVectorObject *vecObj in vecObjs)
    {
        // Note: Cheating
        MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
        label.loc = [vecObj center];
        label.text = vecObj.attributes[@"name"];
        label.layoutImportance = _layout.textMaxSize;
        if (label.text)
            [labels addObject:label];
        // Note: Tossing labels without text
    }
    
    MaplyComponentObject *compObj = [viewC addScreenLabels:labels desc:desc mode:MaplyThreadCurrent];
    if (compObjs)
        [compObjs addObject:compObj];
    
    return compObjs;
}

@end
