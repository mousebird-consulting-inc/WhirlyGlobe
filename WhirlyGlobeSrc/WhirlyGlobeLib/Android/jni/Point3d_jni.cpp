#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_Point3d.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;

// Construct a Java Point3d
JNIEXPORT jobject JNICALL MakePoint3d(JNIEnv *env,const Point3d &pt)
{
	// Make a Java Point3d
	jclass cls = env->FindClass("com/mousebirdconsulting/maply/Point3d");
	jmethodID methodID = env->GetMethodID(cls, "<init>", "(DDD)V");
	if (!methodID)
		throw 1;
	jobject jPt = env->NewObject(cls, methodID,pt.x(),pt.y(),pt.z());
	Point3d *newPt = new Point3d(pt);
	setHandle(env,jPt,newPt);

	return jPt;
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_Point3d_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point3d *pt = new Point3d();
		setHandle(env,obj,pt);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point3d::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_Point3d_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point3d *inst = getHandle<Point3d>(env,obj);
		if (!inst)
			return;
		delete inst;

		clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point3d::dispose()");
	}
}

JNIEXPORT jdouble JNICALL Java_com_mousebirdconsulting_maply_Point3d_getX
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point3d *pt = getHandle<Point3d>(env,obj);
		if (!pt)
			return 0.0;

		return pt->x();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point3d::getX()");
	}
}

JNIEXPORT jdouble JNICALL Java_com_mousebirdconsulting_maply_Point3d_getY
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point3d *pt = getHandle<Point3d>(env,obj);
		if (!pt)
			return 0.0;

		return pt->y();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point3d::getY()");
	}
}

JNIEXPORT jdouble JNICALL Java_com_mousebirdconsulting_maply_Point3d_getZ
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point3d *pt = getHandle<Point3d>(env,obj);
		if (!pt)
			return 0.0;

		return pt->z();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point3d::getZ()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_Point3d_setValue
  (JNIEnv *env, jobject obj, jdouble x, jdouble y, jdouble z)
{
	try
	{
		Point3d *pt = getHandle<Point3d>(env,obj);
		if (!pt)
			return;
		pt->x() = x;
		pt->y() = y;
		pt->z() = z;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point3d::setValue()");
	}
}
