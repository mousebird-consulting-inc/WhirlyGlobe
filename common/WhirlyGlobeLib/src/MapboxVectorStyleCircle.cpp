/*
*  MapboxVectorStyleCircle.h
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

#import "MapboxVectorStyleCircle.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{

bool MapboxVectorCirclePaint::parse(PlatformThreadInfo *inst,
                                    MapboxVectorStyleSetImpl *styleSet,
                                    DictionaryRef styleEntry)
{
    if (!styleSet)
        return false;
    
    radius = styleSet->transDouble("circle-radius", styleEntry, 5.0);
    fillColor = styleSet->colorValue("circle-color",NULL,styleEntry,RGBAColor::black(),false);
    opacity = styleSet->transDouble("circle-opacity",styleEntry,1.0);
    strokeWidth = styleSet->transDouble("circle-stroke-width",styleEntry,0.0);
    strokeColor = styleSet->colorValue("circle-stroke-color",NULL,styleEntry,RGBAColor::black(),false);
    strokeOpacity = styleSet->transDouble("circle-stroke-opacity",styleEntry,1.0);
    
    return true;
}

bool MapboxVectorLayerCircle::parse(PlatformThreadInfo *inst,
                                    DictionaryRef styleEntry,
                                    MapboxVectorStyleLayerRef refLayer,
                                    int drawPriority)
{
    if (!styleEntry)
        return false;
    
    if (!MapboxVectorStyleLayer::parse(inst,styleEntry,refLayer,drawPriority) ||
        !paint.parse(inst, styleSet, styleEntry->getDict("paint")))
        return false;

    RGBAColor theFillColor = *paint.fillColor;
    RGBAColor theStrokeColor = *paint.strokeColor;
    double maxRadius = paint.radius->maxVal();
    double maxStrokeWidth = paint.strokeWidth->maxVal();
    circleTexID = styleSet->makeCircleTexture(inst,maxRadius,theFillColor,theStrokeColor,maxStrokeWidth,&circleSize);

    // Larger circles are slightly more important
    importance = drawPriority/1000 + styleSet->tileStyleSettings->markerImportance + maxRadius / 100000.0;

    uuidField = styleSet->tileStyleSettings->uuidField;
    
    return true;
}

void MapboxVectorLayerCircle::cleanup(ChangeSet &changes)
{
    if (circleTexID != EmptyIdentity)
        changes.push_back(new RemTextureReq(circleTexID));
}

void MapboxVectorLayerCircle::buildObjects(PlatformThreadInfo *inst,
                                           std::vector<VectorObjectRef> &vecObjs,
                                           VectorTileDataRef tileInfo)
{
    if (!visible)
        return;
    
    ComponentObjectRef compObj = styleSet->makeComponentObject(inst);

    // Default settings
    MarkerInfo markerInfo(true);
    markerInfo.zoomSlot = styleSet->zoomSlot;
    if (minzoom != 0 || maxzoom < 1000) {
        markerInfo.minZoomVis = minzoom;
        markerInfo.maxZoomVis = maxzoom;
    }
    const double opacity = paint.opacity->valForZoom(tileInfo->ident.level);
    markerInfo.color = RGBAColor(255,255,255,opacity*255);
    markerInfo.drawPriority = drawPriority;
    markerInfo.programID = styleSet->screenMarkerProgramID;

    // Need to find all the points, way down deep
    std::vector<WhirlyKit::Marker *> markers;
    for (auto vecObj : vecObjs) {
        if (vecObj->getVectorType() == VectorPointType) {
            for (VectorShapeRef shape : vecObj->shapes) {
                if (auto pts = std::dynamic_pointer_cast<VectorPoints>(shape)) {
                    for (auto pt : pts->pts) {
                        // Add a marker per point
                        // todo: exception safety
                        auto marker = new WhirlyKit::Marker();
                        marker->loc = GeoCoord(pt.x(),pt.y());
                        marker->texIDs.push_back(circleTexID);
                        const double radius = paint.radius->valForZoom(tileInfo->ident.level);
                        marker->width = 2*radius * styleSet->tileStyleSettings->markerScale; marker->height = 2*radius * styleSet->tileStyleSettings->markerScale;
                        marker->layoutWidth = marker->width; marker->layoutHeight = marker->height;
                        marker->layoutImportance = MAXFLOAT;
//                        marker->layoutImportance = importance + (101-tileInfo->ident.level)/100.0;
                        if (selectable) {
                            marker->isSelectable = true;
                            marker->selectID = Identifiable::genId();
                            styleSet->addSelectionObject(marker->selectID, vecObj, compObj);
                            compObj->selectIDs.insert(marker->selectID);
                            compObj->isSelectable = true;
                        }
                        if (!uuidField.empty())
                            marker->uniqueID = uuidField;
                        markers.push_back(marker);
                    }
                }
            }
        }
    }
    
    // Set up the markers and get a change set
    SimpleIdentity markerID = styleSet->markerManage->addMarkers(markers, markerInfo, tileInfo->changes);
    for (auto marker: markers)
        delete marker;
    if (markerID != EmptyIdentity)
        compObj->markerIDs.insert(markerID);
    styleSet->compManage->addComponentObject(compObj);
    tileInfo->compObjs.push_back(compObj);
}

}
