/*
 *  GeometryRaw_jni.cpp
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
#import "com_mousebird_maply_GeometryRaw.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRaw_nativeInit
(JNIEnv *env, jclass cls)
{
    GeometryRawClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRaw_initialise
(JNIEnv *env, jobject obj, jobject sceneObj)
{
    try
    {
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!scene)
            return;
        GeometryRaw *geom = new GeometryRaw();
        GeometryRawClassInfo::getClassInfo()->setHandle(env,obj,geom);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRaw::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRaw_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        GeometryRawClassInfo *classInfo = GeometryRawClassInfo::getClassInfo();
        classInfo->clearHandle(env, obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRaw::dispose()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_GeometryRaw_valid
(JNIEnv *env, jobject obj)
{
    try
    {
        GeometryRawClassInfo *classInfo = GeometryRawClassInfo::getClassInfo();
        GeometryRaw *rawGeom = classInfo->getObject(env, obj);
        if (!rawGeom)
            return false;
        
        return rawGeom->isValid();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRaw::valid()");
    }
    
    return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRaw_setTypeNative
(JNIEnv *env, jobject obj, jint type)
{
    try
    {
        GeometryRawClassInfo *classInfo = GeometryRawClassInfo::getClassInfo();
        GeometryRaw *rawGeom = classInfo->getObject(env, obj);
        if (!rawGeom)
            return;
        
        rawGeom->type = (WhirlyKitGeometryRawType)type;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRaw::setType()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRaw_setTextureNative
(JNIEnv *env, jobject obj, jlong texID)
{
    try
    {
        GeometryRawClassInfo *classInfo = GeometryRawClassInfo::getClassInfo();
        GeometryRaw *rawGeom = classInfo->getObject(env, obj);
        if (!rawGeom)
            return;
        
        rawGeom->texId = texID;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRaw::setType()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRaw_addPoints
(JNIEnv *env, jobject obj, jdoubleArray doubleArray)
{
    try
    {
        GeometryRawClassInfo *classInfo = GeometryRawClassInfo::getClassInfo();
        GeometryRaw *rawGeom = classInfo->getObject(env, obj);
        if (!rawGeom)
            return;
        
        ConvertFloat3dArray(env,doubleArray,rawGeom->pts);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRaw::addPoints()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRaw_addNorms
(JNIEnv *env, jobject obj, jdoubleArray doubleArray)
{
    try
    {
        GeometryRawClassInfo *classInfo = GeometryRawClassInfo::getClassInfo();
        GeometryRaw *rawGeom = classInfo->getObject(env, obj);
        if (!rawGeom)
            return;
        
        ConvertFloat3dArray(env,doubleArray,rawGeom->norms);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRaw::addNorms()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRaw_addTexCoords
(JNIEnv *env, jobject obj, jfloatArray floatArray)
{
    try
    {
        GeometryRawClassInfo *classInfo = GeometryRawClassInfo::getClassInfo();
        GeometryRaw *rawGeom = classInfo->getObject(env, obj);
        if (!rawGeom)
            return;

        std::vector<Eigen::Vector2f> coords;
        ConvertFloat2fArray(env,floatArray,coords);
        rawGeom->texCoords.reserve(coords.size());
        for (auto coord : coords)
            rawGeom->texCoords.push_back(TexCoord(coord.x(),coord.y()));
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRaw::addTexCoords()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRaw_addColors
(JNIEnv *env, jobject obj, jintArray intArray)
{
    try
    {
        GeometryRawClassInfo *classInfo = GeometryRawClassInfo::getClassInfo();
        GeometryRaw *rawGeom = classInfo->getObject(env, obj);
        if (!rawGeom)
            return;

        std::vector<int> intVec;
        ConvertIntArray(env,intArray,intVec);
        rawGeom->colors.reserve(intVec.size());
        for (unsigned int ii=0;ii<intVec.size();ii++)
        {
            const int &iVal = intVec[ii];
            RGBAColor color((iVal >> 16) & 0xff,(iVal >> 8) & 0xff,(iVal) & 0xff,(iVal >> 24) & 0xff);
            rawGeom->colors.push_back(color);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRaw::addColors()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryRaw_addTriangles
(JNIEnv *env, jobject obj, jintArray intArray)
{
    try
    {
        GeometryRawClassInfo *classInfo = GeometryRawClassInfo::getClassInfo();
        GeometryRaw *rawGeom = classInfo->getObject(env, obj);
        if (!rawGeom)
            return;
        
        std::vector<int> intVec;
        ConvertIntArray(env,intArray,intVec);
        rawGeom->triangles.resize(intVec.size()/3);
        for (unsigned int ii=0;ii<rawGeom->triangles.size();ii++)
            rawGeom->triangles[ii] = GeometryRaw::RawTriangle(intVec[3*ii+0],intVec[3*ii+1],intVec[3*ii+2]);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryRaw::addTriangles()");
    }
}
