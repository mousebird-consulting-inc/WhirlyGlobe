#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_ChangeSet.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_ChangeSet_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		ChangeSet *changeSet = new ChangeSet();
		setHandle(env,obj,changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ChangeSet::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_ChangeSet_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		ChangeSet *changeSet = getHandle<ChangeSet>(env,obj);
		if (!changeSet)
			return;

		// Be sure to delete the contents
		for (unsigned int ii=0;ii<changeSet->size();ii++)
			delete changeSet->at(ii);

		delete changeSet;

		clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ChangeSet::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_ChangeSet_addTexture
  (JNIEnv *env, jobject obj, jobject texObj)
{
	try
	{
		ChangeSet *changeSet = getHandle<ChangeSet>(env,obj);
		Texture *texture = getHandle<Texture>(env,texObj);
		if (!changeSet || !texture)
			return;

		// We take control of the Texture * as soon as it goes into the change set
		clearHandle(env,texObj);
		changeSet->push_back(new AddTextureReq(texture));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ChangeSet::addTexture()");
	}
}

/*
 * Class:     com_mousebirdconsulting_maply_ChangeSet
 * Method:    removeTexture
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_ChangeSet_removeTexture
  (JNIEnv *env, jobject obj, jlong texID)
{
	try
	{
		ChangeSet *changeSet = getHandle<ChangeSet>(env,obj);
		if (!changeSet)
			return;

		changeSet->push_back(new RemTextureReq(texID));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ChangeSet::removeTexture()");
	}
}
