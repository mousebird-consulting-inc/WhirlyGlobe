#import "SingleLabelAndroid.h"
#import "FontTextureManagerAndroid.h"

namespace WhirlyKit
{

DrawableString *SingleLabelAndroid::generateDrawableString(const LabelInfo *inLabelInfo,FontTextureManager *inFontTexManager,ChangeSet &changes)
{
	FontTextureManagerAndroid *fontTexManager = (FontTextureManagerAndroid *)inFontTexManager;
	return fontTexManager->addString(text,changes);
}

}
