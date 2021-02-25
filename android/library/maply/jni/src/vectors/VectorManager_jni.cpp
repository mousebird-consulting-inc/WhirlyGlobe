/*
 *  VectorManager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2016 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#import "Vectors_jni.h"
#import "Scene_jni.h"
#import "Renderer_jni.h"
#import "com_mousebird_maply_VectorManager.h"

using namespace WhirlyKit;
using namespace Maply;

typedef JavaClassInfo<VectorManager> VectorManagerClassInfo;
template<> VectorManagerClassInfo *VectorManagerClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorManager_nativeInit
  (JNIEnv *env, jclass cls)
{
	VectorManagerClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorManager_initialise
  (JNIEnv *env, jobject obj, jobject sceneObj)
{
	try
	{
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!scene)
            return;
		VectorManager *vecManager = dynamic_cast<VectorManager *>(scene->getManager(kWKVectorManager));
		VectorManagerClassInfo::getClassInfo()->setHandle(env,obj,vecManager);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorManager::initialise()");
	}
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorManager_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorManagerClassInfo *classInfo = VectorManagerClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            classInfo->clearHandle(env,obj);
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorManager::dispose()");
	}
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_VectorManager_addVectors
  (JNIEnv *env, jobject obj, jobjectArray vecObjArray, jobject vecInfoObj, jobject changeSetObj)
{
	try
	{
        VectorManager *vecManager = VectorManagerClassInfo::getClassInfo()->getObject(env,obj);
		VectorInfoRef *vecInfo = VectorInfoClassInfo::getClassInfo()->getObject(env,vecInfoObj);
		ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!vecManager || !vecInfo || !changeSet)
			return EmptyIdentity;

        // Collect up all the shapes to add at once
        VectorObjectClassInfo *vecObjClassInfo = VectorObjectClassInfo::getClassInfo();
		ShapeSet shapes;
		JavaObjectArrayHelper vecHelp(env,vecObjArray);
		while (jobject vecObjObj = vecHelp.getNextObject()) {
			VectorObjectRef *vecObj = vecObjClassInfo->getObject(env,vecObjObj);
			if (vecObj)
				shapes.insert((*vecObj)->shapes.begin(),(*vecObj)->shapes.end());
		}

        // Resolve a missing program
        if ((*vecInfo)->programID == EmptyIdentity)
        {
            ProgramGLES *prog = NULL;
            if ((*vecInfo)->filled)
				prog = (ProgramGLES *)vecManager->getScene()->findProgramByName(MaplyDefaultTriangleShader);
            else
            	prog = (ProgramGLES *)vecManager->getScene()->findProgramByName(MaplyDefaultLineShader);
            if (prog)
				(*vecInfo)->programID = prog->getId();
        }

		SimpleIdentity vecID = vecManager->addVectors(&shapes,*(*vecInfo),*(changeSet->get()));

		return vecID;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorManager::addVectors()");
	}
    
    return EmptyIdentity;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorManager_changeVectors
(JNIEnv *env, jobject obj, jlongArray idArrayObj, jobject vecInfoObj, jobject changeSetObj)
{
    try {
        VectorManager *vecManager = VectorManagerClassInfo::getClassInfo()->getObject(env,obj);
		VectorInfoRef *vecInfo = VectorInfoClassInfo::getClassInfo()->getObject(env,vecInfoObj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!vecManager || !vecInfo || !changeSet)
            return;
        
        JavaLongArray ids(env,idArrayObj);
        SimpleIDSet idSet;
        for (unsigned int ii=0;ii<ids.len;ii++)
        {
            vecManager->changeVectors(ids.rawLong[ii],*(*vecInfo),*(changeSet->get()));
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorManager::changeVectors()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorManager_removeVectors
  (JNIEnv *env, jobject obj, jlongArray idArrayObj, jobject changeSetObj)
{
	try
	{
        VectorManager *vecManager = VectorManagerClassInfo::getClassInfo()->getObject(env,obj);
		ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!vecManager || !changeSet)
			return;

        SimpleIDSet idSet;
        ConvertLongArrayToSet(env,idArrayObj,idSet);

		vecManager->removeVectors(idSet,*(changeSet->get()));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorManager::removeVectors()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorManager_enableVectors
  (JNIEnv *env, jobject obj, jlongArray idArrayObj, jboolean enable, jobject changeSetObj)
{
	try
	{
        VectorManager *vecManager = VectorManagerClassInfo::getClassInfo()->getObject(env,obj);
		ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!vecManager || !changeSet)
			return;

        SimpleIDSet idSet;
        ConvertLongArrayToSet(env,idArrayObj,idSet);

		vecManager->enableVectors(idSet,enable,*(changeSet->get()));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorManager::enableVectors()");
	}
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_VectorManager_instanceVectors
  (JNIEnv *env, jobject obj, jlong vecID, jobject vecInfoObj, jobject changeSetObj)
{
	try
	{
        VectorManager *vecManager = VectorManagerClassInfo::getClassInfo()->getObject(env,obj);
		VectorInfoRef *vecInfo = VectorInfoClassInfo::getClassInfo()->getObject(env,vecInfoObj);
		ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!vecManager || !vecInfo || !changeSet)
			return EmptyIdentity;

		return vecManager->instanceVectors(vecID,*(*vecInfo),*(changeSet->get()));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorManager::instanceVectors()");
	}

	return EmptyIdentity;
}