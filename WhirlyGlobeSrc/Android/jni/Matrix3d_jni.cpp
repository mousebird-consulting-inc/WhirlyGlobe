/*
 *  Matrix3d_jni.cpp
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
#import "Maply_utils_jni.h"
#import "com_mousebird_maply_Matrix3d.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;


JNIEXPORT void JNICALL Java_com_mousebird_maply_Matrix3d_nativeInit
(JNIEnv *env, jclass cls)
{
    Matrix3dClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Matrix3d_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        Matrix3d *mat = new Matrix3d();
        *mat = Matrix3d::Identity();
        Matrix3dClassInfo::getClassInfo()->setHandle(env,obj,mat);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Matrix3d::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Matrix3d_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        Matrix3dClassInfo *classInfo = Matrix3dClassInfo::getClassInfo();
        Matrix3d *inst = classInfo->getObject(env,obj);
        if (!inst)
            return;
        delete inst;
        
        classInfo->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Matrix3d::dispose()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Matrix3d_inverse
(JNIEnv *env, jobject obj)
{
    try
    {
        Matrix3dClassInfo *classInfo = Matrix3dClassInfo::getClassInfo();
        Matrix3d *inst = classInfo->getObject(env,obj);
        if (!inst)
            return NULL;
        
        Matrix3d matInv = inst->inverse();
        return MakeMatrix3d(env,matInv);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Matrix3d::inverse()");
    }
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Matrix3d_transpose
(JNIEnv *env, jobject obj)
{
    try
    {
        Matrix3dClassInfo *classInfo = Matrix3dClassInfo::getClassInfo();
        Matrix3d *inst = classInfo->getObject(env,obj);
        if (!inst)
            return NULL;
        
        Matrix3d matTrans = inst->transpose();
        return MakeMatrix3d(env,matTrans);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Matrix3d::transpose()");
    }
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Matrix3d_multiply__Lcom_mousebird_maply_Point3d_2
(JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        Matrix3dClassInfo *classInfo = Matrix3dClassInfo::getClassInfo();
        Matrix3d *mat = classInfo->getObject(env,obj);
        Point3dClassInfo *ptClassInfo = Point3dClassInfo::getClassInfo();
        Point3d *pt = ptClassInfo->getObject(env,ptObj);
        if (!mat || !pt)
            return NULL;
        
        Point3d ret = (*mat) * (*pt);
        
        return MakePoint3d(env,ret);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Matrix3d::multiply()");
    }
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Matrix3d_multiply__Lcom_mousebird_maply_Matrix3d_2
(JNIEnv *env, jobject obj, jobject mtxObj)
{
    try
    {
        Matrix3dClassInfo *classInfo = Matrix3dClassInfo::getClassInfo();
        Matrix3d *mat = classInfo->getObject(env,obj);
        Matrix3d *mtx = classInfo->getObject(env,mtxObj);
        if (!mat || !mtx)
            return NULL;
        
        Matrix3d ret = (*mat) * (*mtx);
        
        return MakeMatrix3d(env,ret);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Matrix3d::multiply()");
    }
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Matrix3d_traslateX
(JNIEnv *env, jclass cls, jdouble x, jdouble y)
{
    try
    {
        Matrix3dClassInfo *classInfo = Matrix3dClassInfo::getClassInfo(env, cls);
        
        Affine2d trans(Eigen::Translation2d(x,y));
        Matrix3d mat = trans.matrix();

        return MakeMatrix3d(env,mat);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Matrix3d::traslateX()");
    }
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Matrix3d_multiplyTrasX
(JNIEnv *env, jclass cls, jdouble x, jdouble y, jobject ptObj)
{
    try
    {
        Matrix3dClassInfo *classInfo = Matrix3dClassInfo::getClassInfo(env, cls);
        
        Affine2d trans(Eigen::Translation2d(x,y));
        
        Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!pt)
            return NULL;
        
        *pt = trans * (*pt);
        
        return MakePoint2d(env,*pt);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Matrix3d::multiplyTrasX()");
    }
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Matrix3d_scaleX
(JNIEnv *env, jclass cls, jdouble x, jdouble y)
{
    try
    {
        Matrix3dClassInfo *classInfo = Matrix3dClassInfo::getClassInfo(env, cls);
        
        Affine2d trans(Eigen::Scaling(x,y));
        Matrix3d mat = trans.matrix();
        
        return MakeMatrix3d(env,mat);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Matrix3d::scaleX()");
    }
    
    return NULL;
}

jobject MakeMatrix3d(JNIEnv *env,const Eigen::Matrix3d &mat)
{
    // Make a Java Matrix3d
    Matrix3dClassInfo *classInfo = Matrix3dClassInfo::getClassInfo(env,"com/mousebird/maply/Matrix3d");
    Matrix3d *newMat = new Matrix3d(mat);
    return classInfo->makeWrapperObject(env,newMat);
}

