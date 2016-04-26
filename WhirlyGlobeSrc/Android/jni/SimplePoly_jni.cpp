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
#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_SimplePoly.h"
#import "WhirlyGlobe.h"
#import "Maply_utils_jni.h"

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

JNIEXPORT void JNICALL Java_com_mousebird_maply_SimplePoly_initialise__Lcom_mousebird_maply_Texture_2_3FLjava_util_List_2Ljava_util_List_2
(JNIEnv *env, jobject obj, jobject texObj, jfloatArray colorArray, jobject vecObjListPt, jobject vecObjListTC)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = new SimplePoly();
        Texture *tex = TextureClassInfo::getClassInfo()->getObject(env, texObj);
        if (!tex || !inst)
            return;

        inst->texture = *tex;
        //color

        jfloat *colors = env->GetFloatArrayElements(colorArray, 0);
        jsize len = env->GetArrayLength(colorArray);
        RGBAColor *color;
        if (len < 4) {
            color = new RGBAColor(0,0,0,0);
        }
        else {
            color = new RGBAColor(colors[0]*255.0,colors[1]*255.0,colors[2]*255.0,colors[3]*255.0);
        }

        inst->color = color;

        //Pts array

        jclass listClass = env->GetObjectClass(vecObjListPt);
        jclass iterClass = env->FindClass("java/util/Iterator");
        jmethodID literMethod = env->GetMethodID(listClass,"iterator","()Ljava/util/Iterator;");
        jobject liter = env->CallObjectMethod(vecObjListPt,literMethod);
        jmethodID hasNext = env->GetMethodID(iterClass,"hasNext","()Z");
        jmethodID next = env->GetMethodID(iterClass,"next","()Ljava/lang/Object;");
        env->DeleteLocalRef(iterClass);
        env->DeleteLocalRef(listClass);

        Point2dClassInfo *ptClassInfo = Point2dClassInfo::getClassInfo();
        while (env->CallBooleanMethod(liter, hasNext))
        {
            jobject javaVecObj = env->CallObjectMethod(liter, next);
            Point2d *pt = ptClassInfo->getObject(env,javaVecObj);
            inst->pts.push_back(*pt);
            env->DeleteLocalRef(javaVecObj);
        }
        env->DeleteLocalRef(liter);

        //TexCoord array

        listClass = env->GetObjectClass(vecObjListTC);
        literMethod = env->GetMethodID(listClass,"iterator","()Ljava/util/Iterator;");
        liter = env->CallObjectMethod(vecObjListTC,literMethod);
        next = env->GetMethodID(iterClass,"next","()Ljava/lang/Object;");
        env->DeleteLocalRef(iterClass);
        env->DeleteLocalRef(listClass);

        while (env->CallBooleanMethod(liter, hasNext))
        {
            jobject javaVecObj = env->CallObjectMethod(liter, next);
            Point2d *pt = ptClassInfo->getObject(env,javaVecObj);
            inst->texCoords.push_back(TexCoord(pt->x(), pt->y()));
            env->DeleteLocalRef(javaVecObj);
        }
        env->DeleteLocalRef(liter);

        classInfo->setHandle(env, obj, inst);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SimplePoly_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        delete inst;
        classInfo->clearHandle(env, obj);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SimplePoly_addImage
(JNIEnv *env, jobject obj, jobject texObj)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = classInfo->getObject(env, obj);
        Texture *tex = TextureClassInfo::getClassInfo()->getObject(env, texObj);
        if (!inst || !tex)
            return;
        
        inst->texture = *tex;
        
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::addImage()");
    }

}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SimplePoly_addColor
(JNIEnv *env, jobject obj, jfloatArray colorArray)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        jfloat *colors = env->GetFloatArrayElements(colorArray, 0);
        jsize len = env->GetArrayLength(colorArray);
        RGBAColor *color;
        if (len <4){
            color = new RGBAColor(0,0,0,0);
        }
        else{
            color = new RGBAColor(colors[0]*255.0,colors[1]*255.0,colors[2]*255.0,colors[3]*255.0);
        }
        inst->color = color;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::addColor()");
    }

}

JNIEXPORT jfloatArray JNICALL Java_com_mousebird_maply_SimplePoly_getColor
(JNIEnv *env, jobject obj)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        float primaryColors[4];
        inst->color->asUnitFloats(primaryColors);
        jfloatArray result;
        result = env->NewFloatArray(4);
        env->SetFloatArrayRegion(result, 0, 4, primaryColors);
        return result;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::getColor()");
    }
    return NULL;
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
(JNIEnv *env, jobject obj, jobject vecObjList)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        jclass listClass = env->GetObjectClass(vecObjList);
        jclass iterClass = env->FindClass("java/util/Iterator");
        jmethodID literMethod = env->GetMethodID(listClass,"iterator","()Ljava/util/Iterator;");
        jobject liter = env->CallObjectMethod(vecObjList,literMethod);
        jmethodID hasNext = env->GetMethodID(iterClass,"hasNext","()Z");
        jmethodID next = env->GetMethodID(iterClass,"next","()Ljava/lang/Object;");
        env->DeleteLocalRef(iterClass);
        env->DeleteLocalRef(listClass);
        
        Point2dClassInfo *ptClassInfo = Point2dClassInfo::getClassInfo();
        while (env->CallBooleanMethod(liter, hasNext))
        {
            jobject javaVecObj = env->CallObjectMethod(liter, next);
            Point2d *pt = ptClassInfo->getObject(env,javaVecObj);
            inst->pts.push_back(*pt);
            env->DeleteLocalRef(javaVecObj);
        }
        env->DeleteLocalRef(liter);
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
(JNIEnv *env, jobject obj, jobject vecObjList)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        jclass listClass = env->GetObjectClass(vecObjList);
        jclass iterClass = env->FindClass("java/util/Iterator");
        jmethodID literMethod = env->GetMethodID(listClass,"iterator","()Ljava/util/Iterator;");
        jobject liter = env->CallObjectMethod(vecObjList,literMethod);
        jmethodID hasNext = env->GetMethodID(iterClass,"hasNext","()Z");
        jmethodID next = env->GetMethodID(iterClass,"next","()Ljava/lang/Object;");
        env->DeleteLocalRef(iterClass);
        env->DeleteLocalRef(listClass);
        
        
        Point2dClassInfo *ptClassInfo = Point2dClassInfo::getClassInfo();
        while (env->CallBooleanMethod(liter, hasNext))
        {
            jobject javaVecObj = env->CallObjectMethod(liter, next);
            Point2d *pt = ptClassInfo->getObject(env,javaVecObj);
            inst->texCoords.push_back(TexCoord(pt->x(), pt->y()));
            env->DeleteLocalRef(javaVecObj);
        }
        env->DeleteLocalRef(liter);
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

JNIEXPORT jint JNICALL Java_com_mousebird_maply_SimplePoly_getPtsSize
(JNIEnv *env, jobject obj)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        return inst->pts.size();
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::getPtsSize()");
    }
    return -1;
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_SimplePoly_getTexCoordsSize
(JNIEnv *env, jobject obj)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        return inst->texCoords.size();
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::getTexCoordsSize()");
    }
    return -1;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_SimplePoly_getPt
(JNIEnv *env, jobject obj, jint index)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        if (index >= inst->pts.size() )
            return NULL;
        Point2d pt = inst->pts.at(index);
        return MakePoint2d(env, pt);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::getPt()");
    }
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_SimplePoly_getTexCoord
(JNIEnv *env, jobject obj, jint index)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        if (index >= inst->texCoords.size() )
            return NULL;
        TexCoord tc = inst->texCoords.at(index);
        Point2d pt(tc.u(), tc.v());
        return MakePoint2d(env, pt);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::getTexCoord()");
    }
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_SimplePoly_getTexture
(JNIEnv *env, jobject obj)
{
    try
    {
        SimplePolyClassInfo *classInfo = SimplePolyClassInfo::getClassInfo();
        SimplePoly *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        TextureClassInfo *textureClassInfo = TextureClassInfo::getClassInfo(env,"com/mousebird/maply/Texture");
        Texture tex = inst->texture;
        return textureClassInfo->makeWrapperObject(env,&tex);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SimplePoly::getTexture()");
    }
    return NULL;

}
