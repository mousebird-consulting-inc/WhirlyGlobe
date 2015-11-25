/*
 *  FontTextureManageriOS.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/19/14.
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
#import "FontTextureManageriOS.h"
#import "UIColor+Stuff.h"
#import "MaplyTexture_private.h"

namespace WhirlyKit
{

// We scale the fonts up so they look better sampled down.
static const float BogusFontScale = 2.0;

FontTextureManageriOS::FontManageriOS::FontManageriOS(CTFontRef theFont)
{
    font = theFont;
    CFRetain(font);
}
    
FontTextureManageriOS::FontManageriOS::FontManageriOS()
: font(nil)
{
}
    
FontTextureManageriOS::FontManageriOS::~FontManageriOS()
{
    if (font)
        CFRelease(font);
    font = nil;
}
    
// Render the glyph into a buffer
RawDataRef FontTextureManageriOS::renderGlyph(WKGlyph glyph,FontManageriOS *fm,Point2f &size,Point2f &glyphSize,Point2f &offset,Point2f &textureOffset)
{
    int width,height;
    
    // Boundary around the image to capture the full data
    if (fm->outlineSize > 0.0)
    {
        int outlineUp = ceilf(fm->outlineSize);
        textureOffset = Point2f(1+outlineUp, 1+outlineUp);
    } else
        textureOffset = Point2f(1, 1);
    
    CGGlyph cgGlyph;
    CGRect boundRect = CTFontGetBoundingRectsForGlyphs(fm->font,kCTFontDefaultOrientation,&cgGlyph,NULL,1);
    glyph = cgGlyph;
    size.x() = ceilf(boundRect.size.width)+2*textureOffset.x();  size.y() = ceilf(boundRect.size.height)+2*textureOffset.y();
    width = size.x();  height = size.y();
    
    if (width <= 0 || height <= 0)
        return RawDataRef();
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    // Set up a real context for the glyph rendering
    MutableRawDataRef retData(new MutableRawData(width*height*4));
    CGContextRef theContext = CGBitmapContextCreate((void *)retData->getRawData(), width, height, 8, width * 4, colorSpace, kCGImageAlphaPremultipliedLast);
    
    // Flip
    CGContextTranslateCTM(theContext, 0.0f, height);
    CGContextScaleCTM(theContext, 1.0f, -1.0f);
    // Now draw it
    int baselineOffX = (boundRect.origin.x < 0 ? floorf(boundRect.origin.x) : ceilf(boundRect.origin.x));
    int baselineOffY = (boundRect.origin.y < 0 ? floorf(boundRect.origin.y) : ceilf(boundRect.origin.y));
    
    offset = Point2f(baselineOffX,baselineOffY);
    glyphSize = Point2f(boundRect.size.width,boundRect.size.height);
    CGPoint pos = CGPointMake(-baselineOffX+textureOffset.x(),-baselineOffY+textureOffset.y());
    
    // Generate an outline around the glyph
    if (fm->outlineSize > 0.0)
    {
        CGPathRef path = CTFontCreatePathForGlyph(fm->font, glyph, NULL);
        CGContextTranslateCTM(theContext, pos.x, pos.y);
        CGContextAddPath(theContext, path);
        CGContextSetLineWidth(theContext, 2.0*fm->outlineSize);
        UIColor *outlineColor = [UIColor colorFromRGBA:fm->outlineColor];
        CGContextSetStrokeColorWithColor(theContext, outlineColor.CGColor);
        CGContextStrokePath(theContext);
        CGPathRelease(path);
        CGContextTranslateCTM(theContext, -pos.x, -pos.y);
    }
    
    UIColor *textColor = [UIColor colorFromRGBA:fm->color];
    if (textColor == nil)
        textColor = [UIColor whiteColor];
    CGContextSetFillColorWithColor(theContext, textColor.CGColor);
    CTFontDrawGlyphs(fm->font,&cgGlyph,&pos,1,theContext);
    
    // Note: Debugging
    // Draw the baseline
    //    CGContextSetStrokeColorWithColor(theContext,[UIColor whiteColor].CGColor);
    //    CGContextBeginPath(theContext);
    //    CGContextMoveToPoint(theContext, 0.0f, -baselineOffY+textureOffset.y);
    //    CGContextAddLineToPoint(theContext, width, -baselineOffY+textureOffset.y);
    //    CGContextStrokePath(theContext);
    
    // Note: Debugging
    //    UIImage *theImage = [UIImage imageWithRawData:retData width:width height:height];
    //    if (theImage)
    //    {
    //        NSData *imageData = UIImagePNGRepresentation(theImage);
    //        if (imageData)
    //        {
    //            NSArray *myPathList = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    //            NSString *myPath    = [myPathList  objectAtIndex:0];
    //            [imageData writeToFile:[NSString stringWithFormat:@"%@/%d.png",myPath,glyph] atomically:YES];
    //        }
    //    }
    
	CGContextRelease(theContext);
    
    CGColorSpaceRelease(colorSpace);
    
    return retData;
}

FontTextureManageriOS::~FontTextureManageriOS()
{
    for (FontManagerSet::iterator it = fontManagers.begin();
         it != fontManagers.end(); ++it)
        delete *it;
    fontManagers.clear();
}

DrawableString *FontTextureManageriOS::addString(NSAttributedString *str,ChangeSet &changes)
{
    // We could make this more granular
    pthread_mutex_lock(&lock);
    
    // If not initialized, set up texture atlas and such
    init();
    
    DrawableString *drawString = new DrawableString();
    
    // Convert to runs of glyphs
    CTLineRef line = CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)str);
    
    // Work through the runs (which share attributes)
    CFArrayRef runs = CTLineGetGlyphRuns(line);
    CGFloat lineHeight = 0.0, lineWidth = 0.0, ascent = 0.0, descent = 0.0;
    lineWidth = CTLineGetTypographicBounds(line,&ascent,&descent,NULL);
    lineHeight = ascent+descent;
    
    DrawStringRep *drawStringRep = new DrawStringRep(drawString->getId());
    
    drawString->mbr.reset();
    for (unsigned int ii=0;ii<CFArrayGetCount(runs);ii++)
    {
        CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(runs,ii);
        int num = (int)CTRunGetGlyphCount(run);
        if (num > 0)
        {
            CGGlyph *glyphs = (CGGlyph *)malloc(sizeof(CGGlyph)*num);
            CGPoint *offsets = (CGPoint *)malloc(sizeof(CGPoint)*num);
            CFRange range;
            range.location = 0;
            range.length = num;
            CTRunGetGlyphs(run,range,glyphs);
            CTRunGetPositions(run,range,offsets);
            
            // Need the font manager for this run
            NSDictionary *attrs = (__bridge NSDictionary*)CTRunGetAttributes(run);
            UIFont *uiFont = attrs[NSFontAttributeName];
            
            // And outline parameters, if they exist
            UIColor *outlineColor = attrs[kOutlineAttributeColor];
            NSNumber *outlineSize = attrs[kOutlineAttributeSize];
            if (!outlineSize || !outlineColor)
            {
                outlineSize = nil;
                outlineColor = nil;
            }
            UIColor *foregroundColor = attrs[NSForegroundColorAttributeName];
            
            FontManageriOS *fm = nil;
            if ([uiFont isKindOfClass:[UIFont class]])
                fm = findFontManagerForFont(uiFont,foregroundColor,outlineColor,[outlineSize floatValue]);
            if (!fm)
                continue;
            
            GlyphSet glyphsUsed;
            
            // Work through the individual glyphs
            for (unsigned int jj=0;jj<num;jj++)
            {
                CGGlyph glyph = glyphs[jj];
                // Look for an existing one
                FontManager::GlyphInfo *glyphInfo = fm->findGlyph(glyph);
                if (!glyphInfo)
                {
                    // We need to render that Glyph and add it
                    Point2f texSize,glyphSize;
                    Point2f offset,textureOffset;
                    RawDataRef glyphImage = renderGlyph(glyph,fm,texSize,glyphSize,offset,textureOffset);
                    if (glyphImage)
                    {
                        Texture *tex = new Texture("Font Texture Manager",glyphImage,false);
                        tex->setWidth(texSize.x());
                        tex->setHeight(texSize.y());
                        SubTexture subTex;
                        Point2f realSize(glyphSize.x()+2*textureOffset.x(),glyphSize.y()+2*textureOffset.y());
                        std::vector<Texture *> texs;
                        texs.push_back(tex);
                        if (texAtlas->addTexture(texs, &realSize, NULL, subTex, scene->getMemManager(), changes, 0))
                            glyphInfo = fm->addGlyph(glyph, subTex, Point2f(glyphSize.x(),glyphSize.y()), Point2f(offset.x(),offset.y()), Point2f(textureOffset.x(),textureOffset.y()));
                        delete tex;
                    }
                }
                
                if (glyphInfo)
                {
                    // Now we make a rectangle that covers the glyph in its texture atlas
                    DrawableString::Rect rect;
                    CGPoint &offset = offsets[jj];
                    
                    float scale = 1.0/BogusFontScale;
                    
                    // Note: was -1,-1
                    rect.pts[0] = Point2f(glyphInfo->offset.x()*scale-glyphInfo->textureOffset.x()*scale,glyphInfo->offset.y()*scale-glyphInfo->textureOffset.y()*scale)+Point2f(offset.x,offset.y);
                    rect.texCoords[0] = TexCoord(0.0,0.0);
                    // Note: was 2,2
                    rect.pts[1] = Point2f(glyphInfo->size.x()*scale+2*glyphInfo->textureOffset.x()*scale,glyphInfo->size.y()*scale+2*glyphInfo->textureOffset.y()*scale)+rect.pts[0];
                    rect.texCoords[1] = TexCoord(1.0,1.0);
                    
                    rect.subTex = glyphInfo->subTex;
                    drawString->glyphPolys.push_back(rect);
                    drawString->mbr.addPoint(rect.pts[0]);
                    drawString->mbr.addPoint(rect.pts[1]);
                    
                    glyphsUsed.insert(glyphInfo->glyph);
                }
            }
            
            // Keep track of the glyphs we're using
            drawStringRep->addGlyphs(fm->getId(),glyphsUsed);
            fm->addGlyphRefs(glyphsUsed);
            
            free(glyphs);
            free(offsets);
        }
    }
    
    // Need the extents for the whole line
    //    drawString->mbr.ll() = Point2f(0,-descent);
    //    drawString->mbr.ur() = Point2f(lineWidth,ascent);
    
    CFRelease(line);
    
    // If it didn't produce anything, just delete it now
    if (drawString->glyphPolys.empty())
    {
        delete drawString;
        delete drawStringRep;
        drawString = NULL;
    }
    
    // We need to track the glyphs we're using
    drawStringReps.insert(drawStringRep);
    
    pthread_mutex_unlock(&lock);
    
    return drawString;
}

// Look for an existing font that will match the UIFont given
FontTextureManageriOS::FontManageriOS *FontTextureManageriOS::findFontManagerForFont(UIFont *uiFont,UIColor *color,UIColor *outlineColor,float outlineSize)
{
    NSString *fontName = uiFont.fontName;
    float pointSize = uiFont.pointSize;
    
    pointSize *= BogusFontScale;
    
    std::string fontNameStr = [fontName cStringUsingEncoding:NSASCIIStringEncoding];
    RGBAColor colorRGBA = [color asRGBAColor];
    RGBAColor outlineColorRGBA = [outlineColor asRGBAColor];
    
    for (FontManagerSet::iterator it = fontManagers.begin();
         it != fontManagers.end(); ++it)
    {
        FontManageriOS *fm = (FontManageriOS *)*it;
        if (!fontNameStr.compare(fm->fontName) && pointSize == fm->pointSize &&
            (fm->color == colorRGBA) &&
            (fm->outlineColor == outlineColorRGBA) &&
            (fm->outlineSize == outlineSize))
            return fm;
    }
    
    // Create it
    CTFontRef font = CTFontCreateWithName((__bridge CFStringRef)fontName, pointSize, NULL);
    FontManageriOS *fm = new FontManageriOS(font);
    fm->fontName = fontNameStr;
    fm->color = colorRGBA;
    fm->pointSize = pointSize;
    fm->outlineColor = outlineColorRGBA;
    fm->outlineSize = outlineSize;
    //    fm->outlineSize *= BogusFontScale;
    fontManagers.insert(fm);
    if (font)
        CFRelease(font);
    
    return fm;
}

}
