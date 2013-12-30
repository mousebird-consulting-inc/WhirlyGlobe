/*
 * MapScene_jni.cpp
 *
 *  Created on: Dec 19, 2013
 *      Author: sjg
 */



#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_MapScene.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MapScene_initialise
  (JNIEnv *env, jobject obj)
{
	// Note: Porting.  This will leak
	CoordSystemDisplayAdapter *coordAdapter = coordAdapter = new SphericalMercatorDisplayAdapter(0.0, GeoCoord::CoordFromDegrees(-180.0,-90.0), GeoCoord::CoordFromDegrees(180.0,90.0));
	Maply::MapScene *inst = new Maply::MapScene(coordAdapter);
	setHandle(env,obj,inst);
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MapScene_dispose
  (JNIEnv *env, jobject obj)
{
	Maply::MapScene *inst = getHandle<Maply::MapScene>(env,obj);
	if (!inst)
		return;
	delete inst;
	inst = NULL;

	// Note: Porting.  Should tear down coord adapter

	setHandle(env,obj,inst);
}
