#import "LabelInfoAndroid.h"

namespace WhirlyKit
{

LabelInfoAndroid::LabelInfoAndroid()
: typefaceObj(NULL), env(NULL), labelInfoObj(NULL)
{
	layoutEngine = false;
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
