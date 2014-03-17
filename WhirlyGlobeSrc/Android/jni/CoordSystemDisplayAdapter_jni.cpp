#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_CoordSystemDisplayAdapter.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_CoordSystemDisplayAdapter_nativeInit
  (JNIEnv *env, jclass cls)
{
	CoordSystemDisplayAdapterInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_CoordSystemDisplayAdapter_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		CoordSystemDisplayAdapterInfo *classInfo = CoordSystemDisplayAdapterInfo::getClassInfo();
		// Note: Porting.  Just a spherical mercator display adapter for now
		CoordSystemDisplayAdapter *coordAdapter = coordAdapter = new SphericalMercatorDisplayAdapter(0.0, GeoCoord::CoordFromDegrees(-180.0,-90.0), GeoCoord::CoordFromDegrees(180.0,90.0));
		classInfo->setHandle(env,obj,coordAdapter);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystemDisplayAdapter::initialise()");
	}
}

/*
 * Class:     com_mousebird_maply_CoordSystemDisplayAdapter
 * Method:    dispose
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_CoordSystemDisplayAdapter_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		CoordSystemDisplayAdapterInfo *classInfo = CoordSystemDisplayAdapterInfo::getClassInfo();
		CoordSystemDisplayAdapter *coordAdapter = classInfo->getObject(env,obj);
		if (!coordAdapter)
			return;
		delete coordAdapter;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystemDisplayAdapter::dispose()");
	}
}
