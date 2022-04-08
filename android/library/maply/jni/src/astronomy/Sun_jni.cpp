/*  Sun_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/16.
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

#import <AA+.h>
#import "Astronomy_jni.h"
#import "Geometry_jni.h"
#import "com_mousebird_maply_Sun.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> SunClassInfo *SunClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Sun_nativeInit
  (JNIEnv *env, jclass cls)
{
    SunClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Sun_initialise
  (JNIEnv *env, jobject obj)
{
    try
    {
        SunClassInfo::getClassInfo()->setHandle(env,obj,new Sun());
    }
    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Sun_dispose
  (JNIEnv *env, jobject obj)
{
    try
    {
        SunClassInfo *classInfo = SunClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        delete classInfo->getObject(env,obj);
        classInfo->clearHandle(env,obj);
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Sun_getPosition
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (Sun *inst = SunClassInfo::get(env,obj))
        {
            return MakePoint2d(env, Point2d(inst->sunLon, inst->sunLat));
        }
    }
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Sun_getDirection
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (Sun *inst = SunClassInfo::get(env,obj))
        {
            return MakePoint3d(env, inst->getDirection());
        }
    }
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Sun_setTime
  (JNIEnv *env, jobject obj, jdouble theTime, jdouble year, jdouble month, jdouble day, jdouble hour, jdouble minute, jdouble second)
{
    try
    {
        if (Sun *inst = SunClassInfo::get(env,obj))
        {
            inst->time = theTime;
            inst->setTime(year, month, day, hour, minute, second);
        }
    }
    MAPLY_STD_JNI_CATCH()
}
