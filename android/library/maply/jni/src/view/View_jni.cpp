/*  View_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/17/15.
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

#import "View_jni.h"
#import "Geometry_jni.h"
#import "com_mousebird_maply_View.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> ViewClassInfo *ViewClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_View_nativeInit
  (JNIEnv *env, jclass cls)
{
	ViewClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_View_runViewUpdates
  (JNIEnv *env, jobject obj)
{
	try
	{
        if (auto *view = ViewClassInfo::get(env,obj))
        {
            view->runViewUpdates();
        }
	}
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_View_calcModelViewMatrix
  (JNIEnv *env, jobject obj)
{
	try
	{
        if (auto *view = ViewClassInfo::get(env,obj))
        {
            return MakeMatrix4d(env, view->calcViewMatrix() * view->calcModelMatrix());
        }
	}
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_View_heightForMapScale
  (JNIEnv *env, jobject obj, jdouble scale, jdouble frameSizeX, jdouble frameSizeY)
{
    try
    {
        if (auto *view = ViewClassInfo::get(env,obj))
        {
            return view->heightForMapScale(scale,Point2f(frameSizeX,frameSizeY));
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}

/*
 * Class:     com_mousebird_maply_View
 * Method:    currentMapZoom
 * Signature: (Lcom/mousebird/maply/Point2d;D)D
 */
extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_View_currentMapZoom
  (JNIEnv *env, jobject obj, jdouble frameSizeX, jdouble frameSizeY, jdouble lat)
{
    try
    {
        if (auto *view = ViewClassInfo::get(env,obj))
        {
            return view->currentMapZoom(Point2f(frameSizeX, frameSizeY), lat);
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0.0;    
}


/*
 * Class:     com_mousebird_maply_View
 * Method:    currentMapScale
 * Signature: (Lcom/mousebird/maply/Point2d;D)D
 */
extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_View_currentMapScale
  (JNIEnv *env, jobject obj, jdouble frameSizeX, jdouble frameSizeY)
{
    try
    {
        if (auto *view = ViewClassInfo::get(env,obj))
        {
            return view->currentMapScale(Point2f(frameSizeX,frameSizeY));
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}
