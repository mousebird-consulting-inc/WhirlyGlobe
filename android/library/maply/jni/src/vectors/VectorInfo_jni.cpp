/*  VectorInfo_jni.cpp
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

#import "Vectors_jni.h"
#import "com_mousebird_maply_VectorInfo.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> VectorInfoClassInfo *VectorInfoClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_nativeInit
  (JNIEnv *env, jclass cls)
{
	VectorInfoClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorInfoClassInfo::set(env, obj, new VectorInfoRef(std::make_shared<VectorInfo>()));
	}
	MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
		std::lock_guard<std::mutex> lock(disposeMutex);
		delete classInfo->getObject(env,obj);
		classInfo->clearHandle(env,obj);
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setFilled
  (JNIEnv *env, jobject obj, jboolean bVal)
{
	try
	{
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			(*vecInfo)->filled = bVal;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorInfo_getFilled
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			return (*vecInfo)->filled;
		}
	}
	MAPLY_STD_JNI_CATCH()
	return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setSampleEpsilon
  (JNIEnv *env, jobject obj, jdouble sample)
{
	try
	{
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			(*vecInfo)->sample = sample;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorInfo_getSampleEpsilon
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			return (*vecInfo)->sample;
		}
	}
	MAPLY_STD_JNI_CATCH()
	return 0.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setTextureID
  (JNIEnv *env, jobject obj, jlong texID)
{
	try
	{
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			(*vecInfo)->texId = texID;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_VectorInfo_getTextureID
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			return (*vecInfo)->texId;
		}
	}
	MAPLY_STD_JNI_CATCH()
	return EmptyIdentity;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setTexScale
  (JNIEnv *env, jobject obj, jdouble s, jdouble t)
{
	try
	{
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			(*vecInfo)->texScale = {s,t};
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_VectorInfo_getTexScaleX
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			return (*vecInfo)->texScale.x();
		}
	}
	MAPLY_STD_JNI_CATCH()
	return 0.0;
}

extern "C"
JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_VectorInfo_getTexScaleY
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			return (*vecInfo)->texScale.y();
		}
	}
	MAPLY_STD_JNI_CATCH()
	return 0.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setSubdivEps
  (JNIEnv *env, jobject obj, jdouble subdiv)
{
	try
	{
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			(*vecInfo)->subdivEps = subdiv;
			(*vecInfo)->gridSubdiv = true;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorInfo_getSubdivEps
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			return (*vecInfo)->subdivEps;
		}
	}
	MAPLY_STD_JNI_CATCH()
	return 0.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setTextureProjectionNative
  (JNIEnv *env, jobject obj, jint texProj)
{
    try
    {
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			(*vecInfo)->texProj = (TextureProjections)texProj;
		}
    }
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_VectorInfo_getTextureProjectionNative
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			return (*vecInfo)->texProj;
		}
	}
	MAPLY_STD_JNI_CATCH()
	return 0;
}


extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setColorInt
  (JNIEnv *env, jobject obj, jint r, jint g, jint b, jint a)
{
	try
	{
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			(*vecInfo)->color = RGBAColor(r,g,b,a);
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setColor
		(JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
	Java_com_mousebird_maply_VectorInfo_setColorInt(env, obj,
		(jint)(r*255.0f),(jint)(g*255.0f),(jint)(b*255.0f),(jint)(a*255.0f));
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_VectorInfo_getColor
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			return (*vecInfo)->color.asARGBInt();
		}
	}
	MAPLY_STD_JNI_CATCH()
	return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setLineWidth
  (JNIEnv *env, jobject obj, jfloat val)
{
	try
	{
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			(*vecInfo)->lineWidth = val;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_VectorInfo_getLineWidth
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			return (*vecInfo)->lineWidth;
		}
	}
	MAPLY_STD_JNI_CATCH()
	return 0.0f;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setUseCenterNative
  (JNIEnv *env, jobject obj, jboolean use)
{
	try
	{
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			(*vecInfo)->centered = use;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorInfo_getUseCenter
		(JNIEnv *env, jobject obj)
{
	try
	{
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			return (*vecInfo)->centered;
		}
	}
	MAPLY_STD_JNI_CATCH()
	return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setVecCenterNative
  (JNIEnv *env, jobject obj, jdouble centerX, jdouble centerY)
{
    try
    {
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			(*vecInfo)->centered = true;
			(*vecInfo)->vecCenterSet = true;
			(*vecInfo)->vecCenter = Point2f(centerX, centerY);
		}
    }
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorInfo_getVecCenterX
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			return (*vecInfo)->vecCenter.x();
		}
	}
	MAPLY_STD_JNI_CATCH()
	return 0.0;
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorInfo_getVecCenterY
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			return (*vecInfo)->vecCenter.y();
		}
	}
	MAPLY_STD_JNI_CATCH()
	return 0.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setSelectable
  (JNIEnv *env, jobject obj, jboolean enable)
{
	try
	{
		if (const auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			(*vecInfo)->selectable = enable;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorInfo_getSelectable
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (const auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			return (*vecInfo)->selectable;
		}
	}
	MAPLY_STD_JNI_CATCH()
	return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setCloseAreals
  (JNIEnv *env, jobject obj, jboolean close)
{
	try
	{
		if (const auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			(*vecInfo)->closeAreals = close;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorInfo_getCloseAreals
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (const auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			return (*vecInfo)->closeAreals;
		}
	}
	MAPLY_STD_JNI_CATCH()
	return false;
}

extern "C"
JNIEXPORT jstring JNICALL Java_com_mousebird_maply_VectorInfo_toString
  (JNIEnv *env, jobject obj)
{
    try
    {
		if (auto vecInfo = VectorInfoClassInfo::get(env,obj))
		{
			return env->NewStringUTF((*vecInfo)->toString().c_str());
		}
    }
	MAPLY_STD_JNI_CATCH()
    return nullptr;
}
