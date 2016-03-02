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

#import "MapboxVectorStyleBackground.h"

@implementation MapboxVectorBackgroundPaint

- (id)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MaplyMapboxVectorStyleSet *)styleSet viewC:(MaplyBaseViewController *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    _color = [styleSet colorValue:@"background-color" dict:styleEntry defVal:[UIColor blackColor]];
    if (styleEntry[@"background-image"])
    {
        NSLog(@"MapboxStyleSet: Ignoring background image");
    }
    _opacity = [styleSet doubleValue:@"background-opacity" dict:styleEntry defVal:1.0];
    
    return self;
}

@end

@implementation MapboxVectorLayerBackground

- (id)initWithStyleEntry:(NSDictionary *)styleEntry parent:(MaplyMapboxVectorStyleLayer *)refLayer styleSet:(MaplyMapboxVectorStyleSet *)styleSet drawPriority:(int)drawPriority viewC:(MaplyBaseViewController *)viewC
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
    
    return self;
}

- (NSArray *)buildObjects:(NSArray *)vecObjs forTile:(MaplyTileID)tileID viewC:(MaplyBaseViewController *)viewC
{
    return nil;
}

@end
