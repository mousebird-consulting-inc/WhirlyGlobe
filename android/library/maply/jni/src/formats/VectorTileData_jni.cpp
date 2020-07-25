/*
 *  VectorTileData_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg
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

#import <Formats_jni.h>
#import <Geometry_jni.h>
#import <Vectors_jni.h>
#import <Components_jni.h>
#import <Scene_jni.h>
#import "com_mousebird_maply_VectorTileData.h"

using namespace WhirlyKit;
using namespace Eigen;

template<> VectorTileDataClassInfo *VectorTileDataClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorTileData_nativeInit
(JNIEnv *env, jclass cls)
{
    VectorTileDataClassInfo ::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorTileData_initialise__
(JNIEnv *env, jobject obj)
{
    try {
        VectorTileData_AndroidRef *tileData = new VectorTileData_AndroidRef(new VectorTileData_Android());
        VectorTileDataClassInfo::getClassInfo()->setHandle(env,obj,tileData);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorTileData::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorTileData_initialise__IIILcom_mousebird_maply_Point2d_2Lcom_mousebird_maply_Point2d_2Lcom_mousebird_maply_Point2d_2Lcom_mousebird_maply_Point2d_2
(JNIEnv *env, jobject obj, jint x, jint y, jint level, jobject bllobj, jobject burobj, jobject geollobj, jobject geourobj)
{
    try {
        Point2dClassInfo *pt2dClassInfo = Point2dClassInfo::getClassInfo();
        Point2d *boundLL = pt2dClassInfo->getObject(env,bllobj);
        Point2d *boundUR = pt2dClassInfo->getObject(env,burobj);
        Point2d *geoLL = pt2dClassInfo->getObject(env,geollobj);
        Point2d *geoUR = pt2dClassInfo->getObject(env,geourobj);
        if (!boundLL || !boundUR || !geoLL || !geoUR)
            return;
        VectorTileData_AndroidRef *tileData = new VectorTileData_AndroidRef(new VectorTileData_Android());
        (*tileData)->ident = QuadTreeIdentifier(x,y,level);
        (*tileData)->bbox.ll() = *boundLL;
        (*tileData)->bbox.ur() = *boundUR;
        (*tileData)->geoBBox.ll() = *geoLL;
        (*tileData)->geoBBox.ur() = *geoUR;
        VectorTileDataClassInfo::getClassInfo()->setHandle(env,obj,tileData);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorTileData::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorTileData_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorTileDataClassInfo *classInfo = VectorTileDataClassInfo::getClassInfo();

        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            VectorTileData_AndroidRef *inst = classInfo->getObject(env,obj);
            if (!inst)
                return;
            delete inst;
        }

        classInfo->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorTileData::dispose()");
    }
}

JNIEXPORT jintArray JNICALL Java_com_mousebird_maply_VectorTileData_getTileIDNative
        (JNIEnv *env, jobject obj)
{
    try
    {
        VectorTileData_AndroidRef *tileData = VectorTileDataClassInfo::getClassInfo()->getObject(env,obj);
        if (!tileData)
            return NULL;
        std::vector<int> ints(3);
        auto tileID = (*tileData)->ident;
        ints[0] = tileID.x;
        ints[1] = tileID.y;
        ints[2] = tileID.level;

        return BuildIntArray(env,ints);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorTileData::getTileIDNative");
    }

    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorTileData_getBoundsNative
(JNIEnv *env, jobject obj, jobject llObj, jobject urObj)
{
    try
    {
        VectorTileData_AndroidRef *tileData = VectorTileDataClassInfo::getClassInfo()->getObject(env,obj);
        Point2dClassInfo *pt2dClassInfo = Point2dClassInfo::getClassInfo();
        Point2d *ll = pt2dClassInfo->getObject(env,llObj);
        Point2d *ur = pt2dClassInfo->getObject(env,urObj);
        if (!tileData || !ll || !ur)
            return;
        *ll = (*tileData)->bbox.ll();
        *ur = (*tileData)->bbox.ur();
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply","Crash in VectorTileData::getBoundsNative");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorTileData_getGeoBoundsNative
(JNIEnv *env, jobject obj, jobject llObj, jobject urObj)
{
    try
    {
        VectorTileData_AndroidRef *tileData = VectorTileDataClassInfo::getClassInfo()->getObject(env,obj);
        Point2dClassInfo *pt2dClassInfo = Point2dClassInfo::getClassInfo();
        Point2d *ll = pt2dClassInfo->getObject(env,llObj);
        Point2d *ur = pt2dClassInfo->getObject(env,urObj);
        if (!tileData || !ll || !ur)
            return;
        *ll = (*tileData)->geoBBox.ll();
        *ur = (*tileData)->geoBBox.ur();
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply","Crash in VectorTileData::getGeoBoundsNative");
    }
}

JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_VectorTileData_getComponentObjects__
        (JNIEnv *env, jobject obj)
{
    try
    {
        VectorTileData_AndroidRef *tileData = VectorTileDataClassInfo::getClassInfo()->getObject(env,obj);
        if (!tileData)
            return NULL;
        ComponentObjectRefClassInfo *classInfo = ComponentObjectRefClassInfo::getClassInfo();
        std::vector<jobject> compObjs;
        for (ComponentObjectRef compObj : (*tileData)->compObjs) {
            compObjs.push_back(MakeComponentObjectWrapper(env,classInfo,compObj));
        }

        return BuildObjectArray(env,classInfo->getClass(),compObjs);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorTileData::getComponentObjects");
    }

    return NULL;
}

JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_VectorTileData_getComponentObjects__Ljava_lang_String_2
        (JNIEnv *env, jobject obj, jstring jStr)
{
    try
    {
        VectorTileData_AndroidRef *tileData = VectorTileDataClassInfo::getClassInfo()->getObject(env,obj);
        if (!tileData)
            return NULL;

        JavaString str(env,jStr);
        auto it = (*tileData)->categories.find((std::string)str.cStr);
        if (it == (*tileData)->categories.end())
            return NULL;
        std::vector<ComponentObjectRef> &compObjs = it->second;

        ComponentObjectRefClassInfo *classInfo = ComponentObjectRefClassInfo::getClassInfo();
        std::vector<jobject> outCompObjs;
        for (ComponentObjectRef compObj : compObjs) {
            outCompObjs.push_back(MakeComponentObjectWrapper(env,classInfo,compObj));
        }

        return BuildObjectArray(env,classInfo->getClass(),outCompObjs);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply","Crash in VectorTileData::getComponentObjects (by category)");
    }

    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorTileData_addComponentObject
(JNIEnv *env, jobject obj, jobject compObjObj)
{
    try
    {
        VectorTileData_AndroidRef *tileData = VectorTileDataClassInfo::getClassInfo()->getObject(env,obj);
        ComponentObjectRef *compObj = ComponentObjectRefClassInfo::getClassInfo()->getObject(env,compObjObj);
        if (!tileData || !compObj)
            return;
        (*tileData)->compObjs.push_back(*compObj);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorTileData::addComponentObject");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorTileData_addComponentObjects
(JNIEnv *env, jobject obj, jobjectArray compObjArray)
{
    try
    {
        VectorTileData_AndroidRef *tileData = VectorTileDataClassInfo::getClassInfo()->getObject(env,obj);
        ComponentObjectRefClassInfo *classInfo = ComponentObjectRefClassInfo::getClassInfo();
        if (!tileData)
            return;
        JavaObjectArrayHelper compObjHelp(env,compObjArray);
        while (jobject compObjObj = compObjHelp.getNextObject()) {
            ComponentObjectRef *compObj = classInfo->getObject(env,compObjObj);
            (*tileData)->compObjs.push_back(*compObj);
        }
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorTileData::addComponentObjects");
    }
}

JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_VectorTileData_getVectors
        (JNIEnv *env, jobject obj)
{
    try
    {
        VectorTileData_AndroidRef *tileData = VectorTileDataClassInfo::getClassInfo()->getObject(env,obj);
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        if (!tileData)
            return NULL;
        std::vector<jobject> outVecs;
        for (VectorObjectRef vecObjRef : (*tileData)->vecObjs) {
            outVecs.push_back(MakeVectorObjectWrapper(env,classInfo,vecObjRef));
        }
        return BuildObjectArray(env,classInfo->getClass(),outVecs);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorTileData::getVectors");
    }

    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorTileData_getChangeSet
        (JNIEnv *env, jobject obj)
{
    try
    {
        VectorTileData_AndroidRef *tileData = VectorTileDataClassInfo::getClassInfo()->getObject(env,obj);
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        if (!tileData)
            return NULL;
        jobject newObj = MakeChangeSet(env,(*tileData)->changes);
        (*tileData)->changes.clear();
        return newObj;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorTileData::getChangeSet");
    }

    return NULL;
}
