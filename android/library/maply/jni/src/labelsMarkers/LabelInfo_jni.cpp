/*  LabelInfo_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2021 mousebird consulting
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

#import "LabelsAndMarkers_jni.h"
#import "com_mousebird_maply_LabelInfo.h"

using namespace WhirlyKit;

template<> LabelInfoClassInfo *LabelInfoClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_nativeInit(JNIEnv *env, jclass cls)
{
	LabelInfoClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_initialise(JNIEnv *env, jobject obj)
{
	try
	{
		LabelInfoAndroidRef *info = new LabelInfoAndroidRef(new LabelInfoAndroid(true));
		LabelInfoClassInfo::getClassInfo()->setHandle(env,obj,info);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::initialise()");
	}
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_dispose(JNIEnv *env, jobject obj)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            LabelInfoAndroidRef *info = classInfo->getObject(env,obj);
            if (info) {
                PlatformInfo_Android platformInfo(env);
                (*info)->clearRefs(&platformInfo);
                delete info;
                classInfo->clearHandle(env,obj);
            }
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::dispose()");
	}
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_LabelInfo_getTextColor(JNIEnv *env, jobject obj)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfoAndroidRef *info = classInfo->getObject(env,obj);
		if (!info)
			return 0;

		int textColor = (*info)->textColor.asInt();
		return textColor;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::getTextColor()");
	}

    return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setTextColor
  (JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfoAndroidRef *info = classInfo->getObject(env,obj);
		if (!info)
			return;
        (*info)->textColor = RGBAColor(r*255,g*255,b*255,a*255);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setColor()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setBackgroundColor
  (JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
        LabelInfoAndroidRef *info = classInfo->getObject(env,obj);
		if (!info)
			return;
        (*info)->backColor = RGBAColor(r*255,g*255,b*255,a*255);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setColor()");
	}
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_LabelInfo_getBackgroundColor
  (JNIEnv *env, jobject obj)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfoAndroidRef *info = classInfo->getObject(env,obj);
		if (!info)
			return 0;

		int backColor = (*info)->backColor.asInt();
		return backColor;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::getBackColor()");
	}

    return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setTypefaceNative
  (JNIEnv *env, jobject obj, jobject typefaceObj)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfoAndroidRef *info = classInfo->getObject(env,obj);
		if (!info)
			return;
        PlatformInfo_Android platformInfo(env);
        (*info)->setTypeface(&platformInfo,typefaceObj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setFade()");
	}
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_LabelInfo_getTypeface
  (JNIEnv *env, jobject obj)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfoAndroidRef *info = classInfo->getObject(env,obj);
		if (!info)
			return NULL;

		return (*info)->typefaceObj;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::getTypeface()");
	}
    
    return NULL;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setFontSizeNative
  (JNIEnv *env, jobject obj, jfloat fontSize)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfoAndroidRef *info = classInfo->getObject(env,obj);
		if (!info)
			return;

        (*info)->fontSize = fontSize;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setFontSize()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setFontPointSize
        (JNIEnv *env, jobject obj, jfloat fontPointSize)
{
    try
    {
        LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
        LabelInfoAndroidRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;

        (*info)->fontPointSize = fontPointSize;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setFontPointSize()");
    }
}

/*
 * Class:     com_mousebird_maply_LabelInfo
 * Method:    getFontPointSize
 * Signature: ()F
 */
extern "C"
JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_LabelInfo_getFontPointSize
        (JNIEnv *env, jobject obj)
{
    try
    {
        LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
        LabelInfoAndroidRef *info = classInfo->getObject(env,obj);
        if (!info)
            return 0.0;

        return (*info)->fontPointSize;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::getFontPointSize()");
    }

    return 0.0;
}


extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setOutlineColor
(JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
    try
    {
        LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
        LabelInfoAndroidRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;
        (*info)->outlineColor = RGBAColor(r*255,g*255,b*255,a*255);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setOutlineColor()");
    }
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_LabelInfo_getOutlineColor
(JNIEnv *env, jobject obj)
{
    try
    {
        LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
        LabelInfoAndroidRef *info = classInfo->getObject(env,obj);
        if (!info)
            return 0;

        int outlineColor = (*info)->outlineColor.asInt();
        return outlineColor;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::getOutlineColor()");
    }

    return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setOutlineSize
(JNIEnv *env, jobject obj, jfloat outlineSize)
{
    try
    {
        LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
        LabelInfoAndroidRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;

        (*info)->outlineSize = outlineSize;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setOutlineSize()");
    }
}

extern "C"
JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_LabelInfo_getOutlineSize
(JNIEnv *env, jobject obj)
{
    try
    {
        LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
        LabelInfoAndroidRef *info = classInfo->getObject(env,obj);
        if (!info)
            return 0;

        float outlineSize = (*info)->outlineSize;
        return outlineSize;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::getOutlineSize()");
    }

    return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setShadowColor
(JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
    try
    {
        LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
        LabelInfoAndroidRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;
        (*info)->shadowColor = RGBAColor(r*255,g*255,b*255,a*255);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setShadowColor()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setShadowSize(JNIEnv *env, jobject obj, jfloat shadowSize)
{
    try
    {
        LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
        LabelInfoAndroidRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;

        (*info)->shadowSize = shadowSize;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setShadowSize()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setTextJustifyNative(JNIEnv *env, jobject obj, jint textLayout)
{
    try
    {
        LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
        LabelInfoAndroidRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;
        
        (*info)->textJustify = (WhirlyKit::TextJustify)textLayout;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setTextJustifyNativ()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setLineHeight(JNIEnv *env, jobject obj, jfloat lineSize)
{
    try
    {
        LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
        LabelInfoAndroidRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;

        (*info)->lineHeight = lineSize;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setLineHeightNative()");
    }
}

