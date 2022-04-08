/*  FontTextureManagerAndroid.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
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

#import "Maply_jni.h"
#import "WhirlyGlobeLib.h"

namespace WhirlyKit
{

class LabelInfoAndroid;

/**
 *  This is the platform specific font texture manager for Android.
 */
class FontTextureManager_Android : public FontTextureManager
{
public:
    FontTextureManager_Android(PlatformThreadInfo *,SceneRenderer *sceneRender,Scene *scene,jobject charRenderObj);
    virtual ~FontTextureManager_Android();

    // Wrapper for FontManager.
    struct FontManager_Android : public FontManager
    {
        FontManager_Android(PlatformThreadInfo *inst,jobject typefaceObj);
        virtual ~FontManager_Android();

        virtual bool operator <(const FontManager &that) const override;

        // Clear out global refs to Java objects we may be sitting on
        virtual void teardown(PlatformThreadInfo*) override;

        jobject typefaceObj = nullptr;
    };
    typedef std::shared_ptr<FontManager_Android> FontManager_AndroidRef;

    /// Add the given string.  Caller is responsible for deleting the DrawableString
    std::unique_ptr<DrawableString> addString(PlatformThreadInfo *,
                                              const std::vector<int> &codePoints,
                                              const LabelInfoAndroid *,
                                              ChangeSet &);

    virtual void teardown(PlatformThreadInfo *) override;

protected:
    // Find the appropriate font manager
    FontManager_AndroidRef findFontManagerForFont(PlatformInfo_Android *,jobject typefaceObj,const LabelInfo &);

    // Java object that can do the character rendering for us
    jobject charRenderObj = nullptr;
    jobject glyphClassRef = nullptr;
    jmethodID renderMethodID = nullptr;
    jfieldID bitmapID = nullptr;
    jfieldID sizeXID = nullptr;
    jfieldID sizeYID = nullptr;
    jfieldID glyphSizeXID = nullptr;
    jfieldID glyphSizeYID = nullptr;
    jfieldID offsetXID = nullptr;
    jfieldID offsetYID = nullptr;
    jfieldID textureOffsetXID = nullptr;
    jfieldID textureOffsetYID = nullptr;
    jfieldID baselineID = nullptr;
};
typedef std::shared_ptr<FontTextureManager_Android> FontTextureManager_AndroidRef;

}
