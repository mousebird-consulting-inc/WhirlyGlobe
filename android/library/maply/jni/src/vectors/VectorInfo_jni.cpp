/*
 *  VectorInfo_jni.cpp
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

#import "Vectors_jni.h"
#import "com_mousebird_maply_VectorInfo.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> VectorInfoClassInfo *VectorInfoClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_nativeInit
  (JNIEnv *env, jclass cls)
{
	VectorInfoClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorInfoRef *vecInfo = new VectorInfoRef(new VectorInfo());
		VectorInfoClassInfo::getClassInfo()->setHandle(env,obj,vecInfo);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::initialise()");
	}
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
			VectorInfoRef *vecInfo = classInfo->getObject(env,obj);
            if (!vecInfo)
                return;
            delete vecInfo;

            classInfo->clearHandle(env,obj);
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setFilled
  (JNIEnv *env, jobject obj, jboolean bVal)
{
	try
	{
		VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
		VectorInfoRef *vecInfo = classInfo->getObject(env,obj);
		if (!vecInfo)
			return;
		(*vecInfo)->filled = bVal;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setFilled()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setSampleEpsilon
  (JNIEnv *env, jobject obj, jdouble sample)
{
	try
	{
		VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
		VectorInfoRef *vecInfo = classInfo->getObject(env,obj);
		if (!vecInfo)
			return;
		(*vecInfo)->sample = sample;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setSampleEpsilon()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setTextureID
  (JNIEnv *env, jobject obj, jlong texID)
{
	try
	{
		VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
		VectorInfoRef *vecInfo = classInfo->getObject(env,obj);
		if (!vecInfo)
			return;
		(*vecInfo)->texId = texID;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setTextureID()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setTexScale
  (JNIEnv *env, jobject obj, jdouble s, jdouble t)
{
	try
	{
		VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
		VectorInfoRef *vecInfo = classInfo->getObject(env,obj);
		if (!vecInfo)
			return;
		(*vecInfo)->texScale.x() = s;
		(*vecInfo)->texScale.y() = t;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setTexScale()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setSubdivEps
  (JNIEnv *env, jobject obj, jdouble subdiv)
{
	try
	{
		VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
		VectorInfoRef *vecInfo = classInfo->getObject(env,obj);
		if (!vecInfo)
			return;
		(*vecInfo)->subdivEps = subdiv;
		(*vecInfo)->gridSubdiv = true;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setSubdivEps()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setTextureProjectionNative
(JNIEnv *env, jobject obj, jint texProj)
{
    try
    {
        VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
		VectorInfoRef *vecInfo = classInfo->getObject(env,obj);
        if (!vecInfo)
            return;
		(*vecInfo)->texProj = (TextureProjections)texProj;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setTextureProjectionNative()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setColor
  (JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
	try
	{
		VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
		VectorInfoRef *vecInfo = classInfo->getObject(env,obj);
		if (!vecInfo)
			return;
		(*vecInfo)->color = RGBAColor(r*255.0,g*255.0,b*255.0,a*255.0);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setColor()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setLineWidth
  (JNIEnv *env, jobject obj, jfloat val)
{
	try
	{
		VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
		VectorInfoRef *vecInfo = classInfo->getObject(env,obj);
		if (!vecInfo)
			return;
		(*vecInfo)->lineWidth = val;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setLineWidth()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setUseCenter
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
		VectorInfoRef *vecInfo = classInfo->getObject(env,obj);
		if (!vecInfo)
			return;
		(*vecInfo)->centered = true;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setUseCenter()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setVecCenterNative
(JNIEnv *env, jobject obj, jdouble centerX, jdouble centerY)
{
    try
    {
        VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
		VectorInfoRef *vecInfo = classInfo->getObject(env,obj);
        if (!vecInfo)
            return;
		(*vecInfo)->centered = true;
		(*vecInfo)->vecCenterSet = true;
		(*vecInfo)->vecCenter = Point2f(centerX,centerY);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setVecCenterNative()");
    }
}

JNIEXPORT jstring JNICALL Java_com_mousebird_maply_VectorInfo_toString
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
		VectorInfoRef *vecInfo = classInfo->getObject(env,obj);
        if (!vecInfo)
            return NULL;

        std::string outStr = (*vecInfo)->toString();
        return env->NewStringUTF(outStr.c_str());
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::toString()");
    }

    return NULL;
}
