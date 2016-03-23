/*
 *  FontTextureManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/15/13.
 *  Copyright 2011-2016 mousebird consulting
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
#import <math.h>
#import <set>
#import <map>
#import "Identifiable.h"
#import "BasicDrawable.h"
#import "TextureAtlas.h"
#import "DynamicTextureAtlas.h"

namespace WhirlyKit
{
    
/// Defines the outline size (if present) of an NSAttributedString
#define kOutlineAttributeSize @"MaplyOutlineAttributeSize"
/// Defines the outline color of an NSAttributedString
#define kOutlineAttributeColor @"MaplyOutlineAttributeColor"

// This is sufficient for unicode
typedef uint32_t WKGlyph;

typedef std::set<WKGlyph> GlyphSet;

/// Manages the glyphs for a single font
class FontManager : public Identifiable
{
public:
    FontManager(SimpleIdentity theId) : Identifiable(theId) { }
    FontManager();
    virtual ~FontManager();
    
    // Comparison operator
    // Subclass fills this in
    virtual bool operator < (const FontManager &that) const;
    
    // Mapping info from glyph to location in a dynamic texture
    class GlyphInfo
    {
    public:
        GlyphInfo() : glyph(0), refCount(0) { }
        GlyphInfo(WKGlyph glyph) : glyph(glyph), refCount(0) { }
        bool operator < (const GlyphInfo &that) const
        { return glyph < that.glyph; }
        WKGlyph glyph;
        Point2f size;
        Point2f offset;
        Point2f textureOffset;
        SubTexture subTex;
        int refCount;
    };
    
    typedef struct
    {
        bool operator () (const GlyphInfo *a,const GlyphInfo *b) { return a->glyph < b->glyph; }
    } GlyphInfoSorter;
    
    bool empty() { return glyphs.empty(); }
    
    // Look for an existing glyph and return it if it's there
    GlyphInfo *findGlyph(WKGlyph glyph);
    
    // Add the given glyph info
    GlyphInfo *addGlyph(WKGlyph glyph,SubTexture subTex,const Point2f &size,const Point2f &offset,const Point2f &textureOffset);
    
    // Remove references to the given glyphs.
    void addGlyphRefs(const GlyphSet &usedGlyphs);
    
    // Returns a list of texture references to remove
    void removeGlyphRefs(const GlyphSet &usedGlyphs,std::vector<SubTexture> &toRemove);
    
    int refCount;
    RGBAColor color;
    std::string fontName;
    RGBAColor outlineColor;
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
    bool operator () (const FontManager *a,const FontManager *b) { return *a < *b; }
} FontManagerSorter;

typedef std::map<SimpleIdentity,GlyphSet> SimpleIDGlyphMap;

/** Information sufficient to draw a string as 3D geometry.
    All coordinates are in a local space related to the font size.
 */
class DrawableString : public Identifiable
{
public:
    DrawableString() { }
    
    /// A rectangle describing the placement of a single glyph and
    ///  the texture piece used to represent it
    class Rect
    {
    public:
        Point2f pts[2];
        TexCoord texCoords[2];
        SubTexture subTex;
    };
    std::vector<Rect> glyphPolys;
    
    /// Bounding box of the string in coordinates related to the font size
    Mbr mbr;
};

/** Used to manage a dynamic texture set containing glyphs from
    various fonts.
  */
class FontTextureManager
{
public:
    // Construct with a scene
    FontTextureManager(Scene *scene);
    virtual ~FontTextureManager();
    
    // Used to track the draw strings' representations in terms of fonts
    //  and glyphs
    class DrawStringRep : public Identifiable
    {
    public:
        DrawStringRep(SimpleIdentity theId) : Identifiable(theId) { }
        
        // Add the glyphs we're using
        void addGlyphs(SimpleIdentity fontId,const GlyphSet &newGlyphs)
        {
            SimpleIDGlyphMap::iterator it = fontGlyphs.find(fontId);
            GlyphSet existingGlyphs;
            if (it != fontGlyphs.end())
            {
                existingGlyphs = it->second;
            }
            existingGlyphs.insert(newGlyphs.begin(), newGlyphs.end());
            fontGlyphs[fontId] = existingGlyphs;
        }
        
        // The glyphs we're using in a given font
        SimpleIDGlyphMap fontGlyphs;
    };
    typedef std::set<FontManager *,IdentifiableSorter> FontManagerSet;
    
    typedef std::set<DrawStringRep *,IdentifiableSorter> DrawStringRepSet;

    /// Remove resources associated with the given string
    void removeString(SimpleIdentity drawStringId,ChangeSet &changes);
    
    // Tear down everything we've built
    void clear(ChangeSet &changes);
    
protected:    
    void init();

    FontManagerSet fontManagers;

    Scene *scene;
    DynamicTextureAtlas *texAtlas;
    DrawStringRepSet drawStringReps;
    pthread_mutex_t lock;    
};
    
}
