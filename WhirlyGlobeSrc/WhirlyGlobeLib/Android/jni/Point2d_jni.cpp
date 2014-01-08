#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_Point2d.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_Point2d_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point2d *pt = new Point2d();
		setHandle(env,obj,pt);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point2d::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_Point2d_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point2d *inst = getHandle<Point2d>(env,obj);
		if (!inst)
			return;
		delete inst;

		clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point2d::dispose()");
	}
}

JNIEXPORT jdouble JNICALL Java_com_mousebirdconsulting_maply_Point2d_getX
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point2d *pt = getHandle<Point2d>(env,obj);
		if (!pt)
			return 0.0;

		return pt->x();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point2d::getX()");
	}
}

JNIEXPORT jdouble JNICALL Java_com_mousebirdconsulting_maply_Point2d_getY
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point2d *pt = getHandle<Point2d>(env,obj);
		if (!pt)
			return 0.0;

		return pt->y();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point2d::getY()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_Point2d_setValue
  (JNIEnv *env, jobject obj, jdouble x, jdouble y)
{
	try
	{
		Point2d *pt = getHandle<Point2d>(env,obj);
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
