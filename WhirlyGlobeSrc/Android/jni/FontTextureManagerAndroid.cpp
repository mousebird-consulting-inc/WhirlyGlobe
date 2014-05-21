#import "FontTextureManagerAndroid.h"

namespace WhirlyKit
{

FontTextureManagerAndroid::FontManagerAndroid::FontManagerAndroid(jobject font)
: font(font)
{
}

FontTextureManagerAndroid::FontManagerAndroid::FontManagerAndroid()
: font(NULL)
{
}

FontTextureManagerAndroid::FontManagerAndroid::~FontManagerAndroid()
{
	// Note: Clean up the font reference
}

FontTextureManagerAndroid::FontTextureManagerAndroid(Scene *scene)
	: FontTextureManager(scene)
{
}

FontTextureManagerAndroid::~FontTextureManagerAndroid()
{
    for (FontManagerSet::iterator it = fontManagers.begin();
         it != fontManagers.end(); ++it)
        delete *it;
    fontManagers.clear();
}

DrawableString *FontTextureManagerAndroid::addString(const std::string &str,ChangeSet &changes)
{
	// Note: Fill this in
	return NULL;
}

FontTextureManagerAndroid *FontTextureManagerAndroid::findFontManagerForFont(jobject font,const RGBAColor &color,const RGBAColor &outlineColor,float outlineSize)
{
	// Note: Fill this in
	return NULL;
}


}
