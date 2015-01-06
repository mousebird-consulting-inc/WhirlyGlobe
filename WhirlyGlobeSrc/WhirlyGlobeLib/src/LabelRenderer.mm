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

#import "LabelLayer.h"
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
    _drawOffset = [desc intForKey:@"drawOffset" default:0];
    _minVis = [desc floatForKey:@"minVis" default:DrawVisibleInvalid];
    _maxVis = [desc floatForKey:@"maxVis" default:DrawVisibleInvalid];
    NSString *justifyStr = [desc stringForKey:@"justify" default:@"middle"];
    _fade = [desc floatForKey:@"fade" default:0.0];
    _shadowColor = [desc objectForKey:@"shadowColor"];
    _shadowSize = [desc floatForKey:@"shadowSize" default:0.0];
    _outlineSize = [desc floatForKey:@"outlineSize" default:0.0];
    _outlineColor = [desc objectForKey:@"outlineColor" checkType:[UIColor class] default:[UIColor blackColor]];
    _shaderID = [desc intForKey:@"shader" default:EmptyIdentity];
    _enable = [desc boolForKey:@"enable" default:true];
    if (![justifyStr compare:@"middle"])
        _justify = WhirlyKitLabelMiddle;
    else {
        if (![justifyStr compare:@"left"])
            _justify = WhirlyKitLabelLeft;
        else {
            if (![justifyStr compare:@"right"])
                _justify = WhirlyKitLabelRight;
        }
    }
    _drawPriority = [desc intForKey:@"drawPriority" default:LabelDrawPriority];
    _programID = [desc intForKey:@"shader" default:EmptyIdentity];
}

// Initialize a label info with data from the description dictionary
- (id)initWithStrs:(NSArray *)inStrs desc:(NSDictionary *)desc
{
    if ((self = [super init]))
    {
        self.strs = inStrs;
        [self parseDesc:desc];
    }
    
    return self;
}

// Draw into an image of the appropriate size (but no bigger)
// Also returns the text size, for calculating texture coordinates
// Note: We don't need a full RGBA image here
- (UIImage *)renderToImage:(WhirlyKitSingleLabel *)label powOfTwo:(BOOL)usePowOfTwo retSize:(CGSize *)textSize texOrg:(TexCoord *)texOrg texDest:(TexCoord *)texDest useAttributedString:(bool)useAttributedString
{
    // A single label can override a few of the label attributes
    UIColor *theTextColor = self.textColor;
    UIColor *theBackColor = self.backColor;
    UIFont *theFont = self.font;
    UIColor *theShadowColor = self.shadowColor;
    float theShadowSize = self.shadowSize;
    if (label.desc)
    {
        theTextColor = [label.desc objectForKey:@"textColor" checkType:[UIColor class] default:theTextColor];
        theBackColor = [label.desc objectForKey:@"backgroundColor" checkType:[UIColor class] default:theBackColor];
        theFont = [label.desc objectForKey:@"font" checkType:[UIFont class] default:theFont];
        theShadowColor = [label.desc objectForKey:@"shadowColor" checkType:[UIColor class] default:theShadowColor];
        theShadowSize = [label.desc floatForKey:@"shadowSize" default:theShadowSize];
    }
    
    // We'll use attributed strings in one case and regular strings in another
    NSMutableAttributedString *attrStr = nil;
    NSString *regStr = nil;
    NSInteger strLen = 0;
    if (useAttributedString)
    {
        // Figure out the size of the string
        attrStr = [[NSMutableAttributedString alloc] initWithString:label.text];
        strLen = [attrStr length];
        [attrStr addAttribute:NSFontAttributeName value:theFont range:NSMakeRange(0, strLen)];
    } else {
        regStr = label.text;
    }
    
    // Figure out how big this needs to be]
    if (attrStr)
    {
        *textSize = [attrStr size];
    } else {
        *textSize = [regStr sizeWithFont:theFont];
    }
    textSize->width += theShadowSize;
    
    if (textSize->width == 0 || textSize->height == 0)
        return nil;
    
    CGSize size;
    if (usePowOfTwo)
    {
        size.width = NextPowOf2(textSize->width);
        size.height = NextPowOf2(textSize->height);
    } else {
        size.width = textSize->width;
        size.height = textSize->height;
    }
    
	UIGraphicsBeginImageContext(size);
	
	// Draw into the image context
	[theBackColor setFill];
	CGContextRef ctx = UIGraphicsGetCurrentContext();
	CGContextFillRect(ctx, CGRectMake(0,0,size.width,size.height));
	
    // Do the background shadow, if requested
    if (theShadowSize > 0.0)
    {
        if (!theShadowColor)
            theShadowColor = [UIColor blackColor];
        CGContextSetLineWidth(ctx, theShadowSize);
        CGContextSetLineJoin(ctx, kCGLineJoinRound);
        CGContextSetTextDrawingMode(ctx, kCGTextStroke);
        if (attrStr)
        {
            [attrStr addAttribute:NSForegroundColorAttributeName value:theShadowColor range:NSMakeRange(0, strLen)];
            [attrStr drawAtPoint:CGPointMake(theShadowSize,0)];
        } else {
            [theShadowColor setStroke];
            [regStr drawAtPoint:CGPointMake(theShadowSize, 0) withFont:theFont];
        }
    }
    
	CGContextSetTextDrawingMode(ctx, kCGTextFill);	
    if (attrStr)
    {
        [attrStr addAttribute:NSForegroundColorAttributeName value:theTextColor range:NSMakeRange(0, strLen)];
        [attrStr drawAtPoint:CGPointMake(theShadowSize,0)];
    } else {
        [theTextColor setFill];
        [regStr drawAtPoint:CGPointMake(theShadowSize, 0) withFont:theFont];
    }
	// Grab the image and shut things down
	UIImage *retImage = UIGraphicsGetImageFromCurrentImageContext();
	UIGraphicsEndImageContext();
    
    if (usePowOfTwo)
    {
        texOrg->u() = 0.0;  texOrg->v() = textSize->height / size.height;
        texDest->u() = textSize->width / size.width;  texDest->v() = 0.0;
    } else {
        texOrg->u() = 0.0;  texOrg->v() = 1.0;
        texDest->u() = 1.0;  texDest->v() = 0.0;
    }
    
    return retImage;
}

@end

// Used to track the rendered image cache
class RenderedImage
{
public:
    RenderedImage() : image(NULL) { }
    RenderedImage(const RenderedImage &that) : textSize(that.textSize), image(that.image) { }
    ~RenderedImage() { }
    const RenderedImage & operator = (const RenderedImage &that) { textSize = that.textSize;  image = that.image; return *this; }
    RenderedImage(CGSize textSize,UIImage *image) : textSize(textSize), image(image) { }
    CGSize textSize;
    UIImage *image;
};

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
    if (_fontTexManager && _useAttributedString)
        [self renderWithFonts];
    else
        [self renderWithImages];
}

typedef std::map<SimpleIdentity,BasicDrawable *> DrawableIDMap;

- (void)renderWithFonts
{
    NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
    
    // Drawables used for the icons
    IconDrawables iconDrawables;
    
    // Drawables we build up as we go
    DrawableIDMap drawables;

    for (WhirlyKitSingleLabel *label in _labelInfo.strs)
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

        // Build the attributed string
        NSMutableAttributedString *attrStr = [[NSMutableAttributedString alloc] initWithString:label.text];
        NSInteger strLen = [attrStr length];
        [attrStr addAttribute:NSFontAttributeName value:theFont range:NSMakeRange(0, strLen)];
        if (theOutlineSize > 0.0)
        {
            embeddedColor = true;
            [attrStr addAttribute:kOutlineAttributeSize value:[NSNumber numberWithFloat:theOutlineSize] range:NSMakeRange(0, strLen)];
            [attrStr addAttribute:kOutlineAttributeColor value:theOutlineColor range:NSMakeRange(0, strLen)];
            [attrStr addAttribute:NSForegroundColorAttributeName value:theTextColor range:NSMakeRange(0, strLen)];
        }
        Point2d iconOff(0,0);
        ScreenSpaceObject *screenShape = NULL;
        LayoutObject *layoutObject = NULL;
        if (attrStr && strLen > 0)
        {
            DrawableString *drawStr = [_fontTexManager addString:attrStr changes:_changeRequests];
            if (drawStr)
            {
                _labelRep->drawStrIDs.insert(drawStr->getId());

                Point2d justifyOff(0,0);
                switch (_labelInfo.justify)
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
                
                if (_labelInfo.screenObject)
                {
                    // Set if we're letting the layout engine control placement
                    bool layoutEngine = (_labelInfo.layoutEngine || [label.desc boolForKey:@"layout" default:false]);
                    
                    if (layoutEngine)
                    {
                        layoutObject = new LayoutObject();
                        screenShape = layoutObject;
                    } else
                        screenShape = new ScreenSpaceObject();
                    
                    // If we're doing layout, don't justify it
                    if (layoutEngine)
                        justifyOff = Point2d(0,0);
                    
                    screenShape->setDrawPriority(_labelInfo.drawPriority);
                    screenShape->setVisibility(_labelInfo.minVis, _labelInfo.maxVis);
                    screenShape->setKeepUpright(label.keepUpright);
                    
                    if (label.rotation != 0.0)
                        screenShape->setRotation(label.rotation);
                    if (_labelInfo.fade > 0.0)
                        screenShape->setFade(curTime+_labelInfo.fade, curTime);
                    if (label.isSelectable && label.selectID != EmptyIdentity)
                        screenShape->setId(label.selectID);
                    screenShape->setWorldLoc(_coordAdapter->localToDisplay(_coordAdapter->getCoordSystem()->geographicToLocal3d(label.loc)));

                    // If there's an icon, we need to offset
                    float height = drawStr->mbr.ur().y()-drawStr->mbr.ll().y();
                    Point2d iconSize = (label.iconTexture==EmptyIdentity ? Point2d(0,0) : (label.iconSize.width == 0.0 ? Point2d(height,height) : Point2d(label.iconSize.width,label.iconSize.height)));
                    iconOff = iconSize;
                    
                    // Throw a rectangle in the background
                    RGBAColor backColor = [theBackColor asRGBAColor];
                    if (backColor.a != 0.0)
                    {
                        // Note: This is an arbitrary border around the text
                        float backBorder = 4.0;
                        ScreenSpaceObject::ConvexGeometry smGeom;
                        Point2d ll = Point2d(drawStr->mbr.ll().x(),drawStr->mbr.ll().y())+iconOff+Point2d(-backBorder,-backBorder), ur = Point2d(drawStr->mbr.ur().x(),drawStr->mbr.ur().y())+iconOff+Point2d(backBorder,backBorder);
                        smGeom.coords.push_back(Point2d(ll.x()+label.screenOffset.width,-ll.y()+label.screenOffset.height)+justifyOff);
                        smGeom.texCoords.push_back(TexCoord(0,1));
                       
                        smGeom.coords.push_back(Point2d(ll.x()+label.screenOffset.width,-ur.y()+label.screenOffset.height)+justifyOff);
                        smGeom.texCoords.push_back(TexCoord(1,1));

                        smGeom.coords.push_back(Point2d(ur.x()+label.screenOffset.width,-ur.y()+label.screenOffset.height)+justifyOff);
                        smGeom.texCoords.push_back(TexCoord(1,0));

                        smGeom.coords.push_back(Point2d(ur.x()+label.screenOffset.width,-ll.y()+label.screenOffset.height)+justifyOff);
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
                            smGeom.coords.push_back(Point2d(poly.pts[1].x()+label.screenOffset.width,poly.pts[0].y()-label.screenOffset.height) + soff + iconOff + justifyOff);
                            smGeom.texCoords.push_back(TexCoord(poly.texCoords[1].u(),poly.texCoords[0].v()));

                            smGeom.coords.push_back(Point2d(poly.pts[1].x()+label.screenOffset.width,poly.pts[1].y()-label.screenOffset.height) + soff + iconOff + justifyOff);
                            smGeom.texCoords.push_back(TexCoord(poly.texCoords[1].u(),poly.texCoords[1].v()));

                            smGeom.coords.push_back(Point2d(poly.pts[0].x()+label.screenOffset.width,poly.pts[1].y()-label.screenOffset.height) + soff + iconOff + justifyOff);
                            smGeom.texCoords.push_back(TexCoord(poly.texCoords[0].u(),poly.texCoords[1].y()));
                            
                            smGeom.coords.push_back(Point2d(poly.pts[0].x()+label.screenOffset.width,poly.pts[0].y()-label.screenOffset.height) + soff + iconOff + justifyOff);
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
                        float layoutImportance = [label.desc floatForKey:@"layoutImportance" default:_labelInfo.layoutImportance];
                        int layoutPlacement = [label.desc intForKey:@"layoutPlacement" default:(int)(WhirlyKitLayoutPlacementLeft | WhirlyKitLayoutPlacementRight | WhirlyKitLayoutPlacementAbove | WhirlyKitLayoutPlacementBelow)];
                        
                        // Put together the layout info
                        layoutObject->hint = label.text;
                        drawStr->mbr.asPoints(layoutObject->layoutPts);
                        layoutObject->layoutPts.push_back(Point2d(drawStr->mbr.ll().x()+label.screenOffset.width,drawStr->mbr.ll().y()+label.screenOffset.height)+iconOff+justifyOff);
                        layoutObject->layoutPts.push_back(Point2d(drawStr->mbr.ur().x()+label.screenOffset.width,drawStr->mbr.ll().y()+label.screenOffset.height)+iconOff+justifyOff);
                        layoutObject->layoutPts.push_back(Point2d(drawStr->mbr.ur().x()+label.screenOffset.width,drawStr->mbr.ur().y()+label.screenOffset.height)+iconOff+justifyOff);
                        layoutObject->layoutPts.push_back(Point2d(drawStr->mbr.ll().x()+label.screenOffset.width,drawStr->mbr.ur().y()+label.screenOffset.height)+iconOff+justifyOff);
                        layoutObject->selectPts = layoutObject->layoutPts;
                        
//                        layoutObj->iconSize = Point2f(iconSize,iconSize);
                        layoutObject->importance = layoutImportance;
                        layoutObject->acceptablePlacement = layoutPlacement;
                        layoutObject->setEnable(_labelInfo.enable);
                        
                        // The shape starts out disabled
                        screenShape->setEnable(_labelInfo.enable);
                        screenShape->setOffset(Point2d(MAXFLOAT,MAXFLOAT));
                    } else {
                        screenShape->setEnable(_labelInfo.enable);
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
                        Mbr wholeMbr = drawStr->mbr;
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
                        _selectables2D.push_back(select2d);
                    }
                    
                    if (layoutObject)
                        _layoutObjects.push_back(layoutObject);
                    else if (screenShape)
                        _screenObjects.push_back(screenShape);
                }
            
                delete drawStr;
            }
        }        
    }
    
    // Flush out any drawables we created for the labels
    for (DrawableIDMap::iterator it = drawables.begin(); it != drawables.end(); ++it)
        _changeRequests.push_back(new AddDrawableReq(it->second));

    // Flush out the icon drawables as well
    for (IconDrawables::iterator it = iconDrawables.begin();
         it != iconDrawables.end(); ++it)
    {
        BasicDrawable *iconDrawable = it->second;
        
        if (_labelInfo.fade > 0.0)
        {
            NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
            iconDrawable->setFade(curTime,curTime+_labelInfo.fade);
        }
        _changeRequests.push_back(new AddDrawableReq(iconDrawable));
        _labelRep->drawIDs.insert(iconDrawable->getId());
    }
}

- (void)renderWithImages
{
    NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
    
    // Texture atlases we're building up for the labels
    std::vector<TextureAtlas *> texAtlases;
    std::vector<BasicDrawable *> drawables;
    
    // Drawables used for the icons
    IconDrawables iconDrawables;
    
    // Let's only bother for more than one label and if we're not using
    //  the font manager
    bool texAtlasOn = [_labelInfo.strs count] > 1 && (_fontTexManager == nil);
    
    // Keep track of images rendered from text
    std::map<std::string,RenderedImage> renderedImages;
    
    // Work through the labels
    for (WhirlyKitSingleLabel *label in _labelInfo.strs)
    {
        TexCoord texOrg,texDest;
        CGSize textSize;
        
        BasicDrawable *drawable = NULL;
        TextureAtlas *texAtlas = nil;
        UIImage *textImage = nil;
        {
            // Find the image (if we already rendered it) or create it as needed
            std::string labelStr = [label.text asStdString];
            std::string labelKey = [label keyString];
            bool skipReuse = false;
            if (labelStr.length() != [label.text length])
                skipReuse = true;
            std::map<std::string,RenderedImage>::iterator it = renderedImages.find(labelKey);
            if (it != renderedImages.end())
            {
                textSize = it->second.textSize;
                textImage = it->second.image;
            } else {
                textImage = [_labelInfo renderToImage:label powOfTwo:!texAtlasOn retSize:&textSize texOrg:&texOrg texDest:&texDest useAttributedString:_useAttributedString];
                if (!textImage)
                    continue;
                if (!skipReuse)
                    renderedImages[labelKey] = RenderedImage(textSize,textImage);
            }
            
            // Look for a spot in an existing texture atlas
            int foundii = -1;
            
            if (texAtlasOn && textSize.width <= _textureAtlasSize &&
                textSize.height <= _textureAtlasSize)
            {
                for (unsigned int ii=0;ii<texAtlases.size();ii++)
                {
                    if ([texAtlases[ii] addImage:textImage texOrg:texOrg texDest:texDest])
                        foundii = ii;
                }
                if (foundii < 0)
                {
                    // If we didn't find one, add a new one
                    texAtlas = [[TextureAtlas alloc] initWithTexSizeX:_textureAtlasSize texSizeY:_textureAtlasSize cellSizeX:8 cellSizeY:8];
                    foundii = (int)texAtlases.size();
                    texAtlases.push_back(texAtlas);
                    [texAtlas addImage:textImage texOrg:texOrg texDest:texDest];
                    
                    if (!_labelInfo.screenObject)
                    {
                        // And a corresponding drawable
                        BasicDrawable *drawable = new BasicDrawable("Label Layer");
                        drawable->setProgram(_labelInfo.shaderID);
                        drawable->setDrawOffset(_labelInfo.drawOffset);
                        drawable->setType(GL_TRIANGLES);
                        drawable->setColor(RGBAColor(255,255,255,255));
                        drawable->setDrawPriority(_labelInfo.drawPriority);
                        drawable->setVisibleRange(_labelInfo.minVis,_labelInfo.maxVis);
                        drawable->setAlpha(true);
                        drawable->setOnOff(_labelInfo.enable);
                        drawables.push_back(drawable);
                    }
                }
                if (!_labelInfo.screenObject)
                    drawable = drawables[foundii];
                texAtlas = texAtlases[foundii];
            } else {
                if (!_labelInfo.screenObject)
                {
                    // Add a drawable for just the one label because it's too big
                    drawable = new BasicDrawable("Label Layer");
                    drawable->setProgram(_labelInfo.shaderID);
                    drawable->setDrawOffset(_labelInfo.drawOffset);
                    drawable->setType(GL_TRIANGLES);
                    drawable->setColor(RGBAColor(255,255,255,255));
                    drawable->addTriangle(BasicDrawable::Triangle(0,1,2));
                    drawable->addTriangle(BasicDrawable::Triangle(2,3,0));
                    drawable->setDrawPriority(_labelInfo.drawPriority);
                    drawable->setVisibleRange(_labelInfo.minVis,_labelInfo.maxVis);
                    drawable->setOnOff(_labelInfo.enable);
                    drawable->setAlpha(true);
                }
            }
        }
        
        // Figure out the extents in 3-space
        // Note: Probably won't work at the poles
        
        // Width and height can be overriden per label
        float theWidth = _labelInfo.width;
        float theHeight = _labelInfo.height;
        if (label.desc)
        {
            theWidth = [label.desc floatForKey:@"width" default:theWidth];
            theHeight = [label.desc floatForKey:@"height" default:theHeight];
        }
        
        float width2,height2;
        if (theWidth != 0.0)
        {
            height2 = theWidth * textSize.height / ((float)2.0 * textSize.width);
            width2 = theWidth/2.0;
        } else {
            width2 = theHeight * textSize.width / ((float)2.0 * textSize.height);
            height2 = theHeight/2.0;
        }
        
        // If there's an icon, we need to offset the label
        Point2f iconSize = (label.iconTexture==EmptyIdentity ? Point2f(0,0) : (label.iconSize.width == 0.0 ? Point2f(2*height2,2*height2) : Point2f(label.iconSize.width,label.iconSize.height)));
        
        Point3f norm;
        Point3f pts[4],iconPts[4];
        ScreenSpaceObject *screenShape = NULL;
        LayoutObject *layoutObject = NULL;
        if (_labelInfo.screenObject)
        {
            // Set if we're letting the layout engine control placement
            bool layoutEngine = (_labelInfo.layoutEngine || [label.desc boolForKey:@"layout" default:false]);
            
            if (layoutEngine)
            {
                layoutObject = new LayoutObject();
                screenShape = layoutObject;
            } else
                screenShape = new ScreenSpaceObject();

            // Texture coordinates are a little odd because text might not take up the whole texture
            TexCoord texCoord[4];
            texCoord[3].u() = texOrg.u();  texCoord[3].v() = texDest.v();
            texCoord[2].u() = texDest.u();  texCoord[2].v() = texDest.v();
            texCoord[1].u() = texDest.u();  texCoord[1].v() = texOrg.v();
            texCoord[0].u() = texOrg.u();  texCoord[0].v() = texOrg.v();
            
            [label calcScreenExtents2:width2 height2:height2 iconSize:iconSize justify:_labelInfo.justify corners:pts iconCorners:iconPts useIconOffset:(layoutEngine == false)];
            screenShape->setDrawPriority(_labelInfo.drawPriority);
            screenShape->setVisibility(_labelInfo.minVis, _labelInfo.maxVis);
            screenShape->setOffset(Point2d(label.screenOffset.width,label.screenOffset.height));
            if (_labelInfo.fade > 0.0)
                screenShape->setFade(curTime+_labelInfo.fade, curTime);
            if (label.isSelectable && label.selectID != EmptyIdentity)
                screenShape->setId(label.selectID);
            screenShape->setWorldLoc(_coordAdapter->localToDisplay(_coordAdapter->getCoordSystem()->geographicToLocal3d(label.loc)));
            ScreenSpaceObject::ConvexGeometry smGeom;
            smGeom.progID = _labelInfo.programID;
            for (unsigned int ii=0;ii<4;ii++)
            {
                smGeom.coords.push_back(Point2d(pts[ii].x(),pts[ii].y()));
                smGeom.texCoords.push_back(texCoord[ii]);
            }
            //            smGeom.color = labelInfo.color;
            if (!texAtlas)
            {
                // This texture was unique to the object
                Texture *tex = new Texture("Label Layer",textImage);
                if (_labelInfo.screenObject)
                    tex->setUsesMipmaps(false);
                _changeRequests.push_back(new AddTextureReq(tex));
                smGeom.texIDs.push_back(tex->getId());
                _labelRep->texIDs.insert(tex->getId());
            } else
                smGeom.texIDs.push_back(texAtlas.texId);
            screenShape->addGeometry(smGeom);
            
            // If it's being passed to the layout engine, do that as well
            if (layoutEngine)
            {
                float layoutImportance = [label.desc floatForKey:@"layoutImportance" default:_labelInfo.layoutImportance];
                
                // Put together the layout info
                layoutObject->layoutPts.push_back(Point2d(-width2+label.screenOffset.width,-height2+label.screenOffset.height));
                layoutObject->layoutPts.push_back(Point2d(width2+label.screenOffset.width,-height2+label.screenOffset.height));
                layoutObject->layoutPts.push_back(Point2d(width2+label.screenOffset.width,height2+label.screenOffset.height));
                layoutObject->layoutPts.push_back(Point2d(-width2+label.screenOffset.width,height2+label.screenOffset.height));
                layoutObject->selectPts = layoutObject->layoutPts;
                layoutObject->importance = layoutImportance;
                // Note: Should parse out acceptable placements as well
                layoutObject->acceptablePlacement = WhirlyKitLayoutPlacementLeft | WhirlyKitLayoutPlacementRight | WhirlyKitLayoutPlacementAbove | WhirlyKitLayoutPlacementBelow;
                
                // The shape starts out disabled
                screenShape->setEnable(_labelInfo.enable);
                screenShape->setOffset(Point2d(MAXFLOAT,MAXFLOAT));
            } else
                screenShape->setEnable(_labelInfo.enable);
            
            if (layoutObject)
                _layoutObjects.push_back(layoutObject);
            else if (screenShape)
                _screenObjects.push_back(screenShape);
        } else {
            // Texture coordinates are a little odd because text might not take up the whole texture
            TexCoord texCoord[4];
            texCoord[0].u() = texOrg.u();  texCoord[0].v() = texOrg.v();
            texCoord[1].u() = texDest.u();  texCoord[1].v() = texOrg.v();
            texCoord[2].u() = texDest.u();  texCoord[2].v() = texDest.v();
            texCoord[3].u() = texOrg.u();  texCoord[3].v() = texDest.v();
            
            Point3f ll;
            
            [label calcExtents2:width2 height2:height2 iconSize:iconSize justify:_labelInfo.justify corners:pts norm:&norm iconCorners:iconPts coordAdapter:_coordAdapter];
            
            // Add to the drawable we found (corresponding to a texture atlas)
            int vOff = drawable->getNumPoints();
            for (unsigned int ii=0;ii<4;ii++)
            {
                Point3f &pt = pts[ii];
                drawable->addPoint(pt);
                drawable->addNormal(norm);
                drawable->addTexCoord(0,texCoord[ii]);
                Mbr localMbr = drawable->getLocalMbr();
                Point3f localLoc = _coordAdapter->getCoordSystem()->geographicToLocal(label.loc);
                localMbr.addPoint(Point2f(localLoc.x(),localLoc.y()));
                drawable->setLocalMbr(localMbr);
            }
            drawable->addTriangle(BasicDrawable::Triangle(0+vOff,1+vOff,2+vOff));
            drawable->addTriangle(BasicDrawable::Triangle(2+vOff,3+vOff,0+vOff));
            
            // If we don't have a texture atlas (didn't fit), just hand over
            //  the drawable and make a new texture
            if (!texAtlas)
            {
                Texture *tex = new Texture("Label Layer",textImage);
                drawable->setTexId(0,tex->getId());
                
                if (_labelInfo.fade > 0.0)
                {
                    drawable->setFade(curTime,curTime+_labelInfo.fade);
                }
                
                // Pass over to the renderer
                _changeRequests.push_back(new AddTextureReq(tex));
                _changeRequests.push_back(new AddDrawableReq(drawable));
                
                _labelRep->texIDs.insert(tex->getId());
                _labelRep->drawIDs.insert(drawable->getId());
            }
        }
        
        // Register the main label as selectable
        if (label.isSelectable && !layoutObject)
        {
            // If the label doesn't already have an ID, it needs one
            if (!label.selectID)
                label.selectID = Identifiable::genId();
            
            if (_labelInfo.screenObject)
            {
                RectSelectable2D select2d;
                select2d.enable = _labelInfo.enable;
                for (unsigned int pp=0;pp<4;pp++)
                    select2d.pts[pp] = Point2f(pts[pp].x(),pts[pp].y());
                select2d.center = screenShape->getWorldLoc();
                select2d.selectID = label.selectID;
                select2d.minVis = _labelInfo.minVis;
                select2d.maxVis = _labelInfo.maxVis;
                _selectables2D.push_back(select2d);
            } else {
                RectSelectable3D select3d;
                select3d.enable = _labelInfo.enable;
                select3d.selectID = label.selectID;
                for (unsigned int jj=0;jj<4;jj++)
                    select3d.pts[jj] = pts[jj];
                _selectables3D.push_back(select3d);
            }
        }
        
        // If there's an icon, let's add that
        if (label.iconTexture != EmptyIdentity)
        {
            SubTexture subTex = _scene->getSubTexture(label.iconTexture);
            std::vector<TexCoord> texCoord;
            texCoord.resize(4);
            
            // Note: We're not registering icons correctly with the selection layer
            if (_labelInfo.screenObject)
            {
                texCoord[3].u() = 0.0;  texCoord[3].v() = 0.0;
                texCoord[2].u() = 1.0;  texCoord[2].v() = 0.0;
                texCoord[1].u() = 1.0;  texCoord[1].v() = 1.0;
                texCoord[0].u() = 0.0;  texCoord[0].v() = 1.0;
                subTex.processTexCoords(texCoord);

                ScreenSpaceObject::ConvexGeometry iconGeom;
                iconGeom.progID = _labelInfo.programID;
                iconGeom.texIDs.push_back(subTex.texId);
                for (unsigned int ii=0;ii<4;ii++)
                {
                    iconGeom.coords.push_back(Point2d(iconPts[ii].x(),iconPts[ii].y()));
                    iconGeom.texCoords.push_back(texCoord[ii]);
                }
                // For layout objects, we'll put the icons on their own
//                if (layoutObj)
//                {
//                    ScreenSpaceGenerator::ConvexShape *iconScreenShape = new ScreenSpaceGenerator::ConvexShape();
//                    SimpleIdentity iconId = iconScreenShape->getId();
//                    *iconScreenShape = *screenShape;
//                    iconScreenShape->setId(iconId);
//                    iconScreenShape->geom.clear();
//                    iconScreenShape->geom.push_back(iconGeom);
//                    screenObjects.push_back(iconScreenShape);
//                    labelRep->screenIDs.insert(iconScreenShape->getId());
//                    layoutObj->auxIDs.insert(iconScreenShape->getId());
//                } else {
                    screenShape->addGeometry(iconGeom);
//                }
            } else {
                texCoord[0].u() = 0.0;  texCoord[0].v() = 0.0;
                texCoord[1].u() = 1.0;  texCoord[1].v() = 0.0;
                texCoord[2].u() = 1.0;  texCoord[2].v() = 1.0;
                texCoord[3].u() = 0.0;  texCoord[3].v() = 1.0;
                subTex.processTexCoords(texCoord);

                // Try to add this to an existing drawable
                IconDrawables::iterator it = iconDrawables.find(subTex.texId);
                BasicDrawable *iconDrawable = NULL;
                if (it == iconDrawables.end())
                {
                    // Create one
                    iconDrawable = new BasicDrawable("Label Layer");
                    drawable->setProgram(_labelInfo.shaderID);
                    iconDrawable->setDrawOffset(_labelInfo.drawOffset);
                    iconDrawable->setType(GL_TRIANGLES);
                    iconDrawable->setColor(RGBAColor(255,255,255,255));
                    iconDrawable->setDrawPriority(_labelInfo.drawPriority);
                    iconDrawable->setVisibleRange(_labelInfo.minVis,_labelInfo.maxVis);
                    iconDrawable->setAlpha(true);  // Note: Don't know this
                    iconDrawable->setTexId(0,subTex.texId);
                    iconDrawable->setOnOff(_labelInfo.enable);
                    iconDrawables[subTex.texId] = iconDrawable;
                } else
                    iconDrawable = it->second;
                
                // Add to the drawable we found (corresponding to a texture atlas)
                int vOff = iconDrawable->getNumPoints();
                for (unsigned int ii=0;ii<4;ii++)
                {
                    Point3f &pt = iconPts[ii];
                    iconDrawable->addPoint(pt);
                    iconDrawable->addNormal(norm);
                    iconDrawable->addTexCoord(0,texCoord[ii]);
                    Mbr localMbr = iconDrawable->getLocalMbr();
                    Point3f localLoc = _coordAdapter->getCoordSystem()->geographicToLocal(label.loc);
                    localMbr.addPoint(Point2f(localLoc.x(),localLoc.y()));
                    iconDrawable->setLocalMbr(localMbr);
                }
                iconDrawable->addTriangle(BasicDrawable::Triangle(0+vOff,1+vOff,2+vOff));
                iconDrawable->addTriangle(BasicDrawable::Triangle(2+vOff,3+vOff,0+vOff));
            }
        }
    }
    
    // Generate textures from the atlases, point the drawables at them
    //  and hand both over to the rendering thread
    // Keep track of all of this stuff for the label representation (for deletion later)
    for (unsigned int ii=0;ii<texAtlases.size();ii++)
    {
        UIImage *theImage = nil;
        Texture *tex = [texAtlases[ii] createTexture:&theImage];
        if (_labelInfo.screenObject)
            tex->setUsesMipmaps(false);
        //        tex->createInGL(true,scene->getMemManager());
        _changeRequests.push_back(new AddTextureReq(tex));
        _labelRep->texIDs.insert(tex->getId());
        
        if (!_labelInfo.screenObject)
        {
            BasicDrawable *drawable = drawables[ii];
            drawable->setTexId(0,tex->getId());
            
            if (_labelInfo.fade > 0.0)
            {
                NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
                drawable->setFade(curTime,curTime+_labelInfo.fade);
            }
            _changeRequests.push_back(new AddDrawableReq(drawable));
            _labelRep->drawIDs.insert(drawable->getId());
        }
    }
    
    // Flush out the icon drawables as well
    for (IconDrawables::iterator it = iconDrawables.begin();
         it != iconDrawables.end(); ++it)
    {
        BasicDrawable *iconDrawable = it->second;
        
        if (_labelInfo.fade > 0.0)
        {
            NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
            iconDrawable->setFade(curTime,curTime+_labelInfo.fade);
        }
        _changeRequests.push_back(new AddDrawableReq(iconDrawable));
        _labelRep->drawIDs.insert(iconDrawable->getId());
    }
}

@end
