/*
 *  LabelRenderer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/11/13.
 *  Copyright 2011-2017 mousebird consulting
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
#import "NSString+Stuff.h"
#import "NSDictionary+Stuff.h"
#import "UIColor+Stuff.h"
#import "ScreenSpaceBuilder.h"

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
    
}

@implementation WhirlyKitLabelInfo

// Parse label info out of a description
- (void)parseDesc:(NSDictionary *)desc
{
    self.textColor = [desc objectForKey:@"textColor" checkType:[UIColor class] default:[UIColor whiteColor]];
    self.backColor = [desc objectForKey:@"backgroundColor" checkType:[UIColor class] default:[UIColor clearColor]];
    self.font = [desc objectForKey:@"font" checkType:[UIFont class] default:[UIFont systemFontOfSize:32.0]];
    _screenObject = [desc boolForKey:@"screen" default:false];
    _layoutEngine = [desc boolForKey:@"layout" default:false];
    _layoutImportance = [desc floatForKey:@"layoutImportance" default:0.0];
    _width = [desc floatForKey:@"width" default:0.0];
    _height = [desc floatForKey:@"height" default:(_screenObject ? 16.0 : 0.001)];
    NSString *labelJustify = [desc stringForKey:@"justify" default:@"middle"];
    NSString *textJustify = [desc stringForKey:@"textjustify" default:@"left"];
    _shadowColor = [desc objectForKey:@"shadowColor"];
    _shadowSize = [desc floatForKey:@"shadowSize" default:0.0];
    _outlineSize = [desc floatForKey:@"outlineSize" default:0.0];
    _outlineColor = [desc objectForKey:@"outlineColor" checkType:[UIColor class] default:[UIColor blackColor]];
    if (![labelJustify compare:@"middle"])
        _labelJustify = WhirlyKitLabelMiddle;
    else {
        if (![labelJustify compare:@"left"])
            _labelJustify = WhirlyKitLabelLeft;
        else {
            if (![labelJustify compare:@"right"])
                _labelJustify = WhirlyKitLabelRight;
        }
    }
    if (![textJustify compare:@"center"])
        _textJustify = WhirlyKitTextCenter;
    else {
        if (![textJustify compare:@"left"])
            _textJustify = WhirlyKitTextLeft;
        else {
            if (![textJustify compare:@"right"])
                _textJustify = WhirlyKitTextRight;
        }
    }
}

// Initialize a label info with data from the description dictionary
- (id)initWithDesc:(NSDictionary *)desc
{
    if ((self = [super initWithDesc:desc]))
    {
        [self parseDesc:desc];
    }
    
    return self;
}

@end

@implementation WhirlyKitLabelRenderer

- (id)init
{
    self = [super init];
    _useAttributedString = true;
    _scale = 1.0;
    
    return self;
}

- (void)dealloc
{
    for (unsigned int ii=0;ii<_screenObjects.size();ii++)
        delete _screenObjects[ii];
    for (unsigned int ii=0;ii<_layoutObjects.size();ii++)
        delete _layoutObjects[ii];
}

- (void)render
{
    [self renderWithFonts];
}

typedef std::map<SimpleIdentity,BasicDrawable *> DrawableIDMap;

- (void)renderWithFonts
{
    NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
    
    // Drawables used for the icons
    IconDrawables iconDrawables;
    
    // Drawables we build up as we go
    DrawableIDMap drawables;

    for (WhirlyKitSingleLabel *label in _strs)
    {
        UIColor *theTextColor = _labelInfo.textColor;
        UIColor *theBackColor = _labelInfo.backColor;
        UIFont *theFont = _labelInfo.font;
        UIColor *theShadowColor = _labelInfo.shadowColor;
        float theShadowSize = _labelInfo.shadowSize;
        UIColor *theOutlineColor = _labelInfo.outlineColor;
        float theOutlineSize = _labelInfo.outlineSize;
        if (label.desc)
        {
            theTextColor = [label.desc objectForKey:@"textColor" checkType:[UIColor class] default:theTextColor];
            theBackColor = [label.desc objectForKey:@"backgroundColor" checkType:[UIColor class] default:theBackColor];
            theFont = [label.desc objectForKey:@"font" checkType:[UIFont class] default:theFont];
            theShadowColor = [label.desc objectForKey:@"shadowColor" checkType:[UIColor class] default:theShadowColor];
            theShadowSize = [label.desc floatForKey:@"shadowSize" default:theShadowSize];
            theOutlineColor = [label.desc objectForKey:@"outlineColor" checkType:[UIColor class] default:theOutlineColor];
            theOutlineSize = [label.desc floatForKey:@"outlineSize" default:theOutlineSize];
        }
        if (theShadowColor == nil)
            theShadowColor = [UIColor blackColor];
        if (theOutlineColor == nil)
            theOutlineColor = [UIColor blackColor];
        
        // We set this if the color is embedded in the "font"
        bool embeddedColor = false;
        
        // Break the string into lines and convert the lines to DrawableStrings
        std::vector<DrawableString *> drawStrs;
        NSArray *strings = [label.text componentsSeparatedByString:@"\n"];
        Mbr drawMbr;
        Mbr layoutMbr;
        int whichLine = 0;
        CGFloat lineHeight = theFont.lineHeight;
        for (NSString *text in strings)
        {
            // Build the attributed string
            NSMutableAttributedString *attrStr = [[NSMutableAttributedString alloc] initWithString:text];
            NSInteger strLen = [attrStr length];
            [attrStr addAttribute:NSFontAttributeName value:theFont range:NSMakeRange(0, strLen)];
            if (theOutlineSize > 0.0)
            {
                embeddedColor = true;
                [attrStr addAttribute:kOutlineAttributeSize value:[NSNumber numberWithFloat:theOutlineSize] range:NSMakeRange(0, strLen)];
                [attrStr addAttribute:kOutlineAttributeColor value:theOutlineColor range:NSMakeRange(0, strLen)];
                [attrStr addAttribute:NSForegroundColorAttributeName value:theTextColor range:NSMakeRange(0, strLen)];
            }
            DrawableString *drawStr = [_fontTexManager addString:attrStr changes:_changeRequests];
            if (!drawStr)
                continue;
            Mbr thisMbr = drawStr->mbr;
            thisMbr.ll().y() += lineHeight * whichLine;
            thisMbr.ur().y() += lineHeight * whichLine;
            drawMbr.expand(thisMbr);
            Mbr thisLayoutMbr = drawStr->mbr;
            thisLayoutMbr.ll().y() -= lineHeight * whichLine;
            thisLayoutMbr.ur().y() -= lineHeight * whichLine;
            layoutMbr.expand(thisLayoutMbr);
            drawStrs.push_back(drawStr);
            whichLine++;
        }

        Point2d iconOff(0,0);
        ScreenSpaceObject *screenShape = NULL;
        LayoutObject *layoutObject = NULL;
        
        // Set if we're letting the layout engine control placement
        bool layoutEngine = (_labelInfo.layoutEngine || [label.desc boolForKey:@"layout" default:false]);

        // Portions of the label that are shared between substrings
        Point2d justifyOff(0,0);
        if (_labelInfo.screenObject)
        {
            switch (_labelInfo.labelJustify)
            {
                case WhirlyKitLabelLeft:
                    justifyOff = Point2d(0,0);
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
                layoutObject = new LayoutObject();
                screenShape = layoutObject;
            } else
                screenShape = new ScreenSpaceObject();
            
            // If we're doing layout, don't justify it
            if (layoutEngine)
                justifyOff = Point2d(0,0);
            
            screenShape->setDrawPriority(_labelInfo.drawPriority+1);
            screenShape->setVisibility(_labelInfo.minVis, _labelInfo.maxVis);
            screenShape->setKeepUpright(label.keepUpright);
            
            if (label.rotation != 0.0)
                screenShape->setRotation(label.rotation);
            if (_labelInfo.fadeIn > 0.0)
                screenShape->setFade(curTime+_labelInfo.fadeIn, curTime);
            else if (_labelInfo.fadeOutTime != 0.0)
                screenShape->setFade(_labelInfo.fadeOutTime, _labelInfo.fadeOutTime+_labelInfo.fadeOut);
            if (label.isSelectable && label.selectID != EmptyIdentity)
                screenShape->setId(label.selectID);
            screenShape->setWorldLoc(_coordAdapter->localToDisplay(_coordAdapter->getCoordSystem()->geographicToLocal3d(label.loc)));
            if (label.hasMotion)
            {
                screenShape->setMovingLoc(_coordAdapter->localToDisplay(_coordAdapter->getCoordSystem()->geographicToLocal3d(label.endLoc)),label.startTime,label.endTime);
            }
            
            // If there's an icon, we need to offset
            float height = drawMbr.ur().y()-drawMbr.ll().y();
            Point2d iconSize = (label.iconTexture==EmptyIdentity ? Point2d(0,0) : (label.iconSize.width == 0.0 ? Point2d(height,height) : Point2d(label.iconSize.width,label.iconSize.height)));
            iconOff = iconSize;
            
            // Throw a rectangle in the background
            RGBAColor backColor = [theBackColor asRGBAColor];
            if (backColor.a != 0.0)
            {
                // Note: This is an arbitrary border around the text
                float backBorder = 4.0;
                ScreenSpaceObject::ConvexGeometry smGeom;
                smGeom.progID = _labelInfo.programID;
                Point2d ll = Point2d(drawMbr.ll().x(),drawMbr.ll().y())+iconOff+Point2d(-backBorder,-backBorder), ur = Point2d(drawMbr.ur().x(),drawMbr.ur().y())+iconOff+Point2d(backBorder,backBorder);
                smGeom.coords.push_back(Point2d(ur.x()+label.screenOffset.width,ll.y()+label.screenOffset.height)+ iconOff + justifyOff);
                smGeom.coords.push_back(Point2d(ur.x()+label.screenOffset.width,ur.y()+label.screenOffset.height)+ iconOff + justifyOff);
                smGeom.coords.push_back(Point2d(ll.x()+label.screenOffset.width,ur.y()+label.screenOffset.height)+ iconOff + justifyOff);
                smGeom.coords.push_back(Point2d(ll.x()+label.screenOffset.width,ll.y()+label.screenOffset.height)+ iconOff + justifyOff);
                
                smGeom.drawPriority = _labelInfo.drawPriority;
                smGeom.color = backColor;
                // Note: This would be a great place for a texture
                screenShape->addGeometry(smGeom);
            }
            
            // If it's being passed to the layout engine, do that as well
            if (layoutEngine)
            {
                float layoutImportance = [label.desc floatForKey:@"layoutImportance" default:_labelInfo.layoutImportance];
                int layoutPlacement = [label.desc intForKey:@"layoutPlacement" default:(int)(WhirlyKitLayoutPlacementLeft | WhirlyKitLayoutPlacementRight | WhirlyKitLayoutPlacementAbove | WhirlyKitLayoutPlacementBelow)];
                
                // Put together the layout info
                layoutObject->hint = label.text;
                layoutObject->layoutPts.push_back(Point2d(layoutMbr.ll().x()+label.screenOffset.width,layoutMbr.ll().y()+label.screenOffset.height)+iconOff+justifyOff);
                layoutObject->layoutPts.push_back(Point2d(layoutMbr.ur().x()+label.screenOffset.width,layoutMbr.ll().y()+label.screenOffset.height)+iconOff+justifyOff);
                layoutObject->layoutPts.push_back(Point2d(layoutMbr.ur().x()+label.screenOffset.width,layoutMbr.ur().y()+label.screenOffset.height)+iconOff+justifyOff);
                layoutObject->layoutPts.push_back(Point2d(layoutMbr.ll().x()+label.screenOffset.width,layoutMbr.ur().y()+label.screenOffset.height)+iconOff+justifyOff);
                layoutObject->selectPts = layoutObject->layoutPts;
                
                //                        layoutObj->iconSize = Point2f(iconSize,iconSize);
                layoutObject->importance = layoutImportance;
                layoutObject->acceptablePlacement = layoutPlacement;
                layoutObject->setEnable(_labelInfo.enable);
                
                // The shape starts out disabled
                screenShape->setEnable(_labelInfo.enable);
                if (_labelInfo.startEnable != _labelInfo.endEnable)
                    screenShape->setEnableTime(_labelInfo.startEnable, _labelInfo.endEnable);
                screenShape->setOffset(Point2d(MAXFLOAT,MAXFLOAT));
            } else {
                screenShape->setEnable(_labelInfo.enable);
                if (_labelInfo.startEnable != _labelInfo.endEnable)
                    screenShape->setEnableTime(_labelInfo.startEnable, _labelInfo.endEnable);
            }
            
            // Deal with the icon here becaue we need its geometry
            ScreenSpaceObject::ConvexGeometry iconGeom;
            if (label.iconTexture != EmptyIdentity && screenShape)
            {
                SubTexture subTex = _scene->getSubTexture(label.iconTexture);
                std::vector<TexCoord> texCoord;
                texCoord.resize(4);
                texCoord[3].u() = 0.0;  texCoord[3].v() = 0.0;
                texCoord[2].u() = 1.0;  texCoord[2].v() = 0.0;
                texCoord[1].u() = 1.0;  texCoord[1].v() = 1.0;
                texCoord[0].u() = 0.0;  texCoord[0].v() = 1.0;
                subTex.processTexCoords(texCoord);
                
                iconGeom.texIDs.push_back(subTex.texId);
                iconGeom.progID = _labelInfo.programID;
                Point2d iconPts[4];
                iconPts[0] = Point2d(0,0);
                iconPts[1] = Point2d(iconOff.x(),0);
                iconPts[2] = iconOff;
                iconPts[3] = Point2d(0,iconOff.y());
                for (unsigned int ii=0;ii<4;ii++)
                {
                    iconGeom.coords.push_back(Point2d(iconPts[ii].x(),iconPts[ii].y())+Point2d(label.screenOffset.width,label.screenOffset.height));
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
            if (label.isSelectable && !layoutObject)
            {
                // If the label doesn't already have an ID, it needs one
                if (!label.selectID)
                    label.selectID = Identifiable::genId();
                
                RectSelectable2D select2d;
                select2d.center = screenShape->getWorldLoc();
                select2d.enable = _labelInfo.enable;
                Mbr wholeMbr = drawMbr;
                wholeMbr.ll() += Point2f(iconOff.x(),iconOff.y()) + Point2f(justifyOff.x(),justifyOff.y());
                wholeMbr.ur() += Point2f(iconOff.x(),iconOff.y()) + Point2f(justifyOff.x(),justifyOff.y());
                // If there's an icon, just expand the whole thing.
                // Note: Not ideal
                if (iconGeom.coords.size() > 0)
                    for (unsigned int ig=0;ig<iconGeom.coords.size();ig++)
                        wholeMbr.addPoint(iconGeom.coords[ig]);
                Point2f ll = wholeMbr.ll(), ur = wholeMbr.ur();
                select2d.pts[0] = Point2f(ll.x()+label.screenOffset.width,ll.y()+-label.screenOffset.height);
                select2d.pts[1] = Point2f(ll.x()+label.screenOffset.width,ur.y()+-label.screenOffset.height);
                select2d.pts[2] = Point2f(ur.x()+label.screenOffset.width,ur.y()+-label.screenOffset.height);
                select2d.pts[3] = Point2f(ur.x()+label.screenOffset.width,ll.y()+-label.screenOffset.height);
                
                select2d.selectID = label.selectID;
                select2d.minVis = _labelInfo.minVis;
                select2d.maxVis = _labelInfo.maxVis;
                
                if (label.hasMotion)
                {
                    MovingRectSelectable2D movingSelect2d;
                    (RectSelectable2D &)movingSelect2d = select2d;
                    movingSelect2d.endCenter = screenShape->getEndWorldLoc();
                    movingSelect2d.startTime = screenShape->getStartTime();
                    movingSelect2d.endTime = screenShape->getEndTime();
                    _movingSelectables2D.push_back(movingSelect2d);
                } else
                    _selectables2D.push_back(select2d);
            }
        }

        double offsetY = 0.0;

        // Work through the lists of glyphs
        std::reverse(drawStrs.begin(),drawStrs.end());
        for (DrawableString *drawStr : drawStrs)
        {
            if (!drawStr)
                continue;
            
            _labelRep->drawStrIDs.insert(drawStr->getId());
            
            if (_labelInfo.screenObject)
            {
                Point2d lineOff(0.0,0.0);
                switch (_labelInfo.textJustify)
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
                        color = embeddedColor ? [[UIColor whiteColor] asRGBAColor] : [theTextColor asRGBAColor];
                    } else {
                        soff = Point2d(theShadowSize,theShadowSize);
                        color = [theShadowColor asRGBAColor];
                    }
                    for (unsigned int ii=0;ii<drawStr->glyphPolys.size();ii++)
                    {
                        DrawableString::Rect &poly = drawStr->glyphPolys[ii];
                        // Note: Ignoring the desired size in favor of the font size
                        ScreenSpaceObject::ConvexGeometry smGeom;
                        smGeom.progID = _labelInfo.programID;
                        smGeom.coords.push_back(Point2d(poly.pts[1].x()+label.screenOffset.width,poly.pts[0].y()+label.screenOffset.height+ offsetY) + soff + iconOff + justifyOff + lineOff);
                        smGeom.texCoords.push_back(TexCoord(poly.texCoords[1].u(),poly.texCoords[0].v()));

                        smGeom.coords.push_back(Point2d(poly.pts[1].x()+label.screenOffset.width,poly.pts[1].y()+label.screenOffset.height+ offsetY) + soff + iconOff + justifyOff + lineOff);
                        smGeom.texCoords.push_back(TexCoord(poly.texCoords[1].u(),poly.texCoords[1].v()));

                        smGeom.coords.push_back(Point2d(poly.pts[0].x()+label.screenOffset.width,poly.pts[1].y()+label.screenOffset.height+ offsetY) + soff + iconOff + justifyOff + lineOff);
                        smGeom.texCoords.push_back(TexCoord(poly.texCoords[0].u(),poly.texCoords[1].y()));
                        
                        smGeom.coords.push_back(Point2d(poly.pts[0].x()+label.screenOffset.width,poly.pts[0].y()+label.screenOffset.height+ offsetY) + soff + iconOff + justifyOff + lineOff);
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
            _layoutObjects.push_back(layoutObject);
        else if (screenShape)
            _screenObjects.push_back(screenShape);

        for (DrawableString *drawStr : drawStrs)
            if (drawStr)
                delete drawStr;
    }
    
    // Flush out any drawables we created for the labels
    for (DrawableIDMap::iterator it = drawables.begin(); it != drawables.end(); ++it)
        _changeRequests.push_back(new AddDrawableReq(it->second));

    // Flush out the icon drawables as well
    for (IconDrawables::iterator it = iconDrawables.begin();
         it != iconDrawables.end(); ++it)
    {
        BasicDrawable *iconDrawable = it->second;
        
        if (_labelInfo.fadeIn > 0.0)
        {
            NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
            iconDrawable->setFade(curTime,curTime+_labelInfo.fadeIn);
        }
        _changeRequests.push_back(new AddDrawableReq(iconDrawable));
        _labelRep->drawIDs.insert(iconDrawable->getId());
    }
}

@end
