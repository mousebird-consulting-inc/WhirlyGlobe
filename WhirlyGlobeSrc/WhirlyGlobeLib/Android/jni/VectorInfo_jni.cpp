#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_VectorInfo.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorInfo_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorInfo *vecInfo = new VectorInfo();
		setHandle(env,obj,vecInfo);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorInfo_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorInfo *vecInfo = getHandle<VectorInfo>(env,obj);
		if (!vecInfo)
			return;
		delete vecInfo;
		vecInfo = NULL;
		setHandle(env,obj,vecInfo);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorInfo_setEnable
  (JNIEnv *env, jobject obj, jboolean bVal)
{
	try
	{
		VectorInfo *vecInfo = getHandle<VectorInfo>(env,obj);
		if (!vecInfo)
			return;
		vecInfo->enable = bVal;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setEnable()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorInfo_setDrawOffset
  (JNIEnv *env, jobject obj, jfloat fVal)
{
	try
	{
		VectorInfo *vecInfo = getHandle<VectorInfo>(env,obj);
		if (!vecInfo)
			return;
		vecInfo->drawOffset = fVal;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setDrawOffset()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorInfo_setDrawPriority
  (JNIEnv *env, jobject obj, jint val)
{
	try
	{
		VectorInfo *vecInfo = getHandle<VectorInfo>(env,obj);
		if (!vecInfo)
			return;
		vecInfo->priority = val;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setDrawPriority()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorInfo_setMinVis
  (JNIEnv *env, jobject obj, jfloat fVal)
{
	try
	{
		VectorInfo *vecInfo = getHandle<VectorInfo>(env,obj);
		if (!vecInfo)
			return;
		vecInfo->minVis = fVal;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setMinVis()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorInfo_setMaxVis
  (JNIEnv *env, jobject obj, jfloat fVal)
{
	try
	{
		VectorInfo *vecInfo = getHandle<VectorInfo>(env,obj);
		if (!vecInfo)
			return;
		vecInfo->maxVis = fVal;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setMaxVis()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorInfo_setFilled
  (JNIEnv *env, jobject obj, jboolean bVal)
{
	try
	{
		VectorInfo *vecInfo = getHandle<VectorInfo>(env,obj);
		if (!vecInfo)
			return;
		vecInfo->filled = bVal;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setFilled()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorInfo_setTexId
  (JNIEnv *env, jobject obj, jlong val)
{
	try
	{
		VectorInfo *vecInfo = getHandle<VectorInfo>(env,obj);
		if (!vecInfo)
			return;
		vecInfo->texId = val;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setTexId()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorInfo_setTexScale
  (JNIEnv *env, jobject obj, jfloat s, jfloat t)
{
	try
	{
		VectorInfo *vecInfo = getHandle<VectorInfo>(env,obj);
		if (!vecInfo)
			return;
		vecInfo->texScale.x() = s;
		vecInfo->texScale.y() = t;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setEnable()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorInfo_subdivEps
  (JNIEnv *env, jobject obj, jfloat eps)
{
	try
	{
		VectorInfo *vecInfo = getHandle<VectorInfo>(env,obj);
		if (!vecInfo)
			return;
		vecInfo->subdivEps = eps;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::subdivEps()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorInfo_setGridSubdiv
  (JNIEnv *env, jobject obj, jboolean bVal)
{
	try
	{
		VectorInfo *vecInfo = getHandle<VectorInfo>(env,obj);
		if (!vecInfo)
			return;
		vecInfo->gridSubdiv = bVal;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setGridSubdiv()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorInfo_setColor
  (JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
	try
	{
		VectorInfo *vecInfo = getHandle<VectorInfo>(env,obj);
		if (!vecInfo)
			return;
		vecInfo->color = RGBAColor(r*255.0,g*255.0,b*255.0,a*255.0);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setColor()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorInfo_setFade
  (JNIEnv *env, jobject obj, jfloat val)
{
	try
	{
		VectorInfo *vecInfo = getHandle<VectorInfo>(env,obj);
		if (!vecInfo)
			return;
		vecInfo->fade = val;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setFade()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorInfo_setLineWidth
  (JNIEnv *env, jobject obj, jfloat val)
{
	try
	{
		VectorInfo *vecInfo = getHandle<VectorInfo>(env,obj);
		if (!vecInfo)
			return;
		vecInfo->lineWidth = val;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setLineWidth()");
	}
}
