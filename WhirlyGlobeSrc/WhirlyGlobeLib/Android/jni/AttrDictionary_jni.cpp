#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_AttrDictionary.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

// Caching class and method pointers
class AttrClassInfo : public JavaClassInfo<Dictionary>
{
public:
	AttrClassInfo(JNIEnv *env, jclass theClass)
		: JavaClassInfo<Dictionary>(env,theClass)
	{
		initMethodID = env->GetMethodID(theClass, "<init>", "()V");

		jclass intLocalClass = env->FindClass("java/lang/Integer");
		integerClass = (jclass)env->NewGlobalRef(intLocalClass);
	    integerClassInitID = env->GetMethodID(integerClass, "<init>", "(I)V");

	    jclass doubleLocalClass = env->FindClass("java/lang/Double");
	    doubleClass = (jclass)env->NewGlobalRef(doubleLocalClass);
	    doubleClassInitID = env->GetMethodID(doubleClass, "<init>", "(D)V");
	}

	jmethodID initMethodID;
	jclass integerClass;
	jmethodID integerClassInitID;
	jclass doubleClass;
	jmethodID doubleClassInitID;
};

static AttrClassInfo *classInfo = NULL;

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_AttrDictionary_nativeInit
  (JNIEnv *env, jclass theClass)
{
	if (classInfo)
		delete classInfo;

	classInfo = new AttrClassInfo(env,theClass);
}

JNIEXPORT jobject JNICALL MakeAttrDictionary(JNIEnv *env,Dictionary *dict)
{
	if (!classInfo)
	{
		jclass cls = env->FindClass("com/mousebirdconsulting/maply/AttrDictionary");
		classInfo = new AttrClassInfo(env,cls);
		env->DeleteLocalRef(cls);
	}

	jobject dictObj = env->NewObject(classInfo->theClass, classInfo->initMethodID);
	Dictionary *copyDict = new Dictionary(*dict);
	setHandle(env,dictObj,copyDict);

	return dictObj;
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_AttrDictionary_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		Dictionary *dict = new Dictionary();
		classInfo->setHandle(env,obj,dict);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Dictionary::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_AttrDictionary_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		Dictionary *dict = classInfo->getHandle(env,obj);
		if (!dict)
			return;
		delete dict;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Dictionary::dispose()");
	}
}

JNIEXPORT jstring JNICALL Java_com_mousebirdconsulting_maply_AttrDictionary_getString
  (JNIEnv *env, jobject obj, jstring attrNameStr)
{
	try
	{
		Dictionary *dict = classInfo->getHandle(env,obj);
		if (!dict)
			return NULL;

		const char *cStr = env->GetStringUTFChars(attrNameStr,0);
		if (!cStr)
			return NULL;
		std::string attrName(cStr);
		env->ReleaseStringUTFChars(attrNameStr, cStr);

		std::string str = dict->getString(attrName);
		if (!str.empty())
		{
			return env->NewStringUTF(str.c_str());
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Dictionary::getString()");
	}

	return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebirdconsulting_maply_AttrDictionary_getInt
  (JNIEnv *env, jobject obj, jstring attrNameStr)
{
	try
	{
		Dictionary *dict = classInfo->getHandle(env,obj);
		if (!dict)
			return NULL;

		const char *cStr = env->GetStringUTFChars(attrNameStr,0);
		if (!cStr)
			return NULL;
		std::string attrName(cStr);
		env->ReleaseStringUTFChars(attrNameStr, cStr);

		if (dict->hasField(attrName))
		{
			int val = dict->getInt(attrName);
		    return env->NewObject(classInfo->integerClass, classInfo->integerClassInitID, val);
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Dictionary::getInt()");
	}

	return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebirdconsulting_maply_AttrDictionary_getDouble
  (JNIEnv *env, jobject obj, jstring attrNameStr)
{
	try
	{
		Dictionary *dict = classInfo->getHandle(env,obj);
		if (!dict)
			return NULL;

		const char *cStr = env->GetStringUTFChars(attrNameStr,0);
		if (!cStr)
			return NULL;
		std::string attrName(cStr);
		env->ReleaseStringUTFChars(attrNameStr, cStr);

		if (dict->hasField(attrName))
		{
			double val = dict->getDouble(attrName);
		    return env->NewObject(classInfo->doubleClass, classInfo->doubleClassInitID, val);
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Dictionary::getDouble()");
	}

	return NULL;
}
