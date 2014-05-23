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

bool LabelInfoAndroid::typefaceIsSame(const jobject inTypeface) const
{
	// Obviously true here
	if (inTypeface == typefaceObj)
		return true;

	// Now for a deeper comparison
	jclass typefaceClass = env->GetObjectClass(inTypeface);
	jmethodID jmethodID = env->GetMethodID(typefaceClass, "equals", "(Ljava/lang/Object;)Z");
	bool res = env->CallBooleanMethod(typefaceObj,jmethodID,inTypeface);

	return res;
}

void LabelInfoAndroid::setTypeface(JNIEnv *env,jobject inTypefaceObj)
{
	if (typefaceObj)
		clearRefs(env);

	if (inTypefaceObj)
		typefaceObj = env->NewGlobalRef(inTypefaceObj);
}

}
