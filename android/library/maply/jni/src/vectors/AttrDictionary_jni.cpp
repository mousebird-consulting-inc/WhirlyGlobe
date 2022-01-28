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
#import "com_mousebird_maply_AttrDictionary.h"
#import "Vectors_jni.h"
#import "WhirlyGlobe_Android.h"

template<> AttrDictClassInfo *AttrDictClassInfo::classInfoObj = nullptr;

using namespace WhirlyKit;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_nativeInit(JNIEnv *env, jclass theClass)
{
	AttrDictClassInfo::getClassInfo(env,theClass);
}

JNIEXPORT jobject JNICALL MakeAttrDictionaryRef(JNIEnv *env,const MutableDictionary_AndroidRef &dict)
{
	const auto classInfo = AttrDictClassInfo::getClassInfo(env,"com/mousebird/maply/AttrDictionary");
	return classInfo->makeWrapperObject(env, new MutableDictionary_AndroidRef(dict));
}
JNIEXPORT jobject JNICALL MakeAttrDictionaryRef(JNIEnv *env, MutableDictionary_AndroidRef &&dict)
{
	const auto classInfo = AttrDictClassInfo::getClassInfo(env,"com/mousebird/maply/AttrDictionary");
	return classInfo->makeWrapperObject(env, new MutableDictionary_AndroidRef(std::move(dict)));
}

JNIEXPORT jobject JNICALL MakeAttrDictionaryCopy(JNIEnv *env,const MutableDictionary_AndroidRef &dict)
{
	const auto classInfo = AttrDictClassInfo::getClassInfo(env,"com/mousebird/maply/AttrDictionary");

	jobject dictObj = classInfo->makeWrapperObject(env,nullptr);
	if (auto inst = classInfo->getObject(env,dictObj))
	{
		**inst = *dict.get();
	}
	return dictObj;
}

JNIEXPORT jobject JNICALL MakeAttrDictionaryRefOrCopy(JNIEnv *env,const WhirlyKit::DictionaryRef &dict)
{
	return !dict ? nullptr : MakeAttrDictionaryRef(env,
		std::dynamic_pointer_cast<MutableDictionary_Android>(dict) ?:
		std::make_shared<MutableDictionary_Android>(*dict));
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_initialise(JNIEnv *env, jobject obj)
{
	try
	{
		auto dict = new MutableDictionary_AndroidRef(new MutableDictionary_Android());
		AttrDictClassInfo::getClassInfo()->setHandle(env,obj,dict);
	}
	MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_dispose(JNIEnv *env, jobject obj)
{
	try
	{
		AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            MutableDictionary_AndroidRef *dict = classInfo->getObject(env,obj);
            if (!dict)
                return;
            delete dict;

            classInfo->clearHandle(env,obj);
        }
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_AttrDictionary_parseFromJSON(JNIEnv *env, jobject obj, jstring inJsonStr)
{
	try
	{
		AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
		MutableDictionary_AndroidRef *dict = classInfo->getObject(env,obj);
		if (!dict)
			return false;

		JavaString jsonStr(env,inJsonStr);

		return (*dict)->parseJSON(jsonStr.getCString());
	}
	MAPLY_STD_JNI_CATCH()
	return true;	// ?
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_AttrDictionary_hasField(JNIEnv *env, jobject obj, jstring attrNameStr)
{
	try
	{
		AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
		MutableDictionary_AndroidRef *dict = classInfo->getObject(env,obj);
		if (!dict)
			return false;

		JavaString attrName(env,attrNameStr);

		return (*dict)->hasField(attrName.getCString());
	}
	MAPLY_STD_JNI_CATCH()
	return false;
}

extern "C"
JNIEXPORT jstring JNICALL Java_com_mousebird_maply_AttrDictionary_getString(JNIEnv *env, jobject obj, jstring attrNameStr)
{
	try
	{
		AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
		MutableDictionary_AndroidRef *dict = classInfo->getObject(env,obj);
		if (!dict)
			return nullptr;

		JavaString attrName(env,attrNameStr);

		std::string str = (*dict)->getString(attrName.getCString());
		if (!str.empty())
		{
			return env->NewStringUTF(str.c_str());
		}
	}
	MAPLY_STD_JNI_CATCH()
	return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_AttrDictionary_getInt(JNIEnv *env, jobject obj, jstring attrNameStr)
{
	try
	{
		AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
		MutableDictionary_AndroidRef *dict = classInfo->getObject(env,obj);
		if (!dict)
			return nullptr;

		JavaString attrName(env,attrNameStr);

		if ((*dict)->hasField(attrName.getCString()))
		{
			int val = (*dict)->getInt(attrName.getCString(), 0);
			return JavaIntegerClassInfo::getClassInfo(env)->makeInteger(env,val);
		}
	}
	MAPLY_STD_JNI_CATCH()
	return nullptr;
}

extern "C"
JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_AttrDictionary_getArray(JNIEnv *env, jobject obj, jstring attrNameStr)
{
    try
    {
        AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
        MutableDictionary_AndroidRef *dict = classInfo->getObject(env,obj);
        if (!dict)
            return nullptr;

        JavaString attrName(env,attrNameStr);

        if ((*dict)->getType(attrName.getCString()) == DictTypeArray)
        {
            std::vector<jobject> retObjs;
            auto arr = (*dict)->getArray(attrName.getCString());
            retObjs.reserve(arr.size());
            for (const auto &arrEntry : arr) {
                jobject newObj = MakeAttrDictionaryEntry(env,std::dynamic_pointer_cast<DictionaryEntry_Android>(arrEntry));
                retObjs.push_back(newObj);
            }

            jobjectArray retArray = BuildObjectArray(env,AttrDictEntryClassInfo::getClassInfo()->getClass(),retObjs);
            for (jobject objRef: retObjs)
            	env->DeleteLocalRef(objRef);
            retObjs.clear();

            return retArray;
        }
    }
	MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_AttrDictionary_getDouble(JNIEnv *env, jobject obj, jstring attrNameStr)
{
	try
	{
		AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
		MutableDictionary_AndroidRef *dict = classInfo->getObject(env,obj);
		if (!dict)
			return nullptr;

		JavaString attrName(env,attrNameStr);

		if ((*dict)->hasField(attrName.getCString()))
		{
			double val = (*dict)->getDouble(attrName.getCString(), 0);
		    return JavaDoubleClassInfo::getClassInfo(env)->makeDouble(env,val);
		}
	}
	MAPLY_STD_JNI_CATCH()
	return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_AttrDictionary_getIdentity(JNIEnv *env, jobject obj, jstring attrNameStr)
{
	try
	{
		AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
		MutableDictionary_AndroidRef *dict = classInfo->getObject(env,obj);
		if (!dict)
			return nullptr;

		JavaString attrName(env,attrNameStr);

		if ((*dict)->hasField(attrName.getCString()))
		{
			long long val = (*dict)->getIdentity(attrName.getCString());
		    return JavaLongClassInfo::getClassInfo(env)->makeLong(env,val);
		}
	}
	MAPLY_STD_JNI_CATCH()
	return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_AttrDictionary_getDict
  (JNIEnv *env, jobject obj, jstring attrNameStr)
{
	try
	{
		if (const auto attrName =JavaString(env, attrNameStr))
		if (const auto dict = AttrDictClassInfo::get(env,obj))
		if (const auto subDict = (*dict)->getDict(attrName.getCString()))
		{
			// Cast shared pointer type, or make a copy if that fails
			return MakeAttrDictionaryRefOrCopy(env, subDict);
		}
	}
	MAPLY_STD_JNI_CATCH()
	return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_AttrDictionary_getEntry(JNIEnv *env, jobject obj, jstring attrNameStr)
{
	try
	{
		AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
		MutableDictionary_AndroidRef *dict = classInfo->getObject(env,obj);
		if (!dict)
			return nullptr;

		JavaString attrName(env,attrNameStr);

		DictionaryEntry_AndroidRef entry = std::dynamic_pointer_cast<DictionaryEntry_Android>((*dict)->getEntry(attrName.getCString()));
		if (entry)
			return MakeAttrDictionaryEntry(env,entry);
	}
	MAPLY_STD_JNI_CATCH()
	return nullptr;
}

extern "C"
JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_AttrDictionary_getKeys(JNIEnv *env, jobject obj)
{
	try
	{
		AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
		MutableDictionary_AndroidRef *dict = classInfo->getObject(env,obj);
		if (!dict)
			return nullptr;

		auto keys = (*dict)->getKeys();
		return BuildStringArray(env,keys);
	}
	MAPLY_STD_JNI_CATCH()
	return nullptr;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_setString(JNIEnv *env, jobject obj, jstring attrNameObj, jstring strValObj)
{
    try
    {
        AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
        MutableDictionary_AndroidRef *dict = classInfo->getObject(env,obj);
        if (!dict)
            return;
        
        JavaString attrName(env,attrNameObj);
        JavaString attrVal(env,strValObj);

        (*dict)->setString(attrName.getCString(),attrVal.getCString());
    }
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_setInt(JNIEnv *env, jobject obj, jstring attrNameObj, jint iVal)
{
    try
    {
        AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
        MutableDictionary_AndroidRef *dict = classInfo->getObject(env,obj);
        if (!dict)
            return;
        
        JavaString attrName(env,attrNameObj);
        
        (*dict)->setInt(attrName.getCString(),iVal);
    }
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_setDouble(JNIEnv *env, jobject obj, jstring attrNameObj, jdouble dVal)
{
    try
    {
        AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
        MutableDictionary_AndroidRef *dict = classInfo->getObject(env,obj);
        if (!dict)
            return;
        
        JavaString attrName(env,attrNameObj);
        
        (*dict)->setDouble(attrName.getCString(),dVal);
    }
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_setDict(JNIEnv *env, jobject obj, jstring attrNameObj, jobject dictObj)
{
	try
	{
		AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
		MutableDictionary_AndroidRef *dict = classInfo->getObject(env,obj);
		MutableDictionary_AndroidRef *otherDict = classInfo->getObject(env,dictObj);
		if (!dict || !otherDict)
			return;

		JavaString attrName(env,attrNameObj);

		(*dict)->setDict(attrName.getCString(),*otherDict);
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_setArray__Ljava_lang_String_2_3Lcom_mousebird_maply_AttrDictionaryEntry_2
  (JNIEnv *env, jobject obj, jstring attrNameObj, jobjectArray entryArrObj)
{
	try
	{
		AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
		AttrDictEntryClassInfo *entryClassInfo = AttrDictEntryClassInfo::getClassInfo();
		MutableDictionary_AndroidRef *dict = classInfo->getObject(env,obj);
		if (!dict)
			return;
		JavaString attrName(env,attrNameObj);

		// Pull the entries out of the array of objects
		std::vector<DictionaryEntryRef> entries;
		JavaObjectArrayHelper arrayHelper(env,entryArrObj);
		for (unsigned int ii=0;ii<arrayHelper.numObjects();ii++) {
			jobject entryObj = arrayHelper.getNextObject();
			DictionaryEntry_AndroidRef *entry = entryClassInfo->getObject(env,entryObj);
			entries.push_back(DictionaryEntry_AndroidRef(*entry));
		}

		(*dict)->setArray(attrName.getCString(),entries);
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_setArray__Ljava_lang_String_2_3Lcom_mousebird_maply_AttrDictionary_2
  (JNIEnv *env, jobject obj, jstring attrNameObj, jobjectArray dictArrayObj)
{
	try
	{
		AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
		AttrDictClassInfo *entryClassInfo = AttrDictClassInfo::getClassInfo();
		MutableDictionary_AndroidRef *dict = classInfo->getObject(env,obj);
		if (!dict)
			return;
		JavaString attrName(env,attrNameObj);

		// Pull the entries out of the array of objects
		std::vector<DictionaryRef> entries;
		JavaObjectArrayHelper arrayHelper(env,dictArrayObj);
		for (unsigned int ii=0;ii<arrayHelper.numObjects();ii++) {
			jobject entryObj = arrayHelper.getNextObject();
			MutableDictionary_AndroidRef *entry = entryClassInfo->getObject(env,entryObj);
			entries.push_back(MutableDictionary_AndroidRef(*entry));
		}

		(*dict)->setArray(attrName.getCString(),entries);
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jstring JNICALL Java_com_mousebird_maply_AttrDictionary_toString(JNIEnv *env, jobject obj)
{
    try
    {
        AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
        MutableDictionary_AndroidRef *dict = classInfo->getObject(env,obj);
        if (!dict)
            return nullptr;

        std::string str = (*dict)->toString();
        return env->NewStringUTF(str.c_str());
    }
	MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_AttrDictionary_get(JNIEnv *env, jobject obj, jstring attrNameStr)
{
	try
	{
		AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
		MutableDictionary_AndroidRef *dict = classInfo->getObject(env,obj);
		if (!dict)
		{
			return nullptr;
		}

		JavaString attrName(env,attrNameStr);

		if ((*dict)->hasField(attrName.getCString()))
		{
            const DictionaryType dictType = (*dict)->getType(attrName.getCString());
            if (dictType == DictTypeString)
            {
        		const std::string str = (*dict)->getString(attrName.getCString());
        		if (!str.empty())
        		{
        			return env->NewStringUTF(str.c_str());
        		}
            }
            else if (dictType == DictTypeInt)
            {
			    const int val = (*dict)->getInt(attrName.getCString(), 0);
			    return JavaIntegerClassInfo::getClassInfo(env)->makeInteger(env,val);
            }
            else if (dictType == DictTypeDouble)
            {
			    const double val = (*dict)->getDouble(attrName.getCString(), 0);
		        return JavaDoubleClassInfo::getClassInfo(env)->makeDouble(env,val);
            }
        }
	}
	MAPLY_STD_JNI_CATCH()
	return nullptr;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_addEntries(JNIEnv *env, jobject obj, jobject other)
{
	try
	{
		AttrDictClassInfo *classInfo = AttrDictClassInfo::getClassInfo();
		MutableDictionary_AndroidRef *dict = classInfo->getObject(env,obj);
		MutableDictionary_AndroidRef *otherDict = classInfo->getObject(env,other);
		if (dict && other)
		{
			(*dict)->addEntries(otherDict->get());
		}
	}
	MAPLY_STD_JNI_CATCH()
}
