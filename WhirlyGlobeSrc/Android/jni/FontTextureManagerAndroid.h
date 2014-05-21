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
	FontTextureManagerAndroid(Scene *scene);
    ~FontTextureManagerAndroid();

    // Wrapper for FontManager.  Tracks CoreText resources too.
    class FontManagerAndroid : public FontManager
    {
    public:
    	FontManagerAndroid(jobject font);
    	FontManagerAndroid();
        ~FontManagerAndroid();

        jobject font;
    };

    /// Add the given string.  Caller is responsible for deleting
    ///  the DrawableString
    DrawableString *addString(const std::string &str,ChangeSet &changes);

protected:
    // Find the appropriate font manager
    FontTextureManagerAndroid *findFontManagerForFont(jobject font,const RGBAColor &color,const RGBAColor &outlineColor,float outlineSize);

    // Render the glyph with the given font manager
//    RawDataRef renderGlyph(WKGlyph glyph,FontManageriOS *fm,Point2f &size,Point2f &glyphSize,Point2f &offset,Point2f &textureOffset);
};

}
