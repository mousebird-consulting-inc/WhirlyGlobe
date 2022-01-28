/*  AttrDictionary_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2022 mousebird consulting
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
 */

#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_AttrDictionaryEntry.h"
#import "Vectors_jni.h"
#import "WhirlyGlobe_Android.h"

template<> AttrDictEntryClassInfo *AttrDictEntryClassInfo::classInfoObj = nullptr;

using namespace WhirlyKit;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionaryEntry_nativeInit
  (JNIEnv *env, jclass theClass)
{
	AttrDictEntryClassInfo::getClassInfo(env,theClass);
}

JNIEXPORT jobject JNICALL MakeAttrDictionaryEntry(JNIEnv *env,const DictionaryEntry_AndroidRef &entry)
{
	AttrDictEntryClassInfo *classInfo = AttrDictEntryClassInfo::getClassInfo(env,"com/mousebird/maply/AttrDictionaryEntry");

	jobject entryObj = classInfo->makeWrapperObject(env,nullptr);
    if (DictionaryEntry_AndroidRef *inst = classInfo->getObject(env,entryObj))
    {
        **inst = *entry;
    }

	return entryObj;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionaryEntry_initialise
  (JNIEnv *env, jobject obj)
{
    try
    {
		auto *entry = new DictionaryEntry_AndroidRef(new DictionaryEntry_Android());
		AttrDictEntryClassInfo::getClassInfo()->setHandle(env,obj,entry);
    }
    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionaryEntry_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		AttrDictEntryClassInfo *classInfo = AttrDictEntryClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        DictionaryEntry_AndroidRef *entry = classInfo->getObject(env,obj);
        delete entry;
        classInfo->clearHandle(env,obj);
	}
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_AttrDictionaryEntry_getTypeNative
	(JNIEnv *env, jobject obj)
{
	try
	{
		DictionaryEntry_AndroidRef *entry = AttrDictEntryClassInfo::getClassInfo()->getObject(env,obj);
		if (entry)
        {
		    return (*entry)->getType();
        }
	}
    MAPLY_STD_JNI_CATCH()
	return DictTypeNone;
}

extern "C"
JNIEXPORT jstring JNICALL Java_com_mousebird_maply_AttrDictionaryEntry_getString
	(JNIEnv *env, jobject obj)
{
    try
    {
        DictionaryEntry_AndroidRef *entry = AttrDictEntryClassInfo::getClassInfo()->getObject(env,obj);
        if (entry && (*entry)->getType() == DictTypeString)
        {
            return env->NewStringUTF((*entry)->getString().c_str());
        }
    }
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_AttrDictionaryEntry_getInt
	(JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto entry = AttrDictEntryClassInfo::get(env,obj))
        {
            const DictionaryType type = (*entry)->getType();
            if (type == DictTypeDouble || type == DictTypeInt || type == DictTypeIdentity)
            {
                return (*entry)->getInt();
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0;
}

extern "C"
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
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_AttrDictionaryEntry_getDict
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto entry = AttrDictEntryClassInfo::get(env,obj))
        {
            if ((*entry)->getType() == DictTypeDictionary)
            {
                if (const auto entryDict = (*entry)->getDict())
                {
                    // Cast shared pointer type, or make a copy if that fails
                    return MakeAttrDictionaryRefOrCopy(env, entryDict);
                }
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_AttrDictionaryEntry_getIdentity
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
        return (*entry)->getIdentity();
    }
    MAPLY_STD_JNI_CATCH()
    return 0;
}

extern "C"
JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_AttrDictionaryEntry_getArray
  (JNIEnv *env, jobject obj)
{
    try
    {
        DictionaryEntry_AndroidRef *entry = AttrDictEntryClassInfo::getClassInfo()->getObject(env,obj);
        if (!entry || (*entry)->getType() != DictTypeArray)
            return nullptr;
        std::vector<jobject> retObjs;
        auto arr = (*entry)->getArray();
        for (const auto &arrEntry: arr)
        {
            jobject newObj = MakeAttrDictionaryEntry(env,std::dynamic_pointer_cast<DictionaryEntry_Android>(arrEntry));
            retObjs.push_back(newObj);
        }

        jobjectArray retArray = BuildObjectArray(env,AttrDictEntryClassInfo::getClassInfo()->getClass(),retObjs);
        for (jobject objRef: retObjs)
            env->DeleteLocalRef(objRef);
        retObjs.clear();

        return retArray;
    }
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}
