/*  FontTextureManagerAndroid.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
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

#import "FontTextureManager_Android.h"
#import "LabelInfo_Android.h"
#import "LabelsAndMarkers_jni.h"
#import "ScopedEnv_Android.h"
#import <android/bitmap.h>

namespace WhirlyKit
{

static const float BogusFontScale = 1.0f;

FontTextureManager_Android::FontManager_Android::FontManager_Android(JNIEnv *env,jobject inTypefaceObj) :
	jvm(nullptr),
	typefaceObj(env->NewGlobalRef(inTypefaceObj))
{
	env->GetJavaVM(&jvm);
}

FontTextureManager_Android::FontManager_Android::FontManager_Android() :
	jvm(nullptr),
	typefaceObj(nullptr)
{
}

void FontTextureManager_Android::FontManager_Android::clearRefs(JNIEnv* env)
{
	if (env && typefaceObj)
	{
		env->DeleteGlobalRef(typefaceObj);
		typefaceObj = nullptr;
	}
}

FontTextureManager_Android::FontManager_Android::~FontManager_Android()
{
	// should have been cleaned up by now
	assert(typefaceObj == nullptr);
	clearRefs(nullptr);
}

void FontTextureManager_Android::FontManager_Android::deinit(PlatformThreadInfo* platformInfo)
{
	if (typefaceObj)
	{
		if (auto pi = (PlatformInfo_Android*)platformInfo)
		{
			clearRefs(pi->env);
		}
		else if (auto env = ScopedEnv(jvm))
		{
			clearRefs(env);
		}
	}
}

FontTextureManager_Android::FontTextureManager_Android(JNIEnv *env,SceneRenderer *sceneRender,Scene *scene,jobject inCharRenderObj) :
	FontTextureManager(sceneRender,scene),
	charRenderObj(env->NewGlobalRef(inCharRenderObj)),
	env(env)
{
	if (const jclass charRenderClass = env->GetObjectClass(charRenderObj))
	{
		renderMethodID = env->GetMethodID(charRenderClass, "renderChar",
										  "(ILcom/mousebird/maply/LabelInfo;F)Lcom/mousebird/maply/CharRenderer$Glyph;");
		env->DeleteLocalRef(charRenderClass);
	}

	if (const jclass glyphClass = env->FindClass("com/mousebird/maply/CharRenderer$Glyph"))
	{
		bitmapID = env->GetFieldID(glyphClass, "bitmap", "Landroid/graphics/Bitmap;");
		sizeXID = env->GetFieldID(glyphClass, "sizeX", "F");
		sizeYID = env->GetFieldID(glyphClass, "sizeY", "F");
		glyphSizeXID = env->GetFieldID(glyphClass, "glyphSizeX", "F");
		glyphSizeYID = env->GetFieldID(glyphClass, "glyphSizeY", "F");
		offsetXID = env->GetFieldID(glyphClass, "offsetX", "F");
		offsetYID = env->GetFieldID(glyphClass, "offsetY", "F");
		textureOffsetXID = env->GetFieldID(glyphClass, "textureOffsetX", "F");
		textureOffsetYID = env->GetFieldID(glyphClass, "textureOffsetY", "F");
		env->DeleteLocalRef(glyphClass);
	}
}

FontTextureManager_Android::~FontTextureManager_Android()
{
	if (charRenderObj)
	{
		env->DeleteGlobalRef(charRenderObj);
	}
}

DrawableString *FontTextureManager_Android::addString(
		PlatformThreadInfo *inThreadInfo,
		const std::vector<int> &codePoints,
		const LabelInfoAndroid *labelInfo,
		ChangeSet &changes)
{
	auto threadInfo = (PlatformInfo_Android *)inThreadInfo;
	auto classInfo = LabelInfoClassInfo::getClassInfo();
	auto intClassInfo = JavaIntegerClassInfo::getClassInfo(threadInfo->env);

	// Could be more granular if this slows things down
    std::lock_guard<std::mutex> guardLock(lock);

    // If not initialized, set up texture atlas and such
    init();

    auto drawString = new DrawableString();
    auto drawStringRep = new DrawStringRep(drawString->getId());

    // Look for the font manager that manages the typeface/attribute combo we need
    auto fm = findFontManagerForFont(threadInfo,labelInfo->typefaceObj,*labelInfo);

    // Work through the characters
    GlyphSet glyphsUsed;
    float offsetX = 0.0;
    for (int glyph : codePoints)
    {
		// Look for an existing glyph
    	auto glyphInfo = fm->findGlyph(glyph);
    	if (!glyphInfo)
    	{
        	// Call the renderer
        	const jobject glyphObj = threadInfo->env->CallObjectMethod(charRenderObj,renderMethodID,glyph,labelInfo->labelInfoObj,labelInfo->fontSize);
        	if (!glyphObj)
        	{
        		wkLogLevel(Warn,"Bad glyph passed into FontTextureManager_Android: %d",glyph);
				continue;
			}
        	jobject bitmapObj = threadInfo->env->GetObjectField(glyphObj,bitmapID);

        	try
        	{
				// Got a bitmap, so merge that in with our texture atlas
				AndroidBitmapInfo info;
				if (bitmapObj && (AndroidBitmap_getInfo(threadInfo->env, bitmapObj, &info) >= 0))
				{
                    Point2f texSize,glyphSize;
                    Point2f offset,textureOffset;

                    // Pull these values from the glyph
                    texSize.x() = threadInfo->env->GetFloatField(glyphObj,sizeXID);
                    texSize.y() = threadInfo->env->GetFloatField(glyphObj,sizeYID);
                    glyphSize.x() = threadInfo->env->GetFloatField(glyphObj,glyphSizeXID);
                    glyphSize.y() = threadInfo->env->GetFloatField(glyphObj,glyphSizeYID);
                    offset.x() = threadInfo->env->GetFloatField(glyphObj,offsetXID);
                    offset.y() = threadInfo->env->GetFloatField(glyphObj,offsetYID);
                    textureOffset.x() = threadInfo->env->GetFloatField(glyphObj,textureOffsetXID);
                    textureOffset.y() = threadInfo->env->GetFloatField(glyphObj,textureOffsetYID);

                    // Create a texture
					void* bitmapPixels;
					if (AndroidBitmap_lockPixels(threadInfo->env, bitmapObj, &bitmapPixels) < 0)
					{
						throw std::runtime_error("Unable to lock bitmap pixels");
					}
					try
					{
						MutableRawData *rawData = new MutableRawData(bitmapPixels,
																	 info.height * info.width * 4);
						TextureGLES tex("FontTextureManager");
						tex.setRawData(rawData, info.width, info.height);

						// Add it to the texture atlas
						SubTexture subTex;
						Point2f realSize(glyphSize.x() + 2 * textureOffset.x(),
										 glyphSize.y() + 2 * textureOffset.y());
						std::vector<Texture *> texs{&tex};
						if (texAtlas->addTexture(sceneRender, texs, -1, &realSize, NULL, subTex,
												 changes, 0, 0, NULL)) {
							glyphInfo = fm->addGlyph(glyph, subTex,
													 Point2f(glyphSize.x(), glyphSize.y()),
													 Point2f(offset.x(), offset.y()),
													 Point2f(textureOffset.x(), textureOffset.y()));
						}
					}
					catch (...)
					{
						// finally...
						AndroidBitmap_unlockPixels(threadInfo->env, bitmapObj);
						throw;
					}
                    AndroidBitmap_unlockPixels(threadInfo->env, bitmapObj);
				}
        	}
        	catch (...)
        	{
        		// Just don't add the glyph, for now
        	}

            threadInfo->env->DeleteLocalRef(glyphObj);
    	}

        if (glyphInfo)
        {
            // Now we make a rectangle that covers the glyph in its texture atlas
            DrawableString::Rect rect;
            const Point2f offset(offsetX,-glyphInfo->offset.y());
            const float scale = 1.0/BogusFontScale;

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

            offsetX += glyphInfo->size.x();
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

    return drawString;
}

FontTextureManager_Android::FontManager_AndroidRef FontTextureManager_Android::findFontManagerForFont(PlatformInfo_Android *threadInfo,jobject typefaceObj,const LabelInfo &inLabelInfo)
{
	const LabelInfoAndroid &labelInfo = (LabelInfoAndroid &)inLabelInfo;

	for (auto it : fontManagers)
	{
		if (const auto fm = std::dynamic_pointer_cast<FontManager_Android>(it.second))
		{
			if (fm->pointSize == labelInfo.fontSize &&
				fm->color == labelInfo.textColor &&
				fm->outlineColor == labelInfo.outlineColor &&
				fm->outlineSize == labelInfo.outlineSize &&
				labelInfo.typefaceIsSame(threadInfo, fm->typefaceObj))
			{
				return fm;
			}
		}
	}

	// Didn't find it, so create it
	const auto fm = std::make_shared<FontManager_Android>(threadInfo->env,typefaceObj);
	fm->color = labelInfo.textColor;
	fm->pointSize = labelInfo.fontSize;
	fm->outlineColor = labelInfo.outlineColor;
	fm->outlineSize = labelInfo.outlineSize;
	fontManagers[fm->getId()] = fm;

	return fm;
}

}
