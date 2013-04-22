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
    
typedef std::set<CGGlyph> GlyphSet;
    
/// Manages the glyphs for a single font
class FontManager
{
public:
    FontManager(CTFontRef theFont) : font(theFont),refCount(0) { CFRetain(font); }
    ~FontManager()
    {
        CFRelease(font);
        for (std::set<GlyphInfo *,GlyphInfoSorter>::iterator it = glyphs.begin();
             it != glyphs.end(); ++it)
        {
            delete *it;
        }
        glyphs.clear();
    }
    
    CTFontRef font;

    // Mapping info from glyph to location in a dynamic texture
    class GlyphInfo
    {
    public:
        GlyphInfo() : glyph(0), refCount(0) { }
        GlyphInfo(CGGlyph glyph) : glyph(glyph), refCount(0) { }
        bool operator < (const GlyphInfo &that) const
        { return glyph < that.glyph; }
        CGGlyph glyph;
        CGSize size;
        CGPoint offset;
        SubTexture subTex;
        int refCount;
    };

    typedef struct
    {
        bool operator () (const GlyphInfo *a,const GlyphInfo *b) { return a->glyph < b->glyph; }
    } GlyphInfoSorter;

    bool empty() { return glyphs.empty(); }

    // Look for an existing glyph and return it if it's there
    GlyphInfo *findGlyph(CGGlyph glyph)
    {
        GlyphInfo dummyGlyph(glyph);
        GlyphInfoSet::iterator it = glyphs.find(&dummyGlyph);
        if (it != glyphs.end())
        {
            return *it;
        }
        
        return nil;
    }

    // Add the given glyph info
    GlyphInfo *addGlyph(CGGlyph glyph,SubTexture subTex,CGSize size,CGPoint offset)
    {
        GlyphInfo *info = new GlyphInfo(glyph);
        info->size = size;
        info->offset = offset;
        info->subTex = subTex;
        glyphs.insert(info);
        
        return info;
    }
            
    // Remove references to the given glyphs.
    void addGlyphRefs(const GlyphSet &usedGlyphs)
    {
        refCount++;
        for (GlyphSet::iterator it = usedGlyphs.begin();
             it != usedGlyphs.end(); ++it)
        {
            CGGlyph theGlyph = *it;
            GlyphInfo dummy(theGlyph);
            GlyphInfoSet::iterator git = glyphs.find(&dummy);
            if (git != glyphs.end())
            {
                GlyphInfo *glyphInfo = *git;
                glyphInfo->refCount++;
            }
        }
    }
            
    // Returns a list of texture references to remove
    void removeGlyphRefs(const GlyphSet &usedGlyphs,std::vector<SubTexture> &toRemove)
    {
        refCount--;
        for (GlyphSet::iterator it = usedGlyphs.begin();
             it != usedGlyphs.end(); ++it)
        {
            CGGlyph theGlyph = *it;
            GlyphInfo dummy(theGlyph);
            GlyphInfoSet::iterator git = glyphs.find(&dummy);
            if (git != glyphs.end())
            {
                GlyphInfo *glyphInfo = *git;
                glyphInfo->refCount--;
                if (glyphInfo->refCount <= 0)
                {
                    toRemove.push_back(glyphInfo->subTex);
                    glyphs.erase(git);
                }
            }
        }        
    }

    int refCount;
    NSString *fontName;
    float pointSize;
            
protected:
    // Maps Glyphs (shorts) to texture and region
    typedef std::set<GlyphInfo *,GlyphInfoSorter> GlyphInfoSet;
    GlyphInfoSet glyphs;
};

// Used to order a set of these
typedef struct
{
    bool operator () (const FontManager *a,const FontManager *b) { return a->font < b->font; }
} FontManagerSorter;

typedef std::set<FontManager *,FontManagerSorter> FontManagerSet;

}
            
typedef std::map<CTFontRef,GlyphSet> SimpleIDGlyphMap;

// Used to track the draw strings' representations in terms of fonts
//  and glyphs
class DrawStringRep : public Identifiable
{
public:
    DrawStringRep(SimpleIdentity theId) : Identifiable(theId) { }
    
    // Add the glyphs we're using
    void addGlyphs(CTFontRef font,const GlyphSet &newGlyphs)
    {
        SimpleIDGlyphMap::iterator it = fontGlyphs.find(font);
        GlyphSet existingGlyphs;
        if (it != fontGlyphs.end())
        {
            existingGlyphs = it->second;
        }
        existingGlyphs.insert(newGlyphs.begin(), newGlyphs.end());
        fontGlyphs[font] = existingGlyphs;
    }
    
    // The glyphs we're using in a given font
    SimpleIDGlyphMap fontGlyphs;
};
            
typedef std::set<DrawStringRep *,IdentifiableSorter> DrawStringRepSet;
            
@implementation WhirlyKitFontTextureManager
{
    Scene *scene;
    DynamicTextureAtlas *texAtlas;
    FontManagerSet fontManagers;
    DrawStringRepSet drawStringReps;
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
    for (DrawStringRepSet::iterator it = drawStringReps.begin();
         it != drawStringReps.end(); ++it)
        delete *it;
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
    for (DrawStringRepSet::iterator it = drawStringReps.begin();
         it != drawStringReps.end(); ++it)
        delete *it;
    for (FontManagerSet::iterator it = fontManagers.begin();
         it != fontManagers.end(); ++it)
        delete *it;
    fontManagers.clear();
}


// Render the glyph into a buffer
- (NSData *)renderGlyph:(CGGlyph)glyph font:(CTFontRef)font texSize:(CGSize &)size glyphSize:(CGSize &)glyphSize offset:(CGPoint &)offset
{
    int width,height;
        
    CGRect boundRect = CTFontGetBoundingRectsForGlyphs(font,kCTFontDefaultOrientation,&glyph,NULL,1);
    size.width = (int)(boundRect.size.width+0.5);  size.height = (int)(boundRect.size.height+0.5);
    width = size.width;  height = size.height;

    if (width <= 0 || height <= 0)
        return nil;
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    // Set up a real context for the glyph rendering
    NSMutableData *retData = [NSMutableData dataWithLength:width*height*4];
    CGContextRef theContext = CGBitmapContextCreate((void *)[retData bytes], width, height, 8, width * 4, colorSpace, kCGImageAlphaPremultipliedLast);
//    UIGraphicsBeginImageContext(size);
//    CGContextRef theContext = UIGraphicsGetCurrentContext();
    
//    CGContextSetFillColorWithColor(theContext, [UIColor whiteColor].CGColor);
//    CGContextFillRect(theContext, CGRectMake(0, 0, width, height));
    
    // Now draw it
    CGContextSetFillColorWithColor(theContext, [UIColor whiteColor].CGColor);
//    CGContextMoveToPoint(theContext, 0, 0);
//    CGContextAddPath(theContext, paths);
//    CGContextFillPath(theContext);

//    CGPathRelease(path);
//    CGPathRelease(paths);
    
    offset = boundRect.origin;
    glyphSize = boundRect.size;
    CGPoint pos = CGPointMake(-boundRect.origin.x,-boundRect.origin.y);
    CTFontDrawGlyphs(font,&glyph,&pos,1,theContext);
    
    // Note: Output Debugging
//    UIImage *theImage=UIGraphicsGetImageFromCurrentImageContext();
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
//    UIGraphicsEndImageContext();
    
    CGColorSpaceRelease(colorSpace);
    
    return retData;
}
            
// Look for an existing font that will match the UIFont given
- (FontManager *)findFontManagerForFont:(UIFont *)uiFont
{
    NSString *fontName = uiFont.fontName;
    float pointSize = uiFont.pointSize;
    
    for (FontManagerSet::iterator it = fontManagers.begin();
         it != fontManagers.end(); ++it)
    {
        FontManager *fm = *it;
        if (![fontName compare:fm->fontName] && pointSize == fm->pointSize)
            return fm;
    }
    
    // Create it
    CTFontRef font = CTFontCreateWithName((__bridge CFStringRef)fontName, pointSize, NULL);
    FontManager *fm = new FontManager(font);
    fm->fontName = fontName;
    fm->pointSize = pointSize;
    fontManagers.insert(fm);
    if (font)
        CFRelease(font);
    
    return fm;
}

- (WhirlyKit::DrawableString *)addString:(NSAttributedString *)str changes:(std::vector<ChangeRequest *> &)changes
{
    if (!texAtlas)
        // Let's do the biggest possible texture with small cells 24 bits deep
        texAtlas = new DynamicTextureAtlas(2048,16,GL_UNSIGNED_BYTE);
    
    DrawableString *drawString = new DrawableString();
    
    // Convert to runs of glyphs
    CTLineRef line = CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)str);

    // Work through the runs (which share attributes)
    CFArrayRef runs = CTLineGetGlyphRuns(line);
    float lineHeight = 0.0, lineWidth = 0.0, ascent = 0.0, descent = 0.0;
    lineWidth = CTLineGetTypographicBounds(line,&ascent,&descent,NULL);
    lineHeight = ascent+descent;
    
    DrawStringRep *drawStringRep = new DrawStringRep(drawString->getId());
    
    for (unsigned int ii=0;ii<CFArrayGetCount(runs);ii++)
    {
        CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(runs,ii);
        int num = CTRunGetGlyphCount(run);
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
            CFDictionaryRef attrs = CTRunGetAttributes(run);
            UIFont *uiFont = (UIFont *)CFDictionaryGetValue(attrs,kCTFontAttributeName);
            FontManager *fm = nil;
            if ([uiFont isKindOfClass:[UIFont class]])
                fm = [self findFontManagerForFont:uiFont];
            
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
                    CGSize texSize,glyphSize;
                    CGPoint offset;
                    NSData *glyphImage = [self renderGlyph:glyph font:fm->font texSize:texSize glyphSize:glyphSize offset:offset];
                    if (glyphImage)
                    {
                        Texture *tex = new Texture("Font Texture Manager",glyphImage,false);
                        tex->setWidth(texSize.width);
                        tex->setHeight(texSize.height);
                        SubTexture subTex;
                        Point2f realSize(glyphSize.width,glyphSize.height);
                        if (texAtlas->addTexture(tex, &realSize, subTex, scene->getMemManager(), changes, 0))
                            glyphInfo = fm->addGlyph(glyph, subTex, glyphSize, offset);
                    }
                }
                
                if (glyphInfo)
                {
                    // Now we make a rectangle that covers the glyph in its texture atlas
                    DrawableString::Rect rect;
                    CGPoint &offset = offsets[jj];
                    rect.pts[0] = Point2f(glyphInfo->offset.x,glyphInfo->offset.y)+Point2f(offset.x,offset.y);
                    rect.texCoords[0] = TexCoord(0.0,1.0);
                    rect.pts[1] = Point2f(glyphInfo->size.width,glyphInfo->size.height)+rect.pts[0];
                    rect.texCoords[1] = TexCoord(1.0,0.0);
                    rect.subTex = glyphInfo->subTex;
                    drawString->glyphPolys.push_back(rect);
                    
                    glyphsUsed.insert(glyphInfo->glyph);
                }
            }

            // Keep track of the glyphs we're using
            drawStringRep->addGlyphs(fm->font,glyphsUsed);
            fm->addGlyphRefs(glyphsUsed);
            
            free(glyphs);
            free(offsets);
        }
    }
    
    // Need the extents for the whole line
    drawString->mbr.ll() = Point2f(0,-descent);
    drawString->mbr.ur() = Point2f(lineWidth,ascent);
    
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
    
    return drawString;
}
            
- (void)removeString:(SimpleIdentity)drawStringId changes:(std::vector<ChangeRequest *> &)changes
{
    DrawStringRep dummyRep(drawStringId);
    DrawStringRepSet::iterator it = drawStringReps.find(&dummyRep);
    if (it == drawStringReps.end())
        return;
    
    DrawStringRep *theRep = *it;
    drawStringReps.erase(theRep);
    
    // Work through the fonts we're using
    for (SimpleIDGlyphMap::iterator fit = theRep->fontGlyphs.begin();
         fit != theRep->fontGlyphs.end(); ++fit)
    {
        FontManager dummyFm(fit->first);
        FontManagerSet::iterator fmIt = fontManagers.find(&dummyFm);
        if (fmIt != fontManagers.end())
        {
            // Decrement the glyph references
            FontManager *fm = *fmIt;
            std::vector<SubTexture> texRemove;
            fm->removeGlyphRefs(fit->second,texRemove);

            // And possibly remove some sub textures
            if (!texRemove.empty())
                for (unsigned int ii=0;ii<texRemove.size();ii++)
                    texAtlas->removeTexture(texRemove[ii], changes);
            
            // Also see if we're done with the font
            if (fm->refCount <= 0)
            {
                fontManagers.erase(fmIt);
                delete fm;
            }
        }
    }
    
    delete theRep;
}

@end
