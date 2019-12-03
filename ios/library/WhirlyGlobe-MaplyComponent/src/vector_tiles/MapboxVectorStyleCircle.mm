/*
*  MapboxVectorStyleCircle.h
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

#import "MapboxVectorStyleCircle.h"

@implementation MapboxVectorCirclePaint

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    _radius = [styleSet doubleValue:@"circle-radius" dict:styleEntry defVal:5.0];
    _fillColor = [styleSet colorValue:@"circle-color" val:nil dict:styleEntry defVal:[UIColor blackColor] multiplyAlpha:false];
    _opacity = [styleSet doubleValue:@"circle-opacity" dict:styleEntry defVal:1.0];

    _strokeWidth = [styleSet doubleValue:@"circle-stroke-width" dict:styleEntry defVal:0.0];
    _strokeColor = [styleSet colorValue:@"circle-stroke-color" val:nil dict:styleEntry defVal:[UIColor blackColor] multiplyAlpha:false];
    _strokeOpacity = [styleSet doubleValue:@"circle-stroke-opacity" dict:styleEntry defVal:1.0];

    return self;
}

@end

@implementation MapboxVectorLayerCircle
{
    // TODO: When does this get cleaned up?
    MaplyTexture *circleTex;
    CGSize circleSize;
    float importance;
}

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry parent:(MaplyMapboxVectorStyleLayer *)refLayer styleSet:(MapboxVectorStyleSet *)styleSet drawPriority:(int)drawPriority viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super initWithStyleEntry:styleEntry parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:viewC];
    if (!self)
        return nil;

    // We want the texture a bit bigger than specified
    float scale = styleSet.tileStyleSettings.markerScale * 2;

    _paint = [[MapboxVectorCirclePaint alloc] initWithStyleEntry:styleEntry[@"paint"] styleSet:styleSet viewC:viewC];

    // Build an image for the circle
    float buffer = 1.0 * scale;
    float radius = _paint.radius*scale;
    float strokeWidth = _paint.strokeWidth*scale;
    float size = ceil(buffer + radius + strokeWidth)*2;
    circleSize = CGSizeMake(size / 2, size / 2);
    UIGraphicsBeginImageContext(CGSizeMake(size, size));
    // TODO: Use the opacity
    [[UIColor clearColor] setFill];
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGContextFillRect(ctx, CGRectMake(0.0, 0.0, size, size));
    
    // Outer stroke
    if (strokeWidth > 0.0) {
        CGContextBeginPath(ctx);
        CGContextAddEllipseInRect(ctx, CGRectMake(size/2.0-radius-strokeWidth, size/2.0-radius-strokeWidth, 2*(radius+strokeWidth), 2*(radius+strokeWidth)));
        [_paint.strokeColor setFill];
        CGContextDrawPath(ctx, kCGPathFill);
    }

    // Inner circle
    CGContextBeginPath(ctx);
    CGContextAddEllipseInRect(ctx, CGRectMake(size/2.0-radius, size/2.0-radius, 2*radius, 2*radius));
    [_paint.fillColor setFill];
    CGContextDrawPath(ctx, kCGPathFill);
        
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    circleTex = [viewC addTexture:image desc:nil mode:MaplyThreadCurrent];
    
    // Larger circles are slightly more important
    importance = styleSet.tileStyleSettings.markerImportance + radius / 100000.0;
    
    return self;
}

- (void)buildObjects:(NSArray *)vecObjs forTile:(MaplyVectorTileData *)tileData viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    if (!self.visible) {
        return;
    }
    
    NSMutableArray *markers = [NSMutableArray array];
    
    for (MaplyVectorObject *vecObj in vecObjs) {
        MaplyScreenMarker *marker = [[MaplyScreenMarker alloc] init];
        marker.image = circleTex;
        marker.size = circleSize;
        marker.loc = [vecObj center];
        marker.layoutImportance = importance;
        marker.selectable = self.selectable;
        marker.userObject = vecObj;
        [markers addObject:marker];
    }
    
    MaplyComponentObject *compObj = [viewC addScreenMarkers:markers desc:nil mode:MaplyThreadCurrent];
    
    [tileData addComponentObject:compObj];
}

@end

