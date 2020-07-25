/*
 *  FlatView_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/19/19.
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

#import "View_jni.h"
#import "CoordSystem_jni.h"
#import "Geometry_jni.h"
#import "com_mousebird_maply_FlatView.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> FlatViewClassInfo *FlatViewClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_FlatView_nativeInit
        (JNIEnv *env, jclass cls)
{
    FlatViewClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_FlatView_initialise
        (JNIEnv *env, jobject obj, jobject coordAdapterObj)
{
    try
    {
        CoordSystemDisplayAdapter *coordAdapter = CoordSystemDisplayAdapterInfo::getClassInfo()->getObject(env,coordAdapterObj);
        Maply::FlatView *inst = new Maply::FlatView(coordAdapter);
        FlatViewClassInfo::getClassInfo()->setHandle(env,obj,inst);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in FlatView::initialise()");
    }

}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_FlatView_dispose
        (JNIEnv *env, jobject obj)
{
    try
    {
        FlatViewClassInfo *classInfo = FlatViewClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            Maply::FlatView *inst = classInfo->getObject(env,obj);
            if (!inst)
                return;
            delete inst;

            classInfo->clearHandle(env,obj);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in FlatView::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_FlatView_nativeClone
        (JNIEnv *env, jobject obj, jobject destObj)
{
    try
    {
        FlatViewClassInfo *classInfo = FlatViewClassInfo::getClassInfo();
        Maply::FlatView *src = classInfo->getObject(env,obj);
        Maply::FlatView *dest = new Maply::FlatView(*src);
        FlatViewClassInfo::getClassInfo()->setHandle(env,destObj,dest);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in FlatView::nativeClone()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_FlatView_setExtentsNative
        (JNIEnv *env, jobject obj, jobject llObj, jobject urObj)
{
    try
    {
        Maply::FlatView *inst = FlatViewClassInfo::getClassInfo()->getObject(env,obj);
        Point2dClassInfo *pt2dClassInfo = Point2dClassInfo::getClassInfo();
        Point2d *ll = pt2dClassInfo->getObject(env,llObj);
        Point2d *ur = pt2dClassInfo->getObject(env,urObj);
        if (!inst || !ll || !ur)
            return;
        inst->setExtents(MbrD(*ll,*ur));
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in FlatView::setExtentsNative()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_FlatView_setWindow
        (JNIEnv *env, jobject obj, jobject sizeObj, jobject offsetObj)
{
    try
    {
        Maply::FlatView *inst = FlatViewClassInfo::getClassInfo()->getObject(env,obj);
        Point2dClassInfo *pt2dClassInfo = Point2dClassInfo::getClassInfo();
        Point2d *size = pt2dClassInfo->getObject(env,sizeObj);
        Point2d *offset = pt2dClassInfo->getObject(env,offsetObj);
        if (!inst || !size || !offset)
            return;
        inst->setWindow(*size,*offset);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in FlatView::setWindow()");
    }
}
