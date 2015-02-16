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

typedef enum {
  TextPlacementPoint,
  TextPlacementLine,
  TextPlacementVertex,
  TextPlacementInterior
} TextSymbolizerPlacement;

typedef enum {
    TextTransformNone,
    TextTransformUppercase,
    TextTransformLowercase,
    TextTransformCapitalize
} TextSymbolizerTextTransform;

@interface MaplyVectorTileSubStyleText : NSObject
{
@public
    NSMutableDictionary *desc;
    float dx,dy;
    float textSize;
    TextSymbolizerPlacement placement;
    TextSymbolizerTextTransform textTransform;
    NSString *textField;
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
        float alpha = 1.0;
        if(styleEntry[@"opacity"])
            alpha = [styleEntry[@"opacity"] floatValue];
        
        UIColor *fillColor = [UIColor blackColor];
        if (styleEntry[@"fill"])
            fillColor = [MaplyVectorTiles ParseColor:styleEntry[@"fill"] alpha:alpha];
        subStyle->textSize = 12.0;
        if (styleEntry[@"size"])
        {
            subStyle->textSize = [styleEntry[@"size"] floatValue];
        }
        UIFont *font = nil;
        if (settings.fontName)
            font = [UIFont fontWithName:settings.fontName size:subStyle->textSize];
        if (!font)
            font = [UIFont systemFontOfSize:subStyle->textSize];
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
            outlineColor = [MaplyVectorTiles ParseColor:styleEntry[@"halo-fill"] alpha:alpha];
        float outlineSize = 1.0;
        if (styleEntry[@"halo-radius"])
            outlineSize = [styleEntry[@"halo-radius"] floatValue];
        subStyle->dx = 0.0;
        if (styleEntry[@"dx"])
            subStyle->dx = [styleEntry[@"dx"] floatValue] * settings.textScale;
        subStyle->dy = 0.0;
        if (styleEntry[@"dy"])
            subStyle->dy = [styleEntry[@"dy"] floatValue] * settings.textScale;
        
        subStyle->placement = TextPlacementPoint;
        if(styleEntry[@"placement"])
        {
            NSString *placement = styleEntry[@"placement"];
            if([placement isEqualToString:@"line"])
                subStyle->placement = TextPlacementLine;
            else if([placement isEqualToString:@"point"])
                subStyle->placement = TextPlacementPoint;
            else if([placement isEqualToString:@"interior"])
                subStyle->placement = TextPlacementInterior;
            else if([placement isEqualToString:@"vertex"])
                subStyle->placement = TextPlacementVertex;
        }
        
        subStyle->textTransform = TextTransformNone;
        if(styleEntry[@"text-transform"])
        {
            if([styleEntry[@"text-transform"] isEqualToString:@"uppercase"])
                subStyle->textTransform = TextTransformUppercase;
            else if([styleEntry[@"text-transform"] isEqualToString:@"lowercase"])
                subStyle->textTransform = TextTransformLowercase;
            else if([styleEntry[@"text-transform"] isEqualToString:@"capitalize"])
                subStyle->textTransform = TextTransformCapitalize;
        }
        
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
        // Just turn fade off for these
        subStyle->desc[kMaplyFade] = @(0.0);

        [self resolveVisibility:styleEntry settings:settings desc:subStyle->desc];
        
        if(styleEntry[@"value"])
            subStyle->textField = styleEntry[@"value"];
        else
            subStyle->textField = @"[name]";
        
        [subStyles addObject:subStyle];
    }
    
    return self;
}

- (NSArray *)buildObjects:(NSArray *)vecObjs forTile:(MaplyTileID)tileID layer:(MaplyQuadPagingLayer *)layer viewC:(MaplyBaseViewController *)viewC;
{
    MaplyCoordinateSystem *displaySystem = viewC.coordSystem;
    
    NSMutableArray *compObjs = [NSMutableArray array];
    for (MaplyVectorTileSubStyleText *subStyle in subStyles)
    {
        // One label per object
        NSMutableArray *labels = [NSMutableArray array];
        for (MaplyVectorObject *vec in vecObjs)
        {
            MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
            label.selectable = self.selectable;
            label.text = [self formatText:subStyle->textField forObject:vec];
            switch (subStyle->textTransform)
            {
                case TextTransformCapitalize:
                    label.text = [label.text capitalizedString];
                    break;
                case TextTransformLowercase:
                    label.text = [label.text lowercaseString];
                    break;
                case TextTransformUppercase:
                    label.text = [label.text uppercaseString];
                    break;
                default:
                    break;
            }
            label.layoutPlacement = kMaplyLayoutCenter | kMaplyLayoutRight | kMaplyLayoutLeft | kMaplyLayoutAbove | kMaplyLayoutBelow;
            if (label.text)
            {
                if(subStyle->placement == TextPlacementPoint ||
                   subStyle->placement == TextPlacementInterior)
                {
                    MaplyCoordinate center = [vec center];
                    label.loc = center;
                } else if (subStyle->placement == TextPlacementLine)
                {
                    MaplyCoordinate middle;
                    double rot;
                    if ([vec linearMiddle:&middle rot:&rot displayCoordSys:displaySystem])
                    {
                        //TODO: text-max-char-angle-delta
                        //TODO: rotation calculation is not ideal, it is between 2 points, but it needs to be avergared over a longer distance
                        label.loc = middle;
                        label.layoutPlacement = kMaplyLayoutCenter;
                        label.rotation = rot+M_PI/2.0;
                        label.keepUpright = true;
                    } else {
                        label = nil;
                    }
                } else if(subStyle->placement == TextPlacementVertex)
                {
                    MaplyCoordinate vertex;
                    if([vec middleCoordinate:&vertex]) {
                        label.loc = vertex;
                    } else {
                        label = nil;
                    }
                }

                if(label)
                {
                    label.offset = CGPointMake(subStyle->dx, subStyle->dy);
                    // Make bigger text slightly more important
                    label.layoutImportance = 1.0 + subStyle->textSize/1000;
                    label.selectable = false;
                    [labels addObject:label];
                }
            }
        }

        // Note: This should be MaplyThreadCurrent, but...
        //   We need a GL context present for the text rendering
        MaplyComponentObject *compObj = [viewC addScreenLabels:labels desc:subStyle->desc mode:MaplyThreadAny];
        if (compObj)
            [compObjs addObject:compObj];
    }

    return compObjs;
}

@end
