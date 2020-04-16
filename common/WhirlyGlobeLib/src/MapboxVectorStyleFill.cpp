/*
 *  MapboxVectorStyleFill.mm
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

#import "MapboxVectorStyleFill.h"
#import "VectorObject.h"
#import "Tesselator.h"

namespace WhirlyKit
{

bool MapboxVectorFillPaint::parse(MapboxVectorStyleSetImplRef styleSet,DictionaryRef styleEntry)
{
    styleSet->unsupportedCheck("fill-antialias","paint_fill",styleEntry);
    styleSet->unsupportedCheck("fill-translate","paint_fill",styleEntry);
    styleSet->unsupportedCheck("fill-translate-anchor","paint_fill",styleEntry);
    styleSet->unsupportedCheck("fill-image","paint_fill",styleEntry);
    
    opacity = styleSet->transDouble("fill-opacity",styleEntry,1.0);
    color = styleSet->transColor("fill-color",styleEntry,NULL);
    outlineColor = styleSet->transColor("fill-outline-color",styleEntry,NULL);
    
    return true;
}

bool MapboxVectorLayerFill::parse(MapboxVectorStyleSetImplRef styleSet,
                                    DictionaryRef styleEntry,
                                    MapboxVectorStyleLayerRef refLayer,
                                    int drawPriority)
{
    if (!MapboxVectorStyleLayer::parse(styleSet,styleEntry,refLayer,drawPriority) ||
        !paint.parse(styleSet,styleEntry->getDict("paint")))
        return false;
    
    arealShaderID = styleSet->tileStyleSettings->settingsArealShaderID;
    
    // Mess directly with the opacity because we're using it for other purposes
    if (styleEntry->hasField("alphaoverride")) {
        paint.color->setAlphaOverride(styleEntry->getDouble("alphaoverride"));
    }
    
    drawPriority = drawPriority;
    
    return true;
}

void MapboxVectorLayerFill::cleanup(ChangeSet &changes)
{
}

void MapboxVectorLayerFill::buildObjects(std::vector<VectorObjectRef> &vecObjs,
                                           VectorTileDataRef tileInfo)
{
    if (!visible)
        return;
    
    ComponentObjectRef compObj(new ComponentObject());

    // Filled polygons
    if (paint.color) {
        ShapeSet shapes;

        // Gather all the areal features
        for (auto vecObj : vecObjs) {
            if (vecObj->getVectorType() == VectorArealType) {
                shapes.insert(vecObj->shapes.begin(),vecObj->shapes.end());
            }
        }
        
        ShapeSet tessShapes;
        for (ShapeSet::iterator it = shapes.begin();it!=shapes.end();it++)
        {
            VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
            if (ar)
            {
                VectorTrianglesRef trisRef = VectorTriangles::createTriangles();
                TesselateLoops(ar->loops, trisRef);
                trisRef->setAttrDict(ar->getAttrDict());
                trisRef->initGeoMbr();
                tessShapes.insert(trisRef);
            }
        }
        
        // Set up the description for constructing vectors
        VectorInfo vecInfo;
        bool include = true;
        vecInfo.filled = true;
        vecInfo.centered = true;
        if (arealShaderID != EmptyIdentity)
            vecInfo.programID = arealShaderID;
        else
            vecInfo.programID = styleSet->vectorArealProgramID;
        RGBAColorRef color = styleSet->resolveColor(paint.color, paint.opacity, tileInfo->ident.level, MBResolveColorOpacityMultiply);
        if (color) {
            vecInfo.color = *color;
        } else {
            include = false;
        }
        if (drawPriorityPerLevel > 0)
            vecInfo.drawPriority = drawPriority + tileInfo->ident.level * drawPriorityPerLevel;
        else
            vecInfo.drawPriority = drawPriority;
        
        if (include) {
            SimpleIdentity vecID = styleSet->vecManage->addVectors(&tessShapes, vecInfo, tileInfo->changes);
            if (vecID != EmptyIdentity) {
                compObj->vectorIDs.insert(vecID);
                
                if (selectable)
                    compObj->vecObjs = vecObjs;
            }
        }
    }
    
    // Outlines
    if (paint.outlineColor) {
        ShapeSet shapes;

        // Gather all the areal features
        for (auto vecObj : vecObjs) {
            if (vecObj->getVectorType() == VectorArealType) {
                shapes.insert(vecObj->shapes.begin(),vecObj->shapes.end());
            }
        }
        
        // Set up the description for constructing vectors
        VectorInfo vecInfo;
        bool include = true;
        vecInfo.filled = false;
        vecInfo.centered = true;
        if (arealShaderID != EmptyIdentity)
            vecInfo.programID = arealShaderID;
        else
            vecInfo.programID = styleSet->vectorArealProgramID;
        RGBAColorRef color = styleSet->resolveColor(paint.outlineColor, paint.opacity, tileInfo->ident.level, MBResolveColorOpacityMultiply);
        if (color) {
            vecInfo.color = *color;
        } else {
            include = false;
        }
        if (drawPriorityPerLevel > 0)
            vecInfo.drawPriority = drawPriority + tileInfo->ident.level * drawPriorityPerLevel;
        else
            vecInfo.drawPriority = drawPriority;
        
        if (include) {
            SimpleIdentity vecID = styleSet->vecManage->addVectors(&shapes, vecInfo, tileInfo->changes);
            if (vecID != EmptyIdentity) {
                compObj->vectorIDs.insert(vecID);
            }
        }
    }
    
    if (!compObj->vectorIDs.empty()) {
        styleSet->compManage->addComponentObject(compObj);
        tileInfo->compObjs.push_back(compObj);
    }
}

}
