#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_ChangeSet.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_nativeInit
  (JNIEnv *env, jclass cls)
{
	ChangeSetClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		ChangeSet *changeSet = new ChangeSet();
		ChangeSetClassInfo::getClassInfo()->setHandle(env,obj,changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ChangeSet::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		ChangeSetClassInfo *classInfo = ChangeSetClassInfo::getClassInfo();
		ChangeSet *changeSet = classInfo->getObject(env,obj);
		if (!changeSet)
			return;

		// Be sure to delete the contents
		for (unsigned int ii=0;ii<changeSet->size();ii++)
			delete changeSet->at(ii);

		delete changeSet;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ChangeSet::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_merge
  (JNIEnv *env, jobject obj, jobject otherObj)
{
	try
	{
		ChangeSetClassInfo *classInfo = ChangeSetClassInfo::getClassInfo();
		ChangeSet *changeSet = classInfo->getObject(env,obj);
		ChangeSet *otherChangeSet = classInfo->getObject(env,otherObj);
		if (!changeSet || !otherChangeSet)
			return;
		changeSet->insert(changeSet->end(),otherChangeSet->begin(),otherChangeSet->end());
		otherChangeSet->clear();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ChangeSet::merge()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_process
  (JNIEnv *env, jobject obj, jobject sceneObj)
{
	try
	{
		ChangeSetClassInfo *classInfo = ChangeSetClassInfo::getClassInfo();
		ChangeSet *changes = classInfo->getObject(env,obj);
		MapSceneClassInfo *mapSceneClassInfo = MapSceneClassInfo::getClassInfo();
		Scene *scene = mapSceneClassInfo->getObject(env,sceneObj);
		if (!changes || !scene)
			return;

		// Note: Porting Should be using the view
		WhirlyKitGLSetupInfo glSetupInfo;
		glSetupInfo.minZres = 0.0001;

	    bool requiresFlush = false;
	    // Set up anything that needs to be set up
	    ChangeSet changesToAdd;
	    for (unsigned int ii=0;ii<changes->size();ii++)
	    {
	        ChangeRequest *change = changes->at(ii);
	        if (change)
	        {
	            requiresFlush |= change->needsFlush();
	            change->setupGL(&glSetupInfo, scene->getMemManager());
	            changesToAdd.push_back(change);
	        } else
	            // A NULL change request is just a flush request
	            requiresFlush = true;
	    }

	    // If anything needed a flush after that, let's do it
	    if (requiresFlush)
	    {
	        glFlush();
	    }

	    scene->addChangeRequests(changesToAdd);
	    changes->clear();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ChangeSet::process()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_addTexture
  (JNIEnv *env, jobject obj, jobject texObj)
{
	try
	{
		ChangeSetClassInfo *classInfo = ChangeSetClassInfo::getClassInfo();
		ChangeSet *changeSet = classInfo->getObject(env,obj);
		Texture *texture = TextureClassInfo::getClassInfo()->getObject(env,texObj);
		if (!changeSet || !texture)
			return;

		// We take control of the Texture * as soon as it goes into the change set
		TextureClassInfo::getClassInfo()->clearHandle(env,texObj);
		changeSet->push_back(new AddTextureReq(texture));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ChangeSet::addTexture()");
	}
}

/*
 * Class:     com_mousebird_maply_ChangeSet
 * Method:    removeTexture
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_removeTexture
  (JNIEnv *env, jobject obj, jlong texID)
{
	try
	{
		ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,obj);
		if (!changeSet)
			return;

		changeSet->push_back(new RemTextureReq(texID));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ChangeSet::removeTexture()");
	}
}
