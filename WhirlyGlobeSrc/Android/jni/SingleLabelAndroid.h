#import "WhirlyGlobe.h"

namespace WhirlyKit
{

/**
 * The platform specific single label for Android.
 * This knows how to render itself on Android devices.
 */
class SingleLabelAndroid : public SingleLabel
{
public:
	DrawableString *generateDrawableString(const LabelInfo *inLabelInfo,FontTextureManager *fontTexManager,ChangeSet &changes);
};

}
