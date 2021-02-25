/*
 *  FontTextureManagerAndroid.h
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

#import <jni.h>
#import "Maply_jni.h"
#import "WhirlyGlobe.h"

namespace WhirlyKit
{

class LabelInfoAndroid;

/**
 *  This is the platform specific font texture manager for Android.
 */
class FontTextureManager_Android : public FontTextureManager
{
public:
	FontTextureManager_Android(JNIEnv *env,SceneRenderer *sceneRender,Scene *scene,jobject charRenderObj);
    ~FontTextureManager_Android();

    // Wrapper for FontManager.
    class FontManager_Android : public FontManager
    {
    public:
    	FontManager_Android(JNIEnv *env,jobject typefaceObj);
    	FontManager_Android();
        ~FontManager_Android();

        // Clear out global refs to Java objects we may be sitting on
        void clearRefs(JNIEnv *env);

        jobject typefaceObj;
    };
    typedef std::shared_ptr<FontManager_Android> FontManager_AndroidRef;

    /// Add the given string.  Caller is responsible for deleting
    ///  the DrawableString
    DrawableString *addString(PlatformThreadInfo *threadInfo,const std::vector<int> &codePoints,const LabelInfoAndroid *,ChangeSet &changes);
    
protected:
    // Find the appropriate font manager
    FontManager_AndroidRef findFontManagerForFont(PlatformInfo_Android *threadInfo,jobject typefaceObj,const LabelInfo &labelInfo);

    // Render the glyph with the given font manager
//    RawDataRef renderGlyph(WKGlyph glyph,FontManageriOS *fm,Point2f &size,Point2f &glyphSize,Point2f &offset,Point2f &textureOffset);

    // Java object that can do the character rendering for us
    jobject charRenderObj;
    jmethodID renderMethodID;
    jfieldID bitmapID,sizeXID,sizeYID,glyphSizeXID,glyphSizeYID,offsetXID,offsetYID,textureOffsetXID,textureOffsetYID;
};

}
