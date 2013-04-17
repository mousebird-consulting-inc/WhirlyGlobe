/*
 *  FontTextureManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/15/13.
 *  Copyright 2011-2012 mousebird consulting
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

#import "FontTextureManager.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{
    
/// Manages the glyphs for a single font
class FontManager
{
public:
    FontManager(CTFontRef theFont) : font(theFont) { CFRetain(font); }
    ~FontManager() { CFRelease(font); }
    
    CTFontRef font;

    // Mapping info from glyph to location in a dynamic texture
    class GlyphInfo
    {
    public:
        GlyphInfo() : glyph(0) { }
        GlyphInfo(CGGlyph glyph) : glyph(glyph) { }
        bool operator < (const GlyphInfo &that) const
        { return glyph < that.glyph; }
        CGGlyph glyph;
        SubTexture subTex;
    };
            
    bool empty() { return glyphs.empty(); }

    // Look for an existing glyph and return it if it's there
    bool findGlyph(CGGlyph glyph,GlyphInfo &glyphInfo)
    {
        GlyphInfo dummyGlyph(glyph);
        GlyphInfoSet::iterator it = glyphs.find(dummyGlyph);
        if (it != glyphs.end())
        {
            glyphInfo = *it;
            return true;
        }
        
        return false;
    }
            
    // Add the given glyph info
    void addGlyph(CGGlyph glyph,SubTexture subTex)
    {
        GlyphInfo info(glyph);
        info.subTex = subTex;
        glyphs.insert(info);
    }
    
protected:
    // Maps Glyphs (shorts) to texture and region
    typedef std::set<GlyphInfo> GlyphInfoSet;
    GlyphInfoSet glyphs;
};
            
// Used to order a set of these
typedef struct
{
    bool operator () (const FontManager *a,const FontManager *b) { return a->font < b->font; }
} FontManagerSorter;
            
typedef std::set<FontManager *,FontManagerSorter> FontManagerSet;
    
}

@implementation WhirlyKitFontTextureManager
{
    Scene *scene;
    DynamicTextureAtlas *texAtlas;
    FontManagerSet fontManagers;
}

- (id)initWithScene:(WhirlyKit::Scene *)inScene
{
    self = [super init];
    if (!self)
        return nil;
    
    scene = inScene;
    
    return self;
}

- (void)dealloc
{
    if (texAtlas)
        delete texAtlas;
    texAtlas = NULL;
    for (FontManagerSet::iterator it = fontManagers.begin();
         it != fontManagers.end(); ++it)
        delete *it;
    fontManagers.clear();
}
            
- (void)clear:(std::vector<WhirlyKit::ChangeRequest *> &)changes
{
    if (texAtlas)
    {
        texAtlas->shutdown(changes);
        delete texAtlas;
        texAtlas = NULL;
    }
    for (FontManagerSet::iterator it = fontManagers.begin();
         it != fontManagers.end(); ++it)
        delete *it;
    fontManagers.clear();
}


// Render the glyph into a buffer
- (NSData *)renderGlyph:(CGGlyph)glyph font:(CTFontRef)font size:(CGSize &)size
{    
    // Get the bounding rectangle for the glyph
    CGRect boundRect;
    CTFontGetBoundingRectsForGlyphs(font, kCTFontDefaultOrientation, &glyph, &boundRect, 1);
    CGAffineTransform transform;
    CGPathRef path = CTFontCreatePathForGlyph(font,glyph,&transform);
    int width = boundRect.size.width, height = boundRect.size.height;
    size.width = width;  size.height = height;

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    NSMutableData *retData = [NSMutableData dataWithLength:width*height];
    CGContextRef theContext = CGBitmapContextCreate((void *)[retData bytes], width, height, 8, width * 4, colorSpace, 0);

    // Now draw it
    CGContextSetStrokeColorWithColor(theContext, [UIColor whiteColor].CGColor);
    CGContextSetFillColorWithColor(theContext, [UIColor whiteColor].CGColor);
    CGContextAddPath(theContext,path);
    CGPathRelease(path);

	CGContextRelease(theContext);
    CGColorSpaceRelease(colorSpace);
    
    CFRelease(font);

    return retData;
}

- (WhirlyKit::DrawableString *)addString:(NSAttributedString *)str changes:(std::vector<ChangeRequest *> &)changes
{
    if (!texAtlas)
        // Let's do the biggest possible texture with small cells 24 bits deep
        texAtlas = new DynamicTextureAtlas(2048,16,GL_UNSIGNED_BYTE);
    
    // Convert to runs of glyphs
    CTLineRef line = CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)str);

    // Work through the runs (which share attributes)
    CFArrayRef runs = CTLineGetGlyphRuns(line);
    for (unsigned int ii=0;ii<CFArrayGetCount(runs);ii++)
    {
        CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(runs,ii);
        CFRange range;
        range.length = 0;
        int num = CTRunGetGlyphCount(run);
        if (num > 0)
        {
            CGGlyph glyphs[num];
            CTRunGetGlyphs(run,range,glyphs);

            // Need the font manager for this run
            CFDictionaryRef attrs = CTRunGetAttributes(run);
            CTFontRef font = (CTFontRef)CFDictionaryGetValue(attrs,kCTFontAttributeName);
            FontManager dummyFM(font);
            FontManager *fm = NULL;
            FontManagerSet::iterator it = fontManagers.find(&dummyFM);
            if (it == fontManagers.end())
            {
                fm = new FontManager(font);
                fontManagers.insert(fm);
            } else {
                fm = *it;
            }
            
            // Work through the individual glyphs
            for (unsigned int jj=0;jj<num;jj++)
            {
                CGGlyph glyph = glyphs[jj];
                // Look for an existing one
                FontManager::GlyphInfo glyphInfo;
                if (!fm->findGlyph(glyph, glyphInfo))
                {
                    
                    // We need to render that Glyph and add it
                    CGSize size;
                    NSData *glyphImage = [self renderGlyph:glyph font:fm->font size:size];
                    if (glyphImage)
                    {
                        Texture *tex = new Texture("Font Texture Manager",glyphImage,false);
                        tex->setWidth(size.width);
                        tex->setHeight(size.height);
                        SubTexture subTex;
                        if (texAtlas->addTexture(tex, subTex, scene->getMemManager(), changes, 0))
                            fm->addGlyph(glyph, subTex);
                    }
                }
            }
        }
    }
    CFRelease(line);
    
    return nil;
}


@end
