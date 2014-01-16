#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_Texture.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_Texture_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		Texture *tex = new Texture("jni");
		setHandle(env,obj,tex);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Texture::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_Texture_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		Texture *tex = getHandle<Texture>(env,obj);
		if (!tex)
			return;
		delete tex;

		clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Texture::dispose()");
	}
}

