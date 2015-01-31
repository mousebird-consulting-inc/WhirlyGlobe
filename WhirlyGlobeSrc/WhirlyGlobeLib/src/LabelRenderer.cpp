/*
 *  LabelRenderer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/11/13.
 *  Copyright 2011-2013 mousebird consulting
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

#import "LabelRenderer.h"
#import "WhirlyGeometry.h"
#import "GlobeMath.h"
#import "ScreenSpaceGenerator.h"
#import "SharedAttributes.h"
#import "LabelManager.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{

LabelSceneRep::LabelSceneRep()
{
}
    
// We use these for labels that have icons
// Don't want to give them their own separate drawable, obviously
typedef std::map<SimpleIdentity,BasicDrawable *> IconDrawables;
    
LabelInfo::LabelInfo()
    : textColor(255,255,255,255), outlineColor(0,0,0,0), backColor(0,0,0,0), screenObject(true), layoutEngine(true),
    layoutImportance(1.0), width(0), height(0), drawOffset(0), minVis(DrawVisibleInvalid), maxVis(DrawVisibleInvalid), justify(WhirlyKitLabelRight), drawPriority(0), fade(0.0),
    shadowColor(0,0,0,0), shadowSize(0), outlineSize(0), shaderID(EmptyIdentity), enable(true)
{
}

// Parse label info out of a description
void LabelInfo::parseDict(const Dictionary &dict)
{
    textColor = dict.getColor(MaplyTextColor, RGBAColor(255,255,255,255));
    backColor = dict.getColor(MaplyBackgroundColor, RGBAColor(0,0,0,0));
//    self.font = [desc objectForKey:@"font" checkType:[UIFont class] default:[UIFont systemFontOfSize:32.0]];
    screenObject = dict.getBool("screen",false);
    layoutEngine = dict.getBool("layout",false);
    layoutImportance = dict.getDouble("layoutImportance",0.0);
    width = dict.getDouble(MaplyLabelWidth,0.0);
    height = dict.getDouble(MaplyLabelHeight,screenObject ? 16.0 : 0.001);
    drawOffset = dict.getDouble(MaplyDrawOffset,0);
    minVis = dict.getDouble(MaplyMinVis,DrawVisibleInvalid);
    maxVis = dict.getDouble(MaplyMaxVis,DrawVisibleInvalid);
    std::string justifyStr = dict.getString(MaplyJustify);
    fade = dict.getDouble(MaplyFade,0.0);
    shadowColor = dict.getColor(MaplyShadowColor, RGBAColor(0,0,0,255));
    shadowSize = dict.getDouble(MaplyShadowSize, 0.0);
    outlineSize = dict.getDouble(MaplyTextOutlineSize,0.0);
    outlineColor = dict.getColor(MaplyShadowColor, RGBAColor(0,0,0,255));
    shaderID = dict.getInt(MaplyShaderString,EmptyIdentity);
    enable = dict.getBool(MaplyEnable,true);
    if (!justifyStr.compare("middle"))
        justify = WhirlyKitLabelMiddle;
    else {
        if (!justifyStr.compare("left"))
            justify = WhirlyKitLabelLeft;
        else {
            if (!justifyStr.compare("right"))
                justify = WhirlyKitLabelRight;
        }
    }
    drawPriority = dict.getInt(MaplyDrawPriority,LabelDrawPriority);
}

LabelRenderer::LabelRenderer(Scene *scene,FontTextureManager *fontTexManager,const LabelInfo *labelInfo)
    : useAttributedString(true), scene(scene), fontTexManager(fontTexManager), labelInfo(labelInfo),
    textureAtlasSize(2048), labelRep(NULL)
{
    coordAdapter = scene->getCoordAdapter();
}

typedef std::map<SimpleIdentity,BasicDrawable *> DrawableIDMap;

void LabelRenderer::render(std::vector<SingleLabel *> &labels,ChangeSet &changes)
{
    TimeInterval curTime = TimeGetCurrent();

    // Drawables used for the icons
    IconDrawables iconDrawables;
    
    // Drawables we build up as we go
    DrawableIDMap drawables;

    for (unsigned int si=0;si<labels.size();si++)
    {
        SingleLabel *label = labels[si];
        RGBAColor theTextColor = labelInfo->textColor;
        RGBAColor theBackColor = labelInfo->backColor;
        RGBAColor theShadowColor = labelInfo->shadowColor;
        float theShadowSize = labelInfo->shadowSize;
        if (label->desc.numFields() > 0)
        {
            theTextColor = label->desc.getColor(MaplyTextColor,theTextColor);
            theBackColor = label->desc.getColor(MaplyBackgroundColor,theBackColor);
            // Note: Porting
//            theFont = [label->desc objectForKey:@"font" checkType:[UIFont class] default:theFont];
            theShadowColor = label->desc.getColor(MaplyShadowColor,theShadowColor);
            theShadowSize = label->desc.getDouble(MaplyShadowSize,theShadowSize);
        }
        // Note: Porting
//        if (theShadowColor == nil)
//            theShadowColor = [UIColor blackColor];
//        if (theOutlineColor == nil)
//            theOutlineColor = [UIColor blackColor];
        
        // We set this if the color is embedded in the "font"
//        bool embeddedColor = (labelInfo->outlineSize > 0.0 || label->desc.hasField(MaplyTextOutlineSize));
        // Note: Porting.  Not clear if this makes
        bool embeddedColor = true;
        DrawableString *drawStr = label->generateDrawableString(labelInfo,fontTexManager,changes);

        Point2d iconOff(0,0);
        ScreenSpaceObject *screenShape = NULL;
        LayoutObject *layoutObject = NULL;
        if (drawStr)
        {
            labelRep->drawStrIDs.insert(drawStr->getId());

            Point2d justifyOff(0,0);
            switch (labelInfo->justify)
            {
                case WhirlyKitLabelLeft:
                    justifyOff = Point2d(0,0);
                    break;
                case WhirlyKitLabelMiddle:
                    justifyOff = Point2d(-(drawStr->mbr.ur().x()-drawStr->mbr.ll().x())/2.0,0.0);
                    break;
                case WhirlyKitLabelRight:
                    justifyOff = Point2d(-(drawStr->mbr.ur().x()-drawStr->mbr.ll().x()),0.0);
                    break;
            }
            
            if (labelInfo->screenObject)
            {
                // Set if we're letting the layout engine control placement
                bool layoutEngine = (labelInfo->layoutEngine || label->desc.getBool("layout",false));
                
                if (layoutEngine)
                {
                    layoutObject = new LayoutObject();
                    screenShape = layoutObject;
                } else
                    screenShape = new ScreenSpaceObject();

                // If we're doing layout, don't justify it
                if (layoutEngine)
                    justifyOff = Point2d(0,0);
                
                screenShape->setDrawPriority(labelInfo->drawPriority);
                screenShape->setVisibility(labelInfo->minVis, labelInfo->maxVis);
                screenShape->setKeepUpright(label->keepUpright);
                if (label->rotation != 0.0)
                    screenShape->setRotation(label->rotation);
                if (labelInfo->fade > 0.0)
                    screenShape->setFade(curTime+labelInfo->fade, curTime);
                if (label->isSelectable && label->selectID != EmptyIdentity)
                    screenShape->setId(label->selectID);
                screenShape->setWorldLoc(coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(label->loc)));

                // If there's an icon, we need to offset
                float height = drawStr->mbr.ur().y()-drawStr->mbr.ll().y();
                Point2d iconSize = (label->iconTexture==EmptyIdentity ? Point2d(0,0) : (label->iconSize.x() == 0.0 ? Point2d(height,height) : Point2d(label->iconSize.x(),label->iconSize.y())));
                iconOff = iconSize;
                
                // Throw a rectangle in the background
                RGBAColor backColor = theBackColor;
                if (backColor.a != 0.0)
                {
                    // Note: This is an arbitrary border around the text
                    float backBorder = 4.0;
                    ScreenSpaceObject::ConvexGeometry smGeom;
                    Point2d ll = Point2d(drawStr->mbr.ll().x(),drawStr->mbr.ll().y())+iconOff+Point2d(-backBorder,-backBorder), ur = Point2d(drawStr->mbr.ur().x(),drawStr->mbr.ur().y())+iconOff+Point2d(backBorder,backBorder);
                    smGeom.coords.push_back(Point2d(ll.x()+label->screenOffset.x(),-ll.y()+label->screenOffset.y())+justifyOff);
                    smGeom.texCoords.push_back(TexCoord(0,1));
                    
                    smGeom.coords.push_back(Point2d(ll.x()+label->screenOffset.x(),-ur.y()+label->screenOffset.y())+justifyOff);
                    smGeom.texCoords.push_back(TexCoord(1,1));
                    
                    smGeom.coords.push_back(Point2d(ur.x()+label->screenOffset.x(),-ur.y()+label->screenOffset.y())+justifyOff);
                    smGeom.texCoords.push_back(TexCoord(1,0));
                    
                    smGeom.coords.push_back(Point2d(ur.x()+label->screenOffset.x(),-ll.y()+label->screenOffset.y())+justifyOff);
                    smGeom.texCoords.push_back(TexCoord(0,0));
                    
                    smGeom.color = backColor;
                    // Note: This would be a great place for a texture
                    screenShape->addGeometry(smGeom);
                }
                
                // Turn the glyph polys into simple geometry
                // We do this in a weird order to stick the shadow underneath
                for (int ss=((theShadowSize > 0.0) ? 0: 1);ss<2;ss++)
                {
                    Point2d soff;
                    RGBAColor color;
                    if (ss == 1)
                    {
                        soff = Point2d(0,0);
                        // Note: Debugging
                        color = embeddedColor ? RGBAColor(255,255,255,255) : theTextColor;
                    } else {
                        soff = Point2d(theShadowSize,theShadowSize);
                        color = theShadowColor;
                    }
                    for (unsigned int ii=0;ii<drawStr->glyphPolys.size();ii++)
                    {
                        DrawableString::Rect &poly = drawStr->glyphPolys[ii];
                        // Note: Ignoring the desired size in favor of the font size
                        ScreenSpaceObject::ConvexGeometry smGeom;
                        smGeom.progID = labelInfo->programID;
                        smGeom.coords.push_back(Point2d(poly.pts[1].x()+label->screenOffset.x(),poly.pts[0].y()-label->screenOffset.y()) + soff + iconOff + justifyOff);
                        smGeom.texCoords.push_back(TexCoord(poly.texCoords[1].u(),poly.texCoords[0].v()));
                        
                        smGeom.coords.push_back(Point2d(poly.pts[1].x()+label->screenOffset.x(),poly.pts[1].y()-label->screenOffset.y()) + soff + iconOff + justifyOff);
                        smGeom.texCoords.push_back(TexCoord(poly.texCoords[1].u(),poly.texCoords[1].v()));
                        
                        smGeom.coords.push_back(Point2d(poly.pts[0].x()+label->screenOffset.x(),poly.pts[1].y()-label->screenOffset.y()) + soff + iconOff + justifyOff);
                        smGeom.texCoords.push_back(TexCoord(poly.texCoords[0].u(),poly.texCoords[1].y()));
                        
                        smGeom.coords.push_back(Point2d(poly.pts[0].x()+label->screenOffset.x(),poly.pts[0].y()-label->screenOffset.y()) + soff + iconOff + justifyOff);
                        smGeom.texCoords.push_back(TexCoord(poly.texCoords[0].u(),poly.texCoords[0].v()));
                        
                        smGeom.texIDs.push_back(poly.subTex.texId);
                        smGeom.color = color;
                        poly.subTex.processTexCoords(smGeom.texCoords);
                        screenShape->addGeometry(smGeom);
                    }
                }
                
                // If it's being passed to the layout engine, do that as well
                if (layoutEngine)
                {
                    float layoutImportance = label->desc.getDouble("layoutImportance",labelInfo->layoutImportance);
                    int layoutPlacement = label->desc.getInt("layoutPlacement",(int)(WhirlyKitLayoutPlacementLeft | WhirlyKitLayoutPlacementRight | WhirlyKitLayoutPlacementAbove | WhirlyKitLayoutPlacementBelow));
                    
                    // Put together the layout info
//                    layoutObject->hint = label->text;
                    drawStr->mbr.asPoints(layoutObject->layoutPts);
                    layoutObject->layoutPts.push_back(Point2d(drawStr->mbr.ll().x()+label->screenOffset.x(),drawStr->mbr.ll().y()+label->screenOffset.y())+iconOff+justifyOff);
                    layoutObject->layoutPts.push_back(Point2d(drawStr->mbr.ur().x()+label->screenOffset.x(),drawStr->mbr.ll().y()+label->screenOffset.y())+iconOff+justifyOff);
                    layoutObject->layoutPts.push_back(Point2d(drawStr->mbr.ur().x()+label->screenOffset.x(),drawStr->mbr.ur().y()+label->screenOffset.y())+iconOff+justifyOff);
                    layoutObject->layoutPts.push_back(Point2d(drawStr->mbr.ll().x()+label->screenOffset.x(),drawStr->mbr.ur().y()+label->screenOffset.y())+iconOff+justifyOff);
                    layoutObject->selectPts = layoutObject->layoutPts;
                    
                    //                        layoutObj->iconSize = Point2f(iconSize,iconSize);
                    layoutObject->importance = layoutImportance;
                    layoutObject->acceptablePlacement = layoutPlacement;
                    layoutObject->setEnable(labelInfo->enable);
                    
                    // The shape starts out disabled
                    screenShape->setEnable(labelInfo->enable);
                    screenShape->setOffset(Point2d(MAXFLOAT,MAXFLOAT));
                } else {
                    screenShape->setEnable(labelInfo->enable);
                }
                
                // Deal with the icon here becaue we need its geometry
                ScreenSpaceObject::ConvexGeometry iconGeom;
                if (label->iconTexture != EmptyIdentity && screenShape)
                {
                    SubTexture subTex = scene->getSubTexture(label->iconTexture);
                    std::vector<TexCoord> texCoord;
                    texCoord.resize(4);
                    texCoord[3].u() = 0.0;  texCoord[3].v() = 0.0;
                    texCoord[2].u() = 1.0;  texCoord[2].v() = 0.0;
                    texCoord[1].u() = 1.0;  texCoord[1].v() = 1.0;
                    texCoord[0].u() = 0.0;  texCoord[0].v() = 1.0;
                    subTex.processTexCoords(texCoord);
                    
                    iconGeom.texIDs.push_back(subTex.texId);
                    iconGeom.progID = labelInfo->programID;
                    Point2d iconPts[4];
                    iconPts[0] = Point2d(0,0);
                    iconPts[1] = Point2d(iconOff.x(),0);
                    iconPts[2] = iconOff;
                    iconPts[3] = Point2d(0,iconOff.y());
                    for (unsigned int ii=0;ii<4;ii++)
                    {
                        iconGeom.coords.push_back(Point2d(iconPts[ii].x(),iconPts[ii].y())+Point2d(label->screenOffset.x(),label->screenOffset.y()));
                        iconGeom.texCoords.push_back(texCoord[ii]);
                    }
                    // For layout objects, we'll put the icons on their own
                    //            if (layoutObj)
                    //            {
                    //                ScreenSpaceGenerator::ConvexShape *iconScreenShape = new ScreenSpaceGenerator::ConvexShape();
                    //                SimpleIdentity iconId = iconScreenShape->getId();
                    //                *iconScreenShape = *screenShape;
                    //                iconScreenShape->setId(iconId);
                    //                iconScreenShape->geom.clear();
                    //                iconScreenShape->geom.push_back(iconGeom);
                    //                screenObjects.push_back(iconScreenShape);
                    //                labelRep->screenIDs.insert(iconScreenShape->getId());
                    //                layoutObj->auxIDs.insert(iconScreenShape->getId());
                    //            } else {
                    screenShape->addGeometry(iconGeom);
                    //            }
                    
                }
                
                // Register the main label as selectable
                if (label->isSelectable && !layoutObject)
                {
                    // If the label doesn't already have an ID, it needs one
                    if (!label->selectID)
                        label->selectID = Identifiable::genId();
                    
                    RectSelectable2D select2d;
                    select2d.center = screenShape->getWorldLoc();
                    select2d.enable = labelInfo->enable;
                    Mbr wholeMbr = drawStr->mbr;
                    wholeMbr.ll() += Point2f(iconOff.x(),iconOff.y()) + Point2f(justifyOff.x(),justifyOff.y());
                    wholeMbr.ur() += Point2f(iconOff.x(),iconOff.y()) + Point2f(justifyOff.x(),justifyOff.y());
                    // If there's an icon, just expand the whole thing.
                    // Note: Not ideal
                    if (iconGeom.coords.size() > 0)
                        for (unsigned int ig=0;ig<iconGeom.coords.size();ig++)
                            wholeMbr.addPoint(iconGeom.coords[ig]);
                    Point2f ll = wholeMbr.ll(), ur = wholeMbr.ur();
                    select2d.pts[0] = Point2f(ll.x()+label->screenOffset.x(),ll.y()+-label->screenOffset.y());
                    select2d.pts[1] = Point2f(ll.x()+label->screenOffset.x(),ur.y()+-label->screenOffset.y());
                    select2d.pts[2] = Point2f(ur.x()+label->screenOffset.x(),ur.y()+-label->screenOffset.y());
                    select2d.pts[3] = Point2f(ur.x()+label->screenOffset.x(),ll.y()+-label->screenOffset.y());
                    
                    select2d.selectID = label->selectID;
                    select2d.minVis = labelInfo->minVis;
                    select2d.maxVis = labelInfo->maxVis;
                    selectables2D.push_back(select2d);
                }
                
                if (layoutObject)
                    layoutObjects.push_back(*layoutObject);
                else if (screenShape)
                    screenObjects.push_back(*screenShape);
            }
        
            delete drawStr;
        }
    }
    
    // Flush out any drawables we created for the labels
    for (DrawableIDMap::iterator it = drawables.begin(); it != drawables.end(); ++it)
        changes.push_back(new AddDrawableReq(it->second));

    // Flush out the icon drawables as well
    for (IconDrawables::iterator it = iconDrawables.begin();
         it != iconDrawables.end(); ++it)
    {
        BasicDrawable *iconDrawable = it->second;
        
        if (labelInfo->fade > 0.0)
        {
            TimeInterval curTime = TimeGetCurrent();
            iconDrawable->setFade(curTime,curTime+labelInfo->fade);
        }
        changes.push_back(new AddDrawableReq(iconDrawable));
        labelRep->drawIDs.insert(iconDrawable->getId());
    }    
}

}
