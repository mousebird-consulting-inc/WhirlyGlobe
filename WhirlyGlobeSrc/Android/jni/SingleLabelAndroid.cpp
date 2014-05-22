#import "SingleLabelAndroid.h"
#import "FontTextureManagerAndroid.h"
#import "LabelInfoAndroid.h"

namespace WhirlyKit
{

DrawableString *SingleLabelAndroid::generateDrawableString(const LabelInfo *inLabelInfo,FontTextureManager *inFontTexManager,ChangeSet &changes)
{
	FontTextureManagerAndroid *fontTexManager = (FontTextureManagerAndroid *)inFontTexManager;
	const LabelInfoAndroid *labelInfo = (LabelInfoAndroid *)inLabelInfo;
	return fontTexManager->addString(labelInfo->env,text,labelInfo->labelInfoObj,changes);
}

}
