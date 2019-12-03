/*
 *  MapboxVectorStyleLine.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/17/15.
 *  Copyright 2011-2019 mousebird consulting
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

#import "vector_styles/MapboxVectorStyleLine.h"
#import "helpers/MaplyTextureBuilder.h"

@implementation MapboxVectorLineLayout

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super init];
    if (!self)
        return nil;

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

    _opacity = [styleSet transDouble:@"line-opacity" entry:styleEntry defVal:1.0];
    _width = [styleSet transDouble:@"line-width" entry:styleEntry defVal:1.0];
    _color = [styleSet transColor:@"line-color" entry:styleEntry defVal:[UIColor blackColor]];
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
    MaplyTexture *filledLineTex;
    double lineScale;
    double totLen;
    double fade;
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
        totLen = 0.0;
        double maxWidth = [_paint.width maxVal] * styleSet.tileStyleSettings.lineScale;

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
        filledLineTex = [viewC addTexture:lineImage
                                                   desc:@{kMaplyTexFormat: @(MaplyImageIntRGBA),
                                                          kMaplyTexWrapY: @(MaplyImageWrapY)
                                                          }
                                                   mode:MaplyThreadCurrent];
    }
    fade = [styleSet doubleValue:@"fade" dict:styleEntry defVal:0.0];

    lineScale = styleSet.tileStyleSettings.lineScale;

    return self;
}


- (void)buildObjects:(NSArray *)vecObjs forTile:(MaplyVectorTileData *)tileInfo  viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    if (!self.visible) {
        return;
    }

    NSMutableArray *compObjs = [NSMutableArray array];
    
    // TODO: Do level based animation instead
    float levelBias = 0.9;

    // Turn into linears (if not already) and then clip to the bounds
    if (_linearClipToBounds) {
        MaplyCoordinate ll = MaplyCoordinateMake(tileInfo.geoBounds.ll.x, tileInfo.geoBounds.ll.y);
        MaplyCoordinate ur = MaplyCoordinateMake(tileInfo.geoBounds.ur.x, tileInfo.geoBounds.ur.y);
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
    
    // TODO: Eventually we need width animation
    NSMutableDictionary *desc;
    if (filledLineTex) {
        desc = [NSMutableDictionary dictionaryWithDictionary:
                @{kMaplyVecTexture: filledLineTex,
                  kMaplyWideVecCoordType: kMaplyWideVecCoordTypeScreen,
                  // Note: Hack
                  kMaplyWideVecTexRepeatLen: @(totLen/4.0),
                  kMaplyFade: @0.0,
                  kMaplyVecCentered: @YES,
                  kMaplySelectable: @(self.selectable),
                  kMaplyEnable: @NO
                  }];
    } else {
        // Simple filled line
        desc = [NSMutableDictionary dictionaryWithDictionary:
                @{kMaplyDrawPriority: @(self.drawPriority),
                  kMaplyFade: @0.0,
                  kMaplyVecCentered: @YES,
                  kMaplySelectable: @(self.selectable),
                  kMaplyEnable: @NO
                  }];
    }
    
    UIColor *color = [self.styleSet resolveColor:_paint.color opacity:_paint.opacity forZoom:tileInfo.tileID.level+levelBias mode:MBResolveColorOpacityMultiply];
    if (color) {
        desc[kMaplyColor] = color;
    }
    double width = [_paint.width valForZoom:tileInfo.tileID.level+levelBias] * lineScale;
    if (width > 0.0) {
        desc[kMaplyVecWidth] = @(width);
    }
    if (fade != 0.0)
        desc[kMaplyFade] = @(fade);
    bool include = color != nil && width > 0.0;
    
    if (self.drawPriorityPerLevel > 0) {
        desc[kMaplyDrawPriority] = @(self.drawPriority + tileInfo.tileID.level * self.drawPriorityPerLevel);
    } else {
        desc[kMaplyDrawPriority] = @(self.drawPriority);
    }

    if (include)
    {
        MaplyComponentObject *compObj = [viewC addWideVectors:vecObjs desc:desc mode:MaplyThreadCurrent];
        if (compObj)
            [compObjs addObject:compObj];
    }
    
    [tileInfo addComponentObjects:compObjs];
}

@end
