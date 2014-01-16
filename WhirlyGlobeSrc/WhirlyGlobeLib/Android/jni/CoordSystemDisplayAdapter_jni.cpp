#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_CoordSystemDisplayAdapter.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_CoordSystemDisplayAdapter_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		// Note: Porting.  Just a spherical mercator display adapter for now
		CoordSystemDisplayAdapter *coordAdapter = coordAdapter = new SphericalMercatorDisplayAdapter(0.0, GeoCoord::CoordFromDegrees(-180.0,-90.0), GeoCoord::CoordFromDegrees(180.0,90.0));
		setHandle(env,obj,coordAdapter);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystemDisplayAdapter::initialise()");
	}
}

/*
 * Class:     com_mousebirdconsulting_maply_CoordSystemDisplayAdapter
 * Method:    dispose
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_CoordSystemDisplayAdapter_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		CoordSystemDisplayAdapter *coordAdapter = getHandle<CoordSystemDisplayAdapter>(env,obj);
		if (!coordAdapter)
			return;
		delete coordAdapter;

		clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystemDisplayAdapter::dispose()");
	}
}
