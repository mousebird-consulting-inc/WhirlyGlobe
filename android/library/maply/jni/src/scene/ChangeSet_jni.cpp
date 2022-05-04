/*  ChangeSet_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2022 mousebird consulting
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
 */

#import <jni.h>
#import "Scene_jni.h"
#import "Renderer_jni.h"
#import "com_mousebird_maply_ChangeSet.h"

using namespace WhirlyKit;

template<> ChangeSetClassInfo *ChangeSetClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_nativeInit
  (JNIEnv *env, jclass cls)
{
	ChangeSetClassInfo::getClassInfo(env,cls);
}

JNIEXPORT jobject JNICALL MakeChangeSet(JNIEnv *env,const ChangeSet &changeSet)
{
	ChangeSetClassInfo *classInfo = ChangeSetClassInfo::getClassInfo(env,"com/mousebird/maply/ChangeSet");
	jobject newObj = classInfo->makeWrapperObject(env,nullptr);
	WhirlyKit::ChangeSetRef *inst = classInfo->getObject(env,newObj);
	(*inst)->reserve(changeSet.size());
	(*inst)->insert((*inst)->end(),changeSet.begin(),changeSet.end());
	return newObj;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		ChangeSetClassInfo::set(env, obj, new ChangeSetRef(std::make_shared<ChangeSet>()));
	}
	MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		ChangeSetClassInfo *classInfo = ChangeSetClassInfo::getClassInfo();
		std::unique_lock<std::mutex> lock(disposeMutex);
		if (const auto changeSet = classInfo->getObject(env,obj))
		{
			classInfo->clearHandle(env,obj);
			lock.unlock();

			// Be sure to delete the contents
			for (auto &change : **changeSet)
			{
				delete change;
				change = nullptr;
			}
			delete changeSet;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_merge
  (JNIEnv *env, jobject obj, jobject otherObj)
{
	try
	{
		ChangeSetClassInfo *classInfo = ChangeSetClassInfo::getClassInfo();
		if (const auto changeSet = classInfo->getObject(env,obj))
		if (const auto otherSet = classInfo->getObject(env,otherObj))
		if (!(*otherSet)->empty())
		{
			(*changeSet)->reserve((*changeSet)->size() + (*otherSet)->size());
			(*changeSet)->insert((*changeSet)->end(),(*otherSet)->begin(),(*otherSet)->end());
			(*otherSet)->clear();
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_process
  (JNIEnv *env, jobject obj, jobject renderControlObj, jobject sceneObj)
{
	try
	{
		ChangeSetRef *changes = ChangeSetClassInfo::get(env,obj);
		SceneRendererGLES_Android *sceneRender = SceneRendererInfo::get(env,renderControlObj);
		Scene *scene = SceneClassInfo::get(env,sceneObj);
		if (!changes || !sceneRender || !scene)
		{
			return;
		}

	    bool requiresFlush = false;
	    // Set up anything that needs to be set up
	    ChangeSet changesToAdd;
	    changesToAdd.reserve((*changes)->size());
	    for (auto change : **changes)
	    {
	        if (change)
	        {
	            requiresFlush |= change->needsFlush();
	            change->setupForRenderer(sceneRender->getRenderSetupInfo(),scene);
	            changesToAdd.push_back(change);
	        }
	        else
			{
	            // A NULL change request is a flush request
	            requiresFlush = true;
			}
	    }
		(*changes)->clear();

	    // If anything needed a flush after that, let's do it
	    if (requiresFlush)
	    {
	        glFlush();
	    }

	    scene->addChangeRequests(changesToAdd);
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_discard
	(JNIEnv *env, jobject obj)
{
	try
	{
		if (auto *changesRef = ChangeSetClassInfo::get(env,obj))
		{
			ChangeSet changes = std::move(**changesRef);
			for (auto *change : changes)
			{
				delete change;
			}
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_addTexture
  (JNIEnv *env, jobject obj, jobject texObj, jobject sceneObj, jint filterType)
{
	try
	{
		ChangeSetRef *changeSet = ChangeSetClassInfo::get(env,obj);
		Texture *texture = TextureClassInfo::get(env,texObj);
        Scene *scene = SceneClassInfo::get(env,sceneObj);
		if (!changeSet || !texture || !scene)
		{
			return;
		}

        // We take control of the Texture * as soon as it goes into the change set
        TextureClassInfo::getClassInfo()->clearHandle(env,texObj);
        switch (filterType)
        {
            case 0: texture->setInterpType(TexInterpNearest); break;
            default:
            case 1: texture->setInterpType(TexInterpLinear); break;
        }

		(*changeSet)->push_back(new AddTextureReq(texture));
	}
	MAPLY_STD_JNI_CATCH()

}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_removeTexture
  (JNIEnv *env, jobject obj, jlong texID)
{
	try
	{
		if (const auto changeSet = ChangeSetClassInfo::get(env,obj))
		{
			(*changeSet)->push_back(new RemTextureReq(texID));
		}
	}
	MAPLY_STD_JNI_CATCH()

}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ChangeSet_clearRenderTarget
		(JNIEnv *env, jobject obj, jlong renderTargetID)
{
	try
	{
		if (const auto changeSet = ChangeSetClassInfo::get(env,obj))
		{
			(*changeSet)->push_back(new ClearRenderTargetReq(renderTargetID));
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_ChangeSet_count
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (const auto changeSet = ChangeSetClassInfo::get(env,obj))
		{
			return (jint)(*changeSet)->size();
		}
	}
	MAPLY_STD_JNI_CATCH()
	return 0;
}
