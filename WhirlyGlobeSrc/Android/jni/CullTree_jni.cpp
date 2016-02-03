/*
 *  CullTree_jni.cpp
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
#import "com_mousebird_maply_CullTree.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;


JNIEXPORT void JNICALL Java_com_mousebird_maply_CullTree_nativeInit
(JNIEnv *env, jclass cls)
{
    CullTreeClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_CullTree_initialise
(JNIEnv *env, jobject obj, jobject coordSystemAdapterObj, jobject llObj, jobject urObj, jint depth, jint maxDrawPerNode)
{
    try {
        CullTreeClassInfo *classInfo = CullTreeClassInfo::getClassInfo();
        CoordSystemDisplayAdapter *coordAdapter = CoordSystemDisplayAdapterInfo::getClassInfo()->getObject(env,coordSystemAdapterObj);
        Point2f *ll = Point2fClassInfo::getClassInfo()->getObject(env, llObj);
        Point2f *ur = Point2fClassInfo::getClassInfo()->getObject(env, urObj);
        if (!coordAdapter || !ll || !ur)
            return;
        
        Mbr *mbr = new Mbr(*ll, *ur);
        CullTree *cullTree = new CullTree(coordAdapter, *mbr, depth, maxDrawPerNode);
        classInfo->setHandle(env, obj, cullTree);
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CullTree::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_CullTree_dispose
(JNIEnv *env, jobject obj)
{
    try {
        CullTreeClassInfo *classInfo = CullTreeClassInfo::getClassInfo();
        CullTree *inst = classInfo->getObject(env, obj);
        if(!inst)
            return;
        delete inst;
        classInfo->clearHandle(env, obj);
        
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CullTree:dispose()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_CullTree_getTopCullable
(JNIEnv *env, jobject obj)
{
    try {
        CullTreeClassInfo *classInfo = CullTreeClassInfo::getClassInfo();
        CullTree *inst = classInfo->getObject(env, obj);
        if(!inst)
            return NULL;
        CullableClassInfo *classInfoCullable = CullableClassInfo::getClassInfo(env,"com/mousebird/maply/Cullable");
        return classInfoCullable->makeWrapperObject(env,inst->getTopCullable());
        
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CullTree:getTopCullable()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_CullTree_getCount
(JNIEnv *env, jobject obj)
{
    try {
        CullTreeClassInfo *classInfo = CullTreeClassInfo::getClassInfo();
        CullTree *inst = classInfo->getObject(env, obj);
        if(!inst)
            return -1;
        
        return inst->getCount();
        
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CullTree:getCount()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_CullTree_dumpStats
(JNIEnv *env, jobject obj)
{
    try {
        CullTreeClassInfo *classInfo = CullTreeClassInfo::getClassInfo();
        CullTree *inst = classInfo->getObject(env, obj);
        if(!inst)
            return;
        
        inst->dumpStats();
        
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CullTree:dumpStats()");
    }
}
