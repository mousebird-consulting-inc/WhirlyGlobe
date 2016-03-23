/*
 *  Proj4CoordSystem_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/13/16.
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
#import "com_mousebird_maply_Proj4CoordSystem.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Proj4CoordSystem_nativeInit
(JNIEnv *env, jclass cls)
{
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Proj4CoordSystem_initialise
(JNIEnv *env, jobject obj, jstring str)
{
    try
    {
        JavaString jstr(env,str);
        Proj4CoordSystem *coordSystem = new Proj4CoordSystem((std::string)jstr.cStr);
        CoordSystemClassInfo::getClassInfo()->setHandle(env,obj,coordSystem);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Proj4CoordSystem::initialise()");
    }
}
