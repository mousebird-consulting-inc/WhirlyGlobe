/*
 *  MapboxVectorStyleLine.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/17/15.
 *  Copyright 2011-2019 mousebird consulting
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

#import "MapboxVectorStyleLine.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{

static const char * const lineCapVals[] = {"butt","round","square",NULL};
static const char * const joinVals[] = {"bevel","round","miter",NULL};

bool MapboxVectorLineLayout::parse(PlatformThreadInfo *inst,MapboxVectorStyleSetImpl *styleSet,DictionaryRef styleEntry)
{
    cap = styleEntry ? (MapboxVectorLineCap)styleSet->enumValue(styleEntry->getEntry("line-cap"),lineCapVals,(int)MBLineCapButt) : MBLineCapButt;
    join = styleEntry ? (MapboxVectorLineJoin)styleSet->enumValue(styleEntry->getEntry("line-join"),joinVals,(int)MBLineJoinMiter) : MBLineJoinMiter;
    miterLimit = styleSet->doubleValue("line-miter-limit", styleEntry, 2.0);
    roundLimit = styleSet->doubleValue("line-round-limit", styleEntry, 1.0);

    return true;
}

bool MapboxVectorLinePaint::parse(PlatformThreadInfo *inst,MapboxVectorStyleSetImpl *styleSet,DictionaryRef styleEntry)
{
    styleSet->unsupportedCheck("line-translate", "line-paint", styleEntry);
    styleSet->unsupportedCheck("line-translate-anchor", "line-paint", styleEntry);
    styleSet->unsupportedCheck("line-gap-width", "line-paint", styleEntry);
    styleSet->unsupportedCheck("line-blur", "line-paint", styleEntry);
    styleSet->unsupportedCheck("line-image", "line-paint", styleEntry);
    
    opacity = styleSet->transDouble("line-opacity", styleEntry, 1.0);
    width = styleSet->transDouble("line-width", styleEntry, 1.0);
    color = styleSet->transColor("line-color", styleEntry, RGBAColor::black());
    pattern = styleSet->stringValue("line-pattern", styleEntry, "");
    
    if (styleEntry && styleEntry->getType("line-dasharray") == DictTypeArray) {
        auto vecArray = styleEntry->getArray("line-dasharray");
        for (auto entry : vecArray) {
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
                                  DictionaryRef styleEntry,
                                  MapboxVectorStyleLayerRef refLayer,
                                  int drawPriority)
{
    if (!styleEntry)
        return false;
    
    if (!MapboxVectorStyleLayer::parse(inst, styleEntry,refLayer,drawPriority) ||
        !layout.parse(inst, styleSet, styleEntry->getDict("layout")) ||
        !paint.parse(inst, styleSet, styleEntry->getDict("paint")))
        return false;
    
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

        int totLenRounded = NextPowOf2(totLen);
        if (totLenRounded < 64)
            totLenRounded = 64;
        std::vector<double> dashComponents;
        for (double val : paint.lineDashArray)
        {
            double len = val * totLenRounded / totLen;
            dashComponents.push_back(len);
        }
        totLen *= maxWidth;
        
        filledLineTexID = styleSet->makeLineTexture(inst,dashComponents);
    }
    fade = styleSet->doubleValue("fade",styleEntry,0.0);

    lineScale = styleSet->tileStyleSettings->lineScale;

    return true;
}

void MapboxVectorLayerLine::cleanup(PlatformThreadInfo *inst,ChangeSet &changes)
{
}

void MapboxVectorLayerLine::buildObjects(PlatformThreadInfo *inst,
                                         std::vector<VectorObjectRef> &inVecObjs,
                                         VectorTileDataRef tileInfo)
{
    if (!visible)
        return;

    ComponentObjectRef compObj = styleSet->makeComponentObject(inst);

    // Turn into linears (if not already) and then clip to the bounds
    // Slightly different, but we want to clip all the areals that are converted to linears
    std::vector<VectorObjectRef> vecObjs;
    vecObjs.reserve(inVecObjs.size());
    for (auto const &vecObj : inVecObjs) {
        bool clip = linearClipToBounds;
        
        VectorObjectRef newVecObj = vecObj;
        if (dropGridLines)
            newVecObj = newVecObj->filterClippedEdges();
        
        if (newVecObj->getVectorType() == VectorArealType) {
            newVecObj = newVecObj->arealsToLinears();
            clip = true;
        }
        if (newVecObj && clip)
            newVecObj = VectorObjectRef(newVecObj->clipToMbr(tileInfo->geoBBox.ll(), tileInfo->geoBBox.ur()));
        if (newVecObj)
            vecObjs.push_back(newVecObj);
    }

    // Subdivide long-ish lines to the globe, if set
    if (subdivToGlobe > 0.0) {
        std::vector<VectorObjectRef> newVecObjs;
        newVecObjs.reserve(3 * vecObjs.size());
        for (auto vecObj : vecObjs) {
            vecObj->subdivideToGlobe(subdivToGlobe);
            newVecObjs.push_back(vecObj);
        }
        vecObjs = newVecObjs;
    }
    
    // If we have a filled texture, we'll use that
    const float repeatLen = totLen;
    
    // TODO: We can also have a symbol, where we might do the same thing
    // Problem is, we'll need to pass the subtexture logic through to the renderer
    //  because right now it's expecting a single texture that can be strung along the line
    
    const RGBAColorRef color = styleSet->resolveColor(paint.color, paint.opacity, tileInfo->ident.level, MBResolveColorOpacityMultiply);
    const double width = paint.width->valForZoom(tileInfo->ident.level) * lineScale;
    
    if (color && width > 0.0)
    {
        WideVectorInfo vecInfo;
        vecInfo.hasExp = true;
        vecInfo.coordType = WideVecCoordScreen;
        vecInfo.programID = styleSet->wideVectorProgramID;
        vecInfo.fade = fade;
        vecInfo.zoomSlot = styleSet->zoomSlot;
        if (minzoom != 0 || maxzoom < 1000) {
            vecInfo.minZoomVis = minzoom;
            vecInfo.maxZoomVis = maxzoom;
    //        wkLogLevel(Debug, "zoomSlot = %d, minZoom = %f, maxZoom = %f",styleSet->zoomSlot,vecInfo.minZoomVis,vecInfo.maxZoomVis);
        }
        if (filledLineTexID != EmptyIdentity) {
            vecInfo.texID = filledLineTexID;
            vecInfo.repeatSize = repeatLen;
        }

        vecInfo.color = *color;
        vecInfo.width = width;
        vecInfo.widthExp = paint.width->expression();
        // Scale by the lineScale
        if (vecInfo.widthExp)
            vecInfo.widthExp->scaleBy(lineScale);
        vecInfo.colorExp = paint.color->expression();
        vecInfo.opacityExp = paint.opacity->expression();
        vecInfo.drawPriority = drawPriority + tileInfo->ident.level * std::max(0, styleSet->tileStyleSettings->drawPriorityPerLevel)+2;
        // TODO: Switch to stencils
//        vecInfo.drawOrder = tileInfo->tileNumber();

        // Gather all the linear features
        std::vector<VectorShapeRef> shapes;
        for (auto vecObj : vecObjs) {
            if (vecObj->getVectorType() == VectorLinearType)
                std::copy(vecObj->shapes.begin(),vecObj->shapes.end(),std::back_inserter(shapes));
        }
        
        const auto wideVecID = styleSet->wideVecManage->addVectors(shapes, vecInfo, tileInfo->changes);
        if (wideVecID != EmptyIdentity)
            compObj->wideVectorIDs.insert(wideVecID);
    }
    
    if (!compObj->wideVectorIDs.empty()) {
        styleSet->compManage->addComponentObject(compObj);
        tileInfo->compObjs.push_back(compObj);
    }
}

}
