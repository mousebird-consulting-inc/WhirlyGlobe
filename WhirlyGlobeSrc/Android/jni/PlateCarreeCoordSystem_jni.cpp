#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_PlateCarreeCoordSystem.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_PlateCarreeCoordSystem_nativeInit
  (JNIEnv *env, jclass cls)
{
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SphericalMercatorCoordSystem_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		SphericalMercatorCoordSystem *coordSystem = new SphericalMercatorCoordSystem();
		CoordSystemClassInfo::getClassInfo()->setHandle(env,obj,coordSystem);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalMercatorCoordSystem::initialise()");
	}
}
