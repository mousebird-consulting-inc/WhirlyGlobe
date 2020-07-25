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
#import "com_mousebird_maply_AttrDictionaryEntry.h"
#import "Vectors_jni.h"
#import "WhirlyGlobe_Android.h"

template<> AttrDictEntryClassInfo *AttrDictEntryClassInfo::classInfoObj = NULL;

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionaryEntry_nativeInit
(JNIEnv *env, jclass theClass)
{
	AttrDictEntryClassInfo::getClassInfo(env,theClass);
}

JNIEXPORT jobject JNICALL MakeAttrDictionaryEntry(JNIEnv *env,DictionaryEntry_AndroidRef entry)
{
	AttrDictEntryClassInfo *classInfo = AttrDictEntryClassInfo::getClassInfo(env,"com/mousebird/maply/AttrDictionaryEntry");

	jobject entryObj = classInfo->makeWrapperObject(env,NULL);
    DictionaryEntry_AndroidRef *inst = classInfo->getObject(env,entryObj);
    if (inst)
    	*(inst->get()) = *(entry.get());

	return entryObj;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionaryEntry_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
		DictionaryEntry_AndroidRef *entry = new DictionaryEntry_AndroidRef(new DictionaryEntry_Android());
		AttrDictEntryClassInfo::getClassInfo()->setHandle(env,obj,entry);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in AttrDictionaryEntry::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionaryEntry_dispose
(JNIEnv *env, jobject obj)
{
	try
	{
		AttrDictEntryClassInfo *classInfo = AttrDictEntryClassInfo::getClassInfo();
		{
			std::lock_guard<std::mutex> lock(disposeMutex);
			DictionaryEntry_AndroidRef *entry = classInfo->getObject(env,obj);
			if (!entry)
				return;
			delete entry;

			classInfo->clearHandle(env,obj);
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in AttrDictionaryEntry::dispose()");
	}
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_AttrDictionaryEntry_getTypeNative
	(JNIEnv *env, jobject obj)
{
	try
	{
		DictionaryEntry_AndroidRef *entry = AttrDictEntryClassInfo::getClassInfo()->getObject(env,obj);
		if (!entry)
			return DictTypeNone;
		return (*entry)->getType();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in AttrDictionaryEntry::getType()");
	}

	return DictTypeNone;
}

JNIEXPORT jstring JNICALL Java_com_mousebird_maply_AttrDictionaryEntry_getString
		(JNIEnv *env, jobject obj)
{
    try
    {
        DictionaryEntry_AndroidRef *entry = AttrDictEntryClassInfo::getClassInfo()->getObject(env,obj);
        if (!entry || (*entry)->getType() != DictTypeString)
            return NULL;
        return env->NewStringUTF((*entry)->getString().c_str());
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in AttrDictionaryEntry::getString()");
    }

    return NULL;
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_AttrDictionaryEntry_getInt
		(JNIEnv *env, jobject obj)
{
    try
    {
        DictionaryEntry_AndroidRef *entry = AttrDictEntryClassInfo::getClassInfo()->getObject(env,obj);
        if (!entry)
            return 0;
        DictionaryType type = (*entry)->getType();
        if (type != DictTypeDouble && type != DictTypeInt && type != DictTypeIdentity)
            return 0;
        return (*entry)->getInt();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in AttrDictionaryEntry::getInt()");
    }

    return 0;
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_AttrDictionaryEntry_getDouble
		(JNIEnv *env, jobject obj)
{
    try
    {
        DictionaryEntry_AndroidRef *entry = AttrDictEntryClassInfo::getClassInfo()->getObject(env,obj);
        if (!entry)
            return 0.0;
        DictionaryType type = (*entry)->getType();
        if (type != DictTypeDouble && type != DictTypeInt && type != DictTypeIdentity)
            return 0.0;
        return (*entry)->getDouble();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in AttrDictionaryEntry::getDouble()");
    }

    return 0.0;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_AttrDictionaryEntry_getDict
        (JNIEnv *env, jobject obj)
{
    try
    {
        DictionaryEntry_AndroidRef *entry = AttrDictEntryClassInfo::getClassInfo()->getObject(env,obj);
        if (!entry || (*entry)->getType() != DictTypeDictionary)
            return NULL;
        return MakeAttrDictionary(env,std::dynamic_pointer_cast<MutableDictionary_Android>((*entry)->getDict()));
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in AttrDictionaryEntry::getDict()");
    }

    return NULL;
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_AttrDictionaryEntry_getIdentity
		(JNIEnv *env, jobject obj)
{
    try
    {
        DictionaryEntry_AndroidRef *entry = AttrDictEntryClassInfo::getClassInfo()->getObject(env,obj);
        if (!entry)
            return 0.0;
        DictionaryType type = (*entry)->getType();
        if (type != DictTypeDouble && type != DictTypeInt && type != DictTypeIdentity)
            return 0;
        return (*entry)->getIdentity();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in AttrDictionaryEntry::getIdentity()");
    }

    return 0;
}

JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_AttrDictionaryEntry_getArray
		(JNIEnv *env, jobject obj)
{
    try
    {
        DictionaryEntry_AndroidRef *entry = AttrDictEntryClassInfo::getClassInfo()->getObject(env,obj);
        if (!entry || (*entry)->getType() != DictTypeArray)
            return NULL;
        std::vector<jobject> retObjs;
        auto arr = (*entry)->getArray();
        for (auto arrEntry: arr) {
            jobject newObj = MakeAttrDictionaryEntry(env,std::dynamic_pointer_cast<DictionaryEntry_Android>(arrEntry));
            retObjs.push_back(newObj);
        }

        return BuildObjectArray(env,AttrDictEntryClassInfo::getClassInfo()->getClass(),retObjs);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in AttrDictionaryEntry::getArray()");
    }

    return NULL;
}
