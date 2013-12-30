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
  (JNIEnv *env, jobject obj)
{
	// Note: Porting.  Share this
	CoordSystemDisplayAdapter *coordAdapter = coordAdapter = new SphericalMercatorDisplayAdapter(0.0, GeoCoord::CoordFromDegrees(-180.0,-90.0), GeoCoord::CoordFromDegrees(180.0,90.0));
	Maply::MapView *inst = new Maply::MapView(coordAdapter);
	setHandle(env,obj,inst);
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MapView_dispose
  (JNIEnv *env, jobject obj)
{
	Maply::MapView *inst = getHandle<Maply::MapView>(env,obj);
	if (!inst)
		return;
	delete inst;
	inst = NULL;

	// Note: Porting.  Should tear down coord adapter

	setHandle(env,obj,inst);
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MapView_setLoc
  (JNIEnv *env, jobject obj, jdouble x, jdouble y, jdouble z)
{
	Maply::MapView *view = getHandle<Maply::MapView>(env,obj);
	if (!view)
		return;

	view->setLoc(Point3d(x,y,z));
}
