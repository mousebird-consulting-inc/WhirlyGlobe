/*
 *  Scene_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/17/15.
 *  Copyright 2011-2015 mousebird consulting
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
#import "Maply_jni.h"
#import "com_mousebird_maply_Scene.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Scene_nativeInit
  (JNIEnv *env, jclass cls)
{
	SceneClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Scene_addChangesNative
  (JNIEnv *env, jobject obj, jobject changesObj)
{
	try
	{
		SceneClassInfo *classInfo = SceneClassInfo::getClassInfo();
		Scene *scene = classInfo->getObject(env,obj);
		ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
		scene->addChangeRequests(*changes);
		changes->clear();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Scene::addChanges()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Scene_addShaderProgram
(JNIEnv *env, jobject obj, jobject shaderObj, jstring sceneNameStr)
{
    try
    {
        SceneClassInfo *classInfo = SceneClassInfo::getClassInfo();
        Scene *scene = classInfo->getObject(env,obj);
        OpenGLES2ProgramClassInfo *shaderClassInfo = OpenGLES2ProgramClassInfo::getClassInfo();
        OpenGLES2Program *shader = shaderClassInfo->getObject(env,shaderObj);
        
        if (!scene || !shader)
            return;
        
        const char *cName = env->GetStringUTFChars(sceneNameStr,0);
        std::string name = cName;
        
        scene->addProgram(name,shader);
        scene->setSceneProgram(name,shader->getId());
        
        env->ReleaseStringUTFChars(sceneNameStr, cName);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Scene::addShaderProgram()");
    }
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_Scene_getProgramIDBySceneName
(JNIEnv *env, jobject obj, jstring shaderName)
{
	try
	{
		SceneClassInfo *classInfo = SceneClassInfo::getClassInfo();
		Scene *scene = classInfo->getObject(env,obj);

		if (!scene)
			return -1;

		const char *cName = env->GetStringUTFChars(shaderName,0);
		std::string name = cName;

		return scene->getProgramIDByName(name);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Scene::getProgramIDBySceneName()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Scene_teardownGL
(JNIEnv *env, jobject obj)
{
    try
    {
        SceneClassInfo *classInfo = SceneClassInfo::getClassInfo();
        Scene *scene = classInfo->getObject(env,obj);
        if (!scene)
            return;
        
        scene->teardownGL();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Scene::teardownGL()");
    }
}
