/*  FontTextureManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/15/13.
 *  Copyright 2011-2021 mousebird consulting
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

#import "FontTextureManager.h"
#import "WhirlyVector.h"
#import "WhirlyKitLog.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{
    
FontManager::FontManager() :
    refCount(0),
    color(255,255,255,255),
    outlineColor(0,0,0,0),
    backColor(0,0,0,0),
    outlineSize(0.0)
{
}

FontManager::~FontManager()
{
    for (auto glyph : glyphs)
    {
        delete glyph;
    }
    glyphs.clear();
}

// Look for an existing glyph and return it if it's there
FontManager::GlyphInfo *FontManager::findGlyph(WKGlyph glyph)
{
    GlyphInfo dummyGlyph(glyph);
    const auto it = glyphs.find(&dummyGlyph);
    return (it != glyphs.end()) ? *it : nullptr;
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
    for (const auto &theGlyph : usedGlyphs)
    {
        GlyphInfo dummy(theGlyph);
        const auto git = glyphs.find(&dummy);
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
    for (const auto &theGlyph : usedGlyphs)
    {
        GlyphInfo dummy(theGlyph);
        const auto git = glyphs.find(&dummy);
        if (git != glyphs.end())
        {
            GlyphInfo *glyphInfo = *git;
            glyphInfo->refCount--;
            if (glyphInfo->refCount <= 0)
            {
//                wkLogLevel(Info,"Glyph removed: fm = %d, glyph = %d",(int)getId(),(int)theGlyph);

                if (toRemove.empty())
                {
                    toRemove.reserve(usedGlyphs.size());
                }
                toRemove.push_back(glyphInfo->subTex);
                glyphs.erase(git);
                delete glyphInfo;
            }
        }
    }
}

                
FontTextureManager::FontTextureManager(SceneRenderer *sceneRender,Scene *scene)
: sceneRender(sceneRender), scene(scene), texAtlas(nullptr)
{
}

FontTextureManager::~FontTextureManager()
{
    std::lock_guard<std::mutex> guardLock(lock);

    delete texAtlas;
    texAtlas = nullptr;
    for (auto drawStringRep : drawStringReps)
    {
        delete drawStringRep;
    }
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
        texAtlas = nullptr;
    }
    for (auto drawStringRep : drawStringReps)
    {
        delete drawStringRep;
    }
    fontManagers.clear();
}

void FontTextureManager::removeString(PlatformThreadInfo *inst, SimpleIdentity drawStringId,ChangeSet &changes,TimeInterval when)
{
    std::lock_guard<std::mutex> guardLock(lock);

    DrawStringRep *theRep = nullptr;
    {
        DrawStringRep dummyRep(drawStringId);
        auto it = drawStringReps.find(&dummyRep);
        if (it == drawStringReps.end())
        {
            return;
        }

        theRep = *it;
        drawStringReps.erase(it);
    }

    // Work through the fonts we're using
    for (const auto &fontGlyph : theRep->fontGlyphs)
    {
        const auto fmIt = fontManagers.find(fontGlyph.first);
        if (fmIt == fontManagers.end())
        {
            continue;
        }

        // Decrement the glyph references
        const FontManagerRef &fm = fmIt->second;
        std::vector<SubTexture> texRemove;
        fm->removeGlyphRefs(fontGlyph.second,texRemove);

        // And possibly remove some sub textures
        for (const auto &ii : texRemove)
        {
//            wkLogLevel(Info,"Texture removed for glyph");

            texAtlas->removeTexture(ii, changes, when);
        }

        // Also see if we're done with the font
        if (fm->refCount <= 0)
        {
//            wkLogLevel(Info,"Font removed: fm = %d,",(int)fm->getId());

            fm->teardown(inst);
            fontManagers.erase(fmIt);
        }
    }
    
    delete theRep;
}

}
