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

#import "MapboxVectorStyleSymbol.h"
#import "MaplyScreenLabel.h"

@implementation MapboxVectorSymbolLayout

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    _globalTextScale = styleSet.tileStyleSettings.textScale;
    _visible = [styleSet boolValue:@"visibility" dict:styleEntry onValue:@"visible" defVal:true];
    _placement = (MapboxSymbolPlacement)[styleSet enumValue:styleEntry[@"symbol-placement"] options:@[@"point",@"line"] defVal:MBPlacePoint];
    _textTransform = (MapboxTextTransform)[styleSet enumValue:styleEntry[@"text-transform"] options:@[@"none",@"uppercase",@"lowercase"] defVal:MBTextTransNone];
    
    NSString *textField = [styleSet stringValue:@"text-field" dict:styleEntry defVal:nil];
    NSMutableArray *textFields = [NSMutableArray array];
    if (textField) {
        // Note: Doing a brute force replacement of the string name.  There are more subtle variants
        NSString *thisTextField = [[textField stringByReplacingOccurrencesOfString:@"{" withString:@""] stringByReplacingOccurrencesOfString:@"}" withString:@""];
        [textFields addObject:thisTextField];
        // For some reason name:en is sometimes name_en
        NSString *textVariant = [thisTextField stringByReplacingOccurrencesOfString:@":" withString:@"_"];
        if (![textVariant isEqualToString:thisTextField])
            [textFields addObject:textVariant];
        _textFields = textFields;
    }
    NSArray *textFontArray = styleEntry[@"text-font"];
    if ([textFontArray isKindOfClass:[NSArray class]] && [textFontArray count] > 0) {
        NSString *textField = [textFontArray objectAtIndex:0];
        if ([textField isKindOfClass:[NSString class]]) {
            _textFontName = [textField stringByReplacingOccurrencesOfString:@" " withString:@"-"];
        }
    }
    id maxWidthEntry = styleEntry[@"text-max-width"];
    if (maxWidthEntry)
    {
        if ([maxWidthEntry isKindOfClass:[NSNumber class]])
            _textMaxWidth = [styleSet doubleValue:maxWidthEntry defVal:10.0];
        else
            _textMaxWidthFunc = [styleSet stopsValue:maxWidthEntry defVal:nil];
    } else
        _textMaxWidth = 10.0;
    id sizeEntry = styleEntry[@"text-size"];
    if (sizeEntry)
    {
        if ([sizeEntry isKindOfClass:[NSNumber class]])
            _textSize = [styleSet doubleValue:sizeEntry defVal:1.0];
        else
            _textSizeFunc = [styleSet stopsValue:sizeEntry defVal:nil];
    } else
        _textSize = 24.0;

    id textAnchor = styleEntry[@"text-anchor"];
    _textAnchor = MBTextCenter;
    if (textAnchor)
    {
        _textAnchor = (MapboxTextAnchor)[styleSet enumValue:styleEntry[@"text-anchor"] options:@[@"center",@"left",@"right",@"top",@"bottom",@"top-left",@"top-right",@"bottom-left",@"bottom-right"] defVal:MBTextCenter];
    }

    return self;
}

@end

@implementation MapboxVectorSymbolPaint

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    id textColorEntry = styleEntry[@"text-color"];
    if (textColorEntry)
    {
        if ([textColorEntry isKindOfClass:[NSString class]])
            _textColor = [styleSet colorValue:@"text-color" val:nil dict:styleEntry defVal:[UIColor blackColor] multiplyAlpha:false];
        else
            _textColorFunc = [styleSet stopsValue:textColorEntry defVal:nil];
    } else {
        _textColor = [UIColor whiteColor];
    }
    id textOpacityEntry = styleEntry[@"text-opacity"];
    if (textOpacityEntry) {
        if ([textOpacityEntry isKindOfClass:[NSNumber class]])
            _textOpacity = [styleSet doubleValue:@"text-opacity" dict:styleEntry defVal:1.0];
        else
            _textOpacityFunc = [styleSet stopsValue:textOpacityEntry defVal:nil];
    } else {
        _textOpacity = 1.0;
    }
    _textHaloColor = [styleSet colorValue:@"text-halo-color" val:nil dict:styleEntry defVal:nil multiplyAlpha:false];
    _textHaloWidth = [styleSet doubleValue:@"text-halo-width" dict:styleEntry defVal:0.0];

    return self;
}

@end

@implementation MapboxVectorLayerSymbol
{
    NSMutableDictionary *symbolDesc;
}

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry parent:(MaplyMapboxVectorStyleLayer *)refLayer styleSet:(MapboxVectorStyleSet *)styleSet drawPriority:(int)drawPriority viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super initWithStyleEntry:styleEntry parent:refLayer styleSet:styleSet drawPriority:drawPriority viewC:viewC];
    if (!self)
        return nil;
    
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
    
    symbolDesc = [NSMutableDictionary dictionaryWithDictionary:
                  @{
                    kMaplyFade: @(0.0),
                    kMaplyTextJustify: kMaplyTextJustifyCenter,
                    kMaplyEnable: @(NO)
                    }];
    if (_paint.textColor)
        symbolDesc[kMaplyTextColor] = [_paint.textColor colorWithAlphaComponent:_paint.textOpacity];
    if (_paint.textHaloColor && _paint.textHaloWidth > 0.0)
    {
        symbolDesc[kMaplyTextOutlineColor] = _paint.textHaloColor;
        symbolDesc[kMaplyTextOutlineSize] = @(_paint.textHaloWidth);
    }
    
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

- (NSArray *)buildObjects:(NSArray *)vecObjs forTile:(MaplyVectorTileInfo *)tileInfo viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    NSMutableArray *compObjs = [NSMutableArray array];

    // Note: Would be better to do this earlier
    if (!_layout.visible)
        return compObjs;
    
    if (self.minzoom > tileInfo.tileID.level)
        return compObjs;
    if (self.maxzoom < tileInfo.tileID.level)
        return compObjs;
    
    NSDictionary *desc = symbolDesc;
    double textSize = 24.0;
    if (_layout.textSizeFunc)
    {
        textSize = [_layout.textSizeFunc valueForZoom:tileInfo.tileID.level];
    } else {
        textSize = _layout.textSize;
    }
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
    
    UIColor *textColor = _paint.textColor;
    if (_paint.textColorFunc) {
        textColor = [_paint.textColorFunc colorForZoom:tileInfo.tileID.level];
    }
    if (!textColor)
        textColor = [UIColor whiteColor];
    double opacity = _paint.textOpacity;
    if (_paint.textOpacityFunc) {
        opacity = [_paint.textOpacityFunc valueForZoom:tileInfo.tileID.level];
    }
    textColor = [textColor colorWithAlphaComponent:opacity];

    NSMutableDictionary *mutDesc = [NSMutableDictionary dictionaryWithDictionary:desc];
    mutDesc[kMaplyFont] = font;
    mutDesc[kMaplyTextColor] = textColor;
    desc = mutDesc;
    // Note: Made up value for pushing multi-line text together
    mutDesc[kMaplyTextLineSpacing] = @(4.0 / 5.0 * font.lineHeight);

    NSMutableArray *labels = [NSMutableArray array];
    for (MaplyVectorObject *vecObj in vecObjs)
    {
        MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
        label.loc = [vecObj center];
        for (NSString *posTextField in _layout.textFields)
        {
            label.text = vecObj.attributes[posTextField];
            if (label.text)
                break;
        }
        if (!label.text)
        {
            NSLog(@"Failed to find text for label");
            continue;
        }
        if (_uniqueLabel)
            label.uniqueID = [label.text lowercaseString];

        // The rank is most important, followed by the zoom level.  This keeps the countries on top.
        int rank = 0;
        if (vecObj.attributes[@"rank"]) {
            rank = [vecObj.attributes[@"rank"] integerValue];
        }
        // Random tweak to cut down on flashing
        float strHash = [self calcStringHash:label.text];
        label.layoutImportance = 1000000 - rank + (101-tileInfo.tileID.level)/100.0 + strHash/1000.0;
        
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
        double textMaxWidth = _layout.textMaxWidth;
        if (_layout.textMaxWidthFunc)
            textMaxWidth = [_layout.textMaxWidthFunc valueForZoom:tileInfo.tileID.level];
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
    
    MaplyComponentObject *compObj = [viewC addScreenLabels:labels desc:desc mode:MaplyThreadCurrent];
    if (compObjs)
        [compObjs addObject:compObj];
    
    return compObjs;
}

@end
