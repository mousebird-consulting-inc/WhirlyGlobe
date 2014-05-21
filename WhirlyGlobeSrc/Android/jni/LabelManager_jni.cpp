#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_LabelManager.h"
#import "WhirlyGlobe.h"
#import "SingleLabelAndroid.h"

using namespace WhirlyKit;
using namespace Maply;

static const char *SceneHandleName = "nativeSceneHandle";

typedef JavaClassInfo<LabelManager> LabelManagerClassInfo;
template<> LabelManagerClassInfo *LabelManagerClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelManager_nativeInit
  (JNIEnv *env, jclass cls)
{
	LabelManagerClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelManager_initialise
  (JNIEnv *env, jobject obj, jobject sceneObj)
{
	try
	{
		MapScene *scene = MapSceneClassInfo::getClassInfo()->getObject(env,sceneObj);
		LabelManager *labelManager = dynamic_cast<LabelManager *>(scene->getManager(kWKLabelManager));
		LabelManagerClassInfo::getClassInfo()->setHandle(env,obj,labelManager);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelManager::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelManager_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		LabelManagerClassInfo::getClassInfo()->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelManager::dispose()");
	}
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_LabelManager_addLabels
  (JNIEnv *env, jobject obj, jobject labelObjList, jobject labelInfoObj, jobject changeSetObj)
{
	try
	{
		LabelManagerClassInfo *classInfo = LabelManagerClassInfo::getClassInfo();
		LabelManager *labelManager = classInfo->getObject(env,obj);
		LabelInfo *labelInfo = LabelInfoClassInfo::getClassInfo()->getObject(env,labelInfoObj);
		ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!labelManager || !labelInfo || !changeSet)
		{
			__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "One of the inputs was null in LabelManager::addLabels()");
			return EmptyIdentity;
		}

		std::vector<SingleLabel *> labels;
		JavaListInfo *listClassInfo = JavaListInfo::getClassInfo(env);
		jobject iterObj = listClassInfo->getIter(env,labelObjList);

		LabelClassInfo *labelClassInfo = LabelClassInfo::getClassInfo();
		ShapeSet shapes;
		while (listClassInfo->hasNext(env,labelObjList,iterObj))
		{
			jobject javaLabelObj = listClassInfo->getNext(env,labelObjList,iterObj);
			SingleLabelAndroid *label = labelClassInfo->getObject(env,javaLabelObj);
			labels.push_back(label);
			env->DeleteLocalRef(javaLabelObj);
		}
		env->DeleteLocalRef(iterObj);

		SimpleIdentity labelId = labelManager->addLabels(labels,*labelInfo,*changeSet);

		return labelId;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelManager::addLabels()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelManager_removeLabels
  (JNIEnv *env, jobject obj, jlongArray idArrayObj, jobject changeSetObj)
{
	try
	{
		LabelManagerClassInfo *classInfo = LabelManagerClassInfo::getClassInfo();
		LabelManager *labelManager = classInfo->getObject(env,obj);
		ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!labelManager || !changeSet)
			return;

		long long *ids = env->GetLongArrayElements(idArrayObj, NULL);
		int idCount = env->GetArrayLength(idArrayObj);
		if (idCount == 0)
			return;

		SimpleIDSet idSet;
		for (int ii=0;ii<idCount;ii++)
			idSet.insert(ids[ii]);
		env->ReleaseLongArrayElements(idArrayObj,ids, 0);

		labelManager->removeLabels(idSet,*changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelManager::removeLabels()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelManager_enableLabels
  (JNIEnv *env, jobject obj, jlongArray idArrayObj, jboolean enable, jobject changeSetObj)
{
	try
	{
		LabelManagerClassInfo *classInfo = LabelManagerClassInfo::getClassInfo();
		LabelManager *labelManager = classInfo->getObject(env,obj);
		ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!labelManager || !changeSet)
			return;

		int idCount = env->GetArrayLength(idArrayObj);
		long long *ids = env->GetLongArrayElements(idArrayObj, NULL);
		if (idCount == 0)
			return;

		SimpleIDSet idSet;
		for (int ii=0;ii<idCount;ii++)
			idSet.insert(ids[ii]);

		env->ReleaseLongArrayElements(idArrayObj,ids, 0);

		labelManager->enableLabels(idSet,enable,*changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelManager::enableLabels()");
	}

}
