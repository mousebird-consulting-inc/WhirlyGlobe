#import <jni.h>
#import <android/bitmap.h>
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

JNIEXPORT jboolean JNICALL Java_com_mousebirdconsulting_maply_Texture_setBitmap
  (JNIEnv *env, jobject obj, jobject bitmapObj)
{
	try
	{
		Texture *tex = getHandle<Texture>(env,obj);
		if (!tex)
			return false;

		AndroidBitmapInfo info;
		if (AndroidBitmap_getInfo(env, bitmapObj, &info) < 0)
		{
		    return false;
	    }
		if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
		{
			__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Only dealing with 8888 bitmaps in Texture::setBitmap()");
		    return false;
	    }
		// Copy the raw data over to the texture
		void* bitmapPixels;
		if (AndroidBitmap_lockPixels(env, bitmapObj, &bitmapPixels) < 0)
	    {
		    return false;
	    }

		uint32_t* src = (uint32_t*) bitmapPixels;
		MutableRawData *rawData = new MutableRawData(bitmapPixels,info.height*info.width*4);
		tex->setRawData(rawData,info.width,info.height);
		AndroidBitmap_unlockPixels(env, bitmapObj);

		return true;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Texture::setBitmap()");
	}

	return false;
}

JNIEXPORT jlong JNICALL Java_com_mousebirdconsulting_maply_Texture_getID
  (JNIEnv *env, jobject obj)
{
	try
	{
		Texture *tex = getHandle<Texture>(env,obj);
		if (!tex)
			return EmptyIdentity;

		return tex->getId();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Texture::getID()");
	}
}
