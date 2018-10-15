/*
 *  Matrix4d_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
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
#import "com_mousebird_maply_Matrix4d.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Matrix4d_nativeInit
  (JNIEnv *env, jclass cls)
{
	Matrix4dClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Matrix4d_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		Matrix4d *mat = new Matrix4d();
		*mat = Matrix4d::Identity();
		Matrix4dClassInfo::getClassInfo()->setHandle(env,obj,mat);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Matrix4d::initialise()");
	}

}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Matrix4d_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		Matrix4dClassInfo *classInfo = Matrix4dClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            Matrix4d *inst = classInfo->getObject(env,obj);
            if (!inst)
                return;
            delete inst;

            classInfo->clearHandle(env,obj);
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Matrix4d::dispose()");
	}
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Matrix4d_inverse
  (JNIEnv *env, jobject obj)
{
	try
	{
		Matrix4dClassInfo *classInfo = Matrix4dClassInfo::getClassInfo();
		Matrix4d *inst = classInfo->getObject(env,obj);
		if (!inst)
			return NULL;

		Matrix4d matInv = inst->inverse();
		return MakeMatrix4d(env,matInv);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Matrix4d::inverse()");
	}
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Matrix4d_transpose
(JNIEnv *env, jobject obj)
{
    try
    {
        Matrix4dClassInfo *classInfo = Matrix4dClassInfo::getClassInfo();
        Matrix4d *inst = classInfo->getObject(env,obj);
        if (!inst)
            return NULL;
        
        Matrix4d matTrans = inst->transpose();
        return MakeMatrix4d(env,matTrans);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Matrix4d::transpose()");
    }
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Matrix4d_translate
(JNIEnv *env, jclass cls, jdouble x, jdouble y, jdouble z)
{
    try
    {
        Matrix4dClassInfo *classInfo = Matrix4dClassInfo::getClassInfo(env, cls);
        
        Affine3d trans(Eigen::Translation3d(x,y,z));
        Matrix4d mat = trans.matrix();
        
        return MakeMatrix4d(env,mat);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Matrix3d::translate()");
    }
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Matrix4d_scale
(JNIEnv *env, jclass cls, jdouble x, jdouble y, jdouble z)
{
    try
    {
        Matrix4dClassInfo *classInfo = Matrix4dClassInfo::getClassInfo(env, cls);
        
        Affine3d trans(Eigen::Scaling(x,y,z));
        Matrix4d mat = trans.matrix();
        
        return MakeMatrix4d(env,mat);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Matrix3d::scale()");
    }
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Matrix4d_multiply
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		Matrix4dClassInfo *classInfo = Matrix4dClassInfo::getClassInfo();
		Matrix4d *mat = classInfo->getObject(env,obj);
		Point4dClassInfo *ptClassInfo = Point4dClassInfo::getClassInfo();
		Point4d *pt = ptClassInfo->getObject(env,ptObj);
		if (!mat || !pt)
			return NULL;

		Point4d ret = (*mat) * (*pt);

		return MakePoint4d(env,ret);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Matrix4d::multiply()");
	}
    
    return NULL;
}

jobject MakeMatrix4d(JNIEnv *env,const Eigen::Matrix4d &mat)
{
	// Make a Java Matrix4d
	Matrix4dClassInfo *classInfo = Matrix4dClassInfo::getClassInfo(env,"com/mousebird/maply/Matrix4d");
	Matrix4d *newMat = new Matrix4d(mat);
	return classInfo->makeWrapperObject(env,newMat);
}

