/*
 *  GeneralDisplayAdapter_jni.cpp
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
#import "Maply_utils_jni.h"
#import "com_mousebird_maply_GeneralDisplayAdapter.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeneralDisplayAdapter_nativeInit
(JNIEnv *env, jclass cls)
{
    GeneralDisplayAdapterInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeneralDisplayAdapter_initialise
(JNIEnv *env, jobject obj, jobject coordSysObj,jobject llObj, jobject urObj, jobject centerObj, jobject scaleObj)
{
    try
    {
        GeneralDisplayAdapterInfo *classInfo = GeneralDisplayAdapterInfo::getClassInfo();
        CoordSystem *coordSys = CoordSystemClassInfo::getClassInfo()->getObject(env,coordSysObj);
        Point3d *ll = Point3dClassInfo::getClassInfo()->getObject(env,llObj);
        Point3d *ur = Point3dClassInfo::getClassInfo()->getObject(env,urObj);
        Point3d *center = Point3dClassInfo::getClassInfo()->getObject(env,centerObj);
        Point3d *scale = Point3dClassInfo::getClassInfo()->getObject(env,scaleObj);
        GeneralCoordSystemDisplayAdapter *coordAdapter = new GeneralCoordSystemDisplayAdapter(coordSys,*ll,*ur,*center,*scale);
        classInfo->setHandle(env,obj,coordAdapter);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystemDisplayAdapter::initialise()");
    }
}

/*
 * Class:     com_mousebird_maply_CoordSystemDisplayAdapter
 * Method:    dispose
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_GeneralDisplayAdapter_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        GeneralDisplayAdapterInfo *classInfo = GeneralDisplayAdapterInfo::getClassInfo();
        GeneralCoordSystemDisplayAdapter *coordAdapter = classInfo->getObject(env,obj);
        if (!coordAdapter)
            return;
        delete coordAdapter;
        
        classInfo->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystemDisplayAdapter::dispose()");
    }
}
