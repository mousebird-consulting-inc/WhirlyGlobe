/*
 *  ShapeLinear_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg
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

#import "Shapes_jni.h"
#import "Geometry_jni.h"
#import "com_mousebird_maply_ShapeLinear.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> LinearClassInfo *LinearClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeLinear_nativeInit
(JNIEnv *env, jclass cls)
{
    LinearClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeLinear_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        LinearClassInfo *classInfo = LinearClassInfo::getClassInfo();
        Linear *inst = new Linear();
        classInfo->setHandle(env, obj, inst);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeLinear::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeLinear_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        LinearClassInfo *classInfo = LinearClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            Linear *inst = classInfo->getObject(env, obj);
            if (!inst)
                return;
            delete inst;
            classInfo->clearHandle(env, obj);
        }
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeLinear::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeLinear_setCoords
  (JNIEnv *env, jobject obj, jobjectArray ptsArray)
{
    try
    {
        LinearClassInfo *classInfo = LinearClassInfo::getClassInfo();
        Linear *inst = classInfo->getObject(env, obj);
        Point3dClassInfo *ptClassInfo = Point3dClassInfo::getClassInfo();
        if (!inst)
            return;

        JavaObjectArrayHelper ptsHelp(env,ptsArray);
        while (jobject ptObj = ptsHelp.getNextObject()) {
            Point3d *pt = ptClassInfo->getObject(env,ptObj);
            inst->pts.push_back(*pt);
        }
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeLinear::setCoords()");
    }
}
