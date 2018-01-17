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
#import "MaplyTextureBuilder.h"

@implementation MapboxVectorLineLayout

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MaplyMapboxVectorStyleSet *)styleSet viewC:(MaplyBaseViewController *)viewC
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

@implementation MapboxVectorLineDashArray

- (instancetype)initWithStyleEntry:(NSArray *)styleEntry styleSet:(MaplyMapboxVectorStyleSet *)styleSet viewC:(MaplyBaseViewController *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    NSMutableArray *dashes = [NSMutableArray array];
    for (NSNumber *num in styleEntry)
        [dashes addObject:@([num doubleValue])];
    
    _dashes = dashes;
    
    return self;
}

@end

@implementation MapboxVectorLinePaint

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MaplyMapboxVectorStyleSet *)styleSet viewC:(MaplyBaseViewController *)viewC
{
    self = [super init];
    if (!self)
        return nil;

    [styleSet unsupportedCheck:@"line-translate" in:@"line-paint" styleEntry:styleEntry];
    [styleSet unsupportedCheck:@"line-translate-anchor" in:@"line-paint" styleEntry:styleEntry];
    [styleSet unsupportedCheck:@"line-gap-width" in:@"line-paint" styleEntry:styleEntry];
    [styleSet unsupportedCheck:@"line-blur" in:@"line-paint" styleEntry:styleEntry];
    [styleSet unsupportedCheck:@"line-image" in:@"line-paint" styleEntry:styleEntry];

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
    id dashArrayEntry = styleEntry[@"line-dasharray"];
    if (dashArrayEntry)
    {
        if ([dashArrayEntry isKindOfClass:[NSArray class]])
            _lineDashArray = [[MapboxVectorLineDashArray alloc] initWithStyleEntry:dashArrayEntry styleSet:styleSet viewC:viewC];
    }
    
    return self;
}

@end

@implementation MapboxVectorLayerLine
{
    NSMutableDictionary *lineDesc;
}

// Courtesy: http://acius2.blogspot.com/2007/11/calculating-next-power-of-2.html
static unsigned int NextPowOf2(unsigned int val)
{
    val--;
    val = (val >> 1) | val;
    val = (val >> 2) | val;
    val = (val >> 4) | val;
    val = (val >> 8) | val;
    val = (val >> 16) | val;
    
    return (val + 1);
}

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry parent:(MaplyMapboxVectorStyleLayer *)refLayer styleSet:(MaplyMapboxVectorStyleSet *)styleSet drawPriority:(int)drawPriority viewC:(MaplyBaseViewController *)viewC
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

    if (_paint.lineDashArray != nil)
    {
        NSMutableArray *dashComponents = [NSMutableArray array];
        double totLen = 0.0;
        double maxWidth = _paint.width * styleSet.tileStyleSettings.lineScale;
        if (_paint.widthFunc)
            maxWidth = [_paint.widthFunc maxValue] * styleSet.tileStyleSettings.lineScale;

        // Figure out the total length
        for (NSNumber *num in _paint.lineDashArray.dashes)
            totLen += [num doubleValue] * maxWidth;
        
        int totLenRounded = NextPowOf2(totLen);
        for (NSNumber *num in _paint.lineDashArray.dashes)
        {
            double len = [num doubleValue] * maxWidth * totLenRounded / totLen;
            [dashComponents addObject:@(len)];
        }
        
        MaplyLinearTextureBuilder *lineTexBuilder = [[MaplyLinearTextureBuilder alloc] init];
        [lineTexBuilder setPattern:dashComponents];
        UIImage *lineImage = [lineTexBuilder makeImage];
        MaplyTexture *filledLineTex = [viewC addTexture:lineImage
                                            imageFormat:MaplyImageIntRGBA
                                              wrapFlags:MaplyImageWrapY
                                                   mode:MaplyThreadCurrent];
        lineDesc = [NSMutableDictionary dictionaryWithDictionary:
                    @{kMaplyVecWidth: @(_paint.width * styleSet.tileStyleSettings.lineScale),
                      kMaplyColor: _paint.color,
                      kMaplyVecTexture: filledLineTex,
                      kMaplyWideVecCoordType: kMaplyWideVecCoordTypeScreen,
                      // Note: Hack
                      kMaplyWideVecTexRepeatLen: @(totLen/4.0),
                      kMaplyDrawPriority: @(self.drawPriority),
                      kMaplyFade: @0.0,
                      kMaplyVecCentered: @YES,
                      kMaplySelectable: @NO,
                      kMaplyEnable: @NO
                      }];

    } else {
        // Simple filled line
        lineDesc = [NSMutableDictionary dictionaryWithDictionary:
                @{kMaplyVecWidth: @(_paint.width * styleSet.tileStyleSettings.lineScale),
                  kMaplyColor: _paint.color,
                  kMaplyDrawPriority: @(self.drawPriority),
                  kMaplyFade: @0.0,
                  kMaplyVecCentered: @YES,
                  kMaplySelectable: @NO,
                  kMaplyEnable: @NO
                  }];
    }
    
    return self;
}


- (NSArray *)buildObjects:(NSArray *)vecObjs forTile:(MaplyTileID)tileID  viewC:(MaplyBaseViewController *)viewC
{
    NSMutableArray *compObjs = [NSMutableArray array];
    
    NSDictionary *desc = lineDesc;
    bool include = true;
    if (_paint.widthFunc)
    {
        double width = [_paint.widthFunc valueForZoom:tileID.level] * self.styleSet.tileStyleSettings.lineScale;
        if (width > 0.0)
        {
            NSMutableDictionary *mutDesc = [NSMutableDictionary dictionaryWithDictionary:desc];
            mutDesc[kMaplyVecWidth] = @(width/2.0);
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
