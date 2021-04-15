/*  MapboxVectorStyleLine.cpp
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

#import "MapboxVectorStyleLine.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{

static const char * const lineCapVals[] = {"butt","round","square",nullptr};
static const char * const joinVals[] = {"bevel","round","miter",nullptr};

bool MapboxVectorLineLayout::parse(PlatformThreadInfo *,MapboxVectorStyleSetImpl *styleSet,const DictionaryRef &styleEntry)
{
    cap = styleEntry ? (MapboxVectorLineCap)styleSet->enumValue(styleEntry->getEntry("line-cap"),lineCapVals,(int)MBLineCapButt) : MBLineCapButt;
    join = styleEntry ? (MapboxVectorLineJoin)styleSet->enumValue(styleEntry->getEntry("line-join"),joinVals,(int)MBLineJoinMiter) : MBLineJoinMiter;
    miterLimit = styleSet->doubleValue("line-miter-limit", styleEntry, 2.0);
    roundLimit = styleSet->doubleValue("line-round-limit", styleEntry, 1.0);

    return true;
}

bool MapboxVectorLinePaint::parse(PlatformThreadInfo *,MapboxVectorStyleSetImpl *styleSet,const DictionaryRef &styleEntry)
{
    styleSet->unsupportedCheck("line-translate", "line-paint", styleEntry);
    styleSet->unsupportedCheck("line-translate-anchor", "line-paint", styleEntry);
    styleSet->unsupportedCheck("line-gap-width", "line-paint", styleEntry);
    styleSet->unsupportedCheck("line-blur", "line-paint", styleEntry);
    styleSet->unsupportedCheck("line-image", "line-paint", styleEntry);
    
    opacity = styleSet->transDouble("line-opacity", styleEntry, 1.0);
    width = styleSet->transDouble("line-width", styleEntry, 1.0);
    offset = styleSet->transDouble("line-offset", styleEntry, 0.0);
    color = styleSet->transColor("line-color", styleEntry, RGBAColor::black());
    pattern = styleSet->stringValue("line-pattern", styleEntry, "");
    
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
    
    this->drawPriority = styleSet->intValue("drawPriority", styleEntry, drawPriority);
    linearClipToBounds = styleSet->boolValue("linearize-clip-to-bounds", styleEntry, "yes", false);
    dropGridLines = styleSet->boolValue("drop-grid-lines", styleEntry, "yes", false);
    subdivToGlobe = styleSet->doubleValue("subdiv-to-globe", styleEntry, 0.0);

    filledLineTexID = EmptyIdentity;
    if (!paint.lineDashArray.empty())
    {
        totLen = 0.0;
        double maxWidth = paint.width->maxVal() * styleSet->tileStyleSettings->lineScale;

        // Figure out the total length
        for (double val : paint.lineDashArray)
            totLen += val;

        unsigned totLenRounded = NextPowOf2((unsigned)totLen);
        if (totLenRounded < 64)
            totLenRounded = 64;
        std::vector<double> dashComponents;
        dashComponents.reserve(paint.lineDashArray.size());
        for (double val : paint.lineDashArray)
        {
            const double len = val * totLenRounded / totLen;
            dashComponents.push_back(len);
        }
        totLen *= maxWidth;
        
        filledLineTexID = styleSet->makeLineTexture(inst,dashComponents);
    }
    fade = styleSet->doubleValue("fade",styleEntry,0.0);

    repUUIDField = styleSet->stringValue("X-Maply-RepresentationUUIDField", styleEntry, std::string());

    lineScale = styleSet->tileStyleSettings->lineScale;
    
    uuidField = styleSet->tileStyleSettings->uuidField;
    uuidField = styleSet->stringValue("X-Maply-UUIDField", styleEntry, uuidField);

    return true;
}

void MapboxVectorLayerLine::cleanup(PlatformThreadInfo *inst,ChangeSet &changes)
{
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
            newVecObj = newVecObj->filterClippedEdges();
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
    
    const RGBAColorRef color = styleSet->resolveColor(paint.color, paint.opacity, tileInfo->ident.level, MBResolveColorOpacityMultiply);
    const double width = paint.width->valForZoom(tileInfo->ident.level) * lineScale;
    const double offset = paint.offset->valForZoom(tileInfo->ident.level) * lineScale;
    
    if (!color || width <= 0.0)
    {
        return;
    }

    WideVectorInfo vecInfo;
    vecInfo.hasExp = true;
    vecInfo.coordType = WideVecCoordScreen;
    vecInfo.programID = styleSet->wideVectorProgramID;
    vecInfo.fade = fade;
    vecInfo.zoomSlot = styleSet->zoomSlot;
    vecInfo.color = *color;
    vecInfo.width = (float)width;
    vecInfo.offset = (float)-offset;
    vecInfo.widthExp = paint.width->expression();
    vecInfo.offsetExp = paint.offset->expression();
    vecInfo.colorExp = paint.color->expression();
    vecInfo.opacityExp = paint.opacity->expression();
    vecInfo.drawPriority = drawPriority + tileInfo->ident.level * std::max(0, styleSet->tileStyleSettings->drawPriorityPerLevel)+2;
    // TODO: Switch to stencils
//        vecInfo.drawOrder = tileInfo->tileNumber();
    
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

        shapes.reserve(shapes.size() + vecObj->shapes.size());
        std::copy(vecObj->shapes.begin(),vecObj->shapes.end(),std::back_inserter(shapes));
    }

    for (const auto &kvp : shapesByUUID)
    {
        if (cancelFn(inst))
        {
            return;
        }

        const auto &uuid = kvp.first;
        const auto &shapes = kvp.second;

        // Generate one component object per unique UUID (including blank)
        const auto compObj = styleSet->makeComponentObject(inst, desc);

        compObj->uuid = uuid;
        compObj->representation = representation;

        if (const auto wideVecID = styleSet->wideVecManage->addVectors(shapes, vecInfo, tileInfo->changes))
        {
            compObj->wideVectorIDs.insert(wideVecID);
            styleSet->compManage->addComponentObject(compObj, tileInfo->changes);
            tileInfo->compObjs.push_back(compObj);
        }
    }
}

}
