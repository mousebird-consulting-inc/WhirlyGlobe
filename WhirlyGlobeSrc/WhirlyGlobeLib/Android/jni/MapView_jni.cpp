/*
 * MapView_jni.cpp
 *
 *  Created on: Dec 19, 2013
 *      Author: sjg
 */


#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_MapView.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MapView_initialise
  (JNIEnv *env, jobject obj, jobject coordAdapterObj)
{
	try
	{
		CoordSystemDisplayAdapter *coordAdapter = getHandle<CoordSystemDisplayAdapter>(env,coordAdapterObj);
		Maply::MapView *inst = new Maply::MapView(coordAdapter);
		setHandle(env,obj,inst);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapView::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MapView_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		Maply::MapView *inst = getHandle<Maply::MapView>(env,obj);
		if (!inst)
			return;
		delete inst;
		inst = NULL;

		setHandle(env,obj,inst);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapView::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MapView_setLoc
  (JNIEnv *env, jobject obj, jdouble x, jdouble y, jdouble z)
{
	try
	{
		Maply::MapView *view = getHandle<Maply::MapView>(env,obj);
		if (!view)
			return;

		view->setLoc(Point3d(x,y,z));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapView::setLoc()");
	}
}
