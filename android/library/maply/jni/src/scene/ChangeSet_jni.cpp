/*
 *  ChangeSet_jni.cpp
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

#import <jni.h>
#import "Scene_jni.h"
#import "Renderer_jni.h"
#import "com_mousebird_maply_ChangeSet.h"

using namespace WhirlyKit;

template<> ChangeSetClassInfo *ChangeSetClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_nativeInit
  (JNIEnv *env, jclass cls)
{
	ChangeSetClassInfo::getClassInfo(env,cls);
}

JNIEXPORT jobject JNICALL MakeChangeSet(JNIEnv *env,const ChangeSet &changeSet)
{
	ChangeSetClassInfo *classInfo = ChangeSetClassInfo::getClassInfo(env,"com/mousebird/maply/ChangeSet");
	jobject newObj = classInfo->makeWrapperObject(env,NULL);
	WhirlyKit::ChangeSetRef *inst = classInfo->getObject(env,newObj);
	(*inst)->insert((*inst)->end(),changeSet.begin(),changeSet.end());

	return newObj;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		ChangeSetRef *changeSet = new ChangeSetRef(new ChangeSet());
		ChangeSetClassInfo::getClassInfo()->setHandle(env,obj,changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ChangeSet::initialise()");
	}
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		ChangeSetClassInfo *classInfo = ChangeSetClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            ChangeSetRef *changeSet = classInfo->getObject(env,obj);
            if (!changeSet)
                return;

            // Be sure to delete the contents
            for (unsigned int ii = 0;ii<(*changeSet)->size();ii++)
                delete (*changeSet)->at(ii);

            delete changeSet;

            classInfo->clearHandle(env,obj);
        }
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
		ChangeSetRef *changeSet = classInfo->getObject(env,obj);
		ChangeSetRef *otherChangeSet = classInfo->getObject(env,otherObj);
		if (!changeSet || !otherChangeSet)
			return;
		(*changeSet)->insert((*changeSet)->end(),(*otherChangeSet)->begin(),(*otherChangeSet)->end());
		(*otherChangeSet)->clear();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ChangeSet::merge()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_process
  (JNIEnv *env, jobject obj, jobject renderControlObj, jobject sceneObj)
{
	try
	{
		ChangeSetRef *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,obj);
		SceneRendererGLES_Android *sceneRender = SceneRendererInfo::getClassInfo()->getObject(env,renderControlObj);
		Scene *scene = SceneClassInfo::getClassInfo()->getObject(env,sceneObj);
		if (!changes || !sceneRender || !scene)
			return;

	    bool requiresFlush = false;
	    // Set up anything that needs to be set up
	    ChangeSet changesToAdd;
	    for (unsigned int ii = 0;ii<(*changes)->size();ii++)
	    {
	        ChangeRequest *change = (*changes)->at(ii);
	        if (change)
	        {
	            requiresFlush |= change->needsFlush();
	            change->setupForRenderer(sceneRender->getRenderSetupInfo());
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

//	    __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Processed %d changes",changesToAdd.size());

	    scene->addChangeRequests(changesToAdd);
		(*changes)->clear();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ChangeSet::process()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_addTexture
  (JNIEnv *env, jobject obj, jobject texObj, jobject sceneObj, jint filterType)
{
	try
	{
		ChangeSetClassInfo *classInfo = ChangeSetClassInfo::getClassInfo();
		ChangeSetRef *changeSet = classInfo->getObject(env,obj);
		Texture *texture = TextureClassInfo::getClassInfo()->getObject(env,texObj);
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env,sceneObj);
		if (!changeSet || !texture || !scene)
			return;

        // We take control of the Texture * as soon as it goes into the change set
        TextureClassInfo::getClassInfo()->clearHandle(env,texObj);
        switch (filterType)
        {
            case 0:
                texture->setInterpType(TexInterpNearest);
                break;
            case 1:
                texture->setInterpType(TexInterpLinear);
                break;
        }

		(*changeSet)->push_back(new AddTextureReq(texture));
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
		ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,obj);
		if (!changeSet)
			return;

		(*changeSet)->push_back(new RemTextureReq(texID));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ChangeSet::removeTexture()");
	}
}
