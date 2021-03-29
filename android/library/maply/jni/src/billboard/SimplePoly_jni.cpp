/*
 *  SimplePoly_jni.cpp
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
#import "Billboard_jni.h"
#import "Geometry_jni.h"
#import "com_mousebird_maply_SimplePoly.h"
#import <android/bitmap.h>

template<> SimplePolyClassInfo *SimplePolyClassInfo::classInfoObj = NULL;

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_SimplePoly_nativeInit
(JNIEnv *env, jclass cls)
{
    SimplePolyClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SimplePoly_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = new SimplePoly();
        classInfo->setHandle(env, obj, inst);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SimplePoly_initialise__JFFFF_3Lcom_mousebird_maply_Point2d_2_3Lcom_mousebird_maply_Point2d_2
        (JNIEnv *env, jobject obj, jlong texID, jfloat r, jfloat g, jfloat b, jfloat a, jobjectArray ptsArray, jobjectArray texArray)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = new SimplePoly();

        inst->texID = texID;
        //color

        inst->color = RGBAColor(r*255.0,g*255.0,b*255.0,a*255.0);

        //Pts array

        Point2dClassInfo *ptClassInfo = Point2dClassInfo::getClassInfo();
        JavaObjectArrayHelper ptArrayHelper(env,ptsArray);
        while (jobject ptObj = ptArrayHelper.getNextObject()) {
            Point2d *pt = ptClassInfo->getObject(env,ptObj);
            inst->pts.push_back(*pt);
        }

        //TexCoord array

        JavaObjectArrayHelper texArrayHelper(env,texArray);
        while (jobject texObj = texArrayHelper.getNextObject()) {
            Point2d *pt = ptClassInfo->getObject(env,texObj);
            inst->texCoords.push_back(TexCoord(pt->x(), pt->y()));
        }

        classInfo->setHandle(env, obj, inst);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_SimplePoly_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            SimplePoly *inst = classInfo->getObject(env, obj);
            if (!inst)
                return;
            
            delete inst;
            classInfo->clearHandle(env, obj);
        }
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SimplePoly_addTextureNative
        (JNIEnv *env, jobject obj, jlong texID)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->texID = texID;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::addTextureNative()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SimplePoly_addColor
        (JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->color = RGBAColor(r*255.0,g*255.0,b*255.0,a*255.0);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::addColor()");
    }

}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SimplePoly_addPt
(JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = classInfo->getObject(env, obj);
        Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !pt)
            return;
        
        inst->pts.push_back(*pt);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::addPt()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SimplePoly_addPts
(JNIEnv *env, jobject obj, jobjectArray ptsArray)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        Point2dClassInfo *ptClassInfo = Point2dClassInfo::getClassInfo();
        JavaObjectArrayHelper ptArrayHelper(env,ptsArray);
        while (jobject ptObj = ptArrayHelper.getNextObject()) {
            Point2d *pt = ptClassInfo->getObject(env,ptObj);
            inst->pts.push_back(*pt);
        }
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::addPts()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SimplePoly_setPt
(JNIEnv *env, jobject obj, jint index, jobject ptObj)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = classInfo->getObject(env, obj);
        Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !pt)
            return;
        if (index > inst->pts.size())
            return;
        inst->pts.at(index) = *pt;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::setPt()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SimplePoly_addTexCoord
(JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = classInfo->getObject(env, obj);
        Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !pt)
            return;
        
        inst->texCoords.push_back(TexCoord(pt->x(), pt->y()));
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::addTexCoord()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SimplePoly_addTexCoords
(JNIEnv *env, jobject obj, jobjectArray texArray)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        Point2dClassInfo *ptClassInfo = Point2dClassInfo::getClassInfo();
        JavaObjectArrayHelper ptArrayHelper(env,texArray);
        while (jobject ptObj = ptArrayHelper.getNextObject()) {
            Point2d *pt = ptClassInfo->getObject(env,ptObj);
            inst->texCoords.push_back(TexCoord(pt->x(), pt->y()));
        }
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::addTexCoords()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SimplePoly_setTexCoord
(JNIEnv *env, jobject obj, jint index, jobject ptObj)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = classInfo->getObject(env, obj);
        Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !pt)
            return;
        if (index >= inst->texCoords.size())
            return;
        inst->texCoords.at(index) = TexCoord(pt->x(), pt->y());
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::setTexCoord()");
    }
}
