/*
 *  MaplyVectorTextStyle.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/3/14.
 *  Copyright 2011-2014 mousebird consulting
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

#import "MaplyVectorTextStyle.h"
#import "MaplyScreenLabel.h"

@interface MaplyVectorTileSubStyleText : NSObject
{
@public
    NSMutableDictionary *desc;
    float dx,dy;
    float textSize;
}

@end

@implementation MaplyVectorTileSubStyleText
@end

// Text placement styles
@implementation MaplyVectorTileStyleText
{
    NSMutableArray *subStyles;
}

- (id)initWithStyleEntry:(NSDictionary *)styles settings:(MaplyVectorTileStyleSettings *)settings viewC:(MaplyBaseViewController *)viewC
{
    self = [super initWithStyleEntry:styles viewC:viewC];

    NSArray *stylesArray = styles[@"substyles"];
    subStyles = [NSMutableArray array];
    for (NSDictionary *styleEntry in stylesArray)
    {
        MaplyVectorTileSubStyleText *subStyle = [[MaplyVectorTileSubStyleText alloc] init];
        UIColor *fillColor = [UIColor blackColor];
        if (styleEntry[@"fill"])
            fillColor = [MaplyVectorTiles ParseColor:styleEntry[@"fill"]];
        subStyle->textSize = 12.0;
        if (styleEntry[@"size"])
        {
            subStyle->textSize = [styleEntry[@"size"] floatValue];
        }
        UIFont *font = [UIFont systemFontOfSize:subStyle->textSize];
        if (styleEntry[@"face-name"])
        {
            // Note: This doesn't work all that well
            NSString *faceName = styleEntry[@"face-name"];
            UIFont *thisFont = [UIFont fontWithName:[faceName stringByReplacingOccurrencesOfString:@" " withString:@"-"] size:subStyle->textSize];
            if (thisFont)
                font = thisFont;
        }
        UIColor *outlineColor = nil;
        if (styleEntry[@"halo-fill"])
            outlineColor = [MaplyVectorTiles ParseColor:styleEntry[@"halo-fill"]];
        float outlineSize = 1.0;
        if (styleEntry[@"halo-radius"])
            outlineSize = [styleEntry[@"halo-radius"] floatValue];
        subStyle->dx = 0.0;
        if (styleEntry[@"dx"])
            subStyle->dx = [styleEntry[@"dx"] floatValue] * settings.textScale;
        subStyle->dy = 0.0;
        if (styleEntry[@"dy"])
            subStyle->dy = [styleEntry[@"dy"] floatValue] * settings.textScale;
        
        if ([styleEntry[@"tilegeom"] isEqualToString:@"add"])
            self.geomAdditive = true;
        
        subStyle->desc = [NSMutableDictionary dictionary];
        subStyle->desc[kMaplyTextColor] = fillColor;
        subStyle->desc[kMaplyFont] = font;
        if (outlineColor)
        {
            subStyle->desc[kMaplyTextOutlineColor] = outlineColor;
            subStyle->desc[kMaplyTextOutlineSize] = @(outlineSize*settings.textScale);
        }

        [self resolveVisibility:styleEntry settings:settings desc:subStyle->desc];

        [subStyles addObject:subStyle];
    }
    
    return self;
}

- (NSArray *)buildObjects:(NSArray *)vecObjs viewC:(MaplyBaseViewController *)viewC;
{
    NSMutableArray *compObjs = [NSMutableArray array];
    for (MaplyVectorTileSubStyleText *subStyle in subStyles)
    {
        // One label per object
        NSMutableArray *labels = [NSMutableArray array];
        for (MaplyVectorObject *vec in vecObjs)
        {
            MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
            // Note: HACK!
            label.text = vec.attributes[@"NAME"];
            if (!label.text)
                label.text = vec.attributes[@"name"];
            MaplyCoordinate center = [vec center];
            label.loc = center;
            if (label.text)
                [labels addObject:label];
            label.offset = CGPointMake(subStyle->dx, subStyle->dy);
            label.layoutImportance = 1.0;
            label.selectable = false;
        }

        MaplyComponentObject *compObj = [viewC addScreenLabels:labels desc:subStyle->desc];
        if (compObj)
            [compObjs addObject:compObj];
    }

    return compObjs;
}

@end
