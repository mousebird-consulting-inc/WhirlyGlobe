/*
 *  Affine2d_jni.cpp
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
#import "com_mousebird_maply_Affine2d.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Affine2d_nativeInit
(JNIEnv *env, jclass cls)
{
    Affine2dClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Affine2d_initialise
(JNIEnv *env, jobject obj, jdouble x, jdouble y, jint type)
{
    try
    {
        Affine2dClassInfo *classInfo = Affine2dClassInfo::getClassInfo();
        Affine2d *inst;
        switch (type) {
            case 0:
                inst = new Affine2d(Eigen::Scaling(x,y));
                break;
            case 1:
                inst = new Affine2d(Eigen::Translation2d(x,y));
                break;
            default:
                inst = new Affine2d();
                break;
        }
        classInfo->setHandle(env, obj, inst);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Affine2d::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Affine2d_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        Affine2dClassInfo *classInfo = Affine2dClassInfo::getClassInfo();
        Affine2d *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        delete inst;
        classInfo->clearHandle(env, obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Affine2d::dispose()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Affine2d_matrix
(JNIEnv *env, jobject obj)
{
    try
    {
        Affine2dClassInfo *classInfo = Affine2dClassInfo::getClassInfo();
        Affine2d *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        return MakeMatrix3d(env,inst->matrix());

    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Affine2d::matrix()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Affine2d_multiply
(JNIEnv *env, jobject obj, jobject objPt)
{
    try
    {
        Affine2dClassInfo *classInfo = Affine2dClassInfo::getClassInfo();
        Affine2d *inst = classInfo->getObject(env, obj);
        Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env, objPt);
        if (!inst || !pt)
            return NULL;
        return MakePoint2d(env,((*inst)*(*pt)));
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Affine2d::multiply()");
    }
}
