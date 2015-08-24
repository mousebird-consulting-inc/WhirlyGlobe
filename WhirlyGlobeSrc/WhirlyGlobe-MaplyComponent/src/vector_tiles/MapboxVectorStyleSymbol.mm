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
    _textMaxSize = [[MaplyMapboxValueWrapper alloc] initWithDict:styleEntry name:@"text-max-size" dataType:MaplyMapboxValueTypeNumber styleSet:styleSet];
    if (!_textMaxSize)
        _textMaxSize = [[MaplyMapboxValueWrapper alloc] initWithObject:@(24.0)];
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

    _textOpacity = [[MaplyMapboxValueWrapper alloc] initWithDict:styleEntry name:@"text-opacity" dataType:MaplyMapboxValueTypeNumber styleSet:styleSet];
    if (!_textOpacity)
        _textOpacity = [[MaplyMapboxValueWrapper alloc] initWithObject:@(1.0)];
    _textColor = [[MaplyMapboxValueWrapper alloc] initWithDict:styleEntry name:@"text-color" dataType:MaplyMapboxValueTypeColor styleSet:styleSet];
    if (!_textColor)
        _textColor = [[MaplyMapboxValueWrapper alloc] initWithObject:[UIColor blackColor]];
    _textHaloColor = [[MaplyMapboxValueWrapper alloc] initWithDict:styleEntry name:@"text-halo-color" dataType:MaplyMapboxValueTypeColor styleSet:styleSet];
    _textSize = [[MaplyMapboxValueWrapper alloc] initWithDict:styleEntry name:@"text-size" dataType:MaplyMapboxValueTypeNumber styleSet:styleSet];
    if (!_textSize)
        _textSize = [[MaplyMapboxValueWrapper alloc] initWithObject:@(24.0)];
    
    return self;
}

@end

@implementation MapboxVectorLayerSymbol
{
    NSMutableDictionary *symbolDesc;
//    UIFont *font;
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
    
    // Allocate the largest font we need
    // Note: Need to look up the font name
//    font = [UIFont systemFontOfSize:[_paint.textSize maxNumber]];
    
    symbolDesc = [NSMutableDictionary dictionaryWithDictionary:
                  @{kMaplyFade: @(0.0),
                    kMaplyEnable: @(NO)
                    }];
    if (_paint.textHaloColor)
    {
        // Note: Pick this up from the spec
        symbolDesc[kMaplyTextOutlineSize] = @(2.0);
    }
    
    return self;
}

- (NSArray *)buildObjects:(NSArray *)vecObjs forTile:(MaplyTileID)tileID viewC:(MaplyBaseViewController *)viewC
{
    if (tileID.level < self.minzoom || tileID.level > self.maxzoom)
        return nil;
    
    NSMutableArray *compObjs = [NSMutableArray array];
    
    NSMutableDictionary *mutDesc = [NSMutableDictionary dictionaryWithDictionary:symbolDesc];

    bool include = true;
    
    double opacity = [_paint.textOpacity numberForZoom:tileID.level styleSet:self.styleSet];
    if (opacity <= 0.0)
        include = false;
    double textSize = [_paint.textSize numberForZoom:tileID.level styleSet:self.styleSet];
    if (textSize <= 1.0)
        include = false;
    // Note: Should pre-allocate these or at least use a cache
    UIFont *font = [UIFont systemFontOfSize:textSize];
    mutDesc[kMaplyFont] = font;
    mutDesc[kMaplyTextColor] = [self.styleSet color:[_paint.textColor colorForZoom:tileID.level styleSet:self.styleSet] withOpacity:opacity];

    if (_paint.textHaloColor)
    {
        mutDesc[kMaplyTextOutlineColor] = [self.styleSet color:[_paint.textHaloColor colorForZoom:tileID.level styleSet:self.styleSet] withOpacity:opacity];
    }
    
    if (include)
    {
        NSMutableArray *labels = [NSMutableArray array];
        for (MaplyVectorObject *vecObj in vecObjs)
        {
            MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
            label.loc = [vecObj center];
            // Note: Should do name lookup here
            label.text = vecObj.attributes[@"name"];
            label.layoutImportance = MAXFLOAT;
            if (!label.text)
                label.text = vecObj.attributes[@"name_en"];
            if (!label)
                NSLog(@"Missing text for label");
            if (label.text)
                [labels addObject:label];
        }
        MaplyComponentObject *compObj = [viewC addScreenLabels:labels desc:mutDesc mode:MaplyThreadCurrent];
        if (compObj)
            [compObjs addObject:compObj];
    }

    return compObjs;
}

@end
