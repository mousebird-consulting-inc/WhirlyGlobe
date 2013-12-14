/*
 *  FontTextureManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/15/13.
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

#import "FontTextureManager.h"
#import "UIImage+Stuff.h"
#import "UIColor+Stuff.h"
#import "WhirlyVector.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{
    
typedef std::set<CGGlyph> GlyphSet;

// We scale the fonts up so they look better sampled down.
static const float BogusFontScale = 2.0;
    
/// Manages the glyphs for a single font
class FontManager
{
public:
    FontManager(CTFontRef theFont) : font(theFont),refCount(0),color(nil),outlineColor(nil),outlineSize(0.0) { CFRetain(font); }
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
        CGPoint textureOffset;
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
    GlyphInfo *addGlyph(CGGlyph glyph,SubTexture subTex,CGSize size,CGPoint offset,CGPoint textureOffset)
    {
        GlyphInfo *info = new GlyphInfo(glyph);
        info->size = size;
        info->offset = offset;
        info->textureOffset = textureOffset;
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
                    delete glyphInfo;
                }
            }
        }        
    }

    int refCount;
    UIColor *color;
    NSString *fontName;
    UIColor *outlineColor;
    float outlineSize;
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
    pthread_mutex_t lock;
}

- (id)initWithScene:(WhirlyKit::Scene *)inScene
{
    self = [super init];
    if (!self)
        return nil;
    
    scene = inScene;
    pthread_mutex_init(&lock, NULL);
    
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
    pthread_mutex_destroy(&lock);
}
            
- (void)clear:(std::vector<WhirlyKit::ChangeRequest *> &)changes
{
    pthread_mutex_lock(&lock);
    
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
    
    pthread_mutex_unlock(&lock);
}


// Render the glyph into a buffer
- (NSData *)renderGlyph:(CGGlyph)glyph font:(FontManager *)fm texSize:(CGSize &)size glyphSize:(CGSize &)glyphSize offset:(CGPoint &)offset textureOffset:(CGPoint &)textureOffset
{
    int width,height;
    
    // Boundary around the image to capture the full data
    if (fm->outlineSize > 0.0)
    {
        int outlineUp = ceilf(fm->outlineSize);
        textureOffset = CGPointMake(1+outlineUp, 1+outlineUp);
    } else
        textureOffset = CGPointMake(1, 1);

    CGRect boundRect = CTFontGetBoundingRectsForGlyphs(fm->font,kCTFontDefaultOrientation,&glyph,NULL,1);
    size.width = ceilf(boundRect.size.width)+2*textureOffset.x;  size.height = ceilf(boundRect.size.height)+2*textureOffset.y;
    width = size.width;  height = size.height;

    if (width <= 0 || height <= 0)
        return nil;
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    // Set up a real context for the glyph rendering
    NSMutableData *retData = [NSMutableData dataWithLength:width*height*4];
    CGContextRef theContext = CGBitmapContextCreate((void *)[retData bytes], width, height, 8, width * 4, colorSpace, kCGImageAlphaPremultipliedLast);
    
    // Flip
    CGContextTranslateCTM(theContext, 0.0f, height);
    CGContextScaleCTM(theContext, 1.0f, -1.0f);
    // Now draw it
    int baselineOffX = (boundRect.origin.x < 0 ? floorf(boundRect.origin.x) : ceilf(boundRect.origin.x));
    int baselineOffY = (boundRect.origin.y < 0 ? floorf(boundRect.origin.y) : ceilf(boundRect.origin.y));
    
    offset = CGPointMake(baselineOffX,baselineOffY);
    glyphSize = boundRect.size;
    CGPoint pos = CGPointMake(-baselineOffX+textureOffset.x,-baselineOffY+textureOffset.y);

    // Generate an outline around the glyph
    if (fm->outlineSize > 0.0)
    {
        CGPathRef path = CTFontCreatePathForGlyph(fm->font, glyph, NULL);
        CGContextTranslateCTM(theContext, pos.x, pos.y);
        CGContextAddPath(theContext, path);
        CGContextSetLineWidth(theContext, 2.0*fm->outlineSize);
        CGContextSetStrokeColorWithColor(theContext, fm->outlineColor.CGColor);
        CGContextStrokePath(theContext);
        CGPathRelease(path);
        CGContextTranslateCTM(theContext, -pos.x, -pos.y);
    }

    UIColor *textColor = fm->color;
    if (textColor == nil)
        textColor = [UIColor whiteColor];
    CGContextSetFillColorWithColor(theContext, textColor.CGColor);
    CTFontDrawGlyphs(fm->font,&glyph,&pos,1,theContext);
    
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
            
// Look for an existing font that will match the UIFont given
- (FontManager *)findFontManagerForFont:(UIFont *)uiFont color:(UIColor *)color outlineColor:(UIColor *)outlineColor outlineSize:(NSNumber *)outlineSize
{
    NSString *fontName = uiFont.fontName;
    float pointSize = uiFont.pointSize;
    
    pointSize *= BogusFontScale;
    
    for (FontManagerSet::iterator it = fontManagers.begin();
         it != fontManagers.end(); ++it)
    {
        FontManager *fm = *it;
        if (![fontName compare:fm->fontName] && pointSize == fm->pointSize &&
            ((!fm->color && !color) ||
             ([fm->color asRGBAColor]) == [color asRGBAColor]) &&
            ((!fm->outlineColor && !outlineColor) ||
             ([fm->outlineColor asRGBAColor] == [outlineColor asRGBAColor])) &&
            (fm->outlineSize == [outlineSize floatValue]))
            return fm;
    }
    
    // Create it
    CTFontRef font = CTFontCreateWithName((__bridge CFStringRef)fontName, pointSize, NULL);
    FontManager *fm = new FontManager(font);
    fm->fontName = fontName;
    fm->color = color;
    fm->pointSize = pointSize;
    fm->outlineColor = outlineColor;
    fm->outlineSize = [outlineSize floatValue];
//    fm->outlineSize *= BogusFontScale;
    fontManagers.insert(fm);
    if (font)
        CFRelease(font);
    
    return fm;
}

- (WhirlyKit::DrawableString *)addString:(NSAttributedString *)str changes:(ChangeSet &)changes
{
    // We could make this more granular
    pthread_mutex_lock(&lock);

    if (!texAtlas)
    {
        // Let's do the biggest possible texture with small cells 32 bits deep
        texAtlas = new DynamicTextureAtlas(2048,16,GL_UNSIGNED_BYTE);
    }
    
    DrawableString *drawString = new DrawableString();
    
    // Convert to runs of glyphs
    CTLineRef line = CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)str);

    // Work through the runs (which share attributes)
    CFArrayRef runs = CTLineGetGlyphRuns(line);
    float lineHeight = 0.0, lineWidth = 0.0, ascent = 0.0, descent = 0.0;
    lineWidth = CTLineGetTypographicBounds(line,&ascent,&descent,NULL);
    lineHeight = ascent+descent;
    
    DrawStringRep *drawStringRep = new DrawStringRep(drawString->getId());
    
    drawString->mbr.reset();
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

            FontManager *fm = nil;
            if ([uiFont isKindOfClass:[UIFont class]])
                fm = [self findFontManagerForFont:uiFont color:foregroundColor outlineColor:outlineColor outlineSize:outlineSize];
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
                    CGSize texSize,glyphSize;
                    CGPoint offset,textureOffset;
                    NSData *glyphImage = [self renderGlyph:glyph font:fm texSize:texSize glyphSize:glyphSize offset:offset textureOffset:textureOffset];
                    if (glyphImage)
                    {
                        Texture *tex = new Texture("Font Texture Manager",glyphImage,false);
                        tex->setWidth(texSize.width);
                        tex->setHeight(texSize.height);
                        SubTexture subTex;
                        Point2f realSize(glyphSize.width+2*textureOffset.x,glyphSize.height+2*textureOffset.y);
                        std::vector<Texture *> texs;
                        texs.push_back(tex);
                        if (texAtlas->addTexture(texs, &realSize, NULL, subTex, scene->getMemManager(), changes, 0))
                            glyphInfo = fm->addGlyph(glyph, subTex, glyphSize, offset, textureOffset);
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
                    rect.pts[0] = Point2f(glyphInfo->offset.x*scale-glyphInfo->textureOffset.x*scale,glyphInfo->offset.y*scale-glyphInfo->textureOffset.y*scale)+Point2f(offset.x,offset.y);
                    rect.texCoords[0] = TexCoord(0.0,0.0);
                    // Note: was 2,2
                    rect.pts[1] = Point2f(glyphInfo->size.width*scale+2*glyphInfo->textureOffset.x*scale,glyphInfo->size.height*scale+2*glyphInfo->textureOffset.y*scale)+rect.pts[0];
                    rect.texCoords[1] = TexCoord(1.0,1.0);
                    
                    rect.subTex = glyphInfo->subTex;
                    drawString->glyphPolys.push_back(rect);
                    drawString->mbr.addPoint(rect.pts[0]);
                    drawString->mbr.addPoint(rect.pts[1]);
                    
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
            
- (void)removeString:(SimpleIdentity)drawStringId changes:(ChangeSet &)changes
{
    pthread_mutex_lock(&lock);
    
    DrawStringRep dummyRep(drawStringId);
    DrawStringRepSet::iterator it = drawStringReps.find(&dummyRep);
    if (it == drawStringReps.end())
    {
        pthread_mutex_unlock(&lock);
        return;
    }
    
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
    
    pthread_mutex_unlock(&lock);

    delete theRep;
}

@end
