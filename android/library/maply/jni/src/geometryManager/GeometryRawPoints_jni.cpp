/*  GeometryRawPoints_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg
 *  Copyright 2011-2021 mousebird consulting
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
#import "GeometryManager_jni.h"
#import "com_mousebird_maply_GeometryRawPoints.h"

using namespace WhirlyKit;
using namespace Maply;

template<> GeometryRawPointsClassInfo *GeometryRawPointsClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRawPoints_nativeInit(JNIEnv *env, jclass cls)
{
    GeometryRawPointsClassInfo::getClassInfo(env, cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRawPoints_initialise(JNIEnv *env, jobject obj)
{
    try
    {
        GeometryRawPoints *geomPoints = new GeometryRawPoints();
        GeometryRawPointsClassInfo::getClassInfo()->setHandle(env,obj,geomPoints);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRawPoints::initialise()");
    }
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRawPoints_dispose(JNIEnv *env, jobject obj)
{
    try
    {
        const auto classInfo = GeometryRawPointsClassInfo::getClassInfo();
        classInfo->clearHandle(env, obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRawPoints::dispose()");
    }
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_GeometryRawPoints_valid(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto rawGeom = GeometryRawPointsClassInfo::get(env, obj))
        {
            return rawGeom->valid();
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRawPoints::valid()");
    }
    
    return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRawPoints_addIntValues(JNIEnv *env, jobject obj, jstring nameStr, jintArray intArray)
{
    try
    {
        if (auto rawGeom = GeometryRawPointsClassInfo::get(env, obj))
        {
            const JavaString name(env, nameStr);
            const int attrId = rawGeom->findAttribute(StringIndexer::getStringID(name.getCString()));
            if (attrId >= 0)
            {
                std::vector<int> intVec;
                ConvertIntArray(env, intArray, intVec);
                rawGeom->addValues(attrId, intVec);
            }
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRawPoints::addIntValues()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRawPoints_addFloatValues(JNIEnv *env, jobject obj, jstring nameStr, jfloatArray floatArray)
{
    try
    {
        if (auto rawGeom = GeometryRawPointsClassInfo::get(env, obj))
        {
            const JavaString name(env, nameStr);
            const int attrId = rawGeom->findAttribute(StringIndexer::getStringID(name.getCString()));
            if (attrId >= 0)
            {
                std::vector<float> floatVec;
                ConvertFloatArray(env, floatArray, floatVec);
                rawGeom->addValues(attrId, floatVec);
            }
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRawPoints::addFloatValues()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRawPoints_addPoint2fValues(JNIEnv *env, jobject obj, jstring nameStr, jfloatArray floatArray)
{
    try
    {
        if (auto rawGeom = GeometryRawPointsClassInfo::get(env, obj))
        {
            const JavaString name(env, nameStr);
            const int attrId = rawGeom->findAttribute(StringIndexer::getStringID(name.getCString()));
            if (attrId >= 0)
            {
                Point2fVector ptVec;
                ConvertFloat2fArray(env, floatArray, ptVec);
                rawGeom->addPoints(attrId, ptVec);
            }
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRawPoints::addPoint2fValues()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRawPoints_addPoint3fValues(JNIEnv *env, jobject obj, jstring nameStr, jfloatArray floatArray)
{
    try
    {
        if (auto rawGeom = GeometryRawPointsClassInfo::get(env, obj))
        {
            const JavaString name(env, nameStr);
            const int attrId = rawGeom->findAttribute(StringIndexer::getStringID(name.getCString()));
            if (attrId >= 0)
            {
                Point3fVector ptVec;
                ConvertFloat3fArray(env, floatArray, ptVec);
                rawGeom->addPoints(attrId, ptVec);
            }
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRawPoints::addPoint3fValues()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRawPoints_addPoint3dValues(JNIEnv *env, jobject obj, jstring nameStr, jdoubleArray doubleArray)
{
    try
    {
        if (auto rawGeom = GeometryRawPointsClassInfo::get(env, obj))
        {
            const JavaString name(env, nameStr);
            const int attrId = rawGeom->findAttribute(StringIndexer::getStringID(name.getCString()));
            if (attrId >= 0)
            {
                Point3dVector ptVec;
                ConvertFloat3dArray(env, doubleArray, ptVec);
                rawGeom->addPoints(attrId, ptVec);
            }
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRawPoints::addPoint3dValues()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRawPoints_addPoint4fValues
    (JNIEnv *env, jobject obj, jstring nameStr, jfloatArray floatArray)
{
    try
    {
        if (auto rawGeom = GeometryRawPointsClassInfo::get(env, obj))
        {
            const JavaString name(env, nameStr);
            const int attrId = rawGeom->findAttribute(StringIndexer::getStringID(name.getCString()));
            if (attrId >= 0)
            {
                Vector4fVector ptVec;
                ConvertFloat4fArray(env, floatArray, ptVec);
                rawGeom->addPoints(attrId, ptVec);
            }
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRawPoints::addPoint4fValues()");
    }
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_GeometryRawPoints_addAttributeNative(JNIEnv *env, jobject obj, jstring nameStr, int type)
{
    try
    {
        if (auto rawGeom = GeometryRawPointsClassInfo::get(env, obj))
        {
            const JavaString name(env, nameStr);
            return rawGeom->addAttribute(StringIndexer::getStringID(name.getCString()), (GeomRawDataType) type);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRawPoints::addAttribute()");
    }
    return -1;
}
