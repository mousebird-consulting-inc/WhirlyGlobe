/*
 *  QuadTracker_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
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
#import "com_mousebird_maply_QuadTracker.h"
#import "WhirlyGlobe.h"
#import "GlobeView.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTracker_nativeInit
(JNIEnv *env, jclass cls)
{
    QuadTrackerClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTracker_initialise
(JNIEnv *env, jobject obj, jobject gbObj)
{
    try
    {
        QuadTrackerClassInfo *classInfo = QuadTrackerClassInfo::getClassInfo();
        WhirlyGlobe::GlobeView *globe = GlobeViewClassInfo::getClassInfo()->getObject(env, gbObj);
        if (!globe)
            return;
        QuadTracker *inst = new QuadTracker(globe);
        classInfo->setHandle(env, obj, inst);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTracker::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTracker_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        QuadTrackerClassInfo *classInfo = QuadTrackerClassInfo::getClassInfo();
        QuadTracker *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        delete inst;
        classInfo->clearHandle(env, obj);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTracker::dispose()");
    }
}

JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_QuadTracker_tiles
(JNIEnv *env, jobject obj, jobjectArray tilesObj, jint numPts)
{
    try
    {
        QuadTrackerClassInfo *classInfo = QuadTrackerClassInfo::getClassInfo();
        QuadTracker *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        int size = env->GetArrayLength(tilesObj);
        QuadTrackerPointReturn tiles[size];
        for (int i = 0; i < size; i++){
            jobject object = env->GetObjectArrayElement(tilesObj, i);
            QuadTrackerPointReturn *tile = QuadTrackerPointReturnClassInfo::getClassInfo()->getObject(env, object);
            if (!tile){
                return NULL;
            }
            tiles[i] = *tile;
            env->DeleteLocalRef(object);
        }
        inst->tiles(&tiles[0], numPts);
        QuadTrackerPointReturnClassInfo *classInfoRet = QuadTrackerPointReturnClassInfo::getClassInfo(env,"com/mousebird/maply/QuadTrackerPointReturn");
        jclass cl = env->FindClass("com/mousebird/maply/QuadTrackerPointReturn");
        if (!cl){
            __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTracker::tiles()-> Class No Found");
        }
        jobjectArray ret = env->NewObjectArray(size, cl, NULL);
        for (int i = 0; i < size; i++){
            QuadTrackerPointReturn retInst = tiles[i];
            
            jobject object = classInfoRet->makeWrapperObject(env, &retInst);
            env->SetObjectArrayElement(ret, i, object);
            QuadTrackerPointReturn *cmp = QuadTrackerPointReturnClassInfo::getClassInfo()->getObject(env, object);
            __android_log_print(ANDROID_LOG_VERBOSE, "JNI", "Before Value %i: X: %d Y: %d Level: %d", i, cmp->getMaplyTileID().x, cmp->getMaplyTileID().y, cmp->getMaplyTileID().level);
            env->DeleteLocalRef(object);
        }
        return ret;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTracker::tiles()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTracker_addTile
(JNIEnv *env, jobject obj, jint x, jint y, jint level)
{
    try
    {
        QuadTrackerClassInfo *classInfo = QuadTrackerClassInfo::getClassInfo();
        QuadTracker *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        MaplyTileID tileID;
        tileID.x = x;
        tileID.y = y;
        tileID.level = level;
        inst->addTile(tileID);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTracker::addTile()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTracker_removeTile
(JNIEnv *env, jobject obj, jint x, jint y, jint level)
{
    try
    {
        QuadTrackerClassInfo *classInfo = QuadTrackerClassInfo::getClassInfo();
        QuadTracker *inst = classInfo->getObject(env, obj);
        if (!inst)
        return;
        
        MaplyTileID tileID;
        tileID.x = x;
        tileID.y = y;
        tileID.level = level;
        inst->removeTile(tileID);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTracker::removeTile()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTracker_setCoordSystem
(JNIEnv *env, jobject obj, jobject coordObj, jobject ptLLObj, jobject ptUrObj)
{
    try
    {
        QuadTrackerClassInfo *classInfo = QuadTrackerClassInfo::getClassInfo();
        QuadTracker *inst = classInfo->getObject(env, obj);
        CoordSystem *coord = CoordSystemClassInfo::getClassInfo()->getObject(env, coordObj);
        Point2d *ll = Point2dClassInfo::getClassInfo()->getObject(env, ptLLObj);
        Point2d *ur = Point2dClassInfo::getClassInfo()->getObject(env, ptUrObj);
        if (!inst || ! coord || !ll || !ur)
            return;
        
        inst->setCoordSys(coord, *ll, *ur);
        
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTracker::setCoordSystem()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTracker_setMinLevel
(JNIEnv *env, jobject obj, jint minLevel)
{
    try
    {
        QuadTrackerClassInfo *classInfo = QuadTrackerClassInfo::getClassInfo();
        QuadTracker *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->setMinLevel(minLevel);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTracker::setMinLevel()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadTracker_getMinLevel
(JNIEnv *env, jobject obj)
{
    try
    {
        QuadTrackerClassInfo *classInfo = QuadTrackerClassInfo::getClassInfo();
        QuadTracker *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        
        return inst->getMinLevel();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTracker::getMinLevel()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTracker_setAdapter
(JNIEnv *env, jobject obj, jobject adapObj)
{
    try
    {
        QuadTrackerClassInfo *classInfo = QuadTrackerClassInfo::getClassInfo();
        QuadTracker *inst = classInfo->getObject(env, obj);
        CoordSystemDisplayAdapter *adapter = CoordSystemDisplayAdapterInfo::getClassInfo()->getObject(env, adapObj);
        if (!inst || !adapter)
            return;
        
        inst->setAdapter(adapter);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTracker::setAdapter()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTracker_setRenderer
(JNIEnv *env, jobject obj, jobject renderObj)
{
    try
    {
        QuadTrackerClassInfo *classInfo = QuadTrackerClassInfo::getClassInfo();
        QuadTracker *inst = classInfo->getObject(env, obj);
        SceneRendererES *renderer = (SceneRendererES *)MaplySceneRendererInfo::getClassInfo()->getObject(env,renderObj);
        if (!inst || !renderer)
            return;
        inst->setRenderer(renderer);
        
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTracker::setRenderer()");
    }
}
