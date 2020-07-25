/*
 *  FontTextureManager_iOS.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/4/19.
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

#import <UIKit/UIKit.h>
#import <CoreText/CoreText.h>
#import "FontTextureManager_iOS.h"
#import "TextureGLES_iOS.h"
#import "TextureMTL.h"
#import "UIColor+Stuff.h"
#import "Scene.h"
#import "SceneRenderer.h"
#import "RawData_NSData.h"
#import "UIImage+Stuff.h"

// We scale the fonts up so they look better sampled down.
static const float BogusFontScale = 2.0;

namespace WhirlyKit
{
    
FontManager_iOS::FontManager_iOS(CTFontRef inFont)
{
    font = inFont;
    CFRetain(font);
}

FontManager_iOS::~FontManager_iOS()
{
    CFRelease(font);
}
    
bool FontManager_iOS::operator < (const FontManager_iOS &that) const
{
    return font < that.font;
}
    
FontTextureManager_iOS::FontTextureManager_iOS(SceneRenderer *sceneRender,Scene *scene)
: FontTextureManager(sceneRender,scene)
{
}
    
FontTextureManager_iOS::~FontTextureManager_iOS()
{
}
    
// Look for an existing font that will match the UIFont given
FontManager_iOSRef FontTextureManager_iOS::findFontManagerForFont(UIFont *uiFont,UIColor *colorUI,UIColor *backColorUI,UIColor *outlineColorUI,float outlineSize)
{
    // We need to scale the font up so it looks better scaled down
    std::string fontName = [uiFont.fontName cStringUsingEncoding:NSASCIIStringEncoding];
    float pointSize = uiFont.pointSize;
    RGBAColor color = [colorUI asRGBAColor];
    RGBAColor backColor = [backColorUI asRGBAColor];
    RGBAColor outlineColor = [outlineColorUI asRGBAColor];
    pointSize *= BogusFontScale;
    uiFont = [UIFont fontWithDescriptor:uiFont.fontDescriptor size:pointSize];
    
    for (auto it : fontManagers)
    {
        FontManager_iOSRef fm = std::dynamic_pointer_cast<FontManager_iOS>(it.second);
        if (fontName == fm->fontName && pointSize == fm->pointSize &&
            fm->color == color &&
            fm->backColor == backColor &&
            fm->outlineColor == outlineColor &&
            fm->outlineSize == outlineSize)
            return fm;
    }
    
    // Create it
    CTFontRef font;
    uiFont = [UIFont fontWithDescriptor:uiFont.fontDescriptor size:pointSize];
    font = (__bridge CTFontRef)uiFont;
    FontManager_iOSRef fm = FontManager_iOSRef(new FontManager_iOS(font));
    fm->fontName = fontName;
    fm->color = color;
    fm->colorUI = colorUI;
    fm->backColor = backColor;
    fm->backColorUI = backColorUI;
    fm->pointSize = pointSize;
    fm->outlineColor = outlineColor;
    fm->outlineColorUI = outlineColorUI;
    fm->outlineSize = outlineSize;
    //    fm->outlineSize *= BogusFontScale;
    fontManagers[fm->getId()] = fm;

    return fm;
}

// Render the glyph into a buffer
NSData *FontTextureManager_iOS::renderGlyph(CGGlyph glyph,FontManager_iOSRef fm,Point2f &size,Point2f &glyphSize,Point2f &offset,Point2f &textureOffset)
{
    int width,height;
    
    // Boundary around the image to capture the full data
    if (fm->outlineSize > 0.0)
    {
        int outlineUp = ceilf(fm->outlineSize);
        textureOffset = Point2f(1+outlineUp, 1+outlineUp);
    } else
        textureOffset = Point2f(1, 1);
    
        CGRect boundRect = CTFontGetBoundingRectsForGlyphs(fm->font,kCTFontDefaultOrientation,&glyph,NULL,1);
        size.x() = ceilf(boundRect.size.width)+2*textureOffset.x();  size.y() = ceilf(boundRect.size.height)+2*textureOffset.y();
        width = size.x();  height = size.y();
    
        if (width <= 0 || height <= 0)
            return nil;
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    // Set up a real context for the glyph rendering
    NSMutableData *retData = [NSMutableData dataWithLength:width*height*4];
    CGContextRef theContext = CGBitmapContextCreate((void *)[retData bytes], width, height, 8, width * 4, colorSpace, kCGImageAlphaPremultipliedLast);
    
    if (fm->backColor.a != 0)
    {
        CGContextSetFillColorWithColor(theContext, fm->backColorUI.CGColor);
        CGContextBeginPath(theContext);
        CGContextAddRect(theContext, CGRectMake(0, 0, width, height));
        CGContextDrawPath(theContext, kCGPathFill);
    }
    
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
        CGContextSetStrokeColorWithColor(theContext, fm->outlineColorUI.CGColor);
        CGContextStrokePath(theContext);
        CGPathRelease(path);
        CGContextTranslateCTM(theContext, -pos.x, -pos.y);
    }
    
    UIColor *textColor = fm->colorUI;
    if (textColor == nil)
        textColor = [UIColor whiteColor];
    CGContextSetFillColorWithColor(theContext, textColor.CGColor);
    CTFontDrawGlyphs(fm->font,&glyph,&pos,1,theContext);

    // Draw the baseline
    //    CGContextSetStrokeColorWithColor(theContext,[UIColor whiteColor].CGColor);
    //    CGContextBeginPath(theContext);
    //    CGContextMoveToPoint(theContext, 0.0f, -baselineOffY+textureOffset.y);
    //    CGContextAddLineToPoint(theContext, width, -baselineOffY+textureOffset.y);
    //    CGContextStrokePath(theContext);

//        UIImage *theImage = [UIImage imageWithRawData:retData width:width height:height];
//        if (theImage)
//        {
//            NSData *imageData = UIImagePNGRepresentation(theImage);
//            if (imageData)
//            {
//                NSArray *myPathList = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
//                NSString *myPath    = [myPathList  objectAtIndex:0];
//                NSString *path = [NSString stringWithFormat:@"%@/%d.png",myPath,glyph];
//                [imageData writeToFile:path atomically:YES];
//                NSLog(@"Glyph: %@",path);
//            }
//        }

    CGContextRelease(theContext);

    CGColorSpaceRelease(colorSpace);

    return retData;
}

/// Add the given string.  Caller is responsible for deleting the DrawableString
WhirlyKit::DrawableString *FontTextureManager_iOS::addString(PlatformThreadInfo *threadInfo,NSAttributedString *str,ChangeSet &changes)
{
    // We could make this more granular
    std::lock_guard<std::mutex> guardLock(lock);
    
    if (!texAtlas)
    {
        // Let's do the biggest possible texture with small cells 32 bits deep
        texAtlas = new DynamicTextureAtlas("Font Texture Atlas",2048,16,TexTypeUnsignedByte);
    }
    
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
        CFIndex num = CTRunGetGlyphCount(run);
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
            UIColor *backgroundColor = attrs[NSBackgroundColorAttributeName];
            
            FontManager_iOSRef fm;
            if ([uiFont isKindOfClass:[UIFont class]])
                fm = findFontManagerForFont(uiFont,foregroundColor,backgroundColor,outlineColor,[outlineSize floatValue]);
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
                    NSData *glyphImage = renderGlyph(glyph, fm, texSize, glyphSize, offset, textureOffset);
                    if (glyphImage)
                    {
                        Texture *tex = nil;
                        if (sceneRender->getType() == SceneRenderer::RenderGLES) {
                            tex = new TextureGLES_iOS("Font Texture Manager",glyphImage,false);
                            tex->setWidth(texSize.x());
                            tex->setHeight(texSize.y());
                        } else {
                            RawDataRef glyphImageWrap(new RawNSDataReader(glyphImage));
                            tex = new TextureMTL("Font Texture Manager",glyphImageWrap,false);
                            tex->setWidth(texSize.x());
                            tex->setHeight(texSize.y());
                        }
                        SubTexture subTex;
                        Point2f realSize(glyphSize.x()+2*textureOffset.x(),glyphSize.y()+2*textureOffset.y());
                        std::vector<Texture *> texs;
                        texs.push_back(tex);
                        if (texAtlas->addTexture(sceneRender, texs, -1, &realSize, NULL, subTex, changes, 0))
                            glyphInfo = fm->addGlyph(glyph, subTex, Point2f(glyphSize.x(),glyphSize.y()), offset, textureOffset);
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
        drawStringRep = NULL;
        drawString = NULL;
    }
    
    // We need to track the glyphs we're using
    if (drawStringRep != NULL)
        drawStringReps.insert(drawStringRep);
        
    return drawString;
}

}
