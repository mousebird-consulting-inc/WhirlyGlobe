/*  MapboxVectorStyleSymbol.cpp
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/17/15.
 *  Copyright 2011-2021 mousebird consulting
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
#import <vector>
#import <regex>

namespace WhirlyKit
{

static const char * const placementVals[] = {"point","line",nullptr};
static const char * const transformVals[] = {"none","uppercase","lowercase",nullptr};
static const char * const anchorVals[] = {"center","left","right","top","bottom","top-left","top-right","bottom-left","bottom-right",nullptr};

static const char * const justifyVals[] = {"center","left","right",nullptr};

bool MapboxVectorSymbolLayout::parse(PlatformThreadInfo *inst,
                                     MapboxVectorStyleSetImpl *styleSet,
                                     const DictionaryRef &styleEntry)
{
    globalTextScale = styleSet->tileStyleSettings->textScale;
    placement = styleEntry ? (MapboxSymbolPlacement)styleSet->enumValue(styleEntry->getEntry("symbol-placement"), placementVals, (int)MBPlacePoint) : MBPlacePoint;
    textTransform = styleEntry ? (MapboxTextTransform)styleSet->enumValue(styleEntry->getEntry("text-transform"), transformVals, (int)MBTextTransNone) : MBTextTransNone;
    
    textField.parse("text-field",styleSet,styleEntry);

    std::vector<DictionaryEntryRef> textFontArray;
    if (styleEntry)
        textFontArray = styleEntry->getArray("text-font");
    if (!textFontArray.empty()) {
        for (auto & ii : textFontArray) {
            const std::string &textField = ii->getString();
            if (!textField.empty())
                textFontNames.push_back(textField);
        }
    } else {
        // These are the default fonts
        textFontNames.emplace_back("Open Sans Regular");
        textFontNames.emplace_back("Arial Unicode MS Regular");
    }
    textMaxWidth = styleSet->transDouble("text-max-width", styleEntry, 10.0);
    textSize = styleSet->transDouble("text-size", styleEntry, 24.0);

    const auto offsetEntries = styleSet->arrayValue("text-offset", styleEntry);
    textOffsetX = (offsetEntries.size() > 0) ? styleSet->transDouble(offsetEntries[0], 0) : MapboxTransDoubleRef();
    textOffsetY = (offsetEntries.size() > 1) ? styleSet->transDouble(offsetEntries[1], 0) : MapboxTransDoubleRef();

    textAnchor = MBTextCenter;
    if (styleEntry && styleEntry->getType("text-anchor") == DictTypeArray) {
        textAnchor = (MapboxTextAnchor)styleSet->enumValue(styleEntry->getEntry("text-anchor"), anchorVals, (int)MBTextCenter);
    }
    iconAllowOverlap = styleSet->boolValue("icon-allow-overlap", styleEntry, "on", false);
    textAllowOverlap = styleSet->boolValue("text-allow-overlap", styleEntry, "on", false);
    layoutImportance = styleSet->tileStyleSettings->labelImportance;
    textJustifySet = (styleEntry && styleEntry->getEntry("text-justify"));
    textJustify = styleEntry ? (TextJustify)styleSet->enumValue(styleEntry->getEntry("text-justify"), justifyVals, WhirlyKitTextCenter) : WhirlyKitTextCenter;
    
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

    uniqueLabel = styleSet->boolValue("unique-label", styleEntry, "yes", false);

    repUUIDField = styleSet->stringValue("X-Maply-RepresentationUUIDField", styleEntry, std::string());

    uuidField = styleSet->tileStyleSettings->uuidField;
    uuidField = styleSet->stringValue("X-Maply-UUIDField", styleEntry, uuidField);

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
        val += str[ii];
    val /= len * 256.0f;
    
    return val;
}

void MapboxVectorLayerSymbol::cleanup(PlatformThreadInfo *inst,ChangeSet &changes)
{
}

SingleLabelRef MapboxVectorLayerSymbol::setupLabel(PlatformThreadInfo *inst,
                                                   const Point2f &pt,
                                                   const LabelInfoRef &labelInfo,
                                                   const MutableDictionaryRef &attrs,
                                                   const VectorTileDataRef &tileInfo)
{
    // Reconstruct the string from its replacement form
    std::string text = layout.textField.build(attrs);
    
    if (text.empty()) {
        return SingleLabelRef();
    }
    
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
        text = breakUpText(inst,text,textMaxWidth * labelInfo->fontPointSize,labelInfo);
    
    // Construct the label
    SingleLabelRef label = styleSet->makeSingleLabel(inst,text);
    label->loc = GeoCoord(pt.x(),pt.y());
    label->isSelectable = selectable;
    
    if (!uuidField.empty())
        label->uniqueID = attrs->getString(uuidField);
    else if (uniqueLabel) {
        label->uniqueID = text;
        std::transform(label->uniqueID.begin(), label->uniqueID.end(), label->uniqueID.begin(), ::tolower);
    }

    // The rank is most important, followed by the zoom level.  This keeps the countries on top.
    int rank = 1000;
    if (attrs->hasField("rank")) {
        rank = attrs->getInt("rank");
    }
    // Random tweak to cut down on flashing
    // TODO: Move the layout importance into the label itself
    float strHash = calcStringHash(text);
    label->layoutEngine = true;
    if (!layout.textAllowOverlap) {
        // If we're allowing layout, then we need to communicate valid text justification
        //  if the style wanted us to do that
        if (layout.textJustifySet) {
            switch (layout.textJustify) {
                case WhirlyKitTextCenter:
                    label->layoutPlacement = WhirlyKitLayoutPlacementCenter;
                    break;
                case WhirlyKitTextLeft:
                    label->layoutPlacement = WhirlyKitLayoutPlacementLeft;
                    break;
                case WhirlyKitTextRight:
                    label->layoutPlacement = WhirlyKitLayoutPlacementRight;
                    break;
            }
        }
        label->layoutImportance = layout.layoutImportance + 1.0 - (rank + (101-tileInfo->ident.level)/100.0)/1000.0 + strHash/10000.0;
//            wkLogLevel(Debug,"\ntext: %s import = %f, rank = %d, level = %d, strHash = %f",text.c_str(),label->layoutImportance,rank,tileInfo->ident.level,strHash);
    } else
        label->layoutImportance = MAXFLOAT;

    // Anchor options for the layout engine
    switch (layout.textAnchor) {
        case MBTextCenter:
            label->layoutPlacement = WhirlyKitLayoutPlacementCenter;
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
            std::string symbolName = layout.iconImageField->textForZoom(tileInfo->ident.level).build(attrs);
        }
#endif
        return nullptr;
    }
    
    const double size = layout.iconSize->valForZoom(tileInfo->ident.level);
    markerSize.x() *= styleSet->tileStyleSettings->markerScale;
    markerSize.y() *= styleSet->tileStyleSettings->markerScale;
    if (!layout.iconSize->isExpression())
    {
        markerSize.x() *= size;
        markerSize.y() *= size;
    }

    auto marker = std::make_unique<Marker>();
    marker->width = markerSize.x();
    marker->height = markerSize.y();
    marker->loc = GeoCoord(pt.x(),pt.y());
    marker->layoutImportance = layout.iconAllowOverlap ? MAXFLOAT : layout.layoutImportance;

    SimpleIdentity markerTexID = subTex.getId();
    if (markerTexID != EmptyIdentity)
    {
        marker->texIDs.push_back(markerTexID);
    }
    
    return marker;
}

static const int ScreenDrawPriorityOffset = 1000000;

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

    auto const capacity = vecObjs.size() * 5;  // ?
    std::unordered_map<std::string,std::tuple<MarkerPtrVec,VecObjRefVec,LabelRefVec>> markersByUUID(capacity);

    // Render at the max size and then scale dynamically
    double textSize = layout.textSize->maxVal() * layout.globalTextScale;
    textSize = std::max(1.0, std::round(textSize));

    // When there's no dynamic scaling, we need to scale the text size down
    textSize /= styleSet->tileStyleSettings->rendererScale;

    LabelInfoRef labelInfo = styleSet->makeLabelInfo(inst,layout.textFontNames,textSize);
    if (!labelInfo) {
        return;
    }

    labelInfo->hasExp = true;
    labelInfo->zoomSlot = styleSet->zoomSlot;
    if (minzoom != 0 || maxzoom < 1000)
    {
        labelInfo->minZoomVis = minzoom;
        labelInfo->maxZoomVis = maxzoom;
//        wkLogLevel(Debug, "zoomSlot = %d, minZoom = %f, maxZoom = %f",styleSet->zoomSlot,labelInfo->minZoomVis,labelInfo->maxZoomVis);
    }
    labelInfo->screenObject = true;
    labelInfo->fade = 0.0;
    labelInfo->textJustify = layout.textJustify;
    labelInfo->drawPriority = drawPriority + zoomLevel * std::max(0, styleSet->tileStyleSettings->drawPriorityPerLevel) + ScreenDrawPriorityOffset;
    labelInfo->opacityExp = paint.textOpacity->expression();

    // We'll try for one color for the whole thing
    // Note: To fix this we need to blast the text apart into pieces
    const auto textColor = styleSet->resolveColor(paint.textColor, nullptr, zoomLevel, MBResolveColorOpacityReplaceAlpha);
    if (textColor)
    {
        labelInfo->textColor = *textColor;
    }

    // We can apply a scale, but it needs to be scaled to the current text size
    labelInfo->scaleExp = layout.textSize->expression();
    if (labelInfo->scaleExp)
    {
        float maxTextVal = layout.textSize->maxVal();
        for (float &stopOutput : labelInfo->scaleExp->stopOutputs)
        {
            stopOutput /= maxTextVal;
        }
    }

    if (paint.textHaloColor && paint.textHaloWidth)
    {
        labelInfo->outlineColor = paint.textHaloColor->colorForZoom(zoomLevel);
        // Note: We're not using blur right here
        labelInfo->outlineSize = std::max(0.5, (paint.textHaloWidth->valForZoom(zoomLevel) - paint.textHaloBlur->valForZoom(zoomLevel)));
    }

    //    // Note: Made up value for pushing multi-line text together
    //    desc[kMaplyTextLineSpacing] = @(4.0 / 5.0 * font.lineHeight);

    const bool iconInclude = layout.iconImageField && styleSet->sprites;
    const bool textInclude = (textColor && textSize > 0.0 && !layout.textField.chunks.empty());
    if (!textInclude && !iconInclude)
    {
        return;
    }

    // Sort out the image for the marker if we're doing that
    MarkerInfo markerInfo(/*screenObject=*/true);
    markerInfo.hasExp = true;
    markerInfo.zoomSlot = styleSet->zoomSlot;
    markerInfo.scaleExp = layout.iconSize->expression();

    if (minzoom != 0 || maxzoom < 1000)
    {
        markerInfo.minZoomVis = minzoom;
        markerInfo.maxZoomVis = maxzoom;
    }

    if (iconInclude)
    {
        markerInfo.programID = styleSet->screenMarkerProgramID;
        markerInfo.drawPriority = labelInfo->drawPriority;
    }

    ComponentObjectRef compObj = styleSet->makeComponentObject(inst);

    // Calculate the present value of the offsets in ems.
    // This isn't in setupLabel because it only needs to be done once.
    //
    // Positive values indicate right and down, negative values indicate left and up.
    // `label->screenOffset` uses the same convention for Y but the opposite convention for X.
    const Point2d offset = Point2d(layout.textOffsetX ? (layout.textOffsetX->valForZoom(zoomLevel) * textSize) : 0.0,
                                   layout.textOffsetY ? (layout.textOffsetY->valForZoom(zoomLevel) * -textSize) : 0.0);

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

        MarkerPtrVec* markers = nullptr;
        VecObjRefVec* vecObjs = nullptr;
        LabelRefVec*  labels  = nullptr;

        if (vecType == VectorPointType)
        {
            for (const VectorShapeRef &shape : vecObj->shapes)
            {
                if (auto pts = dynamic_cast<VectorPoints*>(shape.get()))
                {
                    if (!markers && !pts->pts.empty())
                    {
                        // Find/create the map entry now that we know there's something to put in it
                        std::tie(markers, vecObjs, labels) = Lookup(uuid, markersByUUID);
                    }

                    for (const auto &pt : pts->pts)
                    {
                        if (textInclude)
                        {
                            if (auto label = setupLabel(inst,pt,labelInfo,attrs,tileInfo))
                            {
                                label->screenOffset = offset;
                                labels->push_back(label);
#if DEBUG
                            }
                            else
                            {
                                wkLogLevel(Warn,"Failed to find text for label");
#endif
                            }
                        }
                        if (iconInclude)
                        {
                            if (auto marker = setupMarker(inst, pt, attrs, tileInfo))
                            {
                                markers->push_back(marker.get());
                                vecObjs->push_back(vecObj);
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
                if (auto line = dynamic_cast<VectorLinear*>(shape.get()))
                {
                    if (!markers)
                    {
                        // Find/create the map entry now that we know there's something to put in it
                        std::tie(markers, vecObjs, labels) = Lookup(uuid, markersByUUID);
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

                    if (textInclude)
                    {
                        if (auto label = setupLabel(inst,pt,labelInfo,attrs,tileInfo))
                        {
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

                            labels->push_back(label);
                        }
                    }
                    
                    if (iconInclude)
                    {
                        if (auto marker = setupMarker(inst, pt, attrs, tileInfo))
                        {
                            markers->push_back(marker.get());
                            vecObjs->push_back(vecObj);
                            markerOwner.emplace_back(std::move(marker));
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
                if (auto areal = dynamic_cast<VectorAreal*>(shape.get()))
                {
                    if (!markers)
                    {
                        // Find/create the map entry now that we know there's something to put in it
                        std::tie(markers, vecObjs, labels) = Lookup(uuid, markersByUUID);
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
                    
                    const auto pt = Point2f(middle.x(), middle.y());

                    if (textInclude)
                    {
                        if (auto label = setupLabel(inst, pt, labelInfo, attrs, tileInfo))
                        {
                            // layout.placement is ignored for polygons
                            // except for offset, which we already calculated so we might as well use
                            label->screenOffset = offset;
                            labels->push_back(label);
                        }
                    }

                    if (iconInclude)
                    {
                        if (auto marker = setupMarker(inst, pt, attrs, tileInfo))
                        {
                            markers->push_back(marker.get());
                            vecObjs->push_back(vecObj);
                            markerOwner.emplace_back(std::move(marker));
                        }
                    }
                }
            }
        }
    }

    for (auto &kvp : markersByUUID)
    {
        if (cancelFn(inst))
        {
            return;
        }

        const auto& uuid = kvp.first;
        const auto& markers = std::get<0>(kvp.second);
        const auto& vecObjs = std::get<1>(kvp.second);
        const auto& labels  = std::get<2>(kvp.second);

        // Generate one component object per unique UUID (including blank)
        const auto compObj = styleSet->makeComponentObject(inst, desc);

        compObj->uuid = uuid;
        compObj->representation = representation;

        if (selectable)
        {
            assert(markers.size() == vecObjs.size());
            const auto count = std::min(markers.size(), vecObjs.size());
            for (auto i = (size_t)0; i < count; ++i)
            {
                if ((i % 100) == 0 && cancelFn(inst))
                {
                    return;
                }

                auto *marker = markers[i];
                const auto &vecObj = vecObjs[i];

                marker->isSelectable = true;
                marker->selectID = Identifiable::genId();
                styleSet->addSelectionObject(marker->selectID, vecObj, compObj);
                compObj->selectIDs.insert(marker->selectID);
                compObj->isSelectable = true;
            }
        }

        if (!labels.empty())
        {
            if (const auto labelID = styleSet->labelManage->addLabels(inst, labels, *labelInfo, tileInfo->changes, cancelFn))
            {
                compObj->labelIDs.insert(labelID);
            }

            if (cancelFn(inst))
            {
                return;
            }
        }

        if (!markers.empty())
        {
            if (const auto markerID = styleSet->markerManage->addMarkers(markers, markerInfo, tileInfo->changes))
            {
                compObj->markerIDs.insert(markerID);
            }

            if (cancelFn(inst))
            {
                return;
            }
        }
        
        if (!compObj->labelIDs.empty() || !compObj->markerIDs.empty())
        {
            styleSet->compManage->addComponentObject(compObj, tileInfo->changes);
            tileInfo->compObjs.push_back(compObj);
        }
    }
}

}   // namespace WhirlyKit
