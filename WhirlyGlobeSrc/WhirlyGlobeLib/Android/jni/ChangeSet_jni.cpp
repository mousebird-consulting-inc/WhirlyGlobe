#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebirdconsulting_maply_ChangeSet.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_ChangeSet_nativeInit
  (JNIEnv *env, jclass cls)
{
	ChangeSetClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_ChangeSet_initialise
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

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_ChangeSet_dispose
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

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_ChangeSet_merge
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

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_ChangeSet_addTexture
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
 * Class:     com_mousebirdconsulting_maply_ChangeSet
 * Method:    removeTexture
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_ChangeSet_removeTexture
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
