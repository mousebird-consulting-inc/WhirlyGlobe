/*
 *  VectorStyleWrapper_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg
 *  Copyright 2011-2020 mousebird consulting
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

#import <Formats_jni.h>
#import <Geometry_jni.h>
#import <Vectors_jni.h>
#import <Components_jni.h>
#import <Scene_jni.h>
#import "com_mousebird_maply_VectorStyleWrapper.h"

using namespace WhirlyKit;
using namespace Eigen;

template<> VectorStyleSetWrapperClassInfo *VectorStyleSetWrapperClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleWrapper_nativeInit
(JNIEnv *env, jclass cls)
{
    VectorStyleSetWrapperClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleWrapper_initialise
(JNIEnv *env, jobject obj, jlongArray idArray, jobjectArray categoryArray, jbooleanArray geomAddArray)
{
    try {
        std::vector<SimpleIdentity > idVec;
        std::vector<std::string> catVec;
        std::vector<bool> geomAddVec;
        ConvertLongLongArray(env,idArray,idVec);
        ConvertStringArray(env,categoryArray, catVec);
        ConvertBoolArray(env,geomAddArray,geomAddVec);

        PlatformInfo_Android threadInst(env);

        VectorStyleSetWrapper_AndroidRef *inst = new VectorStyleSetWrapper_AndroidRef(
                new VectorStyleSetWrapper_Android(
                        &threadInst,
                        obj,
                        idVec, catVec, geomAddVec
                        ));
        VectorStyleSetWrapperClassInfo::getClassInfo()->setHandle(env,obj,inst);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleWrapper::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleWrapper_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSetWrapperClassInfo *classInfo = VectorStyleSetWrapperClassInfo::getClassInfo();

        {
            PlatformInfo_Android threadInst(env);

            std::lock_guard<std::mutex> lock(disposeMutex);
            VectorStyleSetWrapper_AndroidRef *inst = classInfo->getObject(env,obj);
            (*inst)->shutdown(&threadInst);
            if (!inst)
                return;
            delete inst;
        }

        classInfo->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleWrapper::dispose()");
    }
}

