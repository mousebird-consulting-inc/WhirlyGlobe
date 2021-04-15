/*  MapboxVectorStyleBackground.mm
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

#import "MapboxVectorStyleBackground.h"
#import "WhirlyKitLog.h"
#import "Tesselator.h"

namespace WhirlyKit
{

bool MapboxVectorBackgroundPaint::parse(PlatformThreadInfo *inst,
                                        MapboxVectorStyleSetImpl *styleSet,
                                        DictionaryRef styleEntry)
{
    color = styleSet->transColor("background-color",styleEntry,RGBAColor::black());
    styleSet->unsupportedCheck("background-image","paint_background",styleEntry);

    opacity = styleSet->transDouble("background-opacity",styleEntry,1.0);

    return true;
}

bool MapboxVectorLayerBackground::parse(PlatformThreadInfo *inst,
                                        const DictionaryRef &styleEntry,
                                        const MapboxVectorStyleLayerRef &refLayer,
                                        int inDrawPriority)
{
    if (!MapboxVectorStyleLayer::parse(inst,styleEntry,refLayer,drawPriority))
    {
        return false;
    }
    
//    styleSet->unsupportedCheck("layout","background",styleEntry);
    
    if (!paint.parse(inst,styleSet,styleEntry->getDict("paint")))
    {
        return false;
    }
    
    // Mess directly with the opacity because we're using it for other purposes
    if (styleEntry->hasField("alphaoverride"))
    {
        paint.color->setAlphaOverride(styleEntry->getDouble("alphaoverride"));
    }
    
    drawPriority = inDrawPriority;

    return true;
}

void MapboxVectorLayerBackground::buildObjects(PlatformThreadInfo *inst,
                                               const std::vector<VectorObjectRef> &vecObjs,
                                               const VectorTileDataRef &tileInfo,
                                               const Dictionary *desc,
                                               const CancelFunction &)
{
    const auto color = styleSet->backgroundColor(inst, tileInfo->ident.level);
    
    std::vector<VectorRing> loops { VectorRing() };
    const Mbr &bbox = tileInfo->geoBBox;
    bbox.asPoints(loops.back());

    const auto trisRef = VectorTriangles::createTriangles();
    TesselateLoops(loops, trisRef);
    //trisRef->setAttrDict(ar->getAttrDict());
    const auto d = MutableDictionaryMake();
    d->setString("layer_name", "background");
    d->setInt("layer_order", 1);
    d->setInt("geometry_type", 3);
    trisRef->setAttrDict(d);
    trisRef->initGeoMbr();
    ShapeSet tessShapes { trisRef };

    VectorInfo vecInfo;
    vecInfo.hasExp = true;
    vecInfo.filled = true;
    vecInfo.centered = false;
    vecInfo.color = *color;
    vecInfo.zoomSlot = styleSet->zoomSlot;
    vecInfo.zBufferWrite = styleSet->tileStyleSettings->zBufferWrite;
    vecInfo.zBufferRead = styleSet->tileStyleSettings->zBufferRead;
    vecInfo.colorExp = paint.color->expression();
    vecInfo.opacityExp = paint.opacity->expression();
    vecInfo.programID = styleSet->vectorArealProgramID;
    vecInfo.drawPriority = drawPriority + tileInfo->ident.level * std::max(0, styleSet->tileStyleSettings->drawPriorityPerLevel);
    // TODO: Switch to stencils
//    vecInfo.drawOrder = tileInfo->tileNumber();
    
//    wkLogLevel(Debug, "background: tileID = %d: (%d,%d)  drawOrder = %d, drawPriority = %d",tileInfo->ident.level, tileInfo->ident.x, tileInfo->ident.y, vecInfo.drawOrder,vecInfo.drawPriority);

    if (minzoom != 0 || maxzoom < 1000)
    {
        vecInfo.minZoomVis = minzoom;
        vecInfo.maxZoomVis = maxzoom;
    }

    if (const auto vecID = styleSet->vecManage->addVectors(&tessShapes, vecInfo, tileInfo->changes))
    {
        const auto compObj = styleSet->makeComponentObject(inst, desc);

        // not currently supported
        //compObj->representation = representation;

        compObj->vectorIDs.insert(vecID);

        styleSet->compManage->addComponentObject(compObj, tileInfo->changes);
        tileInfo->compObjs.push_back(compObj);
    }
}

}
