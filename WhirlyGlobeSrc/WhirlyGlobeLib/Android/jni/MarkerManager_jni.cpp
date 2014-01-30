#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_MarkerManager.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;

static const char *SceneHandleName = "nativeSceneHandle";

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MarkerManager_initialise
  (JNIEnv *env, jobject obj, jobject sceneObj)
{
	try
	{
		MapScene *scene = getHandle<MapScene>(env,sceneObj);
		setHandleNamed(env,obj,scene,SceneHandleName);
		MarkerManager *markerManager = dynamic_cast<MarkerManager *>(scene->getManager(kWKMarkerManager));
		setHandle(env,obj,markerManager);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerManager::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MarkerManager_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerManager::dispose()");
	}
}

JNIEXPORT jlong JNICALL Java_com_mousebirdconsulting_maply_MarkerManager_addMarkers
  (JNIEnv *env, jobject obj, jobjectArray markerObjArray, jobject markerInfoObj, jobject changeSetObj)
{
	try
	{
		MarkerManager *markerManager = getHandle<MarkerManager>(env,obj);
		MarkerInfo *markerInfo = getHandle<MarkerInfo>(env,markerInfoObj);
		ChangeSet *changeSet = getHandle<ChangeSet>(env,changeSetObj);
		Scene *scene = getHandleNamed<Scene>(env,obj,SceneHandleName);
		if (!markerManager || !markerInfo || !changeSet || !scene)
		{
			__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "One of the inputs was null in MarkerManager::addMarkers()");
			return EmptyIdentity;
		}

		std::vector<Marker *> markers;
		int objCount = env->GetArrayLength(markerObjArray);
		for (int ii=0;ii<objCount;ii++)
		{
			jobject javaMarkerObj = (jobject) env->GetObjectArrayElement(markerObjArray, ii);
			Marker *marker = getHandle<Marker>(env,javaMarkerObj);
			markers.push_back(marker);
		}

		// Note: Porting
		// Note: Shouldn't have to set this
    	markerInfo->markerId = Identifiable::genId();

		SimpleIdentity markerId = markerManager->addMarkers(markers,*markerInfo,*changeSet);

		return markerId;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerManager::addMarkers()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MarkerManager_removeMarkers
  (JNIEnv *env, jobject obj, jlongArray idArrayObj, jobject changeSetObj)
{
	try
	{
		MarkerManager *markerManager = getHandle<MarkerManager>(env,obj);
		ChangeSet *changeSet = getHandle<ChangeSet>(env,changeSetObj);
		Scene *scene = getHandleNamed<Scene>(env,obj,SceneHandleName);

		long long *ids = env->GetLongArrayElements(idArrayObj, NULL);
		int idCount = env->GetArrayLength(idArrayObj);
		if (idCount == 0)
			return;

		SimpleIDSet idSet;
		for (int ii=0;ii<idCount;ii++)
			idSet.insert(ids[ii]);
		env->ReleaseLongArrayElements(idArrayObj,ids, 0);

		markerManager->removeMarkers(idSet,*changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerManager::removeMarkers()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MarkerManager_enableMarkers
  (JNIEnv *env, jobject obj, jlongArray idArrayObj, jboolean enable, jobject changeSetObj)
{
	try
	{
		MarkerManager *markerManager = getHandle<MarkerManager>(env,obj);
		ChangeSet *changeSet = getHandle<ChangeSet>(env,changeSetObj);
		Scene *scene = getHandleNamed<Scene>(env,obj,SceneHandleName);

		int idCount = env->GetArrayLength(idArrayObj);
		long long *ids = env->GetLongArrayElements(idArrayObj, NULL);
		if (idCount == 0)
			return;

		SimpleIDSet idSet;
		for (int ii=0;ii<idCount;ii++)
			idSet.insert(ids[ii]);

//		if (enable)
//			__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Enabling marker: %d",(int)ids[0]);
//		else
//			__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Disabling marker: %d",(int)ids[0]);
		env->ReleaseLongArrayElements(idArrayObj,ids, 0);

		markerManager->enableMarkers(idSet,enable,*changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerManager::enableMarkers()");
	}
}
