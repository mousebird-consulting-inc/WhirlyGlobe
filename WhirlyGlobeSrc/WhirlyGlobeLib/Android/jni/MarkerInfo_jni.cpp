#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_MarkerInfo.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MarkerInfo_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		MarkerInfo *info = new MarkerInfo();
		// Note: Porting
		info->screenObject = true;
		setHandle(env,obj,info);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerInfo::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MarkerInfo_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		MarkerInfo *info = getHandle<MarkerInfo>(env,obj);
		if (!info)
			return;
		delete info;

		clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerInfo::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MarkerInfo_setEnable
  (JNIEnv *env, jobject obj, jboolean enable)
{
	try
	{
		MarkerInfo *info = getHandle<MarkerInfo>(env,obj);
		if (!info)
			return;
		info->enable = enable;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerInfo::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MarkerInfo_setDrawOffset
  (JNIEnv *env, jobject obj, jfloat drawOffset)
{
	try
	{
		MarkerInfo *info = getHandle<MarkerInfo>(env,obj);
		if (!info)
			return;
		info->drawOffset = drawOffset;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerInfo::setDrawOffset()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MarkerInfo_setDrawPriority
  (JNIEnv *env, jobject obj, jint drawPriority)
{
	try
	{
		MarkerInfo *info = getHandle<MarkerInfo>(env,obj);
		if (!info)
			return;
		info->drawPriority = drawPriority;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerInfo::setDrawPriority()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MarkerInfo_setMinVis
  (JNIEnv *env, jobject obj, jfloat minVis)
{
	try
	{
		MarkerInfo *info = getHandle<MarkerInfo>(env,obj);
		if (!info)
			return;
		info->minVis = minVis;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerInfo::setMinViz()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MarkerInfo_setMaxVis
  (JNIEnv *env, jobject obj, jfloat maxVis)
{
	try
	{
		MarkerInfo *info = getHandle<MarkerInfo>(env,obj);
		if (!info)
			return;
		info->maxVis = maxVis;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerInfo::setMaxVis()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MarkerInfo_setColor
  (JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
	try
	{
		MarkerInfo *info = getHandle<MarkerInfo>(env,obj);
		if (!info)
			return;
		info->color = RGBAColor(r*255,g*255,b*255,a*255);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerInfo::setColor()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MarkerInfo_setFade
  (JNIEnv *env, jobject obj, jfloat fade)
{
	try
	{
		MarkerInfo *info = getHandle<MarkerInfo>(env,obj);
		if (!info)
			return;
		info->fade = fade;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerInfo::setFade()");
	}
}
