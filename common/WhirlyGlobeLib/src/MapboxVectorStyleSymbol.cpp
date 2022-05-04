/*  MapboxVectorStyleSymbol.cpp
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/17/15.
 *  Copyright 2011-2022 mousebird consulting
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
 */

#import "MapboxVectorStyleSymbol.h"
#import "Dictionary.h"
#import "WhirlyKitLog.h"
#import <array>
#import <vector>
#import <regex>

namespace WhirlyKit
{

extern const int ScreenDrawPriorityOffset = 1000000;

static const char * const placementVals[] = {"point","line",nullptr};
static const char * const transformVals[] = {"none","uppercase","lowercase",nullptr};
static const char * const anchorVals[] = {"center","left","right","top","bottom","top-left","top-right","bottom-left","bottom-right",nullptr};

static const char * const justifyVals[] = {"center","left","right",nullptr};

bool MapboxVectorSymbolLayout::parse(PlatformThreadInfo *,
                                     MapboxVectorStyleSetImpl *styleSet,
                                     const DictionaryRef &styleEntry)
{
    globalTextScale = styleSet->tileStyleSettings->textScale;
    placement = styleEntry ? (MapboxSymbolPlacement)MapboxVectorStyleSetImpl::enumValue(styleEntry->getEntry("symbol-placement"), placementVals, (int)MBPlacePoint) : MBPlacePoint;
    textTransform = styleEntry ? (MapboxTextTransform)MapboxVectorStyleSetImpl::enumValue(styleEntry->getEntry("text-transform"), transformVals, (int)MBTextTransNone) : MBTextTransNone;
    textField = styleSet->transText("text-field", styleEntry, std::string());

    std::vector<DictionaryEntryRef> textFontArray;
    if (auto entry = styleEntry ? styleEntry->getEntry("text-font") : nullptr)
    {
        // text-font can also be a dictionary for interpolation, which we don't yet support.
        if (entry->getType() == DictionaryType::DictTypeArray)
        {
            textFontArray = entry->getArray();
        }
    }
    if (!textFontArray.empty())
    {
        textFontNames.reserve(textFontArray.size());
        for (const auto &ii : textFontArray)
        {
            std::string fieldI = ii->getString();
            if (!fieldI.empty())
            {
                textFontNames.emplace_back(std::move(fieldI));
            }
        }
    }
    else
    {
        // These are the default fonts
        textFontNames.emplace_back("Open Sans Regular");
        textFontNames.emplace_back("Arial Unicode MS Regular");
    }

    textMaxWidth = styleSet->transDouble("text-max-width", styleEntry, 10.0);
    textSize = styleSet->transDouble("text-size", styleEntry, 24.0);

    const auto offsetEntries = MapboxVectorStyleSetImpl::arrayValue("text-offset", styleEntry);
    textOffsetX = (offsetEntries.size() > 0) ? styleSet->transDouble(offsetEntries[0], 0) : MapboxTransDoubleRef(); //NOLINT
    textOffsetY = (offsetEntries.size() > 1) ? styleSet->transDouble(offsetEntries[1], 0) : MapboxTransDoubleRef();

    if (styleEntry && styleEntry->getType("text-anchor") == DictTypeString)
    {
        textAnchor = (MapboxTextAnchor)MapboxVectorStyleSetImpl::enumValue(
                styleEntry->getEntry("text-anchor"), anchorVals, (int)textAnchor);
    }
    if (styleEntry && styleEntry->getType("icon-anchor") == DictTypeString)
    {
        iconAnchor = (MapboxTextAnchor)MapboxVectorStyleSetImpl::enumValue(
                styleEntry->getEntry("icon-anchor"), anchorVals, (int)iconAnchor);
    }

    iconAllowOverlap = MapboxVectorStyleSetImpl::boolValue("icon-allow-overlap", styleEntry, "on", false);
    textAllowOverlap = MapboxVectorStyleSetImpl::boolValue("text-allow-overlap", styleEntry, "on", false);
    layoutImportance = styleSet->tileStyleSettings->labelImportance;
    textJustifySet = (styleEntry && styleEntry->getEntry("text-justify"));
    textJustify = styleEntry ? (TextJustify)MapboxVectorStyleSetImpl::enumValue(styleEntry->getEntry("text-justify"), justifyVals, WhirlyKitTextCenter) : WhirlyKitTextCenter;
    
    iconImageField = styleSet->transText("icon-image", styleEntry, std::string());
    iconSize = styleSet->transDouble("icon-size", styleEntry, 1.0);

    return true;
}

bool MapboxVectorSymbolPaint::parse(PlatformThreadInfo *,
                                    MapboxVectorStyleSetImpl *styleSet,
                                    const DictionaryRef &styleEntry)
{
    textColor = styleSet->transColor("text-color", styleEntry, RGBAColor::black());
    textOpacity = styleSet->transDouble("text-opacity", styleEntry, 1.0);
    textHaloColor = styleSet->transColor("text-halo-color", styleEntry, RGBAColor::black());
    textHaloBlur = styleSet->transDouble("text-halo-blur", styleEntry, 0.0);
    textHaloWidth = styleSet->transDouble("text-halo-width", styleEntry, 0.0);
    iconOpacity = styleSet->transDouble("icon-opacity", styleEntry, 1.0);

    return true;
}

bool MapboxVectorLayerSymbol::parse(PlatformThreadInfo *inst,
                                    const DictionaryRef &styleEntry,
                                    const MapboxVectorStyleLayerRef &refLayer,
                                    int inDrawPriority)
{
    if (!MapboxVectorStyleLayer::parse(inst,styleEntry,refLayer,drawPriority))
    {
        return false;
    }

    const bool hasLayout = layout.parse(inst,styleSet,styleEntry->getDict("layout"));
    const bool hasPaint = paint.parse(inst,styleSet, styleEntry->getDict("paint"));
    if (!hasLayout && !hasPaint)
        return false;

    uniqueLabel = MapboxVectorStyleSetImpl::boolValue("unique-label", styleEntry, "yes", false);

    repUUIDField = MapboxVectorStyleSetImpl::stringValue("X-Maply-RepresentationUUIDField", styleEntry, std::string());

    uuidField = styleSet->tileStyleSettings->uuidField;
    uuidField = MapboxVectorStyleSetImpl::stringValue("X-Maply-UUIDField", styleEntry, uuidField);

    useZoomLevels = styleSet->tileStyleSettings->useZoomLevels;

    drawPriority = inDrawPriority;
    
    return true;
}

std::string MapboxVectorLayerSymbol::breakUpText(PlatformThreadInfo *inst,
                                                 const std::string &text,
                                                 double textMaxWidth,
                                                 const LabelInfoRef &labelInfo)
{
    // If there are no spaces, let's not break it up
    if (text.find(' ') == std::string::npos)
        return text;
    
    size_t start, end = 0;
    std::vector<std::string> chunks;
    chunks.reserve(text.size() / 5 + 1);
    while ((start = text.find_first_not_of(' ', end)) != std::string::npos) {
        end = text.find(' ',start);
        chunks.push_back(text.substr(start, end - start));
    }

    std::string soFar,retStr,testStr;
    soFar.reserve(text.size());
    retStr.reserve(text.size() + 5);
    for (const auto& chunk : chunks)
    {
        if (soFar.empty())
        {
            soFar = chunk;
            continue;
        }
        
        // Try the string with the next chunk
        if (soFar.empty())
        {
            testStr = chunk;
        }
        else
        {
            testStr.reserve(soFar.size()+chunk.size()+1);
            testStr.assign(soFar).append(" ").append(chunk);
        }
        const double width = styleSet->calculateTextWidth(inst,labelInfo,testStr);
        
        // Flush out what we have so far and start with this new chunk
        if (width > textMaxWidth) {
            if (soFar.empty())
                soFar = testStr;

            if (!retStr.empty()) {
                retStr += '\n';
            }
            retStr.append(soFar);
            soFar = chunk;
        } else {
            // Keep adding to it
            soFar = testStr;
        }
    }
    if (!retStr.empty())
        retStr += '\n';
    retStr.append(soFar);

    return retStr;
}

// Calculate a value [0.0,1.0] for this string
static float calcStringHash(const std::string &str)
{
    unsigned int len = str.length();

    float val = 0.0;
    for (int ii=0;ii<len;ii++)
        val += (float)str[ii];
    return val / (float)len / 256.0f;
}

MapboxVectorStyleLayerRef MapboxVectorLayerSymbol::clone() const
{
    auto layer = std::make_shared<MapboxVectorLayerSymbol>(styleSet);
    layer->copy(*this);
    return layer;
}

MapboxVectorStyleLayer& MapboxVectorLayerSymbol::copy(const MapboxVectorStyleLayer& that)
{
    this->MapboxVectorStyleLayer::copy(that);
    if (const auto line = dynamic_cast<const MapboxVectorLayerSymbol*>(&that))
    {
        // N.B.: paint and symbol settings share refs, may need to deep-copy
        operator=(*line);
    }
    return *this;
}

static int justifyPlacement(TextJustify justify)
{
    switch (justify)
    {
    case WhirlyKitTextCenter: return WhirlyKitLayoutPlacementCenter;
    case WhirlyKitTextLeft:   return WhirlyKitLayoutPlacementLeft;
    case WhirlyKitTextRight:  return WhirlyKitLayoutPlacementRight;
    default:                  return WhirlyKitLayoutPlacementNone;
    }
}

static int anchorPlacement(MapboxTextAnchor anchor)
{
    // Anchor options for the layout engine
    switch (anchor)
    {
    case MBTextCenter:      return WhirlyKitLayoutPlacementCenter;
    case MBTextLeft:        return WhirlyKitLayoutPlacementLeft;
    case MBTextRight:       return WhirlyKitLayoutPlacementRight;
    case MBTextTop:         return WhirlyKitLayoutPlacementAbove;
    case MBTextBottom:      return WhirlyKitLayoutPlacementBelow;
    // Note: The rest of these aren't quite right
    case MBTextTopLeft:     return WhirlyKitLayoutPlacementLeft  | WhirlyKitLayoutPlacementAbove;
    case MBTextTopRight:    return WhirlyKitLayoutPlacementRight | WhirlyKitLayoutPlacementAbove;
    case MBTextBottomLeft:  return WhirlyKitLayoutPlacementLeft  | WhirlyKitLayoutPlacementBelow;
    case MBTextBottomRight: return WhirlyKitLayoutPlacementRight | WhirlyKitLayoutPlacementBelow;
    default:                return WhirlyKitLayoutPlacementNone;
    }
}

static void transformText(std::string &text, MapboxTextTransform tx)
{
    // todo: handle unicode
    switch (tx)
    {
    case MBTextTransUppercase:
        std::transform(text.begin(), text.end(), text.begin(), ::toupper);
        break;
    case MBTextTransLowercase:
        std::transform(text.begin(), text.end(), text.begin(), ::tolower);
        break;
    default: break;
    }
}

SingleLabelRef MapboxVectorLayerSymbol::setupLabel(PlatformThreadInfo *inst,
                                                   const Point2f &pt,
                                                   const LabelInfoRef &labelInfo,
                                                   const MutableDictionaryRef &attrs,
                                                   const VectorTileDataRef &tileInfo,
                                                   bool mergedIcon)
{
    // Reconstruct the string from its replacement form
    std::string text = layout.textField->textForZoom(tileInfo->ident.level).build(attrs);

    if (text.empty())
    {
        return SingleLabelRef();
    }
    
    // Change the text if needed
    transformText(text, layout.textTransform);

    // TODO: Put this back, but we need information about the font
    // Break it up into lines, if necessary
    double textMaxWidth = layout.textMaxWidth->valForZoom(tileInfo->ident.level);
    if (textMaxWidth != 0.0)
    {
        text = breakUpText(inst,text,textMaxWidth * labelInfo->fontPointSize,labelInfo);
    }
    
    // Construct the label
    auto label = styleSet->makeSingleLabel(inst,text);
    label->loc = pt;
    label->isSelectable = selectable;
    
    if (!uuidField.empty())
    {
        label->uniqueID = attrs->getString(uuidField);
    }
    else if (uniqueLabel)
    {
        label->uniqueID = text;
        std::transform(label->uniqueID.begin(), label->uniqueID.end(), label->uniqueID.begin(), ::tolower);
    }

    label->layoutEngine = true;
    label->layoutImportance = MAXFLOAT;     // TODO: Move the layout importance into the label itself
    if (!layout.textAllowOverlap)
    {
        // If we're allowing layout, then we need to communicate valid text justification
        //  if the style wanted us to do that
        if (layout.textJustifySet)
        {
            label->layoutPlacement = justifyPlacement(layout.textJustify);
        }

        // The rank is most important, keeps the countries on top.
        const auto rank = (float)attrs->getInt("rank", 1000);
        const auto rankImport = 1.0f - rank / 1000.0f;
        // Then zoom level.
        const float levelImport = (float)(101 - tileInfo->ident.level) / 100000.0f;
        // Apply a small adjustment to the layout importance based on the string hash so that the
        // values are (almost) certainly unique, making the sort order stable and preventing the
        // layout from changing which items are in front on every pass.
        // todo: this is actually larger than the level value, consider adjusting the denominators
        const auto hashImport = calcStringHash(text) / 10000.0f;
        label->layoutImportance = layout.layoutImportance + rankImport + levelImport + hashImport;
    }

    label->layoutPlacement = anchorPlacement(layout.textAnchor);

    return label;
}

std::unique_ptr<Marker> MapboxVectorLayerSymbol::setupMarker(PlatformThreadInfo *,
                                                             const Point2f &pt,
                                                             const MutableDictionaryRef &attrs,
                                                             const VectorTileDataRef &tileInfo)
{
    // The symbol name might get tricky
    std::string symbolName = layout.iconImageField->textForZoom(tileInfo->ident.level).build(attrs);

    // Sometimes they stick an empty text string in there
    if (symbolName.empty())
        return nullptr;

    Point2d markerSize;
    const auto subTex = styleSet->sprites->getTexture(symbolName,markerSize);

    if (markerSize.x() == 0.0)
    {
#if DEBUG
        wkLogLevel(Warn, "MapboxVectorLayerSymbol: Failed to find symbol %s",symbolName.c_str());
        {
            // Run it again for debugging
            //layout.iconImageField->textForZoom(tileInfo->ident.level).build(attrs);
        }
#endif
        return nullptr;
    }
    
    markerSize *= styleSet->tileStyleSettings->markerScale * styleSet->tileStyleSettings->symbolScale;
    if (!layout.iconSize->isExpression())
    {
        const double size = layout.iconSize->valForZoom(tileInfo->ident.level);
        markerSize *= size;
    }

    auto marker = std::make_unique<Marker>();
    marker->width = markerSize.x();
    marker->height = markerSize.y();
    marker->loc = pt;
    marker->layoutImportance = layout.iconAllowOverlap ? MAXFLOAT : layout.layoutImportance;

    const SimpleIdentity markerTexID = subTex.getId();
    if (markerTexID != EmptyIdentity)
    {
        marker->texIDs.push_back(markerTexID);
    }

    marker->layoutPlacement = anchorPlacement(layout.iconAnchor);

    return marker;
}

using MarkerPtrVec = std::vector<WhirlyKit::Marker*>;
using VecObjRefVec = std::vector<VectorObjectRef>;
using LabelRefVec = std::vector<SingleLabelRef>;
using MarkersByUUIDMap = std::unordered_map<std::string,std::tuple<MarkerPtrVec,VecObjRefVec,LabelRefVec>>;
static const auto emptyMapValue = std::make_tuple(MarkerPtrVec(),VecObjRefVec(),LabelRefVec()); //NOLINT

static std::tuple<MarkerPtrVec*, VecObjRefVec*, LabelRefVec*> Lookup(const std::string &uuid, MarkersByUUIDMap &map)
{
    // Look up the vectors of markers/objects for this uuid (including blank), inserting empty ones if necessary
    const auto result = map.insert(std::make_pair(uuid,emptyMapValue));
    auto &value = result.first->second;
    return std::make_tuple(&std::get<0>(value),&std::get<1>(value),&std::get<2>(value));
}

// Generate a unique ID for matching up the layout objects generated by symbols with both a label and an icon
static std::atomic_int64_t curMergeId;
template <std::size_t N>
static const char *genMergeId(std::array<char,N> &buf)
{
    const size_t id = ++curMergeId;
    const int res = snprintf(&buf[0], buf.size() - 1, "%zd", id);
    return (res > 0 && res < buf.size() - 1) ? &buf[0] : nullptr;
}

void MapboxVectorLayerSymbol::buildObjects(PlatformThreadInfo *inst,
                                           const std::vector<VectorObjectRef> &vecObjs,
                                           const VectorTileDataRef &tileInfo,
                                           const Dictionary *desc,
                                           const CancelFunction &cancelFn)
{
    // If a representation is set, we produce results for non-visible layers
    if (!visible && (representation.empty() || repUUIDField.empty()))
    {
        return;
    }

    const auto zoomLevel = tileInfo->ident.level;

    // We'll try for one color for the whole thing
    // Note: To fix this we need to blast the text apart into pieces
    const auto textColor = MapboxVectorStyleSetImpl::resolveColor(paint.textColor, nullptr, zoomLevel,
                                                                  MBResolveColorOpacityReplaceAlpha);

    const auto textField = (textColor && layout.textField) ?
                           layout.textField->textForZoom(zoomLevel) : MapboxRegexField();

    const bool iconInclude = layout.iconImageField && styleSet->sprites;
    bool textInclude = (textField.valid && !textField.chunks.empty());
    if (!textInclude && !iconInclude)
    {
        return;
    }

    // If we're doing a merged symbol, the font height needs to be treated differently
    const auto merged = textInclude && iconInclude;

    // If we will be producing both icons and text, it's likely that their sizes need to correspond
    // (e.g., highway shields) so we can't apply independent scale factors.  For now, use the marker
    // scales for the text instead of the normal text scale.
    const auto renderScale = styleSet->tileStyleSettings->rendererScale;
    // see setupMarker
    const auto markerCombinedScale =
        styleSet->tileStyleSettings->markerScale * styleSet->tileStyleSettings->symbolScale *
        (layout.iconSize->isExpression() ? 1.0 : layout.iconSize->valForZoom(tileInfo->ident.level));
    // todo: An extra renderScale (or just 2?) seems to be needed here, why?
    const auto textScale = merged ? markerCombinedScale * renderScale : layout.globalTextScale;
    // Render at the max size and then scale dynamically
    const auto textSize = (float)(layout.textSize->maxVal() * textScale / renderScale);

    // todo: if icon size is an expression, make text size an expression based on it?
    //if (layout.iconSize->isExpression())
    //{
    //}

    if (textSize < 1)
    {
        return;
    }

    const auto labelInfo = styleSet->makeLabelInfo(inst,layout.textFontNames,textSize,merged);
    if (!labelInfo)
    {
        return;
    }

    const auto priority = drawPriority + ScreenDrawPriorityOffset + zoomLevel *
                            std::max(0, styleSet->tileStyleSettings->drawPriorityPerLevel);
    if (textInclude)
    {
        labelInfo->zoomSlot = styleSet->zoomSlot;
        if (minzoom != 0 || maxzoom < 1000)
        {
            labelInfo->minZoomVis = minzoom;
            labelInfo->maxZoomVis = maxzoom;
        }
        labelInfo->screenObject = true;
        labelInfo->textJustify = layout.textJustify;
        labelInfo->drawPriority = priority;
        labelInfo->opacityExp = paint.textOpacity->expression();
        labelInfo->textColor = textColor ? *textColor : RGBAColor::white();

        // We can apply a scale, but it needs to be scaled to the current text size.
        // That is, the expression produces [0.0,1.0] when is then multiplied by textSize
        labelInfo->scaleExp = layout.textSize->expression();
        if (labelInfo->scaleExp)
        {
            const auto maxTextVal = (float)layout.textSize->maxVal();
            for (float &stopOutput : labelInfo->scaleExp->stopOutputs)
            {
                stopOutput /= maxTextVal;
            }
        }

        if (paint.textHaloColor && paint.textHaloWidth)
        {
            labelInfo->outlineColor = paint.textHaloColor->colorForZoom(zoomLevel);
            // Note: We're not using blur right here
            labelInfo->outlineSize = std::max(0.5, (paint.textHaloWidth->valForZoom(zoomLevel) -
                                                    paint.textHaloBlur->valForZoom(zoomLevel)));
        }

        labelInfo->hasExp = labelInfo->scaleExp || labelInfo->opacityExp;
    }

    // Note: Made up value for pushing multi-line text together
    //desc[kMaplyTextLineSpacing] = @(4.0 / 5.0 * font.lineHeight);

    // Sort out the image for the marker if we're doing that
    MarkerInfo markerInfo(/*screenObject=*/true);
    markerInfo.zoomSlot = styleSet->zoomSlot;
    markerInfo.scaleExp = layout.iconSize->expression();
    markerInfo.opacityExp = paint.iconOpacity->expression();

    if (minzoom != 0 || maxzoom < 1000)
    {
        markerInfo.minZoomVis = minzoom;
        markerInfo.maxZoomVis = maxzoom;
    }

    if (iconInclude)
    {
        markerInfo.programID = styleSet->screenMarkerProgramID;
        markerInfo.drawPriority = priority;
    }

    markerInfo.hasExp = markerInfo.colorExp || markerInfo.scaleExp || markerInfo.opacityExp;

    // Calculate the present value of the offsets in ems.
    // This isn't in setupLabel because it only needs to be done once.
    //
    // Positive values indicate right and down, negative values indicate left and up.
    // `label->screenOffset` uses the same convention for Y but the opposite convention for X.
    const Point2d offset = Point2d(layout.textOffsetX ? (layout.textOffsetX->valForZoom(zoomLevel) * textSize) : 0.0,
                                   layout.textOffsetY ? (layout.textOffsetY->valForZoom(zoomLevel) * -textSize) : 0.0);

    auto const capacity = vecObjs.size() * 5;  // ?
    std::unordered_map<std::string,std::tuple<MarkerPtrVec,VecObjRefVec,LabelRefVec>> markersByUUID(capacity);

    std::array<char,32LL> mergeIdent = {'\0'};
    std::vector<std::unique_ptr<Marker>> markerOwner;
    for (const auto& vecObj : vecObjs)
    {
        if (cancelFn(inst))
        {
            return;
        }
        const auto vecType = vecObj->getVectorType();
        switch (vecType)
        {
            case VectorPointType:
            case VectorLinearType:
            case VectorArealType: break;
            default: continue;
        }

        const auto &attrs = vecObj->getAttributes();
        const auto uuid = repUUIDField.empty() ? std::string() : attrs->getString(repUUIDField);

        MarkerPtrVec* uuidMarkers = nullptr;
        VecObjRefVec* uuidVecObjs = nullptr;
        LabelRefVec*  uuidLabels  = nullptr;

        if (vecType == VectorPointType)
        {
            for (const VectorShapeRef &shape : vecObj->shapes)
            {
                if (auto pts = dynamic_cast<VectorPoints*>(shape.get()))
                {
                    if (!uuidMarkers && !pts->pts.empty())
                    {
                        // Find/create the map entry now that we know there's something to put in it
                        std::tie(uuidMarkers, uuidVecObjs, uuidLabels) = Lookup(uuid, markersByUUID);
                    }

                    for (const auto &pt : pts->pts)
                    {
                        if (textInclude)
                        {
                            if (auto label = setupLabel(inst,pt,labelInfo,attrs,tileInfo,iconInclude))
                            {
                                if (iconInclude)
                                {
                                    genMergeId(mergeIdent);
                                    label->mergeID = &mergeIdent[0];
                                }

                                label->screenOffset = offset;
                                uuidLabels->push_back(label);
#if DEBUG
                            }
                            else
                            {
                                static std::mutex mutex;
                                static std::set<std::string> warnedFields;
                                std::lock_guard<std::mutex> lock(mutex);
                                const std::string fieldDesc = textField.buildDesc(attrs);
                                if (warnedFields.insert(this->ident+fieldDesc).second)
                                {
                                    wkLogLevel(Warn, "Failed to find text for %s / %s",
                                               this->ident.c_str(), fieldDesc.c_str());
                                }
#endif
                            }
                        }
                        if (iconInclude)
                        {
                            if (auto marker = setupMarker(inst, pt, attrs, tileInfo))
                            {
                                if (textInclude)
                                {
                                    marker->mergeID = &mergeIdent[0];
                                }

                                uuidMarkers->push_back(marker.get());
                                uuidVecObjs->push_back(vecObj);
                                markerOwner.emplace_back(std::move(marker));
                            }
                        }
                    }
                }
            }
        }
        else if (vecType == VectorLinearType)
        {
#if DEBUG
            if (vecObj->shapes.size() > 1)
            {
                static int warned = 0;
                if (!warned++)
                {
                    wkLogLevel(Warn, "MapboxVectorLayerSymbol: Linear vector object contains %d shapes", vecObj->shapes.size());
                }
            }
#endif
            for (const VectorShapeRef &shape : vecObj->shapes)
            {
                // for each line in the shape set ... (we expect exactly one)
                if (__unused auto line = dynamic_cast<VectorLinear*>(shape.get()))
                {
                    if (!uuidMarkers)
                    {
                        // Find/create the map entry now that we know there's something to put in it
                        std::tie(uuidMarkers, uuidVecObjs, uuidLabels) = Lookup(uuid, markersByUUID);
                    }

                    // Place the symbol at the middle of the line
                    // Note that if there are multiple shapes, this will be recalculated unnecessarily.
                    Point2d middle;
                    double rot;
                    if (!vecObj->linearMiddle(middle, rot, styleSet->coordSys))
                    {
#if DEBUG
                        wkLogLevel(Warn, "MapboxVectorLayerSymbol: Failed to compute middle of linear shape");
#endif
                        continue;
                    }
                    
                    const auto pt = Point2f(middle.x(), middle.y());

                    bool markerAdded = false;
                    if (iconInclude)
                    {
                        if (auto marker = setupMarker(inst, pt, attrs, tileInfo))
                        {
                            if (textInclude)
                            {
                                genMergeId(mergeIdent);
                                marker->mergeID = &mergeIdent[0];
                            }

                            uuidMarkers->push_back(marker.get());
                            uuidVecObjs->push_back(vecObj);
                            markerOwner.emplace_back(std::move(marker));
                            markerAdded = true;
                        }
                    }

                    if (textInclude)
                    {
                        if (auto label = setupLabel(inst,pt,labelInfo,attrs,tileInfo,iconInclude))
                        {
                            if (markerAdded)
                            {
                                label->mergeID = &mergeIdent[0];
                            }

                            if (layout.placement == MBPlaceLine)
                            {
                                label->rotation = (float)(-rot + M_PI/2.0);
                                if (label->rotation > M_PI_2 || label->rotation < -M_PI_2)
                                {
                                    label->rotation += M_PI;
                                }
                                label->keepUpright = true;
                            }

                            label->screenOffset = offset;

                            uuidLabels->push_back(label);
                        }
                    }
                }
            }
        }
        else if (vecType == VectorArealType)
        {
#if DEBUG
            if (vecObj->shapes.size() > 1)
            {
                static int warned = 0;
                if (!warned++)
                {
                    wkLogLevel(Warn, "MapboxVectorLayerSymbol: Areal vector object contains %d shapes", vecObj->shapes.size());
                }
            }
#endif
            for (const auto &shape : vecObj->shapes)
            {
                // each polygon in the shape set... (we expect exactly one)
                if (__unused auto areal = dynamic_cast<VectorAreal*>(shape.get()))
                {
                    if (!uuidMarkers)
                    {
                        // Find/create the map entry now that we know there's something to put in it
                        std::tie(uuidMarkers, uuidVecObjs, uuidLabels) = Lookup(uuid, markersByUUID);
                    }

                    // Place the marker at the middle of the polygon.
                    // Note that if there are multiple shapes, this will be recalculated unnecessarily.
                    Point2d middle;
                    if (!vecObj->center(middle))
                    {
#if DEBUG
                        wkLogLevel(Warn, "MapboxVectorLayerSymbol: Failed to compute center of areal shape");
#endif
                        continue;
                    }
                    
                    const Point2f pt = middle.cast<float>();

                    bool markerAdded = false;
                    if (iconInclude)
                    {
                        if (auto marker = setupMarker(inst, pt, attrs, tileInfo))
                        {
                            if (textInclude)
                            {
                                genMergeId(mergeIdent);
                                marker->mergeID = &mergeIdent[0];
                            }

                            uuidMarkers->push_back(marker.get());
                            uuidVecObjs->push_back(vecObj);
                            markerOwner.emplace_back(std::move(marker));
                            markerAdded = true;
                        }
                    }

                    if (textInclude)
                    {
                        if (auto label = setupLabel(inst, pt, labelInfo, attrs, tileInfo, iconInclude))
                        {
                            if (markerAdded)
                            {
                                label->mergeID = &mergeIdent[0];
                            }

                            // layout.placement is ignored for polygons
                            // except for offset, which we already calculated so we might as well use
                            label->screenOffset = offset;
                            uuidLabels->push_back(label);
                        }
                    }
                }
            }
        }
    }

    for (auto &kvp : markersByUUID)
    {
        const auto& uuid = kvp.first;
        const auto& uuidMarkers = std::get<0>(kvp.second);
        const auto& uuidVecObjs = std::get<1>(kvp.second);
        const auto& uuidLabels  = std::get<2>(kvp.second);

        if (uuidLabels.empty() && uuidMarkers.empty())
        {
            continue;
        }
        if (cancelFn(inst) || styleSet->markerManage->isShuttingDown())
        {
            break;
        }

        // Generate one component object per unique UUID (including blank)
        auto uuidCompObj = styleSet->makeComponentObject(inst, desc);

        if (selectable)
        {
            assert(uuidMarkers.size() == uuidVecObjs.size());
            const auto count = std::min(uuidMarkers.size(), uuidVecObjs.size());
            for (auto i = (size_t)0; i < count; ++i)
            {
                auto *marker = uuidMarkers[i];
                const auto &vecObj = uuidVecObjs[i];

                marker->isSelectable = true;
                marker->selectID = Identifiable::genId();
                styleSet->addSelectionObject(marker->selectID, vecObj, uuidCompObj);
                uuidCompObj->selectIDs.insert(marker->selectID);
                uuidCompObj->isSelectable = true;
            }
        }

        if (!uuidLabels.empty())
        {
            if (const auto labelID = styleSet->labelManage->addLabels(inst, uuidLabels, *labelInfo, tileInfo->changes, cancelFn))
            {
                uuidCompObj->labelIDs.insert(labelID);
            }
        }

        if (!uuidMarkers.empty())
        {
            if (const auto markerID = styleSet->markerManage->addMarkers(uuidMarkers, markerInfo, tileInfo->changes))
            {
                uuidCompObj->markerIDs.insert(markerID);
            }
        }
        
        if (!uuidCompObj->labelIDs.empty() || !uuidCompObj->markerIDs.empty())
        {
            uuidCompObj->uuid = uuid;
            uuidCompObj->representation = representation;

            styleSet->compManage->addComponentObject(uuidCompObj, tileInfo->changes);
            tileInfo->compObjs.push_back(std::move(uuidCompObj));
        }
    }
}

}   // namespace WhirlyKit
