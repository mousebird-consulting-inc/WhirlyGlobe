/*
 *  MapboxVectorStyleSymbol.h
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

#import "vector_styles/MapboxVectorStyleSymbol.h"
#import "visual_objects/MaplyScreenLabel.h"
#import <vector>

// Used to track text data
class TextChunk {
public:
    // Set if this is a simple string
    NSString *str;

    // Possible key names in the data. Tried in this order.
    // Not set if this is a simple string
    std::vector<NSString *> keys;
};

@implementation MapboxVectorSymbolLayout
{
@public
    std::vector<TextChunk> textChunks;
    float layoutImportance;
}

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    _globalTextScale = styleSet.tileStyleSettings.textScale;
    _placement = (MapboxSymbolPlacement)[styleSet enumValue:styleEntry[@"symbol-placement"] options:@[@"point",@"line"] defVal:MBPlacePoint];
    _textTransform = (MapboxTextTransform)[styleSet enumValue:styleEntry[@"text-transform"] options:@[@"none",@"uppercase",@"lowercase"] defVal:MBTextTransNone];
    
    NSString *textField = [styleSet stringValue:@"text-field" dict:styleEntry defVal:nil];
    if (textField) {
        // Parse out the {} groups in the text
        // TODO: We're missing a boatload of stuff in the spec
        NSMutableCharacterSet *charSet = [[NSMutableCharacterSet alloc] init];
        [charSet addCharactersInString:@"{}"];
        NSArray *chunks = [textField componentsSeparatedByCharactersInSet:charSet];
        bool isJustText = [textField characterAtIndex:0] != '{';
        for (NSString *chunk in chunks) {
            if ([chunk length] == 0)
                continue;
            TextChunk textChunk;
            if (isJustText) {
                textChunk.str = chunk;
            } else {
                textChunk.keys.push_back(chunk);
                // For some reason name:en is sometimes name_en
                NSString *textVariant = [chunk stringByReplacingOccurrencesOfString:@":" withString:@"_"];
                textChunk.keys.push_back(textVariant);
            }
            textChunks.push_back(textChunk);
            isJustText = !isJustText;
        }
    }
    NSArray *textFontArray = styleEntry[@"text-font"];
    if ([textFontArray isKindOfClass:[NSArray class]] && [textFontArray count] > 0) {
        NSString *textField = [textFontArray objectAtIndex:0];
        if ([textField isKindOfClass:[NSString class]]) {
            _textFontName = [textField stringByReplacingOccurrencesOfString:@" " withString:@"-"];
        }
    }
    _textMaxWidth = [styleSet transDouble:@"text-max-width" entry:styleEntry defVal:10.0];
    _textSize = [styleSet transDouble:@"text-size" entry:styleEntry defVal:24.0];

    id textAnchor = styleEntry[@"text-anchor"];
    _textAnchor = MBTextCenter;
    if (textAnchor)
    {
        _textAnchor = (MapboxTextAnchor)[styleSet enumValue:styleEntry[@"text-anchor"] options:@[@"center",@"left",@"right",@"top",@"bottom",@"top-left",@"top-right",@"bottom-left",@"bottom-right"] defVal:MBTextCenter];
    }
    layoutImportance = styleSet.tileStyleSettings.labelImportance;

    return self;
}

@end

@implementation MapboxVectorSymbolPaint

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    _textColor = [styleSet transColor:@"text-color" entry:styleEntry defVal:[UIColor blackColor]];
    _textOpacity = [styleSet transDouble:@"text-opacity" entry:styleEntry defVal:1.0];
    _textHaloColor = [styleSet colorValue:@"text-halo-color" val:nil dict:styleEntry defVal:nil multiplyAlpha:false];
    _textHaloWidth = [styleSet doubleValue:@"text-halo-width" dict:styleEntry defVal:0.0];

    return self;
}

@end

@implementation MapboxVectorLayerSymbol
{
    NSString *uuidField;
    MaplyVectorStyleSettings *styleSettings;
}

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry parent:(MaplyMapboxVectorStyleLayer *)refLayer styleSet:(MapboxVectorStyleSet *)styleSet drawPriority:(int)drawPriority viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super initWithStyleEntry:styleEntry parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:viewC];
    if (!self)
        return nil;
    styleSettings = styleSet.tileStyleSettings;
    
    _layout = [[MapboxVectorSymbolLayout alloc] initWithStyleEntry:styleEntry[@"layout"] styleSet:styleSet viewC:viewC];
    _paint = [[MapboxVectorSymbolPaint alloc] initWithStyleEntry:styleEntry[@"paint"] styleSet:styleSet viewC:viewC];
    _uniqueLabel = [styleSet boolValue:@"unique-label" dict:styleEntry onValue:@"yes" defVal:false];

    if (!_layout)
    {
        NSLog(@"Expecting layout in symbol layer.");
        return nil;
    }
    if (!_paint)
    {
        NSLog(@"Expecting paint in symbol layer.");
        return nil;
    }
    
    uuidField = styleSet.tileStyleSettings.uuidField;
        
    return self;
}

// Break up text into multiple lines if needed
- (NSString *)breakUpText:(NSString *)text width:(double)maxWidth font:(UIFont *)font
{
    // If there's no spaces, let's not
    if (![text containsString:@" "])
        return text;
    
    NSMutableString *retStr = [[NSMutableString alloc] init];

    // Unfortunately this stuff will break long names across character boundaries which is completely awful
//    NSAttributedString *textAttrStr = [[NSAttributedString alloc] initWithString:text attributes:@{NSFontAttributeName:font}];
//    NSTextContainer *textCon = [[NSTextContainer alloc] initWithSize:CGSizeMake(maxWidth,CGFLOAT_MAX)];
//    textCon.lineBreakMode = NSLineBreakByWordWrapping;
//    NSLayoutManager *layoutMan = [[NSLayoutManager alloc] init];
//    NSTextStorage *textStore = [[NSTextStorage alloc] initWithAttributedString:textAttrStr];
//    [textStore addLayoutManager:layoutMan];
//    [layoutMan addTextContainer:textCon];
//    bool __block started = false;
//    [layoutMan enumerateLineFragmentsForGlyphRange:NSMakeRange(0, layoutMan.numberOfGlyphs)
//                                        usingBlock:^(CGRect rect, CGRect usedRect, NSTextContainer * _Nonnull textContainer, NSRange glyphRange, BOOL * _Nonnull stop)
//    {
//        NSRange r = [layoutMan characterRangeForGlyphRange:glyphRange  actualGlyphRange:nil];
//        NSString *lineStr = [textAttrStr.string substringWithRange:r];
//        if (started)
//            [retStr appendString:@"\n"];
//        [retStr appendString:lineStr];
//        started = true;
//    }];
//
//    CGSize size = [textAttrStr size];
//    NSLog(@"Input: %@, output: %@, size = (%f,%f)",text,retStr,size.width,size.height);
    
    // Work through the string chunk by chunk
    NSArray *pieces = [text componentsSeparatedByString:@" "];
    NSString *soFar = nil;
    for (NSString *chunk in pieces) {
        if ([soFar length] == 0) {
            soFar = chunk;
            continue;
        }
        
        // Try the string with the next chunk
        NSString *testStr = [[NSString alloc] initWithFormat:@"%@ %@",soFar,chunk];
        NSAttributedString *testAttrStr = [[NSAttributedString alloc] initWithString:testStr attributes:@{NSFontAttributeName:font}];
        CGSize size = [testAttrStr size];
        
        // Flush out what we have so far and start with this new chunk
        if (size.width > maxWidth) {
            if ([retStr length] > 0)
                [retStr appendString:@"\n"];
            [retStr appendString:soFar];
            soFar = chunk;
        } else {
            // Keep adding to this string
            soFar = testStr;
        }
    }
    if ([retStr length] > 0)
        [retStr appendString:@"\n"];
    [retStr appendString:soFar];
        
    return retStr;
}

// Calculate a value [0.0,1.0] for this string
- (float)calcStringHash:(NSString *)str
{
    unsigned int len = [str length];
    unichar buffer[len];
    
    [str getCharacters:buffer range:NSMakeRange(0, len)];
    float val = 0.0;
    for (int ii=0;ii<len;ii++) {
        val += buffer[ii] / 256.0;
    }
    val /= len;
    
    return val;
}

- (void)buildObjects:(NSArray *)vecObjs forTile:(MaplyVectorTileData *)tileInfo viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    if (!self.visible) {
        return;
    }
    
    // TODO: They mean displayed level here, which is different from loaded level
    if (styleSettings.useZoomLevels) {
      if (self.minzoom > tileInfo.tileID.level)
          return;
      if (self.maxzoom < tileInfo.tileID.level)
          return;
    }
    
    NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:
                  @{
                    kMaplyFade: @(0.0),
                    kMaplyTextJustify: kMaplyTextJustifyCenter,
                    kMaplyEnable: @(NO)
                    }];
    UIColor *textColor = [self.styleSet resolveColor:_paint.textColor opacity:_paint.textOpacity forZoom:tileInfo.tileID.level mode:MBResolveColorOpacityReplaceAlpha];
    if (textColor)
        desc[kMaplyTextColor] = textColor;
    if (_paint.textHaloColor && _paint.textHaloWidth > 0.0)
    {
        desc[kMaplyTextOutlineColor] = _paint.textHaloColor;
        desc[kMaplyTextOutlineSize] = @(_paint.textHaloWidth);
    }
    double textSize = [_layout.textSize valForZoom:tileInfo.tileID.level];
    // Snap to an integer.  Not clear we need to, just because.
    textSize = (int)(textSize * _layout.globalTextScale+0.5);

    // Note: Cache the font.
    UIFont *font = nil;
    if (_layout.textFontName) {
        UIFontDescriptor *fontDesc = [[UIFontDescriptor alloc] initWithFontAttributes:@{UIFontDescriptorNameAttribute: _layout.textFontName}];
        font = [UIFont fontWithDescriptor:fontDesc size:textSize];
//        NSLog(@"Asked for: %@,  Got: %@, %f",_layout.textFontName,font.fontName,textSize);
        if (!font)
            NSLog(@"Found unsupported font %@",fontDesc);
    }
    if (!font)
        font = [UIFont systemFontOfSize:textSize];
    desc[kMaplyFont] = font;
    
    // Note: Made up value for pushing multi-line text together
    desc[kMaplyTextLineSpacing] = @(4.0 / 5.0 * font.lineHeight);
    
    bool include = textColor != nil && textSize > 0.0;
    if (!include)
        return;

    NSMutableArray *labels = [NSMutableArray array];
    for (MaplyVectorObject *vecObj in vecObjs)
    {
        MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
        label.selectable = self.selectable;
        label.userObject = vecObj;
        label.loc = [vecObj center];
        
        // Reconstruct the string from its replacement form
        NSMutableString *text = [NSMutableString new];
        for (auto textChunk : _layout->textChunks) {
            if (textChunk.str) {
                [text appendString:textChunk.str];
            } else {
                for (NSString *key : textChunk.keys) {
                    NSString *keyVal = vecObj.attributes[key];
                    if (keyVal) {
                        [text appendString:keyVal];
                        break;
                    }
                }
            }
        }
        label.text = text;

        if (!label.text)
        {
            NSLog(@"Failed to find text for label");
            continue;
        }
        if (uuidField) {
            label.uniqueID  = [vecObj.attributes objectForKey:uuidField];
        } else if (_uniqueLabel)
            label.uniqueID = [label.text lowercaseString];

        // The rank is most important, followed by the zoom level.  This keeps the countries on top.
        int rank = 0;
        if (vecObj.attributes[@"rank"]) {
            rank = [vecObj.attributes[@"rank"] integerValue];
        }
        // Random tweak to cut down on flashing
        float strHash = [self calcStringHash:label.text];
        label.layoutImportance = _layout->layoutImportance + 1.0 - (rank + (101-tileInfo.tileID.level)/100.0)/1000.0 + strHash/1000.0;
        
        // Change the text if needed
        switch (_layout.textTransform)
        {
            case MBTextTransNone:
                break;
            case MBTextTransUppercase:
                label.text = [label.text uppercaseString];
                break;
            case MBTextTransLowercase:
                label.text = [label.text lowercaseString];
                break;
        }
        // Break it up into lines, if necessary
        double textMaxWidth = [_layout.textMaxWidth valForZoom:tileInfo.tileID.level];
        if (textMaxWidth != 0.0) {
            label.text = [self breakUpText:label.text width:textMaxWidth * font.pointSize * _layout.globalTextScale font:font];
        }
        
        // Point or line placement
        if (_layout.placement == MBPlaceLine) {
            MaplyCoordinate middle;
            double rot;
            [vecObj linearMiddle:&middle rot:&rot displayCoordSys:viewC.coordSystem];
            label.loc = middle;
            label.rotation = -1 * rot+M_PI/2.0;
            if(label.rotation > M_PI_2 || label.rotation < -M_PI_2) {
                label.rotation += M_PI;
            }
            label.keepUpright = true;
        }
        
        // Anchor options for the layout engine
        switch (_layout.textAnchor) {
            case MBTextCenter:
                label.layoutPlacement = kMaplyLayoutCenter;
                break;
            case MBTextLeft:
                label.layoutPlacement = kMaplyLayoutLeft;
                break;
            case MBTextRight:
                label.layoutPlacement = kMaplyLayoutRight;
                break;
            case MBTextTop:
                label.layoutPlacement = kMaplyLayoutAbove;
                break;
            case MBTextBottom:
                label.layoutPlacement = kMaplyLayoutBelow;
                break;
                // Note: The rest of these aren't quite right
            case MBTextTopLeft:
                label.layoutPlacement = kMaplyLayoutLeft;
                break;
            case MBTextTopRight:
                label.layoutPlacement = kMaplyLayoutRight;
                break;
            case MBTextBottomLeft:
                label.layoutPlacement = kMaplyLayoutBelow;
                break;
            case MBTextBottomRight:
                label.layoutPlacement = kMaplyLayoutBelow;
                break;
        }
        [labels addObject:label];
    }
    
    [tileInfo addComponentObject:[viewC addScreenLabels:labels desc:desc mode:MaplyThreadCurrent]];
}

@end
