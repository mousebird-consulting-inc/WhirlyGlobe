#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_AttrDictionary.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT jobject JNICALL MakeAttrDictionary(JNIEnv *env,Dictionary *dict)
{
	// Make a Java Point3d
	jclass cls = env->FindClass("com/mousebirdconsulting/maply/AttrDictionary");
	jmethodID methodID = env->GetMethodID(cls, "<init>", "()V");
	if (!methodID)
		throw 1;
	jobject dictObj = env->NewObject(cls, methodID);
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
		setHandle(env,obj,dict);
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
		Dictionary *dict = getHandle<Dictionary>(env,obj);
		if (!dict)
			return;
		delete dict;

		clearHandle(env,obj);
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
		Dictionary *dict = getHandle<Dictionary>(env,obj);
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
		Dictionary *dict = getHandle<Dictionary>(env,obj);
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
		    jclass cls = env->FindClass("java/lang/Integer");
		    jmethodID methodID = env->GetMethodID(cls, "<init>", "(I)V");
		    return env->NewObject(cls, methodID, val);
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
		Dictionary *dict = getHandle<Dictionary>(env,obj);
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
		    jclass cls = env->FindClass("java/lang/Double");
		    jmethodID methodID = env->GetMethodID(cls, "<init>", "(D)V");
		    return env->NewObject(cls, methodID, val);
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Dictionary::getDouble()");
	}

	return NULL;
}
