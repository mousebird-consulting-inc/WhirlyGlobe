/*  ShapeLinear_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg
 *  Copyright 2011-2022 mousebird consulting
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

#import "Shapes_jni.h"
#import "Geometry_jni.h"
#import "com_mousebird_maply_ShapeLinear.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> LinearClassInfo *LinearClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeLinear_nativeInit
  (JNIEnv *env, jclass cls)
{
    LinearClassInfo::getClassInfo(env, cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeLinear_initialise
  (JNIEnv *env, jobject obj)
{
    try
    {
        LinearClassInfo::set(env, obj, new Linear());
    }
    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeLinear_dispose
  (JNIEnv *env, jobject obj)
{
    try
    {
        LinearClassInfo *classInfo = LinearClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        delete classInfo->getObject(env, obj);
        classInfo->clearHandle(env, obj);
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeLinear_setCoords
  (JNIEnv *env, jobject obj, jobjectArray ptsArray)
{
    try
    {
        if (Linear *inst = LinearClassInfo::get(env, obj))
        {
            Point3dClassInfo *ptClassInfo = Point3dClassInfo::getClassInfo();
            JavaObjectArrayHelper ptsHelp(env, ptsArray);
            inst->pts.reserve(ptsHelp.numObjects());
            while (jobject ptObj = ptsHelp.getNextObject())
            {
                inst->pts.push_back(*ptClassInfo->getObject(env, ptObj));
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
}
