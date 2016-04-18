/*
 *  SingleBillboardPoly_jni.cpp
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

#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_SingleBillboardPoly.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_SingleBillboardPoly_nativeInit
(JNIEnv *env, jclass cls)
{
    SingleBillboardPolyClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SingleBillboardPoly_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        SingleBillboardPoly *inst = new SingleBillboardPoly();
        SingleBillboardPolyClassInfo *classInfo = SingleBillboardPolyClassInfo::getClassInfo();
        classInfo->setHandle(env, obj, inst);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleBillboardPoly::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SingleBillboardPoly_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        SingleBillboardPolyClassInfo *classInfo = SingleBillboardPolyClassInfo::getClassInfo();
        SingleBillboardPoly *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        delete inst;
        classInfo->clearHandle(env, obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleBillboardPoly::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SingleBillboardPoly_addPoint
(JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        SingleBillboardPolyClassInfo *classInfo = SingleBillboardPolyClassInfo::getClassInfo();
        SingleBillboardPoly *inst = classInfo->getObject(env, obj);
        Point2d *newPt = Point2dClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !newPt)
            return;
        inst->pts.push_back(*newPt);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleBillboardPoly::addPt()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SingleBillboardPoly_addTexCoord
(JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        SingleBillboardPolyClassInfo *classInfo = SingleBillboardPolyClassInfo::getClassInfo();
        SingleBillboardPoly *inst = classInfo->getObject(env, obj);
        Point2d *newPt = Point2dClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !newPt)
            return;
        inst->texCoords.push_back(TexCoord(newPt->x(), newPt->y()));
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleBillboardPoly::addTexCoord()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SingleBillboardPoly_addColor
(JNIEnv *env, jobject obj, jfloat r, jfloat b, jfloat g, jfloat a)
{
    try
    {
        SingleBillboardPolyClassInfo *classInfo = SingleBillboardPolyClassInfo::getClassInfo();
        SingleBillboardPoly *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->color = new RGBAColor(r*255.0,g*255.0,b*255.0,a*255.0);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleBillboardPoly::addColor()");
    }
}

JNIEXPORT jfloatArray JNICALL Java_com_mousebird_maply_SingleBillboardPoly_getColor
(JNIEnv *env, jobject obj)
{
    try
    {
        SingleBillboardPolyClassInfo *classInfo = SingleBillboardPolyClassInfo::getClassInfo();
        SingleBillboardPoly *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        float * primaryColors = new float[4];
        inst->color->asUnitFloats(primaryColors);
        jfloatArray result;
        result = env->NewFloatArray(4);
        env->SetFloatArrayRegion(result, 0, 4, primaryColors);
        free(primaryColors);
        return result;

    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleBillboardPoly::getColor()");
    }
    return NULL;

}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SingleBillboardPoly_setTexID
(JNIEnv *env, jobject obj, jlong texID)
{
    try
    {
        SingleBillboardPolyClassInfo *classInfo = SingleBillboardPolyClassInfo::getClassInfo();
        SingleBillboardPoly *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->texId = texID;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleBillboardPoly::setTexID()");
    }
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_SingleBillboardPoly_getTexID
(JNIEnv *env, jobject obj)
{
    try
    {
        SingleBillboardPolyClassInfo *classInfo = SingleBillboardPolyClassInfo::getClassInfo();
        SingleBillboardPoly *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        return inst->texId;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleBillboardPoly::getTexID()");
    }
    return -1;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SingleBillboardPoly_addVertexAttribute
(JNIEnv *env, jobject obj, jobject vertexObj)
{
    try
    {
        SingleBillboardPolyClassInfo *classInfo = SingleBillboardPolyClassInfo::getClassInfo();
        SingleBillboardPoly *inst = classInfo->getObject(env, obj);
        SingleVertexAttribute *vertex = SingleVertexAttributeClassInfo::getClassInfo()->getObject(env, vertexObj);
        if (!inst || !vertex)
            return;
        inst->vertexAttrs.insert(*vertex);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleBillboardPoly::addVertexAttribute()");
    }
}