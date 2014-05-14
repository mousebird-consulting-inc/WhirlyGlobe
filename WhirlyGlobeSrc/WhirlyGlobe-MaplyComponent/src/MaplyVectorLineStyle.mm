/*
 *  MaplyVectorLineStyle.mm
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

#import "MaplyVectorLineStyle.h"

// Line styles
@implementation MaplyVectorTileStyleLine
{
    NSMutableArray *subStyles;
}

- (id)initWithStyleEntry:(NSDictionary *)style settings:(MaplyVectorTileStyleSettings *)settings viewC:(MaplyBaseViewController *)viewC
{
    self = [super initWithStyleEntry:style viewC:viewC];

    subStyles = [NSMutableArray array];
    NSArray *subStylesArray = style[@"substyles"];
    for (NSDictionary *styleEntry in subStylesArray)
    {
        float strokeWidth = 1.0;
        int red = 255,green = 255,blue = 255;
        float alpha = 1.0;
        
        // Build up the vector description dictionary
        if (styleEntry[@"stroke-width"])
            strokeWidth = [styleEntry[@"stroke-width"] floatValue];
        if (styleEntry[@"stroke-opacity"])
        {
            alpha = [styleEntry[@"stroke-opacity"] floatValue];
        } else if(styleEntry[@"opacity"]) {
            alpha = [styleEntry[@"opacity"] floatValue];
        }
        if (styleEntry[@"stroke"])
        {
            NSString *colorStr = styleEntry[@"stroke"];
            // parse the hex
            NSScanner *scanner = [NSScanner scannerWithString:colorStr];
            unsigned int colorVal;
            [scanner setScanLocation:1]; // bypass #
            [scanner scanHexInt:&colorVal];
            blue = colorVal & 0xFF;
            green = (colorVal >> 8) & 0xFF;
            red = (colorVal >> 16) & 0xFF;
        }
        int drawPriority = 0;
        if (styleEntry[@"drawpriority"])
        {
            drawPriority = (int)[styleEntry[@"drawpriority"] integerValue];
        }
        NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:
                @{kMaplyVecWidth: @(settings.lineScale * strokeWidth),
                 kMaplyColor: [UIColor colorWithRed:red/255.0*alpha green:green/255.0*alpha blue:blue/255.0*alpha alpha:alpha],
                 kMaplyDrawPriority: @(drawPriority+kMaplyVectorDrawPriorityDefault),
                 kMaplyEnable: @(NO),
                  kMaplyFade: @(0.0),
                  kMaplyVecCentered: @(true),
                  kMaplySelectable: @(self.selectable)
                 }];
        [self resolveVisibility:styleEntry settings:settings desc:desc];

        [subStyles addObject:desc];
    }
    
    return self;
}

- (NSArray *)buildObjects:(NSArray *)vecObjs viewC:(MaplyBaseViewController *)viewC;
{
    MaplyComponentObject *baseObj = nil;
    NSMutableArray *compObjs = [NSMutableArray array];
    for (NSDictionary *desc in subStyles)
    {
        MaplyComponentObject *compObj = nil;
        if (!baseObj)
            baseObj = compObj = [viewC addVectors:vecObjs desc:desc mode:MaplyThreadCurrent];
        else
            // Note: Should do current thread here
            compObj = [viewC instanceVectors:baseObj desc:desc mode:MaplyThreadCurrent];
        if (compObj)
            [compObjs addObject:compObj];
    }
    
    return compObjs;
}

@end
