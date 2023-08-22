/*  MapboxVectorStyleLine.cpp
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

#import "MapboxVectorStyleLine.h"
#import "WhirlyKitLog.h"

#include <algorithm>
#include <numeric>

namespace WhirlyKit
{

static const char * const lineCapVals[] = {"butt","round","square",nullptr};
static const char * const joinVals[] = {"bevel","round","miter",nullptr};

bool MapboxVectorLineLayout::parse(PlatformThreadInfo *,MapboxVectorStyleSetImpl *, const DictionaryRef &styleEntry)
{
    if (const auto entry = styleEntry ? styleEntry->getEntry("line-join") : nullptr)
    {
        const auto joinVal = MapboxVectorStyleSetImpl::enumValue(entry,joinVals, -1);
        if (joinVal >= 0) {
            join = (MapboxVectorLineJoin)joinVal;
            joinSet = true;
        }
    }
    cap = styleEntry ? (MapboxVectorLineCap)MapboxVectorStyleSetImpl::enumValue(styleEntry->getEntry("line-cap"),lineCapVals,(int)MBLineCapButt) : MBLineCapButt;
    miterLimit = MapboxVectorStyleSetImpl::doubleValue("line-miter-limit", styleEntry, 2.0);
    roundLimit = MapboxVectorStyleSetImpl::doubleValue("line-round-limit", styleEntry, 1.0);

    return true;
}

bool MapboxVectorLinePaint::parse(PlatformThreadInfo *,MapboxVectorStyleSetImpl *styleSet,const DictionaryRef &styleEntry)
{
    MapboxVectorStyleSetImpl::unsupportedCheck("line-translate", "line-paint", styleEntry);
    MapboxVectorStyleSetImpl::unsupportedCheck("line-translate-anchor", "line-paint", styleEntry);
    MapboxVectorStyleSetImpl::unsupportedCheck("line-gap-width", "line-paint", styleEntry);
    MapboxVectorStyleSetImpl::unsupportedCheck("line-blur", "line-paint", styleEntry);
    MapboxVectorStyleSetImpl::unsupportedCheck("line-image", "line-paint", styleEntry);
    
    opacity = styleSet->transDouble("line-opacity", styleEntry, 1.0);
    width = styleSet->transDouble("line-width", styleEntry, 1.0);
    offset = styleSet->transDouble("line-offset", styleEntry, 0.0);
    color = styleSet->transColor("line-color", styleEntry, RGBAColor::black());
    pattern = MapboxVectorStyleSetImpl::stringValue("line-pattern", styleEntry, "");
    
    if (styleEntry && styleEntry->getType("line-dasharray") == DictTypeArray) {
        auto vecArray = styleEntry->getArray("line-dasharray");
        for (const auto &entry : vecArray) {
            if (entry->getType() == DictTypeDouble || entry->getType() == DictTypeInt) {
                lineDashArray.push_back(entry->getDouble());
            } else {
                wkLogLevel(Warn,"Encountered non-double type in MapboxVectorLinePaint dashArray");
            }
        }
    }
    
    return true;
}

bool MapboxVectorLayerLine::parse(PlatformThreadInfo *inst,
                                  const DictionaryRef &styleEntry,
                                  const MapboxVectorStyleLayerRef &refLayer,
                                  int drawPriority)
{
    if (!MapboxVectorStyleLayer::parse(inst, styleEntry,refLayer,drawPriority) ||
        !layout.parse(inst, styleSet, styleEntry->getDict("layout")) ||
        !paint.parse(inst, styleSet, styleEntry->getDict("paint")))
    {
        return false;
    }
    
    this->drawPriority = MapboxVectorStyleSetImpl::intValue("drawPriority", styleEntry, drawPriority);
    linearClipToBounds = MapboxVectorStyleSetImpl::boolValue("linearize-clip-to-bounds", styleEntry, "yes", false);
    dropGridLines = MapboxVectorStyleSetImpl::boolValue("drop-grid-lines", styleEntry, "yes", false);
    subdivToGlobe = MapboxVectorStyleSetImpl::doubleValue("subdiv-to-globe", styleEntry, 0.0);

    filledLineTexID = EmptyIdentity;
    if (!paint.lineDashArray.empty())
    {
        const double width = (paint.width ? paint.width->maxVal() : 1.0);
        const double maxWidth = width * styleSet->tileStyleSettings->lineScale;

        // Figure out the total length
        totLen = std::accumulate(paint.lineDashArray.begin(), paint.lineDashArray.end(), 0.0);

        const double factor = std::max(64U, NextPowOf2((unsigned)totLen)) / totLen;

        std::vector<double> dashComponents(paint.lineDashArray.size());
        std::transform(paint.lineDashArray.begin(), paint.lineDashArray.end(),
                       dashComponents.begin(), [=](double n){ return n * factor; });

        totLen *= maxWidth;

        filledLineTexID = styleSet->makeLineTexture(inst, std::move(dashComponents));
    }
    fade = MapboxVectorStyleSetImpl::doubleValue("fade",styleEntry,0.0);

    repUUIDField = MapboxVectorStyleSetImpl::stringValue("X-Maply-RepresentationUUIDField", styleEntry, std::string());

    lineScale = styleSet->tileStyleSettings->lineScale;
    
    uuidField = styleSet->tileStyleSettings->uuidField;
    uuidField = MapboxVectorStyleSetImpl::stringValue("X-Maply-UUIDField", styleEntry, uuidField);

    return true;
}

MapboxVectorStyleLayerRef MapboxVectorLayerLine::clone() const
{
    auto layer = std::make_shared<MapboxVectorLayerLine>(styleSet);
    layer->copy(*this);
    return layer;
}

MapboxVectorStyleLayer& MapboxVectorLayerLine::copy(const MapboxVectorStyleLayer& that)
{
    this->MapboxVectorStyleLayer::copy(that);
    if (const auto line = dynamic_cast<const MapboxVectorLayerLine*>(&that))
    {
        operator=(*line);
    }
    return *this;
}

static const std::string colorStr = "color"; // NOLINT(cert-err58-cpp)   constructor can throw

static WideVectorLineJoinType convertJoin(MapboxVectorLineJoin join)
{
    switch (join)
    {
        default:
        case MBLineJoinMiter: return WideVecMiterJoin;
        case MBLineJoinBevel: return WideVecBevelJoin;
        case MBLineJoinRound: return WideVecRoundJoin;
    }
}

static WideVectorLineCapType convertCap(MapboxVectorLineCap cap)
{
    switch (cap)
    {
        default:
        case MBLineCapButt: return WideVecButtCap;
        case MBLineCapRound: return WideVecRoundCap;
        case MBLineCapSquare: return WideVecSquareCap;
    }
}

void MapboxVectorLayerLine::buildObjects(PlatformThreadInfo *inst,
                                         const std::vector<VectorObjectRef> &inVecObjs,
                                         const VectorTileDataRef &tileInfo,
                                         const Dictionary *desc,
                                         const CancelFunction &cancelFn)
{
    // If a representation is set, we produce results for non-visible layers
    if (!visible && (representation.empty() || repUUIDField.empty()))
    {
        return;
    }

    // Turn into linears (if not already) and then clip to the bounds
    // Slightly different, but we want to clip all the areals that are converted to linears
    std::vector<VectorObjectRef> vecObjs;
    vecObjs.reserve(inVecObjs.size());
    for (auto const &vecObj : inVecObjs)
    {
        bool clip = linearClipToBounds;
        
        VectorObjectRef newVecObj = vecObj;
        if (dropGridLines)
        {
            if (auto clipped = newVecObj->filterClippedEdges())
            {
                newVecObj = std::move(clipped);
            }
        }
        
        if (newVecObj->getVectorType() == VectorArealType)
        {
            newVecObj = newVecObj->arealsToLinears();
            clip = true;
        }
        if (newVecObj && clip)
        {
            newVecObj = newVecObj->clipToMbr(tileInfo->geoBBox.ll(), tileInfo->geoBBox.ur());
        }
        if (newVecObj)
        {
            vecObjs.push_back(newVecObj);
        }
    }

    // Subdivide long-ish lines to the globe, if set
    if (subdivToGlobe > 0.0)
    {
        std::vector<VectorObjectRef> newVecObjs;
        newVecObjs.reserve(3 * vecObjs.size());
        for (const auto &vecObj : vecObjs)
        {
            vecObj->subdivideToGlobe((float)subdivToGlobe);
            newVecObjs.push_back(vecObj);
        }
        vecObjs = newVecObjs;
    }
    
    // If we have a filled texture, we'll use that
    const auto repeatLen = (float)totLen;
    
    // TODO: We can also have a symbol, where we might do the same thing
    // Problem is, we'll need to pass the sub-texture logic through to the renderer
    //  because right now it's expecting a single texture that can be strung along the line

    MBResolveColorType resolveMode = MBResolveColorOpacityComposeAlpha;
#ifdef __ANDROID__
    // On Android, pre-multiply the alpha on static colors.
    // When the color or opacity is dynamic, we need to do it in the tweaker.
    if ((!paint.color || !paint.color->isExpression()) &&
        (!paint.opacity || !paint.opacity->isExpression()))
    {
        resolveMode = MBResolveColorOpacityMultiply;
    }
#endif

    const RGBAColorRef color = MapboxVectorStyleSetImpl::resolveColor(paint.color, paint.opacity, tileInfo->ident.level, resolveMode);

    const double width = paint.width->valForZoom(tileInfo->ident.level) * lineScale;
    const double offset = paint.offset->valForZoom(tileInfo->ident.level) * lineScale;
    
    if (!color || width <= 0.0)
    {
        return;
    }

    WideVectorInfo vecInfo;
    vecInfo.coordType = WideVecCoordScreen;
    vecInfo.fadeIn = fade;
    vecInfo.fadeOut = fade;
    vecInfo.zoomSlot = styleSet->zoomSlot;
    vecInfo.color = *color;
    vecInfo.width = (float)width;
    vecInfo.offset = (float)-offset;
    vecInfo.joinType = layout.joinSet ? convertJoin(layout.join) : WideVecMiterSimpleJoin;
    vecInfo.capType = convertCap(layout.cap);
    vecInfo.widthExp = paint.width->expression();
    vecInfo.offsetExp = paint.offset->expression();
    vecInfo.colorExp = paint.color->expression();
    vecInfo.opacityExp = paint.opacity->expression();
    vecInfo.hasExp = vecInfo.widthExp || vecInfo.offsetExp || vecInfo.colorExp || vecInfo.opacityExp;
    vecInfo.drawPriority = drawPriority + tileInfo->ident.level * std::max(0, styleSet->tileStyleSettings->drawPriorityPerLevel)+2;
    vecInfo.implType = styleSet->tileStyleSettings->perfWideVec ? WideVecImplPerf : WideVecImplBasic;
    vecInfo.programID = styleSet->tileStyleSettings->perfWideVec ? styleSet->wideVectorPerfProgramID : styleSet->wideVectorProgramID;
    // TODO: Switch to stencils
//        vecInfo.drawOrder = tileInfo->tileNumber();

    // Legacy wide vectors have limited join support
    if (!styleSet->tileStyleSettings->perfWideVec)
    {
        switch (vecInfo.joinType)
        {
            case WideVecMiterClipJoin:
            case WideVecMiterSimpleJoin:
            case WideVecRoundJoin:
            case WideVecNoneJoin:
                vecInfo.joinType = WideVecMiterJoin;
            default: break;
        }
    }

    if (minzoom != 0 || maxzoom < 1000)
    {
        vecInfo.minZoomVis = minzoom;
        vecInfo.maxZoomVis = maxzoom;
    }
    if (filledLineTexID != EmptyIdentity)
    {
        vecInfo.texID = filledLineTexID;
        vecInfo.repeatSize = repeatLen;
    }
    if (vecInfo.widthExp)
    {
        vecInfo.widthExp->scaleBy(lineScale);
    }
    if (vecInfo.offsetExp)
    {
        vecInfo.offsetExp->scaleBy(-lineScale);
    }

    using ShapeRefVec = std::vector<VectorShapeRef>;
    auto const capacity = inVecObjs.size() * 5;  // ?
    std::unordered_map<std::string,ShapeRefVec> shapesByUUID(capacity);

    const bool colorOverride = styleSet->tileStyleSettings->enableOverrideColor;

    // Gather all the linear features
    for (const auto &vecObj : vecObjs)
    {
        if (vecObj->getVectorType() != VectorLinearType)
        {
            continue;
        }
        if (cancelFn(inst))
        {
            return;
        }

        const auto attrs = vecObj->getAttributes();
        const auto uuid = repUUIDField.empty() ? std::string() : attrs->getString(repUUIDField);

        // Look up the vectors of markers/objects for this uuid (including blank), inserting empty ones if necessary
        const auto result = shapesByUUID.insert(std::make_pair(std::ref(uuid), ShapeRefVec()));
        auto &shapes = result.first->second;

        if (shapes.empty())
            shapes.reserve(shapes.size() + vecObj->shapes.size());
        std::copy(vecObj->shapes.begin(),vecObj->shapes.end(),std::back_inserter(shapes));

        // If individual vector objects aren't allowed to override colors, drop the color attribute.
        if (!colorOverride)
        {
            attrs->removeField(colorStr);
        }
    }

    for (const auto &kvp : shapesByUUID)
    {
        if (cancelFn(inst))
        {
            return;
        }

        const auto &uuid = kvp.first;
        const auto &shapes = kvp.second;

        if (const auto wideVecID = styleSet->wideVecManage->addVectors(shapes, vecInfo, tileInfo->changes))
        {
            // Generate one component object per unique UUID (including blank)
            auto compObj = styleSet->makeComponentObject(inst, desc);

            compObj->uuid = uuid;
            compObj->representation = representation;
            compObj->wideVectorIDs.insert(wideVecID);
            styleSet->compManage->addComponentObject(compObj, tileInfo->changes);
            tileInfo->compObjs.push_back(std::move(compObj));
        }
    }
}

}
