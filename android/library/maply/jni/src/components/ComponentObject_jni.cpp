/*
 *  ComponentObject_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/19/19.
 *  Copyright 2011-2019 mousebird consulting
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
#import "Components_jni.h"
#import "Vectors_jni.h"
#import "com_mousebird_maply_ComponentObject.h"

using namespace WhirlyKit;

template<> ComponentObjectRefClassInfo *ComponentObjectRefClassInfo::classInfoObj = NULL;

JNIEXPORT jobject JNICALL MakeComponentObjectWrapper(JNIEnv *env,ComponentObjectRefClassInfo *classInfo,ComponentObjectRef compObj)
{
    return classInfo->makeWrapperObject(env,new ComponentObjectRef(compObj));
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentObject_nativeInit
  (JNIEnv *env, jclass cls)
{
	ComponentObjectRefClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentObject_initialise
  (JNIEnv *env, jobject obj)
{
    try
    {
        ComponentObjectRefClassInfo *classInfo = ComponentObjectRefClassInfo::getClassInfo();
        ComponentObjectRef *inst = new ComponentObjectRef(new ComponentObject());
        (*inst)->enable = true;
        classInfo->setHandle(env, obj, inst);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentObject::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentObject_dispose
  (JNIEnv *env, jobject obj)
{
    try
    {
        ComponentObjectRefClassInfo *classInfo = ComponentObjectRefClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            ComponentObjectRef *inst = classInfo->getObject(env, obj);
            if (!inst)
                return;
            delete inst;

            classInfo->clearHandle(env, obj);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentObject::dispose()");
    }
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_ComponentObject_getID
  (JNIEnv *env, jobject obj)
{
    try
    {
        ComponentObjectRef *inst = ComponentObjectRefClassInfo::getClassInfo()->getObject(env,obj);
        if (!inst)
            return EmptyIdentity;
        return (*inst)->getId();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentObject::getID()");
    }

    return EmptyIdentity;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentObject_addSelectID
  (JNIEnv *env, jobject obj, jlong selectID)
{
    try
    {
        ComponentObjectRef *inst = ComponentObjectRefClassInfo::getClassInfo()->getObject(env,obj);
        if (!inst)
            return;
        (*inst)->selectIDs.insert(selectID);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentObject::addSelectID()");
    }
}

JNIEXPORT jlongArray JNICALL Java_com_mousebird_maply_ComponentObject_getSelectIDs
  (JNIEnv *env, jobject obj)
{
    try
    {
        ComponentObjectRef *inst = ComponentObjectRefClassInfo::getClassInfo()->getObject(env,obj);
        if (!inst)
            return NULL;
        std::vector<SimpleIdentity> ids((*inst)->selectIDs.begin(),(*inst)->selectIDs.end());
        return BuildLongArray(env,ids);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentObject::getSelectIDs()");
    }

    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentObject_addMarkerID
  (JNIEnv *env, jobject obj, jlong markerID)
{
    try
    {
        ComponentObjectRef *inst = ComponentObjectRefClassInfo::getClassInfo()->getObject(env,obj);
        if (!inst)
            return;
        (*inst)->markerIDs.insert(markerID);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentObject::addMarkerID()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentObject_addStickerID
  (JNIEnv *env, jobject obj, jlong stickerID)
{
    try
    {
        ComponentObjectRef *inst = ComponentObjectRefClassInfo::getClassInfo()->getObject(env,obj);
        if (!inst)
            return;
        (*inst)->chunkIDs.insert(stickerID);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentObject::addStickerID()");
    }
}

JNIEXPORT jlongArray JNICALL Java_com_mousebird_maply_ComponentObject_getStickerIDs
  (JNIEnv *env, jobject obj)
{
    try
    {
        ComponentObjectRef *inst = ComponentObjectRefClassInfo::getClassInfo()->getObject(env,obj);
        if (!inst)
            return NULL;
        std::vector<SimpleIdentity> ids((*inst)->chunkIDs.begin(),(*inst)->chunkIDs.end());
        return BuildLongArray(env,ids);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentObject::getStickerIDs()");
    }

    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentObject_addVectorID
  (JNIEnv *env, jobject obj, jlong vectorID)
{
    try
    {
        ComponentObjectRef *inst = ComponentObjectRefClassInfo::getClassInfo()->getObject(env,obj);
        if (!inst)
            return;
        (*inst)->vectorIDs.insert(vectorID);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentObject::addVectorID()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentObject_addLoftID
        (JNIEnv *env, jobject obj, jlong loftID)
{
    try
    {
        ComponentObjectRef *inst = ComponentObjectRefClassInfo::getClassInfo()->getObject(env,obj);
        if (!inst)
            return;
        (*inst)->loftIDs.insert(loftID);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentObject::addLoftID()");
    }
}

JNIEXPORT jlongArray JNICALL Java_com_mousebird_maply_ComponentObject_getVectorIDs
  (JNIEnv *env, jobject obj)
{
    try
    {
        ComponentObjectRef *inst = ComponentObjectRefClassInfo::getClassInfo()->getObject(env,obj);
        if (!inst)
            return NULL;
        std::vector<SimpleIdentity> ids((*inst)->vectorIDs.begin(),(*inst)->vectorIDs.end());
        return BuildLongArray(env,ids);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentObject::getVectorIDs()");
    }

    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentObject_addWideVectorID
  (JNIEnv *env, jobject obj, jlong wideVectorID)
{
    try
    {
        ComponentObjectRef *inst = ComponentObjectRefClassInfo::getClassInfo()->getObject(env,obj);
        if (!inst)
            return;
        (*inst)->wideVectorIDs.insert(wideVectorID);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentObject::addWideVectorID()");
    }
}

JNIEXPORT jlongArray JNICALL Java_com_mousebird_maply_ComponentObject_getWideVectorIDs
  (JNIEnv *env, jobject obj)
{
    try
    {
        ComponentObjectRef *inst = ComponentObjectRefClassInfo::getClassInfo()->getObject(env,obj);
        if (!inst)
            return NULL;
        std::vector<SimpleIdentity> ids((*inst)->wideVectorIDs.begin(),(*inst)->wideVectorIDs.end());
        return BuildLongArray(env,ids);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentObject::getWideVectorIDs()");
    }

    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentObject_addLabelID
  (JNIEnv *env, jobject obj, jlong labelID)
{
    try
    {
        ComponentObjectRef *inst = ComponentObjectRefClassInfo::getClassInfo()->getObject(env,obj);
        if (!inst)
            return;
        (*inst)->labelIDs.insert(labelID);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentObject::addLabelID()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentObject_addShapeID
  (JNIEnv *env, jobject obj, jlong shapeID)
{
    try
    {
        ComponentObjectRef *inst = ComponentObjectRefClassInfo::getClassInfo()->getObject(env,obj);
        if (!inst)
            return;
        (*inst)->shapeIDs.insert(shapeID);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentObject::addShapeID()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentObject_addBillboardID
  (JNIEnv *env, jobject obj, jlong billboardID)
{
    try
    {
        ComponentObjectRef *inst = ComponentObjectRefClassInfo::getClassInfo()->getObject(env,obj);
        if (!inst)
            return;
        (*inst)->billIDs.insert(billboardID);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentObject::addBillboardID()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentObject_addParticleSystemID
  (JNIEnv *env, jobject obj, jlong partID)
{
    try
    {
        ComponentObjectRef *inst = ComponentObjectRefClassInfo::getClassInfo()->getObject(env,obj);
        if (!inst)
            return;
        (*inst)->partSysIDs.insert(partID);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentObject::addParticleSystemID()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentObject_addGeometryID
  (JNIEnv *env, jobject obj, jlong geomID)
{
    try
    {
        ComponentObjectRef *inst = ComponentObjectRefClassInfo::getClassInfo()->getObject(env,obj);
        if (!inst)
            return;
        (*inst)->geomIDs.insert(geomID);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentObject::addGeometryID()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentObject_addVector
        (JNIEnv *env, jobject obj, jobject vecObjObj)
{
    try
    {
        ComponentObjectRef *inst = ComponentObjectRefClassInfo::getClassInfo()->getObject(env,obj);
        VectorObjectRef *vecObj = VectorObjectClassInfo::getClassInfo()->getObject(env,vecObjObj);
        if (!inst)
            return;
        (*inst)->isSelectable = true;
        (*inst)->vecObjs.push_back(*vecObj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentObject::addGeometryID()");
    }
}
