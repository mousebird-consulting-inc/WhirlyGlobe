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
#import "Dictionary.h"
#import "WhirlyKitLog.h"
#import <vector>
#import <regex>

namespace WhirlyKit
{

static const char *placementVals[] = {"point","line",NULL};
static const char *transformVals[] = {"none","uppercase","lowercase",NULL};
static const char *anchorVals[] = {"center","left","right","top","bottom","top-left","top-right","bottom-left","bottom-right",NULL};

bool MapboxVectorSymbolLayout::RegexField::parse(const std::string &fieldName,
                                                 MapboxVectorStyleSetImpl *styleSet,
                                                 DictionaryRef styleEntry)
{
    std::string textField = styleSet->stringValue(fieldName, styleEntry, "");
    if (!textField.empty()) {
        // Parse out the {} groups in the text
        // TODO: We're missing a boatload of stuff in the spec
        std::regex regex{R"([{}]+)"};
        std::sregex_token_iterator it{textField.begin(), textField.end(), regex, -1};
        std::vector<std::string> regexChunks{it, {}};
        bool isJustText = textField[0] != '{';
        for (auto regexChunk : regexChunks) {
            if (regexChunk.empty())
                continue;
            MapboxVectorSymbolLayout::TextChunk textChunk;
            if (isJustText) {
                textChunk.str = regexChunk;
            } else {
                textChunk.keys.push_back(regexChunk);
                // For some reason name:en is sometimes name_en
                std::string textVariant = regexChunk;
                std::regex_replace(textVariant, std::regex(":"), "_");
                textChunk.keys.push_back(textVariant);
            }
            chunks.push_back(textChunk);
            isJustText = !isJustText;
        }

        valid = true;
    }
    
    return true;
}

std::string MapboxVectorSymbolLayout::RegexField::build(DictionaryRef attrs)
{
    std::string text = "";
    
    for (auto chunk : chunks) {
        if (!chunk.str.empty())
            text += chunk.str;
        else {
            for (auto key : chunk.keys) {
                if (attrs->hasField(key)) {
                    std::string keyVal = attrs->getString(key);
                    if (!keyVal.empty()) {
                        text += keyVal;
                        break;
                    }
                }
            }
        }
    }

    return text;
}

bool MapboxVectorSymbolLayout::parse(PlatformThreadInfo *inst,
                                     MapboxVectorStyleSetImpl *styleSet,
                                     DictionaryRef styleEntry)
{
    globalTextScale = styleSet->tileStyleSettings->textScale;
    placement = (MapboxSymbolPlacement)styleSet->enumValue(styleEntry->getEntry("symbol-placement"), placementVals, (int)MBPlacePoint);
    textTransform = (MapboxTextTransform)styleSet->enumValue(styleEntry->getEntry("text-transform"), transformVals, (int)MBTextTransNone);
    
    textField.parse("text-field",styleSet,styleEntry);

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
    
    iconImageField.parse("icon-image",styleSet,styleEntry);
    iconSize = styleSet->doubleValue("icon-size", styleEntry, 1.0);
    
    return true;
}

bool MapboxVectorSymbolPaint::parse(PlatformThreadInfo *inst,
                                    MapboxVectorStyleSetImpl *styleSet,
                                    DictionaryRef styleEntry)
{
    if (!styleEntry)
        return false;
    
    textColor = styleSet->transColor("text-color", styleEntry, RGBAColor::black());
    textOpacity = styleSet->transDouble("text-opacity", styleEntry, 1.0);
    textHaloColor = styleSet->colorValue("text-halo-color", NULL, styleEntry, RGBAColorRef(), false);
    textHaloWidth = styleSet->doubleValue("text-halo-width", styleEntry, 0.0);
    
    return true;
}

bool MapboxVectorLayerSymbol::parse(PlatformThreadInfo *inst,
                                    DictionaryRef styleEntry,
                                   MapboxVectorStyleLayerRef refLayer,
                                   int inDrawPriority)
{
    if (!MapboxVectorStyleLayer::parse(inst,styleEntry,refLayer,drawPriority))
        return false;
    bool hasLayout = layout.parse(inst,styleSet,styleEntry->getDict("layout"));
    bool hasPaint = paint.parse(inst,styleSet, styleEntry->getDict("paint"));
    if (!hasLayout && !hasPaint)
        return false;

    uniqueLabel = styleSet->boolValue("unique-label", styleEntry, "yes", false);
    
    uuidField = styleSet->tileStyleSettings->uuidField;
    
    useZoomLevels = styleSet->tileStyleSettings->useZoomLevels;

    drawPriority = inDrawPriority;
    
    return true;
}

std::string MapboxVectorLayerSymbol::breakUpText(PlatformThreadInfo *inst,const std::string &text,double textMaxWidth,LabelInfoRef labelInfo)
{
    // If there are no spaces, let's not break it up
    if (text.find(" ") == std::string::npos)
        return text;
    
    size_t start, end = 0;
    std::vector<std::string> chunks;
    while ((start = text.find_first_not_of(" ", end)) != std::string::npos) {
        end = text.find(" ",start);
        chunks.push_back(text.substr(start, end - start));
    }
    
    std::string soFar,retStr;
    for (auto chunk : chunks) {
        if (soFar.empty()) {
            soFar = chunk;
            continue;
        }
        
        // Try the string with the next chunk
        std::string testStr = soFar.empty() ? chunk : soFar + " " + chunk;
        double width = styleSet->calculateTextWidth(inst,labelInfo,testStr);
        
        // Flush out what we have so far and start with this new chunk
        if (width > textMaxWidth) {
            if (soFar.empty())
                soFar = testStr;

            if (retStr.size() > 0) {
                retStr.append("\n");
            }
            retStr.append(soFar);
            soFar = chunk;
        } else {
            // Keep adding to it
            soFar = testStr;
        }
    }
    if (retStr.size() > 0)
        retStr.append("\n");
    retStr.append(soFar);
    
    return retStr;
}

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

void MapboxVectorLayerSymbol::cleanup(PlatformThreadInfo *inst,ChangeSet &changes)
{
}

static const int ScreenDrawPriorityOffset = 1000000;

void MapboxVectorLayerSymbol::buildObjects(PlatformThreadInfo *inst,
                                           std::vector<VectorObjectRef> &vecObjs,
                                           VectorTileDataRef tileInfo)
{
    if (!visible)
        return;

    ComponentObjectRef compObj = styleSet->makeComponentObject(inst);

    // TODO: They mean displayed level here, which is different from loaded level
//    if (useZoomLevels) {
//        if (minzoom > tileInfo->ident.level)
//          return;
//      if (maxzoom < tileInfo->ident.level)
//          return;
//    }
    
    double textSize = layout.textSize->valForZoom(tileInfo->ident.level);
    // Snap to an integer.  Not clear we need to, just because.
    textSize = (int)(textSize * layout.globalTextScale+0.5);

    LabelInfoRef labelInfo = styleSet->makeLabelInfo(inst,layout.textFontName,textSize);
    labelInfo->screenObject = true;
    labelInfo->fade = 0.0;
    labelInfo->textJustify = WhirlyKitTextCenter;

    if (drawPriorityPerLevel > 0)
        labelInfo->drawPriority = drawPriority + tileInfo->ident.level * drawPriorityPerLevel + ScreenDrawPriorityOffset;
    else
        labelInfo->drawPriority = drawPriority + ScreenDrawPriorityOffset;

    RGBAColorRef textColor = styleSet->resolveColor(paint.textColor, paint.textOpacity, tileInfo->ident.level, MBResolveColorOpacityReplaceAlpha);
    if (textColor)
        labelInfo->textColor = *textColor;

    if (paint.textHaloColor && paint.textHaloWidth > 0.0)
    {
        labelInfo->outlineColor = *paint.textHaloColor;
        labelInfo->outlineSize = paint.textHaloWidth;
    }
    
    // Sort out the image for the marker if we're doing that
    MarkerInfo markerInfo(true);
    bool iconInclude = layout.iconImageField.valid && styleSet->sprites;
    if (iconInclude) {
        markerInfo.programID = styleSet->screenMarkerProgramID;
        markerInfo.drawPriority = labelInfo->drawPriority;
    }
    
//    // Note: Made up value for pushing multi-line text together
//    desc[kMaplyTextLineSpacing] = @(4.0 / 5.0 * font.lineHeight);
    
    bool textInclude = (textColor && textSize > 0.0);
    if (!textInclude && !iconInclude)
        return;
    
    std::vector<SingleLabelRef> labels;
    std::vector<Marker *> markers;
    for (auto vecObj : vecObjs) {
        if (vecObj->getVectorType() == VectorPointType) {
            for (VectorShapeRef shape : vecObj->shapes) {
                VectorPointsRef pts = std::dynamic_pointer_cast<VectorPoints>(shape);
                if (pts) {
                    for (auto pt : pts->pts) {
                        if (textInclude) {
                            // Reconstruct the string from its replacement form
                            std::string text = layout.textField.build(vecObj->getAttributes());
                            
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
                                double textMaxWidth = layout.textMaxWidth->valForZoom(tileInfo->ident.level);
                                if (textMaxWidth != 0.0)
                                    text = breakUpText(inst,text,textMaxWidth * labelInfo->fontPointSize * styleSet->tileStyleSettings->textScale,labelInfo);
                                
                                // Construct the label
                                SingleLabelRef label = styleSet->makeSingleLabel(inst,text);
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
                                float strHash = calcStringHash(text);
                                label->layoutEngine = true;
                                label->layoutImportance = layout.layoutImportance + 1.0 - (rank + (101-tileInfo->ident.level)/100.0)/1000.0 + strHash/1000.0;

                                // Point or line placement
                                if (layout.placement == MBPlaceLine) {
                                    Point2d middle;
                                    double rot;
                                    vecObj->linearMiddle(middle, rot, styleSet->coordSys);
                                    label->loc = GeoCoord(middle.x(),middle.y());
                                    label->rotation = -1 * rot + M_PI/2.0;
                                    if (label->rotation > M_PI_2 || label->rotation < -M_PI_2)
                                        label->rotation += M_PI;
                                    label->keepUpright = true;
                                }
                                
                                // Anchor options for the layout engine
                                switch (layout.textAnchor) {
                                    case MBTextCenter:
                                        label->layoutPlacement = WhirlyKitLayoutPlacementNone;
                                        break;
                                    case MBTextLeft:
                                        label->layoutPlacement = WhirlyKitLayoutPlacementLeft;
                                        break;
                                    case MBTextRight:
                                        label->layoutPlacement = WhirlyKitLayoutPlacementRight;
                                        break;
                                    case MBTextTop:
                                        label->layoutPlacement = WhirlyKitLayoutPlacementAbove;
                                        break;
                                    case MBTextBottom:
                                        label->layoutPlacement = WhirlyKitLayoutPlacementBelow;
                                        break;
                                        // Note: The rest of these aren't quite right
                                    case MBTextTopLeft:
                                        label->layoutPlacement = WhirlyKitLayoutPlacementLeft | WhirlyKitLayoutPlacementAbove;
                                        break;
                                    case MBTextTopRight:
                                        label->layoutPlacement = WhirlyKitLayoutPlacementRight | WhirlyKitLayoutPlacementAbove;
                                        break;
                                    case MBTextBottomLeft:
                                        label->layoutPlacement = WhirlyKitLayoutPlacementLeft | WhirlyKitLayoutPlacementBelow;
                                        break;
                                    case MBTextBottomRight:
                                        label->layoutPlacement = WhirlyKitLayoutPlacementRight | WhirlyKitLayoutPlacementBelow;
                                        break;
                                }
                                
                                labels.push_back(label);
                            } else {
                                wkLogLevel(Warn,"Failed to find text for label");
                            }
                        }
                        if (iconInclude) {
                            // The symbol name might get tricky
                            std::string symbolName = layout.iconImageField.build(vecObj->getAttributes());
                                                        
                            Point2d markerSize;
                            auto subTex = styleSet->sprites->getTexture(symbolName,markerSize);
                            
                            if (markerSize.x() == 0.0) {
                                wkLogLevel(Warn, "MapboxVectorLayerSymbol: Failed to find symbol %s",symbolName.c_str());
                                continue;
                            }
                            
                            markerSize.x() *= layout.iconSize;
                            markerSize.y() *= layout.iconSize;
                            SimpleIdentity markerTexID = subTex.getId();

                            Marker *marker = new Marker();
                            marker->width = markerSize.x();
                            marker->height = markerSize.y();
                            marker->loc = GeoCoord(pt.x(),pt.y());
                            if (selectable) {
                                marker->isSelectable = true;
                                marker->selectID = Identifiable::genId();
                                styleSet->addSelectionObject(marker->selectID, vecObj, compObj);
                                compObj->selectIDs.insert(marker->selectID);
                            }
                            if (markerTexID != EmptyIdentity)
                                marker->texIDs.push_back(markerTexID);
                            markers.push_back(marker);
                        }
                    }
                }
            }
        }
    }

    if (!labels.empty()) {
        SimpleIdentity labelID = styleSet->labelManage->addLabels(inst, labels, *labelInfo, tileInfo->changes);
        if (labelID != EmptyIdentity)
            compObj->labelIDs.insert(labelID);
    }
    
    if (!markers.empty()) {
        SimpleIdentity markerID = styleSet->markerManage->addMarkers(markers, markerInfo, tileInfo->changes);
        if (markerID != EmptyIdentity)
            compObj->markerIDs.insert(markerID);
    }
    
    if (!compObj->labelIDs.empty() || !compObj->markerIDs.empty()) {
        styleSet->compManage->addComponentObject(compObj);
        tileInfo->compObjs.push_back(compObj);
    }
}

}
