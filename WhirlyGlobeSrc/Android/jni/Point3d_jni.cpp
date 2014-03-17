#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_Point3d.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point3d_nativeInit
  (JNIEnv *env, jclass cls)
{
	Point3dClassInfo::getClassInfo(env,cls);
}

// Construct a Java Point3d
JNIEXPORT jobject JNICALL MakePoint3d(JNIEnv *env,const Point3d &pt)
{
	Point3dClassInfo *classInfo = Point3dClassInfo::getClassInfo(env,"com/mousebird/maply/Point3d");
	Point3d *newPt = new Point3d(pt);
	return classInfo->makeWrapperObject(env,newPt);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point3d_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point3dClassInfo *classInfo = Point3dClassInfo::getClassInfo();
		Point3d *pt = new Point3d();
		classInfo->setHandle(env,obj,pt);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point3d::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point3d_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point3dClassInfo *classInfo = Point3dClassInfo::getClassInfo();
		Point3d *inst = classInfo->getObject(env,obj);
		if (!inst)
			return;
		delete inst;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point3d::dispose()");
	}
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point3d_getX
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point3dClassInfo *classInfo = Point3dClassInfo::getClassInfo();
		Point3d *pt = classInfo->getObject(env,obj);
		if (!pt)
			return 0.0;

		return pt->x();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point3d::getX()");
	}
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point3d_getY
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point3dClassInfo *classInfo = Point3dClassInfo::getClassInfo();
		Point3d *pt = classInfo->getObject(env,obj);
		if (!pt)
			return 0.0;

		return pt->y();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point3d::getY()");
	}
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point3d_getZ
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point3dClassInfo *classInfo = Point3dClassInfo::getClassInfo();
		Point3d *pt = classInfo->getObject(env,obj);
		if (!pt)
			return 0.0;

		return pt->z();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point3d::getZ()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point3d_setValue
  (JNIEnv *env, jobject obj, jdouble x, jdouble y, jdouble z)
{
	try
	{
		Point3dClassInfo *classInfo = Point3dClassInfo::getClassInfo();
		Point3d *pt = classInfo->getObject(env,obj);
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
