/*
 *  GeometryRawPoints_jni.cpp
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
#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_GeometryRawPoints.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRawPoints_nativeInit
(JNIEnv *env, jclass cls)
{
    GeometryRawPointsClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRawPoints_initialise
(JNIEnv *env, jobject obj, jobject sceneObj)
{
    try
    {
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!scene)
            return;
        GeometryRawPoints *geomPoints = new GeometryRawPoints();
        GeometryRawPointsClassInfo::getClassInfo()->setHandle(env,obj,geomPoints);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRawPoints::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRawPoints_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        GeometryRawPointsClassInfo *classInfo = GeometryRawPointsClassInfo::getClassInfo();
        classInfo->clearHandle(env, obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRawPoints::dispose()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_GeometryRawPoints_valid
(JNIEnv *env, jobject obj)
{
    try
    {
        GeometryRawPointsClassInfo *classInfo = GeometryRawPointsClassInfo::getClassInfo();
        GeometryRawPoints *rawGeom = classInfo->getObject(env, obj);
        if (!rawGeom)
            return false;
        
        return rawGeom->valid();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRawPoints::valid()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRawPoints_addIntValues
(JNIEnv *env, jobject obj, jstring nameStr, jintArray intArray)
{
    try
    {
        GeometryRawPointsClassInfo *classInfo = GeometryRawPointsClassInfo::getClassInfo();
        GeometryRawPoints *rawGeom = classInfo->getObject(env, obj);

        if (!rawGeom)
            return;
        JavaString name(env,nameStr);
        int attrId = rawGeom->findAttribute(name.cStr);
        if (attrId < 0)
            return;
        
        std::vector<int> intVec;
        ConvertIntArray(env,intArray,intVec);
        
        rawGeom->addAttribute(name.cStr,GeomRawIntType);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRawPoints::addIntValues()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRawPoints_addFloatValues
(JNIEnv *env, jobject obj, jstring nameStr, jfloatArray floatArray)
{
    try
    {
        GeometryRawPointsClassInfo *classInfo = GeometryRawPointsClassInfo::getClassInfo();
        GeometryRawPoints *rawGeom = classInfo->getObject(env, obj);

        if (!rawGeom)
            return;
        JavaString name(env,nameStr);
        int attrId = rawGeom->findAttribute(name.cStr);
        if (attrId < 0)
            return;
        
        std::vector<float> floatVec;
        ConvertFloatArray(env,floatArray,floatVec);
        
        rawGeom->addAttribute(name.cStr,GeomRawFloatType);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRawPoints::addFloatValues()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRawPoints_addPoint2fValues
(JNIEnv *env, jobject obj, jstring nameStr, jfloatArray floatArray)
{
    try
    {
        GeometryRawPointsClassInfo *classInfo = GeometryRawPointsClassInfo::getClassInfo();
        GeometryRawPoints *rawGeom = classInfo->getObject(env, obj);
        
        if (!rawGeom)
            return;
        JavaString name(env,nameStr);
        int attrId = rawGeom->findAttribute(name.cStr);
        if (attrId < 0)
            return;
        
        std::vector<Eigen::Vector2f> ptVec;
        ConvertFloat2fArray(env,floatArray,ptVec);
        
        rawGeom->addAttribute(name.cStr,GeomRawFloat2Type);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRawPoints::addPoint2fValues()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRawPoints_addPoint3fValues
(JNIEnv *env, jobject obj, jstring nameStr, jfloatArray floatArray)
{
    try
    {
        GeometryRawPointsClassInfo *classInfo = GeometryRawPointsClassInfo::getClassInfo();
        GeometryRawPoints *rawGeom = classInfo->getObject(env, obj);
        
        if (!rawGeom)
            return;
        JavaString name(env,nameStr);
        int attrId = rawGeom->findAttribute(name.cStr);
        if (attrId < 0)
            return;
        
        std::vector<Eigen::Vector3f> ptVec;
        ConvertFloat3fArray(env,floatArray,ptVec);
        
        rawGeom->addAttribute(name.cStr,GeomRawFloat3Type);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRawPoints::addPoint3fValues()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRawPoints_addPoint3dValues
(JNIEnv *env, jobject obj, jstring nameStr, jdoubleArray doubleArray)
{
    try
    {
        GeometryRawPointsClassInfo *classInfo = GeometryRawPointsClassInfo::getClassInfo();
        GeometryRawPoints *rawGeom = classInfo->getObject(env, obj);
        
        if (!rawGeom)
            return;
        JavaString name(env,nameStr);
        int attrId = rawGeom->findAttribute(name.cStr);
        if (attrId < 0)
            return;
        
        std::vector<Eigen::Vector3d> ptVec;
        ConvertFloat3dArray(env,doubleArray,ptVec);
        
        rawGeom->addAttribute(name.cStr,GeomRawDouble3Type);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRawPoints::addPoint3dValues()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRawPoints_addPoint4fValues
(JNIEnv *env, jobject obj, jstring nameStr, jfloatArray floatArray)
{
    try
    {
        GeometryRawPointsClassInfo *classInfo = GeometryRawPointsClassInfo::getClassInfo();
        GeometryRawPoints *rawGeom = classInfo->getObject(env, obj);
        
        if (!rawGeom)
            return;
        JavaString name(env,nameStr);
        int attrId = rawGeom->findAttribute(name.cStr);
        if (attrId < 0)
            return;
        
        std::vector<Eigen::Vector4f> ptVec;
        ConvertFloat4fArray(env,floatArray,ptVec);
        
        rawGeom->addAttribute(name.cStr,GeomRawFloat4Type);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRawPoints::addPoint4fValues()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_GeometryRawPoints_addAttribute
(JNIEnv *env, jobject obj, jstring nameStr, int type)
{
    try
    {
        GeometryRawPointsClassInfo *classInfo = GeometryRawPointsClassInfo::getClassInfo();
        GeometryRawPoints *rawGeom = classInfo->getObject(env, obj);
        
        if (!rawGeom)
            return -1;

        JavaString name(env,nameStr);
        return rawGeom->addAttribute(name.cStr,(GeomRawDataType)type);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRawPoints::addAttribute()");
    }
}
