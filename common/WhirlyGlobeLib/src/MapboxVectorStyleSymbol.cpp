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
#import "WhirlyKitLog.h"
#import <vector>
#import <regex>

namespace WhirlyKit
{

static const char *placementVals[] = {"point","line",NULL};
static const char *transformVals[] = {"none","uppercase","lowercase",NULL};
static const char *anchorVals[] = {"center","left","right","top","bottom","top-left","top-right","bottom-left","bottom-right",NULL};

bool MapboxVectorSymbolLayout::parse(MapboxVectorStyleSetImpl *styleSet,DictionaryRef styleEntry)
{
    globalTextScale = styleSet->tileStyleSettings->textScale;
    placement = (MapboxSymbolPlacement)styleSet->enumValue(styleEntry->getEntry("symbol-placement"), placementVals, (int)MBPlacePoint);
    textTransform = (MapboxTextTransform)styleSet->enumValue(styleEntry->getEntry("text-transform"), transformVals, (int)MBTextTransNone);
    
    std::string textField = styleSet->stringValue("text-field", styleEntry, "");
    if (!textField.empty()) {
        // Parse out the {} groups in the text
        // TODO: We're missing a boatload of stuff in the spec
        std::regex regex{R"([{}]+)"};
        std::sregex_token_iterator it{textField.begin(), textField.end(), regex, -1};
        std::vector<std::string> chunks{it, {}};
        bool isJustText = textField[0] != '{';
        for (auto chunk : chunks) {
            if (chunk.empty())
                continue;
            MapboxVectorSymbolLayout::TextChunk textChunk;
            if (isJustText) {
                textChunk.str = chunk;
            } else {
                textChunk.keys.push_back(chunk);
                // For some reason name:en is sometimes name_en
                std::string textVariant = chunk;
                std::regex_replace(textVariant, std::regex(":"), "_");
                textChunk.keys.push_back(textVariant);
            }
            textChunks.push_back(textChunk);
            isJustText = !isJustText;
        }
    }
    auto textFontArray = styleEntry->getArray("text-font");
    if (!textFontArray.empty() && textFontArray[0]->getType() == DictTypeString) {
        std::string textField = textFontArray[0]->getString();
        textFontName = textField;
        std::regex_replace(textFontName, std::regex(" "), "-");
    }
    textMaxWidth = styleSet->transDouble("text-max-width", styleEntry, 10.0);
    textSize = styleSet->transDouble("text-size", styleEntry, 24.0);

    textAnchor = MBTextCenter;
    if (styleEntry->getType("text-anchor") == DictTypeArray) {
        textAnchor = (MapboxTextAnchor)styleSet->enumValue(styleEntry->getEntry("text-anchor"), anchorVals, (int)MBTextCenter);
    }
    layoutImportance = styleSet->tileStyleSettings->labelImportance;
    
    return true;
}

bool MapboxVectorSymbolPaint::parse(MapboxVectorStyleSetImpl *styleSet,DictionaryRef styleEntry)
{
    textColor = styleSet->transColor("text-color", styleEntry, RGBAColor::black());
    textOpacity = styleSet->transDouble("text-opacity", styleEntry, 1.0);
    textHaloColor = styleSet->colorValue("text-halo-color", NULL, styleEntry, RGBAColorRef(), false);
    textHaloWidth = styleSet->doubleValue("text-halo-width", styleEntry, 0.0);

    return true;
}

bool MapboxVectorLayerSymbol::parse(DictionaryRef styleEntry,
                                   MapboxVectorStyleLayerRef refLayer,
                                   int inDrawPriority)
{
    if (!MapboxVectorStyleLayer::parse(styleEntry,refLayer,drawPriority) ||
        !layout.parse(styleSet,styleEntry->getDict("layout")) ||
        !paint.parse(styleSet, styleEntry->getDict("paint")))
        return false;

    uniqueLabel = styleSet->boolValue("unique-label", styleEntry, "yes", false);
    
    uuidField = styleSet->tileStyleSettings->uuidField;
    
    useZoomLevels = styleSet->tileStyleSettings->useZoomLevels;

    drawPriority = inDrawPriority;
    
    return true;
}

// Break up text into multiple lines if needed
//- (NSString *)breakUpText:(NSString *)text width:(double)maxWidth font:(UIFont *)font
//{
//    // If there's no spaces, let's not
//    if (![text containsString:@" "])
//        return text;
//
//    NSMutableString *retStr = [[NSMutableString alloc] init];
//
//    // Unfortunately this stuff will break long names across character boundaries which is completely awful
////    NSAttributedString *textAttrStr = [[NSAttributedString alloc] initWithString:text attributes:@{NSFontAttributeName:font}];
////    NSTextContainer *textCon = [[NSTextContainer alloc] initWithSize:CGSizeMake(maxWidth,CGFLOAT_MAX)];
////    textCon.lineBreakMode = NSLineBreakByWordWrapping;
////    NSLayoutManager *layoutMan = [[NSLayoutManager alloc] init];
////    NSTextStorage *textStore = [[NSTextStorage alloc] initWithAttributedString:textAttrStr];
////    [textStore addLayoutManager:layoutMan];
////    [layoutMan addTextContainer:textCon];
////    bool __block started = false;
////    [layoutMan enumerateLineFragmentsForGlyphRange:NSMakeRange(0, layoutMan.numberOfGlyphs)
////                                        usingBlock:^(CGRect rect, CGRect usedRect, NSTextContainer * _Nonnull textContainer, NSRange glyphRange, BOOL * _Nonnull stop)
////    {
////        NSRange r = [layoutMan characterRangeForGlyphRange:glyphRange  actualGlyphRange:nil];
////        NSString *lineStr = [textAttrStr.string substringWithRange:r];
////        if (started)
////            [retStr appendString:@"\n"];
////        [retStr appendString:lineStr];
////        started = true;
////    }];
////
////    CGSize size = [textAttrStr size];
////    NSLog(@"Input: %@, output: %@, size = (%f,%f)",text,retStr,size.width,size.height);
//
//    // Work through the string chunk by chunk
//    NSArray *pieces = [text componentsSeparatedByString:@" "];
//    NSString *soFar = nil;
//    for (NSString *chunk in pieces) {
//        if ([soFar length] == 0) {
//            soFar = chunk;
//            continue;
//        }
//
//        // Try the string with the next chunk
//        NSString *testStr = [[NSString alloc] initWithFormat:@"%@ %@",soFar,chunk];
//        NSAttributedString *testAttrStr = [[NSAttributedString alloc] initWithString:testStr attributes:@{NSFontAttributeName:font}];
//        CGSize size = [testAttrStr size];
//
//        // Flush out what we have so far and start with this new chunk
//        if (size.width > maxWidth) {
//            if ([retStr length] > 0)
//                [retStr appendString:@"\n"];
//            [retStr appendString:soFar];
//            soFar = chunk;
//        } else {
//            // Keep adding to this string
//            soFar = testStr;
//        }
//    }
//    if ([retStr length] > 0)
//        [retStr appendString:@"\n"];
//    [retStr appendString:soFar];
//
//    return retStr;
//}

// Calculate a value [0.0,1.0] for this string
static float calcStringHash(const std::string &str)
{
    unsigned int len = str.length();

    float val = 0.0;
    for (int ii=0;ii<len;ii++)
        val += str[ii] / 256.0;
    val /= len;
    
    return val;
}

void MapboxVectorLayerSymbol::cleanup(ChangeSet &changes)
{
}

void MapboxVectorLayerSymbol::buildObjects(std::vector<VectorObjectRef> &vecObjs,VectorTileDataRef tileInfo)
{
    if (!visible)
        return;
    
    ComponentObjectRef compObj = styleSet->makeComponentObject();

    // TODO: They mean displayed level here, which is different from loaded level
    if (useZoomLevels) {
        if (minzoom > tileInfo->ident.level)
          return;
      if (maxzoom < tileInfo->ident.level)
          return;
    }
    
    double textSize = layout.textSize->valForZoom(tileInfo->ident.level);
    // Snap to an integer.  Not clear we need to, just because.
    textSize = (int)(textSize * layout.globalTextScale+0.5);

    LabelInfoRef labelInfo = styleSet->makeLabelInfo(layout.textFontName,textSize);
    labelInfo->screenObject = true;
    labelInfo->fade = 0.0;
    labelInfo->textJustify = WhirlyKitTextCenter;

    if (drawPriorityPerLevel > 0)
        labelInfo->drawPriority = drawPriority + tileInfo->ident.level * drawPriorityPerLevel;
    else
        labelInfo->drawPriority = drawPriority;

    RGBAColorRef textColor = styleSet->resolveColor(paint.textColor, paint.textOpacity, tileInfo->ident.level, MBResolveColorOpacityReplaceAlpha);
    if (textColor)
        labelInfo->textColor = *textColor;

    if (paint.textHaloColor && paint.textHaloWidth > 0.0)
    {
        labelInfo->outlineColor = *paint.textHaloColor;
        labelInfo->outlineSize = paint.textHaloWidth;
    }

//    // Note: Cache the font.
//    UIFont *font = nil;
//    if (_layout.textFontName) {
//        UIFontDescriptor *fontDesc = [[UIFontDescriptor alloc] initWithFontAttributes:@{UIFontDescriptorNameAttribute: _layout.textFontName}];
//        font = [UIFont fontWithDescriptor:fontDesc size:textSize];
////        NSLog(@"Asked for: %@,  Got: %@, %f",_layout.textFontName,font.fontName,textSize);
//        if (!font)
//            NSLog(@"Found unsupported font %@",fontDesc);
//    }
//    if (!font)
//        font = [UIFont systemFontOfSize:textSize];
//    desc[kMaplyFont] = font;
    
//    // Note: Made up value for pushing multi-line text together
//    desc[kMaplyTextLineSpacing] = @(4.0 / 5.0 * font.lineHeight);
    
    bool include = textColor && textSize > 0.0;
    if (!include)
        return;
    
    std::vector<SingleLabelRef> labels;
    for (auto vecObj : vecObjs) {
        if (vecObj->getVectorType() == VectorPointType) {
            for (VectorShapeRef shape : vecObj->shapes) {
                VectorPointsRef pts = std::dynamic_pointer_cast<VectorPoints>(shape);
                if (pts) {
                    for (auto pt : pts->pts) {
                        // Reconstruct the string from its replacement form
                        std::string text;
                        for (auto textChunk : layout.textChunks) {
                            if (!textChunk.str.empty())
                                text += textChunk.str;
                            else {
                                for (auto key : textChunk.keys) {
                                    if (vecObj->getAttributes()->hasField(key)) {
                                        std::string keyVal = vecObj->getAttributes()->getString(key);
                                        if (!keyVal.empty())
                                            text += keyVal;
                                    }
                                }
                            }
                        }
                        
                        if (!text.empty()) {
                            // Change the text if needed
                            switch (layout.textTransform)
                            {
                                case MBTextTransNone:
                                    break;
                                case MBTextTransUppercase:
                                    std::transform(text.begin(), text.end(), text.begin(), ::toupper);
                                    break;
                                case MBTextTransLowercase:
                                    std::transform(text.begin(), text.end(), text.begin(), ::tolower);
                                    break;
                            }

                            // TODO: Put this back, but we need information about the font
                            // Break it up into lines, if necessary
//                            double textMaxWidth = [_layout.textMaxWidth valForZoom:tileInfo.tileID.level];
//                            if (textMaxWidth != 0.0) {
//                                label.text = [self breakUpText:label.text width:textMaxWidth * font.pointSize * _layout.globalTextScale font:font];
//                            }
                            
                            // Construct the label
                            SingleLabelRef label = styleSet->makeSingleLabel(text);
                            label->loc = GeoCoord(pt.x(),pt.y());
                            label->isSelectable = selectable;
                            
                            if (!uuidField.empty())
                                label->uniqueID = vecObj->getAttributes()->getString(uuidField);
                            else if (uniqueLabel) {
                                label->uniqueID = text;
                                std::transform(label->uniqueID.begin(), label->uniqueID.end(), label->uniqueID.begin(), ::tolower);
                            }
                            
                            // The rank is most important, followed by the zoom level.  This keeps the countries on top.
                            int rank = 0;
                            if (vecObj->getAttributes()->hasField("rank")) {
                                rank = vecObj->getAttributes()->getInt("rank");
                            }
                            // Random tweak to cut down on flashing
                            // TODO: Move the layout importance into the label itself
//                            float strHash = calcStringHash(text);
//                            label->layoutImportance = layout.layoutImportance + 1.0 - (rank + (101-tileInfo->ident.level)/100.0)/1000.0 + strHash/1000.0;
                            
                            // TODO: Need access to the coordinate system
//                            // Point or line placement
//                            if (layout.placement == MBPlaceLine) {
//                                MaplyCoordinate middle;
//                                double rot;
//                                [vecObj linearMiddle:&middle rot:&rot displayCoordSys:viewC.coordSystem];
//                                label.loc = middle;
//                                label.rotation = -1 * rot+M_PI/2.0;
//                                if(label.rotation > M_PI_2 || label.rotation < -M_PI_2) {
//                                    label.rotation += M_PI;
//                                }
//                                label.keepUpright = true;
//                            }
                            
                            // TODO: Move the layoutPlacement into the label itself
                            // Anchor options for the layout engine
//                            switch (layout.textAnchor) {
//                                case MBTextCenter:
//                                    label->layoutPlacement = kMaplyLayoutCenter;
//                                    break;
//                                case MBTextLeft:
//                                    label.layoutPlacement = kMaplyLayoutLeft;
//                                    break;
//                                case MBTextRight:
//                                    label.layoutPlacement = kMaplyLayoutRight;
//                                    break;
//                                case MBTextTop:
//                                    label.layoutPlacement = kMaplyLayoutAbove;
//                                    break;
//                                case MBTextBottom:
//                                    label.layoutPlacement = kMaplyLayoutBelow;
//                                    break;
//                                    // Note: The rest of these aren't quite right
//                                case MBTextTopLeft:
//                                    label.layoutPlacement = kMaplyLayoutLeft;
//                                    break;
//                                case MBTextTopRight:
//                                    label.layoutPlacement = kMaplyLayoutRight;
//                                    break;
//                                case MBTextBottomLeft:
//                                    label.layoutPlacement = kMaplyLayoutBelow;
//                                    break;
//                                case MBTextBottomRight:
//                                    label.layoutPlacement = kMaplyLayoutBelow;
//                                    break;
//                            }
                            
                            labels.push_back(label);
                        } else {
                            wkLogLevel(Warn,"Failed to find text for label");
                        }
                    }
                }
            }
        }
    }

    if (!labels.empty()) {
        SimpleIdentity labelID = styleSet->labelManage->addLabels(labels, *labelInfo, tileInfo->changes);
        if (labelID != EmptyIdentity)
            compObj->labelIDs.insert(labelID);
    }
    
    if (!compObj->labelIDs.empty()) {
        styleSet->compManage->addComponentObject(compObj);
        tileInfo->compObjs.push_back(compObj);
    }
}

}
