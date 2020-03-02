/*
 *  MapboxVectorStyleBackground.mm
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

#import "vector_styles/MapboxVectorStyleBackground.h"

@implementation MapboxVectorBackgroundPaint

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    _color = [styleSet transColor:@"background-color" entry:styleEntry defVal:[UIColor blackColor]];
    if (styleEntry[@"background-image"])
    {
        NSLog(@"MapboxStyleSet: Ignoring background image");
    }
    _opacity = [styleSet transDouble:@"background-opacity" entry:styleEntry defVal:1.0];
    
    return self;
}

@end

@implementation MapboxVectorLayerBackground

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry parent:(MaplyMapboxVectorStyleLayer *)refLayer styleSet:(MapboxVectorStyleSet *)styleSet drawPriority:(int)drawPriority viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super initWithStyleEntry:styleEntry parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:viewC];
    if (!self)
        return nil;
    
    if (styleEntry[@"layout"])
    {
        NSLog(@"Ignoring background layout");
    }
    
    _paint = [[MapboxVectorBackgroundPaint alloc] initWithStyleEntry:styleEntry[@"paint"] styleSet:styleSet viewC:viewC];
    if (!_paint)
    {
        NSLog(@"Expecting paint in background layer");
        return nil;
    }
    
    // Mess directly with the opacity because we're using it for other purposes
    if (styleEntry[@"alphaoverride"])
    {
        [_paint.color setAlphaOverride:[styleEntry[@"alphaoverride"] doubleValue]];
    }

    return self;
}

- (void)buildObjects:(NSArray *)vecObjs forTile:(MaplyVectorTileData *)tileInfo viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
}

@end
