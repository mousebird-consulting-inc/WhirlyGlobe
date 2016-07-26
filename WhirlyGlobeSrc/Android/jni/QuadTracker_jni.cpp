/*
 *  QuadTracker_jni.cpp
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
(JNIEnv *env, jobject obj, jobject globeViewObj, jobject maplyRenderObj, jobject coordAdapterObj, jobject coordSysObj, jobject llObj, jobject urObj, jint minLevel)
{
    try
    {
        QuadTrackerClassInfo *classInfo = QuadTrackerClassInfo::getClassInfo();
        WhirlyGlobe::GlobeView *globe = GlobeViewClassInfo::getClassInfo()->getObject(env, globeViewObj);
        MaplySceneRenderer *renderer = MaplySceneRendererInfo::getClassInfo()->getObject(env,maplyRenderObj);
        CoordSystemDisplayAdapter *coordAdapter = CoordSystemDisplayAdapterInfo::getClassInfo()->getObject(env,coordAdapterObj);
        CoordSystem *coordSys = CoordSystemClassInfo::getClassInfo()->getObject(env,coordSysObj);
        Point2dClassInfo *pt2dClassInfo = Point2dClassInfo::getClassInfo();
        Point2d *ll = pt2dClassInfo->getObject(env,llObj);
        Point2d *ur = pt2dClassInfo->getObject(env,urObj);
        if (!globe || !renderer || !coordAdapter || !coordSys || !ll || !ur)
            return;
        SceneRendererES2 *rendererES2 = dynamic_cast<SceneRendererES2 *>(renderer);
        QuadTracker *inst = new QuadTracker(globe,rendererES2,coordAdapter);
        inst->setCoordSys(coordSys,*ll,*ur);
        inst->setMinLevel(minLevel);
        classInfo->setHandle(env, obj, inst);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTracker::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTracker_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        QuadTrackerClassInfo *classInfo = QuadTrackerClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            QuadTracker *inst = classInfo->getObject(env, obj);
            if (!inst)
                return;
            delete inst;
            classInfo->clearHandle(env, obj);
        }
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTracker::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTracker_queryTilesNative
(JNIEnv *env, jobject obj, jint numPts, jdoubleArray screenLocsArray, jintArray tileIDsArray, jdoubleArray coordLocsArray, jdoubleArray tileLocsArray)
{
    try
    {
        QuadTrackerClassInfo *classInfo = QuadTrackerClassInfo::getClassInfo();
        QuadTracker *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        JavaDoubleArray screenLocs(env,screenLocsArray);
        JavaIntArray tileIDs(env,tileIDsArray);
        JavaDoubleArray coordLocs(env,coordLocsArray);
        JavaDoubleArray tileLocs(env,tileLocsArray);

        QuadTrackerPointReturn ptReturn(numPts,screenLocs.rawDouble,tileIDs.rawInt,coordLocs.rawDouble,tileLocs.rawDouble);
        inst->tiles(&ptReturn, numPts);
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
        
        Quadtree::Identifier tileID;
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
        
        Quadtree::Identifier tileID;
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
