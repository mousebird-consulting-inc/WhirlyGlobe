#import "LabelInfoAndroid.h"

namespace WhirlyKit
{

LabelInfoAndroid::LabelInfoAndroid()
: typefaceObj(NULL), env(NULL), labelInfoObj(NULL)
{
	// Note: Porting
	justify = WhirlyKitLabelLeft;
	layoutEngine = false;
	textColor = RGBAColor(0,0,255,255);
	backColor = RGBAColor(255,0,0,255);
}

void LabelInfoAndroid::clearRefs(JNIEnv *env)
{
	if (typefaceObj)
	{
		env->DeleteGlobalRef(typefaceObj);
		typefaceObj = NULL;
	}
}

void LabelInfoAndroid::setTypeface(JNIEnv *env,jobject inTypefaceObj)
{
	if (typefaceObj)
		clearRefs(env);

	if (inTypefaceObj)
		typefaceObj = env->NewGlobalRef(inTypefaceObj);
}

}
