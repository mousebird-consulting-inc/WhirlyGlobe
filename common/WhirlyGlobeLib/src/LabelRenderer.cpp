/*
 *  LabelRenderer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/11/13.
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

#import "LabelManager.h"
#import "LabelRenderer.h"
#import "WhirlyGeometry.h"
#import "GlobeMath.h"
#import "LabelManager.h"
#import "SharedAttributes.h"
#import "WhirlyKitLog.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{

LabelInfo::LabelInfo(bool screenObject)
: hasTextColor(false), textColor(255,255,255,255), backColor(0,0,0,0),
    screenObject(screenObject),
    width(-1.0), height(-1.0),
    labelJustify(WhirlyKitLabelMiddle), textJustify(WhirlyKitTextCenter),
    shadowColor(0,0,0,0), shadowSize(-1.0),
    outlineColor(0,0,0,0), outlineSize(-1.0),
    lineHeight(0.0), fontPointSize(16.0),
    layoutOffset(0.0), layoutSpacing(20.0), layoutRepeat(0)
{
    if (screenObject) {
        width = 16.0;
        height = 16.0;
    } else {
        width = 0.001;
        height = 0.001;
    }
}

LabelInfo::LabelInfo(const LabelInfo &that)
: BaseInfo(that), hasTextColor(that.hasTextColor), textColor(that.textColor), backColor(that.backColor),
screenObject(that.screenObject), width(that.width), height(that.height),
labelJustify(that.labelJustify), textJustify(that.textJustify),
shadowColor(that.shadowColor), shadowSize(that.shadowSize),
outlineColor(that.outlineColor), outlineSize(that.outlineSize),
lineHeight(that.lineHeight), fontPointSize(that.fontPointSize),
layoutOffset(that.layoutOffset), layoutSpacing(that.layoutSpacing), layoutRepeat(that.layoutRepeat)
{
}

LabelInfo::LabelInfo(const Dictionary &dict, bool screenObject)
: BaseInfo(dict), screenObject(screenObject), fontPointSize(16.0)
{
    hasTextColor = dict.hasField(MaplyTextColor);
    textColor = dict.getColor(MaplyTextColor, RGBAColor(255,255,255,255));
    backColor = dict.getColor(MaplyBackgroundColor, RGBAColor(0,0,0,0));
    width = dict.getDouble(MaplyLabelWidth,0.0);
    height = dict.getDouble(MaplyLabelHeight,screenObject ? 16.0 : 0.001);
    std::string labelJustifyStr = dict.getString(MaplyLabelJustifyName);
    std::string textJustifyStr = dict.getString(MaplyTextJustify);
    shadowColor = dict.getColor(MaplyShadowColor, RGBAColor(0,0,0,255));
    shadowSize = dict.getDouble(MaplyShadowSize, 0.0);
    outlineSize = dict.getDouble(MaplyTextOutlineSize,0.0);
    outlineColor = dict.getColor(MaplyTextOutlineColor, RGBAColor(0,0,0,255));
    labelJustify = WhirlyKitLabelMiddle;
    if (!labelJustifyStr.compare(MaplyLabelJustifyNameMiddle))
        labelJustify = WhirlyKitLabelMiddle;
    else {
        if (!labelJustifyStr.compare(MaplyLabelJustifyNameLeft))
            labelJustify = WhirlyKitLabelLeft;
        else {
            if (!labelJustifyStr.compare(MaplyLabelJustifyNameRight))
                labelJustify = WhirlyKitLabelRight;
        }
    }
    if (!textJustifyStr.compare(MaplyTextJustifyCenter))
        textJustify = WhirlyKitTextCenter;
    else {
        if (!textJustifyStr.compare(MaplyTextJustifyLeft))
            textJustify = WhirlyKitTextLeft;
        else {
            if (!textJustifyStr.compare(MaplyTextJustifyRight))
                textJustify = WhirlyKitTextRight;
        }
    }
    layoutOffset = dict.getDouble(MaplyTextLayoutOffset,layoutOffset);
    layoutSpacing = dict.getDouble(MaplyTextLayoutSpacing,layoutSpacing);
    layoutRepeat = dict.getInt(MaplyTextLayoutRepeat,layoutRepeat);
    lineHeight = dict.getDouble(MaplyTextLineHeight,0.0);
}

LabelSceneRep::LabelSceneRep()
{
}
    
// We use these for labels that have icons
// Don't want to give them their own separate drawable, obviously
typedef std::map<SimpleIdentity,BasicDrawable *> IconDrawables;

LabelRenderer::LabelRenderer(Scene *scene,FontTextureManagerRef &fontTexManager,const LabelInfo *labelInfo)
    : useAttributedString(true), scene(scene), fontTexManager(fontTexManager), labelInfo(labelInfo),
    textureAtlasSize(2048), labelRep(NULL)
{
    coordAdapter = scene->getCoordAdapter();
}

typedef std::map<SimpleIdentity,BasicDrawable *> DrawableIDMap;

Point3dVector LabelRenderer::convertGeoPtsToModelSpace(const VectorRing &inPts)
{
    CoordSystemDisplayAdapter *coordAdapt = scene->getCoordAdapter();
    CoordSystem *coordSys = coordAdapt->getCoordSystem();

    Point3dVector outPts;
    outPts.reserve(inPts.size());
    
    for (auto pt: inPts) {
        auto localPt = coordSys->geographicToLocal3d(GeoCoord(pt.x(),pt.y()));
        Point3d pt3d = coordAdapt->localToDisplay(localPt);
        outPts.push_back(pt3d);
    }
    
    return outPts;
}

void LabelRenderer::render(PlatformThreadInfo *threadInfo,const std::vector<SingleLabel *> &labels,ChangeSet &changes)
{
    TimeInterval curTime = scene->getCurrentTime();

    // Drawables used for the icons
    IconDrawables iconDrawables;
    
    // Drawables we build up as we go
    DrawableIDMap drawables;

    for (unsigned int si=0;si<labels.size();si++)
    {
        SingleLabel *label = labels[si];
        RGBAColor theTextColor = labelInfo->textColor;
        const RGBAColor theBackColor = labelInfo->backColor;
        const RGBAColor theShadowColor = labelInfo->shadowColor;
        const float theShadowSize = labelInfo->shadowSize;
        if (label->infoOverride)
        {
            if (label->infoOverride->hasTextColor)
                theTextColor = label->infoOverride->textColor;
        }
        
        // We set this if the color is embedded in the "font"
        const bool embeddedColor = labelInfo->outlineSize > 0.0 || (label->infoOverride && label->infoOverride->outlineSize > 0.0);

        // Ask the label to build the strings.  There are OS specific things in there
        // We also need the real line height back (because it's in the font)
        float lineHeight=0.0;
        std::vector<DrawableString *> drawStrs = label->generateDrawableStrings(threadInfo,labelInfo,fontTexManager,lineHeight,changes);
        Mbr drawMbr;
        Mbr layoutMbr;

        // Calculate total draw and layout MBRs
        for (DrawableString *drawStr : drawStrs)
        {
            drawMbr.expand(drawStr->mbr);
            layoutMbr.expand(drawStr->mbr);
        }
        const float heightAboveBaseline = drawMbr.ur().y();
        
        // Override the layout size, but do so from the middle
        if (label->layoutSize.x() >= 0.0 && label->layoutSize.y() >= 0.0) {
            Point2f center = layoutMbr.mid();
            
            Point2f layoutSize(label->layoutSize.x(),label->layoutSize.y());
            layoutMbr.ll() = center - layoutSize/2.0;
            layoutMbr.ur() = center + layoutSize/2.0;
        }

        // Set if we're letting the layout engine control placement
        const bool layoutEngine = label->layoutEngine;
        const float layoutImportance = label->layoutImportance;
        const int layoutPlacement = label->layoutPlacement;
        
        ScreenSpaceObject scratchScreenSpaceObject;
        LayoutObject scratchLayoutObject;
        ScreenSpaceObject *screenShape = nullptr;
        LayoutObject *layoutObject = nullptr;

        // Portions of the label that are shared between substrings
        Point2d iconOff(0,0);
        Point2d justifyOff(0,0);
        if (labelInfo->screenObject)
        {
            float height = drawMbr.ur().y()-drawMbr.ll().y();

            switch (labelInfo->labelJustify)
            {
                case WhirlyKitLabelLeft:
                    justifyOff = Point2d(0,0.0);
                    break;
                case WhirlyKitLabelMiddle:
                    justifyOff = Point2d(-(drawMbr.ur().x()-drawMbr.ll().x())/2.0,0.0);
                    break;
                case WhirlyKitLabelRight:
                    justifyOff = Point2d(-(drawMbr.ur().x()-drawMbr.ll().x()),0.0);
                    break;
            }
            
            if (layoutEngine)
            {
                screenShape = layoutObject = &scratchLayoutObject;
            } else {
                screenShape = &scratchScreenSpaceObject;
            }

            // If we're doing layout, don't justify it
            if (layoutEngine)
                justifyOff = Point2d(0,0);

#ifndef __ANDROID__
            // Except we do need to tweak things a little, even for the layout engine
            // Note: But only on iOS because... reasons
            justifyOff.y() += heightAboveBaseline/2.0;
#endif

            screenShape->setDrawOrder(labelInfo->drawOrder);
            screenShape->setDrawPriority(labelInfo->drawPriority+1);
            screenShape->setVisibility(labelInfo->minVis, labelInfo->maxVis);
            screenShape->setZoomInfo(labelInfo->zoomSlot, labelInfo->minZoomVis, labelInfo->maxZoomVis);
            screenShape->setScaleExp(labelInfo->scaleExp);
            screenShape->setOpacityExp(labelInfo->opacityExp);
            screenShape->setKeepUpright(label->keepUpright);
            if (label->rotation != 0.0)
                screenShape->setRotation(label->rotation);
            if (labelInfo->fadeIn > 0.0)
                screenShape->setFade(curTime+labelInfo->fadeIn, curTime);
            else if (labelInfo->fadeOutTime != 0.0)
                screenShape->setFade(labelInfo->fadeOutTime, labelInfo->fadeOutTime+labelInfo->fadeOut);
            if (label->isSelectable && label->selectID != EmptyIdentity)
                screenShape->setId(label->selectID);
            screenShape->setWorldLoc(coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(label->loc)));
            
            // If there's an icon, we need to offset
            Point2d iconSize = (label->iconTexture==EmptyIdentity ? Point2d(0,0) : (label->iconSize.x() == 0.0 ? Point2d(height,height) : Point2d(label->iconSize.x(),label->iconSize.y())));
            iconOff = iconSize;
            
            // Throw a rectangle in the background
            RGBAColor backColor = theBackColor;
            double backBorder = 0.0;
            if (backColor.a != 0.0)
            {
                // Note: This is an arbitrary border around the text
                backBorder = 4.0;
                justifyOff.x() += backBorder;
                ScreenSpaceObject::ConvexGeometry smGeom;
                smGeom.progID = labelInfo->programID;
                Point2d ll = Point2d(drawMbr.ll().x(),drawMbr.ll().y())+iconOff+Point2d(-backBorder,-backBorder);
                Point2d ur = Point2d(drawMbr.ur().x(),drawMbr.ur().y())+iconOff+Point2d(backBorder,backBorder);
                smGeom.coords.push_back(Point2d(ur.x()+label->screenOffset.x(),ll.y()+label->screenOffset.y())+iconOff+justifyOff);
                smGeom.texCoords.push_back(TexCoord(0,1));
                
                smGeom.coords.push_back(Point2d(ur.x()+label->screenOffset.x(),ur.y()+label->screenOffset.y())+iconOff+justifyOff);
                smGeom.texCoords.push_back(TexCoord(0,0));
                
                smGeom.coords.push_back(Point2d(ll.x()+label->screenOffset.x(),ur.y()+label->screenOffset.y())+iconOff+justifyOff);
                smGeom.texCoords.push_back(TexCoord(1,0));
                
                smGeom.coords.push_back(Point2d(ll.x()+label->screenOffset.x(),ll.y()+label->screenOffset.y())+iconOff+justifyOff);
                smGeom.texCoords.push_back(TexCoord(1,1));
                
                smGeom.drawPriority = labelInfo->drawPriority;
                smGeom.color = backColor;
                screenShape->addGeometry(smGeom);
            }

            // If it's being passed to the layout engine, do that as well
            if (layoutEngine)
            {
                // Put together the layout info
                //                    layoutObject->hint = label->text;
                layoutObject->layoutPts.push_back(Point2d(layoutMbr.ll().x()+label->screenOffset.x()-backBorder,
                                                          layoutMbr.ll().y()+label->screenOffset.y()-backBorder)+iconOff+justifyOff);
                layoutObject->layoutPts.push_back(Point2d(layoutMbr.ur().x()+label->screenOffset.x()+backBorder,
                                                          layoutMbr.ll().y()+label->screenOffset.y()-backBorder)+iconOff+justifyOff);
                layoutObject->layoutPts.push_back(Point2d(layoutMbr.ur().x()+label->screenOffset.x()+backBorder,
                                                          layoutMbr.ur().y()+label->screenOffset.y()+backBorder)+iconOff+justifyOff);
                layoutObject->layoutPts.push_back(Point2d(layoutMbr.ll().x()+label->screenOffset.x()+backBorder,
                                                          layoutMbr.ur().y()+label->screenOffset.y()+backBorder)+iconOff+justifyOff);
                layoutObject->selectPts = layoutObject->layoutPts;
                
                //                        layoutObj->iconSize = Point2f(iconSize,iconSize);
                layoutObject->importance = layoutImportance;
                layoutObject->acceptablePlacement = layoutPlacement;
                layoutObject->setEnable(labelInfo->enable);
                
                // Setup layout points if we have them
                if (!label->layoutShape.empty()) {
                    layoutObject->layoutShape = convertGeoPtsToModelSpace(label->layoutShape);
                    layoutObject->layoutRepeat = labelInfo->layoutRepeat;
                    layoutObject->layoutOffset = labelInfo->layoutOffset;
                    layoutObject->layoutSpacing = labelInfo->layoutSpacing;
                }
                
                // The shape starts out disabled
                screenShape->setEnable(labelInfo->enable);
                if (labelInfo->startEnable != labelInfo->endEnable)
                    screenShape->setEnableTime(labelInfo->startEnable, labelInfo->endEnable);
                screenShape->setOffset(Point2d(MAXFLOAT,MAXFLOAT));
            } else {
                screenShape->setEnable(labelInfo->enable);
                if (labelInfo->startEnable != labelInfo->endEnable)
                    screenShape->setEnableTime(labelInfo->startEnable, labelInfo->endEnable);
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
                Mbr wholeMbr = drawMbr;
                wholeMbr.ll() += Point2f(iconOff.x(),iconOff.y()) + Point2f(justifyOff.x(),justifyOff.y());
                wholeMbr.ur() += Point2f(iconOff.x(),iconOff.y()) + Point2f(justifyOff.x(),justifyOff.y());
                // If there's an icon, just expand the whole thing.
                // Note: Not ideal
                if (iconGeom.coords.size() > 0)
                    for (unsigned int ig=0;ig<iconGeom.coords.size();ig++)
                        wholeMbr.addPoint(iconGeom.coords[ig]);
                const Point2f ll = wholeMbr.ll();
                const Point2f ur = wholeMbr.ur();
                double flip = 1.0;
#ifdef __ANDROID__
                flip = -1.0;
#endif
                select2d.pts[0] = Point2f(ll.x()+label->screenOffset.x(),ll.y()+-flip*label->screenOffset.y());
                select2d.pts[1] = Point2f(ll.x()+label->screenOffset.x(),ur.y()+-flip*label->screenOffset.y());
                select2d.pts[2] = Point2f(ur.x()+label->screenOffset.x(),ur.y()+-flip*label->screenOffset.y());
                select2d.pts[3] = Point2f(ur.x()+label->screenOffset.x(),ll.y()+-flip*label->screenOffset.y());
                
                select2d.selectID = label->selectID;
                select2d.minVis = labelInfo->minVis;
                select2d.maxVis = labelInfo->maxVis;
                
                if (label->hasMotion)
                {
                    MovingRectSelectable2D movingSelect2d;
                    (RectSelectable2D &)movingSelect2d = select2d;
                    movingSelect2d.endCenter = screenShape->getEndWorldLoc();
                    movingSelect2d.startTime = screenShape->getStartTime();
                    movingSelect2d.endTime = screenShape->getEndTime();
                    movingSelectables2D.push_back(movingSelect2d);
                } else
                    selectables2D.push_back(select2d);
            }
        }

        // Work through the lines
        double offsetY = 0.0;
        for (auto it = drawStrs.rbegin(); it != drawStrs.rend(); ++it)
        {
            const auto drawStr = *it;
            if (!drawStr)
                continue;
            
            labelRep->drawStrIDs.insert(drawStr->getId());

            if (labelInfo->screenObject)
            {
                Point2d lineOff(0.0,0.0);
                switch (labelInfo->textJustify)
                {
                    case WhirlyKitTextCenter:
                        lineOff.x() = (drawMbr.ur().x()-drawMbr.ll().x() - (drawStr->mbr.ur().x()-drawStr->mbr.ll().x()))/2.0;
                        break;
                    case WhirlyKitTextLeft:
                        // Leave it alone
                        break;
                    case WhirlyKitTextRight:
                        lineOff.x() = drawMbr.ur().x()-drawMbr.ll().x() - (drawStr->mbr.ur().x()-drawStr->mbr.ll().x());
                        break;
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
                        color = embeddedColor ? RGBAColor(255,255,255,255) : theTextColor;
                    } else {
                        soff = Point2d(theShadowSize,theShadowSize);
                        color = theShadowColor;
                    }
                    for (unsigned int ii=0;ii<drawStr->glyphPolys.size();ii++)
                    {
                        const DrawableString::Rect &poly = drawStr->glyphPolys[ii];
                        // Note: Ignoring the desired size in favor of the font size
                        ScreenSpaceObject::ConvexGeometry smGeom;
                        smGeom.progID = labelInfo->programID;
                        smGeom.coords.push_back(Point2d(poly.pts[1].x()+label->screenOffset.x(),poly.pts[0].y()+label->screenOffset.y() + offsetY) + soff + iconOff + justifyOff + lineOff);
                        smGeom.texCoords.push_back(TexCoord(poly.texCoords[1].u(),poly.texCoords[0].v()));
                        
                        smGeom.coords.push_back(Point2d(poly.pts[1].x()+label->screenOffset.x(),poly.pts[1].y()+label->screenOffset.y() + offsetY) + soff + iconOff + justifyOff + lineOff);
                        smGeom.texCoords.push_back(TexCoord(poly.texCoords[1].u(),poly.texCoords[1].v()));
                        
                        smGeom.coords.push_back(Point2d(poly.pts[0].x()+label->screenOffset.x(),poly.pts[1].y()+label->screenOffset.y() + offsetY) + soff + iconOff + justifyOff + lineOff);
                        smGeom.texCoords.push_back(TexCoord(poly.texCoords[0].u(),poly.texCoords[1].y()));
                        
                        smGeom.coords.push_back(Point2d(poly.pts[0].x()+label->screenOffset.x(),poly.pts[0].y()+label->screenOffset.y() + offsetY) + soff + iconOff + justifyOff + lineOff);
                        smGeom.texCoords.push_back(TexCoord(poly.texCoords[0].u(),poly.texCoords[0].v()));
                        
                        smGeom.texIDs.push_back(poly.subTex.texId);
                        smGeom.color = color;
                        poly.subTex.processTexCoords(smGeom.texCoords);
                        screenShape->addGeometry(smGeom);
                    }
                }
            }

            offsetY += lineHeight;
        }
        
        if (layoutObject)
            layoutObjects.push_back(*layoutObject);
        else if (screenShape)
            screenObjects.push_back(*screenShape);
        
        for (DrawableString *drawStr : drawStrs)
            if (drawStr)
                delete drawStr;
    }
    
    // Flush out any drawables we created for the labels
    for (DrawableIDMap::iterator it = drawables.begin(); it != drawables.end(); ++it)
        changes.push_back(new AddDrawableReq(it->second));

    // Flush out the icon drawables as well
    for (IconDrawables::iterator it = iconDrawables.begin();
         it != iconDrawables.end(); ++it)
    {
        BasicDrawable *iconDrawable = it->second;
        
        if (labelInfo->fadeIn > 0.0)
        {
            TimeInterval curTime = scene->getCurrentTime();
            iconDrawable->setFade(curTime,curTime+labelInfo->fadeIn);
        }
        changes.push_back(new AddDrawableReq(iconDrawable));
        labelRep->drawIDs.insert(iconDrawable->getId());
    }
}

}
