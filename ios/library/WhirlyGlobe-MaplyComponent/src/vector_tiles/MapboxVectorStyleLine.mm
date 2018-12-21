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

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super init];
    if (!self)
        return nil;

    _visible = [styleSet boolValue:@"visibility" dict:styleEntry onValue:@"visible" defVal:true];
    _cap = (MapboxVectorLineCap)[styleSet enumValue:styleEntry[@"line-cap"] options:@[@"butt",@"round",@"square"] defVal:MBLineCapButt];
    _join = (MapboxVectorLineJoin)[styleSet enumValue:styleEntry[@"line-join"] options:@[@"bevel",@"round",@"miter"] defVal:MBLineJoinMiter];
    _miterLimit = [styleSet doubleValue:@"line-miter-limit" dict:styleEntry defVal:2.0];
    _roundLimit = [styleSet doubleValue:@"line-round-limit" dict:styleEntry defVal:1.0];
    
    return self;
}

@end

@implementation MapboxVectorLineDashArray

- (instancetype)initWithStyleEntry:(NSArray *)styleEntry styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
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

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
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
    id widthEntry = [styleSet constantSubstitution:styleEntry[@"line-width"] forField:@"line-width"];
    if (widthEntry)
    {
        if ([widthEntry isKindOfClass:[NSNumber class]])
            _width = [styleSet doubleValue:widthEntry defVal:1.0];
        else
            _widthFunc = [styleSet stopsValue:widthEntry defVal:nil];
    } else
        _width = 1.0;
    id colorEntry = [styleSet constantSubstitution:styleEntry[@"line-color"] forField:@"line-color"];
    if (colorEntry)
    {
        if ([colorEntry isKindOfClass:[NSString class]])
            _color = [styleSet colorValue:@"line-color" val:nil dict:styleEntry defVal:[UIColor blackColor] multiplyAlpha:true];
        else
            _colorFunc = [styleSet stopsValue:colorEntry defVal:nil];
    } else
        _color = [UIColor blackColor];
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
    int drawPriorityPerLevel;
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

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry parent:(MaplyMapboxVectorStyleLayer *)refLayer styleSet:(MapboxVectorStyleSet *)styleSet drawPriority:(int)drawPriority viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super initWithStyleEntry:styleEntry parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:viewC];
    if (!self)
        return nil;
    
    _layout = [[MapboxVectorLineLayout alloc] initWithStyleEntry:styleEntry[@"layout"] styleSet:styleSet viewC:viewC];
    _paint = [[MapboxVectorLinePaint alloc] initWithStyleEntry:styleEntry[@"paint"] styleSet:styleSet viewC:viewC];
    self.drawPriority = [styleSet intValue:@"drawPriority" dict:styleEntry defVal:drawPriority];
    _linearClipToBounds = [styleSet boolValue:@"linearize-clip-to-bounds" dict:styleEntry onValue:@"yes" defVal:false];
    _dropGridLines = [styleSet boolValue:@"drop-grid-lines" dict:styleEntry onValue:@"yes" defVal:false];
    _subdivToGlobe = [styleSet doubleValue:@"subdiv-to-globe" dict:styleEntry defVal:0.0];
    
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
                                                   desc:@{kMaplyTexFormat: @(MaplyImageIntRGBA),
                                                          kMaplyTexWrapY: @(MaplyImageWrapY)
                                                          }
                                                   mode:MaplyThreadCurrent];
        lineDesc = [NSMutableDictionary dictionaryWithDictionary:
                    @{kMaplyVecWidth: @(_paint.width * styleSet.tileStyleSettings.lineScale),
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
                  kMaplyDrawPriority: @(self.drawPriority),
                  kMaplyFade: @0.0,
                  kMaplyVecCentered: @YES,
                  kMaplySelectable: @NO,
                  kMaplyEnable: @NO
                  }];
    }
    if (_paint.color) {
        lineDesc[kMaplyColor] = _paint.color;
    }
    
    double fade = [styleSet doubleValue:@"fade" dict:styleEntry defVal:0.0];
    if (fade != 0.0)
        lineDesc[kMaplyFade] = @(fade);
    
    drawPriorityPerLevel = styleSet.tileStyleSettings.drawPriorityPerLevel;

    return self;
}


- (NSArray *)buildObjects:(NSArray *)vecObjs forTile:(MaplyVectorTileInfo *)tileInfo  viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    NSMutableArray *compObjs = [NSMutableArray array];

    // Note: Would be better to do this earlier
    if (!_layout.visible)
        return compObjs;
    
    // Turn into linears (if not already) and then clip to the bounds
    if (_linearClipToBounds) {
        MaplyCoordinate ll = MaplyCoordinateMake(tileInfo.geoBBox.ll.x, tileInfo.geoBBox.ll.y);
        MaplyCoordinate ur = MaplyCoordinateMake(tileInfo.geoBBox.ur.x, tileInfo.geoBBox.ur.y);
        NSMutableArray *outVecObjs = [NSMutableArray array];
        for (MaplyVectorObject *vecObj in vecObjs) {
            MaplyVectorObject *linVec = nil;
            if (_dropGridLines)
                linVec = [vecObj filterClippedEdges];
            else
                linVec = [vecObj arealsToLinears];
            MaplyVectorObject *clipVec = [linVec clipToMbr:ll upperRight:ur];
            [outVecObjs addObject:clipVec];
        }
        vecObjs = outVecObjs;
    }

    // Subdivide long-ish lines to the globe, if set
    if (_subdivToGlobe > 0.0) {
        for (MaplyVectorObject *vecObj in vecObjs) {
            [vecObj subdivideToGlobe:_subdivToGlobe];
        }
    }
    
    NSDictionary *desc = lineDesc;
    bool include = true;
    if (_paint.widthFunc)
    {
        double width = [_paint.widthFunc valueForZoom:tileInfo.tileID.level] * self.styleSet.tileStyleSettings.lineScale;
        if (width > 0.0)
        {
            NSMutableDictionary *mutDesc = [NSMutableDictionary dictionaryWithDictionary:desc];
            mutDesc[kMaplyVecWidth] = @(width);
            desc = mutDesc;
        } else
            include = false;
    }
    UIColor *color = _paint.color;
    if (_paint.colorFunc) {
        color = [_paint.colorFunc colorForZoom:tileInfo.tileID.level];
    }
    if (!color)
        color = [UIColor blackColor];
    if (include) {
        if (_paint.opacityFunc)
        {
            double opacity = [_paint.opacityFunc valueForZoom:tileInfo.tileID.level];
            if (opacity > 0.0)
            {
                color = [self.styleSet color:color withOpacity:opacity];
            } else
                include = false;
        } else if (_paint.opacity < 1.0) {
            color = [self.styleSet color:color withOpacity:_paint.opacity];
        }
    }
    NSMutableDictionary *mutDesc = [NSMutableDictionary dictionaryWithDictionary:desc];
    mutDesc[kMaplyColor] = color;
    
    if (drawPriorityPerLevel > 0) {
        mutDesc[kMaplyDrawPriority] = @(self.drawPriority + tileInfo.tileID.level * drawPriorityPerLevel);
    }
    
    desc = mutDesc;
    
    if (include)
    {
        MaplyComponentObject *compObj = [viewC addWideVectors:vecObjs desc:desc mode:MaplyThreadCurrent];
        if (compObj)
            [compObjs addObject:compObj];
    }
    
    return compObjs;
}

@end
