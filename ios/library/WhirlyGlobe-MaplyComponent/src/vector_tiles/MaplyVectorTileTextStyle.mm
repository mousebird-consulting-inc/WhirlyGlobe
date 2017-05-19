/*
 *  MaplyVectorTextStyle.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/3/14.
 *  Copyright 2011-2017 mousebird consulting
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

#import "MaplyVectorTileTextStyle.h"
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
    float layoutImportance;
    NSNumber *layoutPlacement;
}

@end

@implementation MaplyVectorTileSubStyleText
@end

// Text placement styles
@implementation MaplyVectorTileStyleText
{
    NSMutableArray *subStyles;
}

- (instancetype)initWithStyleEntry:(NSDictionary *)styles settings:(MaplyVectorStyleSettings *)settings viewC:(MaplyBaseViewController *)viewC
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
            
        } else if (styleEntry[@"font-family"] || styleEntry[@"font-style"] || styleEntry[@"font-weight"] || styleEntry[@"font-size"]) {
            
            NSString *fontFamily = styleEntry[@"font-family"];
            NSString *fontStyle = styleEntry[@"font-style"];
            NSString *fontWeight= styleEntry[@"font-weight"];
            NSString *fontSize = styleEntry[@"font-size"];
            
            bool italic = false;
            bool bold = false;
            
            if (!fontFamily)
                fontFamily = [font familyName];
            
            if (fontStyle && ([fontStyle isEqualToString:@"italic"] || [fontStyle isEqualToString:@"oblique"]))
                italic = true;
            
            if (fontWeight && !([fontWeight isEqualToString:@"normal"] || [fontWeight isEqualToString:@"lighter"]))
                bold = true;
            
            if (fontSize) {
                
                fontSize = [fontSize stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
                
                if ([fontSize isEqualToString:@"xx-small"])
                    subStyle->textSize = 8.0;
                else if ([fontSize isEqualToString:@"x-small"])
                    subStyle->textSize = 10.0;
                else if ([fontSize isEqualToString:@"small"])
                    subStyle->textSize = 11.0;
                else if ([fontSize isEqualToString:@"medium"])
                    subStyle->textSize = 12.0;
                else if ([fontSize isEqualToString:@"large"])
                    subStyle->textSize = 18.0;
                else if ([fontSize isEqualToString:@"x-large"])
                    subStyle->textSize = 24.0;
                else if ([fontSize isEqualToString:@"xx-large"])
                    subStyle->textSize = 36.0;
                else {
                    
                    NSString *numericalFontSize = fontSize;
                    if ([fontSize hasSuffix:@"px"] || [fontSize hasSuffix:@"em"])
                        numericalFontSize = [fontSize substringToIndex:fontSize.length-2];
                    else if ([fontSize hasSuffix:@"%"])
                        numericalFontSize = [fontSize substringToIndex:fontSize.length-1];
                    
                    NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
                    [numberFormatter setNumberStyle:NSNumberFormatterDecimalStyle];
                    
                    NSNumber *nFontSize = [numberFormatter numberFromString:numericalFontSize];
                    
                    if (nFontSize) {
                        if ([fontSize hasSuffix:@"px"])
                            subStyle->textSize = [nFontSize floatValue];
                        else if ([fontSize hasSuffix:@"em"])
                            subStyle->textSize = [nFontSize floatValue] * 12.0;
                        else if ([fontSize hasSuffix:@"%"])
                            subStyle->textSize = [nFontSize floatValue] / 100.0 * 12.0;
                        else
                            subStyle->textSize = [nFontSize floatValue];
                    }
                    
                }
            }
            UIFontDescriptor *fontDescriptor = [UIFontDescriptor fontDescriptorWithFontAttributes:@{@"NSFontFamilyAttribute" : fontFamily, @"NSFontFaceAttribute" : (bold && italic ? @"Bold Italic" : (bold ? @"Bold" : (italic ? @"Italic" : @"Regular")))}];
            
            UIFont *testFont = [UIFont fontWithDescriptor:fontDescriptor size:subStyle->textSize];
            if (testFont)
                font = testFont;
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

        if(styleEntry[@"layout-placement"]) {
            NSString *sLayoutPlacement = styleEntry[@"layout-placement"];
            @try {
                int layoutPlacement = [sLayoutPlacement intValue];
                subStyle->layoutPlacement = @(layoutPlacement);
            } @finally {
            }
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
        if (styleEntry[@"drawpriority"])
            subStyle->desc[kMaplyDrawPriority] = @([styleEntry[@"drawpriority"] integerValue]);
            
        // Just turn fade off for these
        subStyle->desc[kMaplyFade] = @(0.0);
        subStyle->desc[kMaplyEnable] = @NO;

        [self resolveVisibility:styleEntry settings:settings desc:subStyle->desc];
        
        if(styleEntry[@"value"])
            subStyle->textField = styleEntry[@"value"];
        else
            subStyle->textField = @"[name]";
        
        if(styleEntry[@"layout-importance"])
            subStyle->layoutImportance = [styleEntry[@"layout-importance"] floatValue];
        else
            // Make bigger text slightly more important
            subStyle->layoutImportance = 1.0 + subStyle->textSize/1000;
            
        [subStyles addObject:subStyle];
    }
    
    return self;
}

- (NSArray *)buildObjects:(NSArray *)vecObjs forTile:(MaplyTileID)tileID viewC:(MaplyBaseViewController *)viewC;
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
                        label.rotation = -1 * rot+M_PI/2.0;
                        if(label.rotation > M_PI_2 || label.rotation < -M_PI_2) {
                            label.rotation += M_PI;
                        }

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
                    label.layoutImportance = subStyle->layoutImportance;
                    label.selectable = false;
                    [labels addObject:label];
                }
            }
            if (subStyle->layoutPlacement)
                label.layoutPlacement = [subStyle->layoutPlacement intValue];
        }

        // Note: This should be MaplyThreadCurrent, but...
        //   We need a GL context present for the text rendering
        MaplyComponentObject *compObj = [viewC addScreenLabels:labels desc:subStyle->desc mode:MaplyThreadCurrent];
        if (compObj)
            [compObjs addObject:compObj];
    }

    return compObjs;
}

@end
