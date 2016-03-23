/*
 *  FontTextureManagerAndroid.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
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

#import "FontTextureManagerAndroid.h"
#import "LabelInfoAndroid.h"
#import <android/bitmap.h>

namespace WhirlyKit
{

static const float BogusFontScale = 2.0;

FontTextureManagerAndroid::FontManagerAndroid::FontManagerAndroid(JNIEnv *env,jobject inTypefaceObj)
{
	typefaceObj = env->NewGlobalRef(inTypefaceObj);
}

FontTextureManagerAndroid::FontManagerAndroid::FontManagerAndroid()
: typefaceObj(NULL)
{
}

void FontTextureManagerAndroid::FontManagerAndroid::clearRefs(JNIEnv *savedEnv)
{
	if (typefaceObj)
	{
		savedEnv->DeleteGlobalRef(typefaceObj);
		typefaceObj = NULL;
	}
}

FontTextureManagerAndroid::FontManagerAndroid::~FontManagerAndroid()
{
}

FontTextureManagerAndroid::FontTextureManagerAndroid(JNIEnv *env,Scene *scene,jobject inCharRenderObj)
	: FontTextureManager(scene), charRenderObj(NULL)
{
	// Note: Porting.  This will leak
	charRenderObj = env->NewGlobalRef(inCharRenderObj);
	jclass charRenderClass =  env->GetObjectClass(charRenderObj);
	renderMethodID = env->GetMethodID(charRenderClass, "renderChar", "(ILcom/mousebird/maply/LabelInfo;F)Lcom/mousebird/maply/CharRenderer$Glyph;");
	jclass glyphClass = env->FindClass("com/mousebird/maply/CharRenderer$Glyph");
	bitmapID = env->GetFieldID(glyphClass,"bitmap","Landroid/graphics/Bitmap;");
	sizeXID = env->GetFieldID(glyphClass,"sizeX","F");
	sizeYID = env->GetFieldID(glyphClass,"sizeY","F");
	glyphSizeXID = env->GetFieldID(glyphClass,"glyphSizeX","F");
	glyphSizeYID = env->GetFieldID(glyphClass,"glyphSizeY","F");
	offsetXID = env->GetFieldID(glyphClass,"offsetX","F");
	offsetYID = env->GetFieldID(glyphClass,"offsetY","F");
	textureOffsetXID = env->GetFieldID(glyphClass,"textureOffsetX","F");
	textureOffsetYID = env->GetFieldID(glyphClass,"textureOffsetY","F");
    env->DeleteLocalRef(glyphClass);
    env->DeleteLocalRef(charRenderClass);
}

FontTextureManagerAndroid::~FontTextureManagerAndroid()
{
    for (FontManagerSet::iterator it = fontManagers.begin();
         it != fontManagers.end(); ++it)
        delete *it;
    fontManagers.clear();
}

DrawableString *FontTextureManagerAndroid::addString(JNIEnv *env,const std::vector<int> &codePoints,jobject labelInfoObj,ChangeSet &changes)
{
	LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
	LabelInfoAndroid *labelInfo = (LabelInfoAndroid *)classInfo->getObject(env,labelInfoObj);

	// Could be more granular if this slows things down
    pthread_mutex_lock(&lock);

    // If not initialized, set up texture atlas and such
    init();

    DrawableString *drawString = new DrawableString();
    DrawStringRep *drawStringRep = new DrawStringRep(drawString->getId());

    // Look for the font manager that manages the typeface/attribute combo we need
    FontManagerAndroid *fm = findFontManagerForFont(labelInfo->typefaceObj,*labelInfo);

	JavaIntegerClassInfo *intClassInfo = JavaIntegerClassInfo::getClassInfo(env);

    // Work through the characters
    GlyphSet glyphsUsed;
    float offsetX = 0.0;
    for (int glyph : codePoints)
    {
    	// Look for an existing glyph
    	FontManager::GlyphInfo *glyphInfo = fm->findGlyph(glyph);
    	if (!glyphInfo)
    	{
        	// Call the renderer
        	jobject glyphObj = env->CallObjectMethod(charRenderObj,renderMethodID,glyph,labelInfoObj,labelInfo->fontSize);
        	jobject bitmapObj = env->GetObjectField(glyphObj,bitmapID);

        	try
        	{
				// Got a bitmap, so merge that in with our texture atlas
				AndroidBitmapInfo info;
				if (bitmapObj && (AndroidBitmap_getInfo(env, bitmapObj, &info) >= 0))
				{
                    Point2f texSize,glyphSize;
                    Point2f offset,textureOffset;

                    // Pull these values from the glyph
                    texSize.x() = env->GetFloatField(glyphObj,sizeXID);
                    texSize.y() = env->GetFloatField(glyphObj,sizeYID);
                    glyphSize.x() = env->GetFloatField(glyphObj,glyphSizeXID);
                    glyphSize.y() = env->GetFloatField(glyphObj,glyphSizeYID);
                    offset.x() = env->GetFloatField(glyphObj,offsetXID);
                    offset.y() = env->GetFloatField(glyphObj,offsetYID);
                    textureOffset.x() = env->GetFloatField(glyphObj,textureOffsetXID);
                    textureOffset.y() = env->GetFloatField(glyphObj,textureOffsetYID);

                    // Create a texture
					void* bitmapPixels;
					if (AndroidBitmap_lockPixels(env, bitmapObj, &bitmapPixels) < 0)
						throw 1;
					uint32_t* src = (uint32_t*) bitmapPixels;
					int testVal = src[20];
					MutableRawData *rawData = new MutableRawData(bitmapPixels,info.height*info.width*4);
					Texture tex("FontTextureManager");
					tex.setRawData(rawData,info.width,info.height);

					// Add it to the texture atlas
                    SubTexture subTex;
                    Point2f realSize(glyphSize.x()+2*textureOffset.x(),glyphSize.y()+2*textureOffset.y());
                    std::vector<Texture *> texs;
                    texs.push_back(&tex);
                    if (texAtlas->addTexture(texs, -1, &realSize, NULL, subTex, scene->getMemManager(), changes, 0, 0, NULL))
                        glyphInfo = fm->addGlyph(glyph, subTex, Point2f(glyphSize.x(),glyphSize.y()), Point2f(offset.x(),offset.y()), Point2f(textureOffset.x(),textureOffset.y()));
                    
                    AndroidBitmap_unlockPixels(env, bitmapObj);
				}
        	}
        	catch (...)
        	{
        		// Just don't add the glyph, for now
        	}

        	env->DeleteLocalRef(glyphObj);
    	}

        if (glyphInfo)
        {
            // Now we make a rectangle that covers the glyph in its texture atlas
            DrawableString::Rect rect;
            Point2f offset(offsetX,0.0);

            float scale = 1.0/BogusFontScale;

            // Note: was -1,-1
            rect.pts[0] = Point2f(glyphInfo->offset.x()*scale-glyphInfo->textureOffset.x()*scale,glyphInfo->offset.y()*scale-glyphInfo->textureOffset.y()*scale)+offset;
            rect.texCoords[0] = TexCoord(0.0,1.0);
            // Note: was 2,2
            rect.pts[1] = Point2f(glyphInfo->size.x()*scale+2*glyphInfo->textureOffset.x()*scale,glyphInfo->size.y()*scale+2*glyphInfo->textureOffset.y()*scale)+rect.pts[0];
            rect.texCoords[1] = TexCoord(1.0,0.0);

            rect.subTex = glyphInfo->subTex;
            drawString->glyphPolys.push_back(rect);
            drawString->mbr.addPoint(rect.pts[0]);
            drawString->mbr.addPoint(rect.pts[1]);

            glyphsUsed.insert(glyphInfo->glyph);

            offsetX += rect.pts[1].x()-rect.pts[0].x();
        }
    }

    drawStringRep->addGlyphs(fm->getId(),glyphsUsed);
    fm->addGlyphRefs(glyphsUsed);

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

FontTextureManagerAndroid::FontManagerAndroid *FontTextureManagerAndroid::findFontManagerForFont(jobject typefaceObj,const LabelInfo &inLabelInfo)
{
	const LabelInfoAndroid &labelInfo = (LabelInfoAndroid &)inLabelInfo;

	for (FontManagerSet::iterator it = fontManagers.begin();
			it != fontManagers.end(); ++it)
	{
		FontManagerAndroid *fm = (FontManagerAndroid *)*it;

		if (labelInfo.typefaceIsSame(fm->typefaceObj) &&
				fm->color == labelInfo.textColor &&
				fm->outlineColor == labelInfo.outlineColor &&
				fm->outlineSize == labelInfo.outlineSize)
			return fm;
	}

	// Didn't find it, so create it
	FontManagerAndroid *fm = new FontManagerAndroid(labelInfo.env,typefaceObj);
	fm->fontName = "";
	fm->color = labelInfo.textColor;
	fm->pointSize = labelInfo.height;
	fm->outlineColor = labelInfo.outlineColor;
	fm->outlineSize = labelInfo.outlineSize;
	fontManagers.insert(fm);

	return fm;
}

}
