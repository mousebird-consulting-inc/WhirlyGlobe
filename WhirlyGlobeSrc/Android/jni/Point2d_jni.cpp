#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_Point2d.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point2d_nativeInit
  (JNIEnv *env, jclass cls)
{
	Point2dClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point2d_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point2d *pt = new Point2d();
		Point2dClassInfo::getClassInfo()->setHandle(env,obj,pt);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point2d::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point2d_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point2dClassInfo *classInfo = Point2dClassInfo::getClassInfo();
		Point2d *inst = classInfo->getObject(env,obj);
		if (!inst)
			return;
		delete inst;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point2d::dispose()");
	}
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point2d_getX
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point2dClassInfo *classInfo = Point2dClassInfo::getClassInfo();
		Point2d *pt = classInfo->getObject(env,obj);
		if (!pt)
			return 0.0;

		return pt->x();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point2d::getX()");
	}
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point2d_getY
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point2dClassInfo *classInfo = Point2dClassInfo::getClassInfo();
		Point2d *pt = classInfo->getObject(env,obj);
		if (!pt)
			return 0.0;

		return pt->y();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point2d::getY()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point2d_setValue
  (JNIEnv *env, jobject obj, jdouble x, jdouble y)
{
	try
	{
		Point2dClassInfo *classInfo = Point2dClassInfo::getClassInfo();
		Point2d *pt = classInfo->getObject(env,obj);
		if (!pt)
			return;
		pt->x() = x;
		pt->y() = y;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point2d::setValue()");
	}
}
