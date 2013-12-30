/*
 * VectorManager_jni.cpp
 *
 *  Created on: Dec 19, 2013
 *      Author: sjg
 */




#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_VectorManager.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;

// Note: Bogus!
static MapScene *mapScene = NULL;

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorManager_initialise
  (JNIEnv *env, jobject obj, jobject sceneObj)
{
	MapScene *scene = getHandle<MapScene>(env,sceneObj);
	mapScene = scene;
	VectorManager *vecManager = dynamic_cast<VectorManager *>(scene->getManager(kWKVectorManager));
	setHandle(env,obj,vecManager);
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorManager_dispose
  (JNIEnv *env, jobject obj)
{
	VectorManager *inst = NULL;
	setHandle(env,obj,inst);
}

JNIEXPORT jlong JNICALL Java_com_mousebirdconsulting_maply_VectorManager_addVector
  (JNIEnv *env, jobject obj, jobject vecObj)
{
	VectorManager *vecManager = getHandle<VectorManager>(env,obj);
	VectorObject *vector = getHandle<VectorObject>(env,vecObj);
	if (!vecManager || !vector)
		return EmptyIdentity;

	// Note: This needs to come from outside
	VectorInfo desc;

	// Note: Need to hand these back
	ChangeSet changes;
	SimpleIdentity vecId = vecManager->addVectors(&vector->shapes,desc,changes);

	// Note: Shouldn't do this here
	mapScene->addChangeRequests(changes);

	return vecId;
}
