/*
 *  AttrDictionary_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2016 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_AttrDictionary.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_nativeInit
  (JNIEnv *env, jclass theClass)
{
	AttrDictClassInfo::getClassInfo(env,theClass);
}

JNIEXPORT jobject JNICALL MakeAttrDictionary(JNIEnv *env,Dictionary *dict)
{
	AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo(env,"com/mousebird/maply/AttrDictionary");

//	Dictionary *copyDict = new Dictionary(*dict);
	// Note: Just wrapping what's passed in
	jobject dictObj = classInfo->makeWrapperObject(env,dict);

	return dictObj;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		Dictionary *dict = new Dictionary();
		AttrDictClassInfo::getClassInfo()->setHandle(env,obj,dict);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Dictionary::initialise()");
	}
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            Dictionary *dict = classInfo->getObject(env,obj);
            if (!dict)
                return;
		// Note: This only works because we're not copying dictionaries
//		delete dict;

            classInfo->clearHandle(env,obj);
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Dictionary::dispose()");
	}
}

JNIEXPORT jstring JNICALL Java_com_mousebird_maply_AttrDictionary_getString
  (JNIEnv *env, jobject obj, jstring attrNameStr)
{
	try
	{
		AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
		Dictionary *dict = classInfo->getObject(env,obj);
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

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_AttrDictionary_getInt
  (JNIEnv *env, jobject obj, jstring attrNameStr)
{
	try
	{
		AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
		Dictionary *dict = classInfo->getObject(env,obj);
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
			return JavaIntegerClassInfo::getClassInfo(env)->makeInteger(env,val);
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Dictionary::getInt()");
	}

	return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_AttrDictionary_getDouble
  (JNIEnv *env, jobject obj, jstring attrNameStr)
{
	try
	{
		AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
		Dictionary *dict = classInfo->getObject(env,obj);
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
		    return JavaDoubleClassInfo::getClassInfo(env)->makeDouble(env,val);
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Dictionary::getDouble()");
	}

	return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_setString
(JNIEnv *env, jobject obj, jstring attrNameObj, jstring strValObj)
{
    try
    {
        AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
        Dictionary *dict = classInfo->getObject(env,obj);
        if (!dict)
            return;
        
        JavaString attrName(env,attrNameObj);
        JavaString attrVal(env,strValObj);

        dict->setString(attrName.cStr,attrVal.cStr);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Dictionary::setString()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_setInt
(JNIEnv *env, jobject obj, jstring attrNameObj, jint iVal)
{
    try
    {
        AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
        Dictionary *dict = classInfo->getObject(env,obj);
        if (!dict)
            return;
        
        JavaString attrName(env,attrNameObj);
        
        dict->setInt(attrName.cStr,iVal);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Dictionary::setInt()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_setDouble
(JNIEnv *env, jobject obj, jstring attrNameObj, jdouble dVal)
{
    try
    {
        AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
        Dictionary *dict = classInfo->getObject(env,obj);
        if (!dict)
            return;
        
        JavaString attrName(env,attrNameObj);
        
        dict->setDouble(attrName.cStr,dVal);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Dictionary::setDouble()");
    }
}

JNIEXPORT jstring JNICALL Java_com_mousebird_maply_AttrDictionary_toString
(JNIEnv *env, jobject obj)
{
    try
    {
        AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
        Dictionary *dict = classInfo->getObject(env,obj);
        if (!dict)
            return NULL;

        std::string str = dict->toString();
        return env->NewStringUTF(str.c_str());
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Dictionary::setDouble()");
    }
    
    return NULL;
}
