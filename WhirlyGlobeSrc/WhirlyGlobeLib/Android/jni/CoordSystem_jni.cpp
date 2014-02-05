#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebirdconsulting_maply_CoordSystem.h"
#import "Maply_utils_jni.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_CoordSystem_nativeInit
  (JNIEnv *env, jclass cls)
{
	CoordSystemClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_CoordSystem_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "CoordSystem::initialise() called.  This is a base class.  Oops.");
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
		CoordSystemClassInfo *classInfo = CoordSystemClassInfo::getClassInfo();
		CoordSystem *coordSys = classInfo->getObject(env,obj);
		if (!coordSys)
			return;

		delete coordSys;

		classInfo->clearHandle(env,obj);
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
		CoordSystemClassInfo *classInfo = CoordSystemClassInfo::getClassInfo();
		CoordSystem *coordSys = classInfo->getObject(env,obj);
		Point3d *pt = Point3dClassInfo::getClassInfo()->getObject(env,ptObj);
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
		CoordSystem *coordSys = CoordSystemClassInfo::getClassInfo()->getObject(env,obj);
		Point3d *pt = Point3dClassInfo::getClassInfo()->getObject(env,ptObj);
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
