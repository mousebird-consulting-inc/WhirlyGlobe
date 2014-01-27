#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_CoordSystem.h"
#import "Maply_utils_jni.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_CoordSystem_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "CoordSystem::initialise() called.  This is a base class.  Oops.");
		clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystem::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_CoordSystem_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		CoordSystem *coordSys = getHandle<CoordSystem>(env,obj);
		if (!coordSys)
			return;

		delete coordSys;

		clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystem::dispose()");
	}
}

JNIEXPORT jobject JNICALL Java_com_mousebirdconsulting_maply_CoordSystem_geographicToLocal
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		CoordSystem *coordSys = getHandle<CoordSystem>(env,obj);
		Point3d *pt = getHandle<Point3d>(env,ptObj);
		if (!coordSys || !pt)
			return NULL;

		Point3d newPt = coordSys->geographicToLocal3d(GeoCoord(pt->x(),pt->y()));
		return MakePoint3d(env,newPt);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystem::geographicToLocal()");
	}
}

JNIEXPORT jobject JNICALL Java_com_mousebirdconsulting_maply_CoordSystem_localToGeographic
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		CoordSystem *coordSys = getHandle<CoordSystem>(env,obj);
		Point3d *pt = getHandle<Point3d>(env,ptObj);
		if (!coordSys || !pt)
			return NULL;

		GeoCoord newCoord = coordSys->localToGeographic(*pt);
		return MakePoint3d(env,Point3d(newCoord.x(),newCoord.y(),0.0));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystem::localToGeographic()");
	}
}
