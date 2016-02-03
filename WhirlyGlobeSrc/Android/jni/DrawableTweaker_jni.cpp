/*
 *  DrawableTweaker_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2015 mousebird consulting
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
#import "com_mousebird_maply_DrawableTweaker.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;



JNIEXPORT void JNICALL Java_com_mousebird_maply_DrawableTweaker_nativeInit
(JNIEnv *env, jclass cls)
{
    DrawableTweakerClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_DrawableTweaker_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        DrawableTweakerClassInfo *classInfo = DrawableTweakerClassInfo::getClassInfo();
        DrawableTweaker *inst = new DrawableTweaker();
        classInfo->setHandle(env, obj, inst);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in DrawableTweaker::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_DrawableTweaker_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        DrawableTweakerClassInfo *classInfo = DrawableTweakerClassInfo::getClassInfo();
        DrawableTweaker *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        delete inst;
        classInfo->clearHandle(env, obj);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in DrawableTweaker::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_DrawableTweaker_tweakForFrame
(JNIEnv *env, jobject obj, jobject drawObj, jobject frameObj)
{
    try
    {
        DrawableTweakerClassInfo *classInfo = DrawableTweakerClassInfo::getClassInfo();
        DrawableTweaker *inst = classInfo->getObject(env, obj);
        Drawable *draw = DrawableClassInfo::getClassInfo()->getObject(env, drawObj);
        RendererFrameInfo *frame = RendererFrameInfoClassInfo::getClassInfo()->getObject(env, frameObj);
        if (!inst || !draw || !frame )
            return;
        inst->tweakForFrame(draw, frame);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in DrawableTweaker::tweakForFrame()");
    }
}
