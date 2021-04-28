/*  Matrix3d_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
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

#import <jni.h>
#import "Geometry_jni.h"
#import "com_mousebird_maply_Matrix3d.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> Matrix3dClassInfo *Matrix3dClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Matrix3d_nativeInit(JNIEnv *env, jclass cls)
{
    Matrix3dClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Matrix3d_initialise(JNIEnv *env, jobject obj)
{
    try
    {
        auto mat = new Matrix3d(Matrix3d::Identity());
        Matrix3dClassInfo::getClassInfo()->setHandle(env,obj,mat);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in Matrix3d::initialise()");
    }
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Matrix3d_dispose(JNIEnv *env, jobject obj)
{
    try
    {
        Matrix3dClassInfo *classInfo = Matrix3dClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            Matrix3d *inst = classInfo->getObject(env,obj);
            delete inst;
            classInfo->clearHandle(env,obj);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in Matrix3d::dispose()");
    }
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Matrix3d_inverse(JNIEnv *env, jobject obj)
{
    try
    {
        Matrix3dClassInfo *classInfo = Matrix3dClassInfo::getClassInfo();
        if (Matrix3d *inst = classInfo->getObject(env,obj))
        {
            return MakeMatrix3d(env,inst->inverse());
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in Matrix3d::inverse()");
    }
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Matrix3d_transpose(JNIEnv *env, jobject obj)
{
    try
    {
        Matrix3dClassInfo *classInfo = Matrix3dClassInfo::getClassInfo();
        if (Matrix3d *inst = classInfo->getObject(env,obj))
        {
            return MakeMatrix3d(env,inst->transpose());
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in Matrix3d::transpose()");
    }
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Matrix3d_multiply__Lcom_mousebird_maply_Point3d_2(JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        Matrix3dClassInfo *classInfo = Matrix3dClassInfo::getClassInfo();
        Point3dClassInfo *ptClassInfo = Point3dClassInfo::getClassInfo();
        if (Matrix3d *mat = classInfo->getObject(env,obj))
        {
            if (Point3d *pt = ptClassInfo->getObject(env,ptObj))
            {
                return MakePoint3d(env,(*mat) * (*pt));
            }
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in Matrix3d::multiply()");
    }
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Matrix3d_multiply__Lcom_mousebird_maply_Matrix3d_2(JNIEnv *env, jobject obj, jobject mtxObj)
{
    try
    {
        Matrix3dClassInfo *classInfo = Matrix3dClassInfo::getClassInfo();
        if (auto mat = classInfo->getObject(env,obj))
        {
            if (auto mtx = classInfo->getObject(env, mtxObj))
            {
                return MakeMatrix3d(env, (*mat) * (*mtx));
            }
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in Matrix3d::multiply()");
    }
    
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Matrix3d_translate(JNIEnv *env, jclass, jdouble x, jdouble y)
{
    try
    {
        return MakeMatrix3d(env,Affine2d(Eigen::Translation2d(x,y)).matrix());
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in Matrix3d::translateX()");
    }
    
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Matrix3d_scale(JNIEnv *env, jclass, jdouble x, jdouble y)
{
    try
    {
        return MakeMatrix3d(env,Affine2d(Eigen::Scaling(x,y)).matrix());
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in Matrix3d::scaleX()");
    }
    
    return nullptr;
}

jobject MakeMatrix3d(JNIEnv *env,const Eigen::Matrix3d &mat)
{
    // Make a Java Matrix3d
    Matrix3dClassInfo *classInfo = Matrix3dClassInfo::getClassInfo(env,"com/mousebird/maply/Matrix3d");
    jobject newObj = classInfo->makeWrapperObject(env,NULL);
    Eigen::Matrix3d *inst = classInfo->getObject(env,newObj);
    *inst = mat;

    return newObj;
}

