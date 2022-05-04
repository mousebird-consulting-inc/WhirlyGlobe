/*  FontTextureManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/15/13.
 *  Copyright 2011-2022 mousebird consulting
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
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    FontManager() = default;
    FontManager(SimpleIdentity theId) : Identifiable(theId) { }
    virtual ~FontManager();
    
    // Comparison operator
    // Subclass fills this in
    virtual bool operator < (const FontManager &that) const = 0;

    virtual void teardown(PlatformThreadInfo *) { }

    // Mapping info from glyph to location in a dynamic texture
    struct GlyphInfo
    {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

        GlyphInfo() = default;
        GlyphInfo(WKGlyph glyph) : glyph(glyph) { }
        bool operator < (const GlyphInfo &that) const { return glyph < that.glyph; }

        WKGlyph glyph = 0;
        Point2f size = {0.0f, 0.0f};
        Point2f offset = {0.0f, 0.0f};
        Point2f textureOffset = {0.0f, 0.0f};
        SubTexture subTex = 0;
        int refCount = 0;
        float baseline = 0.0f;
    };
    
    typedef struct GlyphInfoSorter
    {
        bool operator () (const GlyphInfo *a,const GlyphInfo *b) const { return a->glyph < b->glyph; }
    } GlyphInfoSorter;
    
    bool empty() const { return glyphs.empty(); }
    
    // Look for an existing glyph and return it if it's there
    GlyphInfo *findGlyph(WKGlyph glyph);
    
    // Add the given glyph info
    GlyphInfo *addGlyph(WKGlyph glyph,const SubTexture &subTex,
                        const Point2f &size,const Point2f &offset,
                        const Point2f &textureOffset);
    
    // Remove references to the given glyphs.
    void addGlyphRefs(const GlyphSet &usedGlyphs);
    
    // Returns a list of texture references to remove
    void removeGlyphRefs(const GlyphSet &usedGlyphs,std::vector<SubTexture> &toRemove);
    
    int refCount = 0;
    RGBAColor color = RGBAColor::white();
    RGBAColor backColor = RGBAColor::black();
    RGBAColor outlineColor = RGBAColor::black();
    std::string fontName;
    float outlineSize = 0.0f;
    float pointSize = 0.0f;

protected:
    // Maps Glyphs (shorts) to texture and region
    typedef std::set<GlyphInfo *,GlyphInfoSorter> GlyphInfoSet;
    GlyphInfoSet glyphs;
};

typedef std::shared_ptr<FontManager> FontManagerRef;
typedef std::map<SimpleIdentity,GlyphSet> SimpleIDGlyphMap;

/** Information sufficient to draw a string as 3D geometry.
    All coordinates are in a local space related to the font size.
 */
struct DrawableString : public Identifiable
{
    /// A rectangle describing the placement of a single glyph and
    ///  the texture piece used to represent it
    struct Rect
    {
        Point2f pts[2];
        TexCoord texCoords[2];
        SubTexture subTex = 0;
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
    FontTextureManager(SceneRenderer *sceneRender,Scene *scene);
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
    typedef std::map<SimpleIdentity,FontManagerRef> FontManagerMap;
    
    typedef std::set<DrawStringRep *,IdentifiableSorter> DrawStringRepSet;

    /// Remove resources associated with the given string
    virtual void removeString(PlatformThreadInfo *,SimpleIdentity drawStringId,ChangeSet &changes,TimeInterval when);
    
    // Tear down everything we've built
    void clear(ChangeSet &changes);

    virtual void teardown(PlatformThreadInfo*) = 0;

protected:    
    void init();
    void clearNoLock(ChangeSet &changes);

    FontManagerMap fontManagers;

    SceneRenderer *sceneRender = nullptr;
    Scene *scene = nullptr;
    DynamicTextureAtlas *texAtlas = nullptr;
    DrawStringRepSet drawStringReps;
    std::mutex lock;    
};
    
typedef std::shared_ptr<FontTextureManager> FontTextureManagerRef;
    
}
