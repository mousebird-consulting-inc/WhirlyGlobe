#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_LabelInfo.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

template<> LabelInfoClassInfo *LabelInfoClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_nativeInit
  (JNIEnv *env, jclass cls)
{
	LabelInfoClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		LabelInfo *info = new LabelInfo();
		// Note: Porting
		info->screenObject = true;
		LabelInfoClassInfo::getClassInfo()->setHandle(env,obj,info);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		delete info;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setEnable
  (JNIEnv *env, jobject obj, jboolean enable)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->enable = enable;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setDrawOffset
  (JNIEnv *env, jobject obj, jfloat drawOffset)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->drawOffset = drawOffset;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setDrawOffset()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setDrawPriority
  (JNIEnv *env, jobject obj, jint drawPriority)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->drawPriority = drawPriority;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setDrawPriority()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setMinVis
  (JNIEnv *env, jobject obj, jfloat minVis)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->minVis = minVis;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setMinViz()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setMaxVis
  (JNIEnv *env, jobject obj, jfloat maxVis)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->maxVis = maxVis;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setMaxVis()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setTextColor
  (JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->textColor = RGBAColor(r*255,g*255,b*255,a*255);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setColor()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setBackgroundColor
  (JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->backColor = RGBAColor(r*255,g*255,b*255,a*255);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setColor()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setFade
  (JNIEnv *env, jobject obj, jfloat fade)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->fade = fade;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setFade()");
	}
}
