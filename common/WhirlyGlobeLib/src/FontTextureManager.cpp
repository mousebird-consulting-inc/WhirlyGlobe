/*
 *  FontTextureManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/15/13.
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

#import "FontTextureManager.h"
#import "WhirlyVector.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{
    
FontManager::FontManager()
: refCount(0),color(255,255,255,255),outlineColor(0,0,0,0),backColor(0,0,0,0),outlineSize(0.0)
{
}

FontManager::~FontManager()
{
    for (std::set<GlyphInfo *,GlyphInfoSorter>::iterator it = glyphs.begin();
         it != glyphs.end(); ++it)
    {
        delete *it;
    }
    glyphs.clear();
}

// Comparison operator
// Subclass fills this in
bool FontManager::operator < (const FontManager &that) const
{
    return false;
}
    
// Look for an existing glyph and return it if it's there
FontManager::GlyphInfo *FontManager::findGlyph(WKGlyph glyph)
{
    GlyphInfo dummyGlyph(glyph);
    GlyphInfoSet::iterator it = glyphs.find(&dummyGlyph);
    if (it != glyphs.end())
    {
        return *it;
    }
    
    return NULL;
}

// Add the given glyph info
FontManager::GlyphInfo *FontManager::addGlyph(WKGlyph glyph,SubTexture subTex,const Point2f &size,const Point2f &offset,const Point2f &textureOffset)
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
void FontManager::addGlyphRefs(const GlyphSet &usedGlyphs)
{
    refCount++;
    for (GlyphSet::iterator it = usedGlyphs.begin();
         it != usedGlyphs.end(); ++it)
    {
        WKGlyph theGlyph = *it;
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
void FontManager::removeGlyphRefs(const GlyphSet &usedGlyphs,std::vector<SubTexture> &toRemove)
{
    refCount--;
    for (GlyphSet::iterator it = usedGlyphs.begin();
         it != usedGlyphs.end(); ++it)
    {
        WKGlyph theGlyph = *it;
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

                
FontTextureManager::FontTextureManager(SceneRenderer *sceneRender,Scene *scene)
: sceneRender(sceneRender), scene(scene), texAtlas(NULL)
{
}

FontTextureManager::~FontTextureManager()
{
    if (texAtlas)
        delete texAtlas;
    texAtlas = NULL;
    for (DrawStringRepSet::iterator it = drawStringReps.begin();
         it != drawStringReps.end(); ++it)
        delete *it;
    drawStringReps.clear();
    fontManagers.clear();
}
    
void FontTextureManager::init()
{
    if (!texAtlas)
    {
        // Let's do the biggest possible texture with small cells 32 bits deep
        // Note: Porting.  We've turned main thread merge on here, which shouldn't be needed.
        //       If we leave it off, we get corruption of the dynamic textures
        texAtlas = new DynamicTextureAtlas("Font Texture Atlas",2048,16,TexTypeUnsignedByte,1,true);
    }
}
            
void FontTextureManager::clear(ChangeSet &changes)
{
    std::lock_guard<std::mutex> guardLock(lock);
    
    if (texAtlas)
    {
        texAtlas->teardown(changes);
        delete texAtlas;
        texAtlas = NULL;
    }
    for (DrawStringRepSet::iterator it = drawStringReps.begin();
         it != drawStringReps.end(); ++it)
        delete *it;
    fontManagers.clear();
}

void FontTextureManager::removeString(SimpleIdentity drawStringId,ChangeSet &changes,TimeInterval when)
{
    std::lock_guard<std::mutex> guardLock(lock);

    DrawStringRep dummyRep(drawStringId);
    DrawStringRepSet::iterator it = drawStringReps.find(&dummyRep);
    if (it == drawStringReps.end())
    {
        return;
    }
    
    DrawStringRep *theRep = *it;
    drawStringReps.erase(theRep);
    
    // Work through the fonts we're using
    for (SimpleIDGlyphMap::iterator fit = theRep->fontGlyphs.begin();
         fit != theRep->fontGlyphs.end(); ++fit)
    {
        auto fmIt = fontManagers.find(fit->first);
        if (fmIt != fontManagers.end())
        {
            // Decrement the glyph references
            FontManagerRef fm = fmIt->second;
            std::vector<SubTexture> texRemove;
            fm->removeGlyphRefs(fit->second,texRemove);

            // And possibly remove some sub textures
            if (!texRemove.empty())
                for (unsigned int ii=0;ii<texRemove.size();ii++)
                    texAtlas->removeTexture(texRemove[ii], changes, when);

            // Also see if we're done with the font
            if (fm->refCount <= 0)
            {
                fontManagers.erase(fmIt);
            }
        }
    }
    
    delete theRep;
}

}
