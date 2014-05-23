#import <jni.h>
#import "Maply_jni.h"
#import "WhirlyGlobe.h"

namespace WhirlyKit
{

/**
 *  This is the platform specific font texture manager for Android.
 */
class FontTextureManagerAndroid : public FontTextureManager
{
public:
	FontTextureManagerAndroid(JNIEnv *env,Scene *scene,jobject charRenderObj);
    ~FontTextureManagerAndroid();

    // Wrapper for FontManager.  Tracks CoreText resources too.
    class FontManagerAndroid : public FontManager
    {
    public:
    	FontManagerAndroid(JNIEnv *env,jobject typefaceObj);
    	FontManagerAndroid();
        ~FontManagerAndroid();

        // Clear out global refs to Java objects we may be sitting on
        void clearRefs(JNIEnv *env);

        jobject typefaceObj;
    };

    /// Add the given string.  Caller is responsible for deleting
    ///  the DrawableString
    DrawableString *addString(JNIEnv *env,const std::string &str,jobject labelInfoObj,ChangeSet &changes);

protected:
    // Find the appropriate font manager
    FontManagerAndroid *findFontManagerForFont(jobject typefaceObj,const LabelInfo &labelInfo);

    // Render the glyph with the given font manager
//    RawDataRef renderGlyph(WKGlyph glyph,FontManageriOS *fm,Point2f &size,Point2f &glyphSize,Point2f &offset,Point2f &textureOffset);

    // Java object that can do the character rendering for us
    jobject charRenderObj;
    jmethodID renderMethodID;
    jfieldID bitmapID,sizeXID,sizeYID,glyphSizeXID,glyphSizeYID,offsetXID,offsetYID,textureOffsetXID,textureOffsetYID;
};

}
