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

		std::vector<Marker *> markers;
		int objCount = env->GetArrayLength(markerObjArray);
		for (int ii=0;ii<objCount;ii++)
		{
			jobject javaMarkerObj = (jobject) env->GetObjectArrayElement(markerObjArray, ii);
			Marker *marker = getHandle<Marker>(env,javaMarkerObj);
			markers.push_back(marker);
		}

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
			idSet.insert(idCount);

		markerManager->removeMarkers(idSet,*changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerManager::removeMarkers()");
	}
}

