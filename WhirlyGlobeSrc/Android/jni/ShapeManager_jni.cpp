/*
 *  ShapeManager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
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
#import "Maply_jni.h"
#import "WhirlyGlobe.h"
#import "com_mousebird_maply_ShapeManager.h"

using namespace WhirlyKit;


JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeManager_nativeInit
(JNIEnv *env, jclass cls)
{
    ShapeManagerClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeManager_initialise
(JNIEnv *env, jobject obj, jobject sceneObj)
{
    try
    {
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!scene)
            return;
        ShapeManager *shapeManager = dynamic_cast<ShapeManager *>(scene->getManager(kWKShapeManager));
        ShapeManagerClassInfo::getClassInfo()->setHandle(env,obj,shapeManager);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeManager::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeManager_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeManagerClassInfo *classInfo = ShapeManagerClassInfo::getClassInfo();
        classInfo->clearHandle(env, obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeManager::dispose()");
    }
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_ShapeManager_addShapes
(JNIEnv *env, jobject obj, jobject arrayObj, jobject shapeInfoObj, jobject changeObj)
{
    try
    {
        ShapeManagerClassInfo *classInfo = ShapeManagerClassInfo::getClassInfo();
        ShapeManager *inst = classInfo->getObject(env, obj);
        WhirlyKitShapeInfo *shapeInfo = ShapeInfoClassInfo::getClassInfo()->getObject(env, shapeInfoObj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env, changeObj);
        
        if (!inst || !shapeInfo || !changeSet)
            return EmptyIdentity;
        
        // Get the iterator
        // Note: Look these up once
        jclass listClass = env->GetObjectClass(arrayObj);
        jclass iterClass = env->FindClass("java/util/Iterator");
        jmethodID literMethod = env->GetMethodID(listClass,"iterator","()Ljava/util/Iterator;");
        jobject liter = env->CallObjectMethod(arrayObj,literMethod);
        jmethodID hasNext = env->GetMethodID(iterClass,"hasNext","()Z");
        jmethodID next = env->GetMethodID(iterClass,"next","()Ljava/lang/Object;");
        env->DeleteLocalRef(iterClass);
        env->DeleteLocalRef(listClass);
        
        std::vector<WhirlyKitShape*> shapes;
        ShapeSphereClassInfo *shapeClassInfo = ShapeSphereClassInfo::getClassInfo();
        while (env->CallBooleanMethod(liter, hasNext))
        {
            jobject javaVecObj = env->CallObjectMethod(liter, next);
            WhirlyKitSphere *shapeObj = shapeClassInfo->getObject(env,javaVecObj);
            shapes.push_back(shapeObj);
            env->DeleteLocalRef(javaVecObj);
        }
        env->DeleteLocalRef(liter);
        
        if (shapeInfo->programID == EmptyIdentity)
        {
            shapeInfo->programID = inst->getScene()->getProgramIDBySceneName(kToolkitDefaultTriangleProgram);
        }
        
        SimpleIdentity shapeId = inst->addShapes(shapes, shapeInfo, *changeSet);
        return shapeId;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeManager::addShapes()");
    }
    
    return EmptyIdentity;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeManager_removeShapes
(JNIEnv *env, jobject obj, jlongArray idArrayObj, jobject changeObj)
{
    try
    {
        ShapeManagerClassInfo *classInfo = ShapeManagerClassInfo::getClassInfo();
        ShapeManager *inst = classInfo->getObject(env, obj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env, changeObj);
        if (!inst || !changeSet)
            return;

        JavaLongArray ids(env,idArrayObj);
        SimpleIDSet idSet;
        for (unsigned int ii=0;ii<ids.len;ii++)
        {
            idSet.insert(ids.rawLong[ii]);
        }
        
        inst->removeShapes(idSet, *changeSet);
    }
    
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeManager::removeShapes()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeManager_enableShapes
(JNIEnv *env, jobject obj, jlongArray idArrayObj, jboolean enable, jobject changeObj)
{
    try
    {
        ShapeManagerClassInfo *classInfo = ShapeManagerClassInfo::getClassInfo();
        ShapeManager *inst = classInfo->getObject(env, obj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env, changeObj);
        if (!inst || !changeSet)
            return;

        JavaLongArray ids(env,idArrayObj);
        SimpleIDSet idSet;
        for (unsigned int ii=0;ii<ids.len;ii++)
        {
            idSet.insert(ids.rawLong[ii]);
        }
        
        inst->enableShapes(idSet, enable, *changeSet);
    }
    
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeManager::enableShapes()");
    }
}
