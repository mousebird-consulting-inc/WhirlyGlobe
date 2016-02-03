/*
 *  Cullable_jni.cpp
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
#import "com_mousebird_maply_Cullable.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;



JNIEXPORT void JNICALL Java_com_mousebird_maply_Cullable_nativeInit
(JNIEnv *env, jclass cls)
{
    CullableClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Cullable_initialise
(JNIEnv *env, jobject obj, jobject coordSystemAdapObj, jobject llObj, jobject urObj, jint depth)
{
    try {
        CullableClassInfo *classInfo = CullableClassInfo::getClassInfo();
        CoordSystemDisplayAdapter *adapter = CoordSystemDisplayAdapterInfo::getClassInfo()->getObject(env, coordSystemAdapObj);
        Point2d *ll = Point2dClassInfo::getClassInfo()->getObject(env, llObj);
        Point2d *ur = Point2dClassInfo::getClassInfo()->getObject(env, urObj);
        if (!adapter || !ll || !ur)
            return;
        
        Mbr *mbr = new Mbr(ll, ur);
        Cullable *inst = new Cullable(adapter, mbr, depth);
        classInfo->setHandle(env,  obj, inst)
        
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Cullable:initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Cullable_dispose
(JNIEnv *env, jobject obj)
{
    try {
        CullableClassInfo *classInfo = CullableClassInfo::getClassInfo();
        Cullable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        delete inst;
        classInfo->clearHandle(env, obj);
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Cullable:dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Cullable_addDrawable
(JNIEnv *, jobject obj, jobject cullTreeObj, jobject llObj, jobject urObj, jobject dwObj)
{
    try {
        CullableClassInfo *clasInfo = CullableClassInfo::getClassInfo();
        Cullable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        Point2d *ll = Point2dClassInfo::getClassInfo()->getObject(env, llObj);
        Point2d *ur = Point2dClassInfo::getClassInfo()->getObject(env, urObj);
        Drawable *dw = DrawableClassInfo::getClassInfo()->getObjet(env, dwObj);
        if (!ll || !ur | !dw)
            return;
        Mbr *mbr = new Mbr(ll, ur);
        inst->addDrawable(inst, mbr, dw);
        
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Cullable:addDrawable()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Cullable_getDrawables
(JNIEnv *env, jobject obj)
{
    try {
        CullableClassInfo *classInfo = CullableClassInfo::getClassInfo();
        Cullable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        jclass clsArrayList = env->FindClass("java/util/ArrayList");
        jmethodID constructor = env->GetMethodID(clsArrayList, "<init>", "()V");
        jobject objArrayList = env->NewObject(clsArrayList, constructor, "");
        jmethodID arrayListAdd = env->GetMethodID(clsArrayList, "add", "(Ljava/lang/Object;)Z");
        
        DrawableClassInfo *drawableClassInfo = DrawableClassInfo::getClassInfo(env, "com/mousebird/maply/Drawable");
    
        for(auto item : inst->drawables) {
            jobject objDrawable = drawableClassInfo->makeWrapperObject(env, item);
            env->CallObjectMethod(objArrayList, arrayListAdd, objDrawable);
        }
        env->DeleteLocalRef(clsArrayList);
        return objArrayList;
        
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Cullable:getDrawables()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Cullable_getChildDrawables
(JNIEnv *env, jobject obj)
{
    try {
        CullableClassInfo *classInfo = CullableClassInfo::getClassInfo();
        Cullable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        jclass clsArrayList = env->FindClass("java/util/List");
        jmethodID constructor = env->GetMethodID(clsArrayList, "<init>", "()V");
        jobject objArrayList = env->NewObject(clsArrayList, constructor, "");
        jmethodID arrayListAdd = env->GetMethodID(clsArrayList, "add", "(Ljava/lang/Object;)Z");
        
        DrawableClassInfo *drawableClassInfo = DrawableClassInfo::getClassInfo(env, "com/mousebird/maply/Drawable");
        
        for(auto item : inst->childDrawables) {
            jobject objDrawable = drawableClassInfo->makeWrapperObject(env, item);
            env->CallObjectMethod(objArrayList, arrayListAdd, objDrawable);
        }
        env->DeleteLocalRef(clsArrayList);
        return objArrayList;
        
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Cullable:getChildDrawables()");
    }
    
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Cullable_hasChildren
(JNIEnv *env, jobject obj)
{
    try {
        CullableClassInfo *classInfo = CullableClassInfo::getClassInfo();
        Cullable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        return inst->hasChildren();
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Cullable:hasChildren()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Cullable_isEmpty
(JNIEnv *env, jobject obj)
{
    try {
        CullableClassInfo *classInfo = CullableClassInfo::getClassInfo();
        Cullable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        return inst->isEmpty();
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Cullable:isEmpty()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Cullable_getChild
(JNIEnv *env, jobject obj, jint which)
{
    try {
        CullableClassInfo *classInfo = CullableClassInfo::getClassInfo();
        Cullable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        CullableClassInfo *classInfoCullable = CullableClassInfo::getClassInfo(env,"com/mousebird/maply/Cullable");
        return classInfoCullable->makeWrapperObject(env, inst->children[which]);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Cullable:getChild()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Cullable_getMbrLL
(JNIEnv *env, jobject obj)
{
    try {
        CullableClassInfo *classInfo = CullableClassInfo::getClassInfo();
        Cullable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        Point2dClassInfo *classInfoPoint2d = Point2dClassInfo::getClassInfo(env,"com/mousebird/maply/Point2d");
        return classInfoPoint2d->makeWrapperObject(env, inst->localMbr->ll());
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Cullable:getMbrLL()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Cullable_getMbrUr
(JNIEnv *env, jobject obj)
{
    try {
        CullableClassInfo *classInfo = CullableClassInfo::getClassInfo();
        Cullable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        Point2dClassInfo *classInfoPoint2d = Point2dClassInfo::getClassInfo(env,"com/mousebird/maply/Point2d");
        return classInfoPoint2d->makeWrapperObject(env, inst->localMbr->ur());
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Cullable:getMbrUr()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_Cullable_countNodes
(JNIEnv *env, jobject obj)
{
    try {
        CullableClassInfo *classInfo = CullableClassInfo::getClassInfo();
        Cullable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        return inst->countNodes();
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Cullable:countNodes()");
    }
}
