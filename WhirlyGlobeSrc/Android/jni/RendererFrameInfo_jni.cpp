/*
 *  RendererFrameInfo_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 23/1/16.
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
#import "com_mousebird_maply_RendererFrameInfo.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_nativeInit
(JNIEnv *env, jclass cls)
{
    RendererFrameInfoClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *frameInfo = new RendererFrameInfo();
        classInfo->setHandle(env, obj, frameInfo);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        delete inst;
        classInfo->clearHandle(env, obj);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setOGLVersion
(JNIEnv *env, jobject obj, jint version)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->setOGLVersion(version);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::setOGLVersion()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_RendererFrameInfo_getOGLVersion
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        return inst->getOGLVersion();
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::getOGLVersion()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setSceneRendererES
(JNIEnv *env, jobject obj, jobject sceneObj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        SceneRendererES *renderer = SceneRendererESClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!inst || !renderer)
            return;
        
        inst->setSceneRendererES(renderer);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::setSceneRendererES()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getSceneRendererES
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        SceneRendererESClassInfo *sceneRendererESClassInfo = SceneRendererESClassInfo::getClassInfo(env,"com/mousebird/maply/SceneRendererES");
        return sceneRendererESClassInfo->makeWrapperObject(env, inst->sceneRenderer);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::getSceneRendererES()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setView
(JNIEnv *env, jobject obj, jobject viewObj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        View *view = ViewClassInfo::getClassInfo()->getObject(env, viewObj);
        if (!inst || !view)
            return;
        
        inst->setView(view);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::setView()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getView
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        ViewClassInfo *viewClassInfo = ViewClassInfo::getClassInfo(env,"com/mousebird/maply/View");
        return viewClassInfo->makeWrapperObject(env, inst->theView);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::getView()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setModelTrans
(JNIEnv *env, jobject obj, jobject modObj )
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        Matrix4f *matrix = Matrix4fClassInfo::getClassInfo()->getObject(env, modObj);
        if (!inst || !matrix)
            return;
        inst->modelTrans = matrix;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::setModelTrans()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getModelTrans
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        Matrix4fClassInfo *matrix4fClassInfo = Matrix4fClassInfo::getClassInfo(env,"com/mousebird/maply/Matrix4f");
        return matrix4fClassInfo->makeWrapperObject(env, inst->modelTrans);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::getModelTrans()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setViewTrans
(JNIEnv *env, jobject obj, jobject viewObj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        Matrix4f *matrix = Matrix4fClassInfo::getClassInfo()->getObject(env, viewObj);
        if (!inst || !matrix)
            return ;
        inst->ViewTrans = matrix;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::setViewTrans()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getViewTrans
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        Matrix4fClassInfo *matrix4fClassInfo = Matrix4fClassInfo::getClassInfo(env,"com/mousebird/maply/Matrix4f");
        return matrix4fClassInfo->makeWrapperObject(env, inst->viewTrans);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::getViewTrans()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setModelTrans4d
(JNIEnv *env, jobject obj, jobject modObj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        Matrix4d *matrix = Matrix4dClassInfo::getClassInfo()->getObject(env, modObj);
        if (!inst || !matrix)
            return;
        
        inst->modelTrans4d = matrix;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::setModelTrans4d()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getModelTrans4d
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        Matrix4dClassInfo *matrix4dClassInfo = Matrix4dClassInfo::getClassInfo(env,"com/mousebird/maply/Matrix4d");
        return matrix4dClassInfo->makeWrapperObject(env, inst->modelTrans4d);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::getModelTrans4d()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setViewTrans4d
(JNIEnv *env, jobject obj, jobject viewObj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        Matrix4d *matrix = Matrix4dClassInfo::getClassInfo()->getObject(env, viewObj);
        if (!inst || !matrix)
            return;
        
        inst->viewModel4d = matrix;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::setViewTrans4d()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getViewTrans4d
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        Matrix4dClassInfo *matrix4dClassInfo = Matrix4dClassInfo::getClassInfo(env,"com/mousebird/maply/Matrix4d");
        return matrix4dClassInfo->makeWrapperObject(env, inst->viewTrans4d);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::getViewTrans4d()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setProjMat
(JNIEnv *env, jobject obj, jobject projMatObj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        Matrix4f *projMat = Matrix4fClassInfo::getClassInfo()->getObject(env, projMatObj);
        if (!inst || !projMat)
            return;
        
        inst->projMat = projMat;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::setProjMat()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getProjMat
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        Matrix4fClassInfo *matrix4fClassInfo = Matrix4fClassInfo::getClassInfo(env,"com/mousebird/maply/Matrix4f");
        return matrix4fClassInfo->makeWrapperObject(env, inst->projMat);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::getProjMat()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setProjMat4d
(JNIEnv *env, jobject obj, jobject projMat4dObj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        Matrix4d *projMat4d = Matrix4dClassInfo::getClassInfo()->getObject(env, projMat4dObj);
        if (!inst || !projMat4d)
            return;
        
        inst->projMat4d = projMat4d;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::setProjMat4d()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getProjMat4d
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        Matrix4dClassInfo *matrix4dClassInfo = Matrix4fClassInfo::getClassInfo(env,"com/mousebird/maply/Matrix4d");
        return matrix4dClassInfo->makeWrapperObject(env, inst->projMat4d);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::getProjMat4d()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setMvpNormalMat
(JNIEnv *env, jobject obj, jobject matObj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        Matrix4f *matrix = Matrix4fClassInfo::getClassInfo()->getObject(env, matObj);
        if (!inst || !matrix)
            return;
        
        inst->mvpNormalMat = matrix;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::setMvpNormalMat()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getMvpNormalMat
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        Matrix4fClassInfo *matrix4fClassInfo = Matrix4fClassInfo::getClassInfo(env,"com/mousebird/maply/Matrix4f");
        return matrix4fClassInfo->makeWrapperObject(env, inst->mvpNormalMat);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::getMvpNormalMat()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setViewAndModelMat
(JNIEnv *env, jobject obj, jobject matObj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        Matrix4f *matrix = Matrix4fClassInfo::getClassInfo()->getObject(env, matObj);
        if (!inst || !matrix)
            return;
        
        inst->viewAndModelMat = matrix;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::setViewAndModelMat()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getViewAndModelMat
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        Matrix4fClassInfo *matrix4fClassInfo = Matrix4fClassInfo::getClassInfo(env,"com/mousebird/maply/Matrix4f");
        return matrix4fClassInfo->makeWrapperObject(env, inst->viewAndModelMat);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::getViewAndModelMat()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setViewAndModelMat4d
(JNIEnv *env, jobject obj, jobject matObj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        Matrix4d *matrix = Matrix4dClassInfo::getClassInfo()->getObject(env, matObj);
        if (!inst || !matrix)
            return;
        
        inst->viewAndModelMat4d = matrix;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::setViewAndModelMat4d()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getViewAndModelMat4d
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        Matrix4dClassInfo *matrix4dClassInfo = Matrix4dClassInfo::getClassInfo(env,"com/mousebird/maply/Matrix4d");
        return matrix4dClassInfo->makeWrapperObject(env, inst->viewAndModelMat4d);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::getViewAndModelMat4d()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setMvpMat
(JNIEnv *env, jobject obj, jobject matObj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        Matrix4f *matrix = Matrix4fClassInfo::getClassInfo()->getObject(env, matObj);
        if (!inst || !matrix)
            return;
        
        inst->mvpMat = matrix;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::setMvpMat()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getMvpMat
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        Matrix4fClassInfo *matrix4fClassInfo = Matrix4fClassInfo::getClassInfo(env,"com/mousebird/maply/Matrix4f");
        return matrix4fClassInfo->makeWrapperObject(env, inst->mvpMat);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::getMvpMat()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setViewModelNormalMat
(JNIEnv *env, jobject obj, jobject matObj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        Matrix4f *matrix = Matrix4fClassInfo::getClassInfo()->getObject(env, matObj);
        if (!inst || !matrix)
            return;
        
        inst->viewModelNormalMat = matrix;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::setViewModelNormalMat()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getViewModelNormalMat
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        Matrix4fClassInfo *matrix4fClassInfo = Matrix4fClassInfo::getClassInfo(env,"com/mousebird/maply/Matrix4f");
        return matrix4fClassInfo->makeWrapperObject(env, inst->viewModelNormalMat);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::getViewModelNormalMat()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setPvMat4d
(JNIEnv *env, jobject obj, jobject matObj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        Matrix4d *matrix = Matrix4dClassInfo::getClassInfo()->getObject(env, matObj);
        if (!inst || !matrix)
            return;
        
        inst->pvMat4d = matrix;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::setPvMat4d()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getPvMat4d
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        Matrix4dClassInfo *matrix4dClassInfo = Matrix4dClassInfo::getClassInfo(env,"com/mousebird/maply/Matrix4d");
        return matrix4dClassInfo->makeWrapperObject(env, inst->pvMat4d);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::getPvMat4d()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setPvMat
(JNIEnv *env, jobject obj, jobject matObj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        Matrix4f *matrix = Matrix4fClassInfo::getClassInfo()->getObject(env, matObj);
        if (!inst || !matrix)
            return;
        
        inst->pvMat = matrix;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::setPvMat()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getPvMat
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        Matrix4fClassInfo *matrix4fClassInfo = Matrix4fClassInfo::getClassInfo(env,"com/mousebird/maply/Matrix4f");
        return matrix4fClassInfo->makeWrapperObject(env, inst->pvMat);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::getPvMat()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setOffSetMatrices
(JNIEnv *env, jobject obj, jobject listObj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        jclass listClass = env->GetObjectClass(listObj);
        jclass iterClass = env->FindClass("java/util/Iterator");
        jmethodID literMethod = env->GetMethodID(listClass, "iterator", "()Ljava/util/Iterator;");
        jobject liter = env->CallObjectMethod(objList, literMethod);
        jmethodID hasNext = env->GetMethodID(iterClass, "hasNext", "()Z");
        jmethodID next = env->GetMethodID(iterClass, "next", "()Ljava/lang/Object");
        
        Matrix4dClassInfo *matrixClassInfo = Matrix4dClassInfo::getClassInfo();
        
        while (env->CallBooleanMethod(liter, hasNext)) {
            jobject matrixObj = env->CallObjectMethod(liter, next);
            Matrix4d *matrix = matrixClassInfo->getObject(env, matrixObj);
            if (matrix) {
                inst->offsetMatrices.insert(matrix);
            }
            env->DeleteLocalRef(matrixObj);
        }
        env->DeleteLocalRef(liter);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::setOffSetMatrices()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getOffSetMatrices
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        jclass clsArrayList = env->FindClass("java/util/List");
        jmethodID constructor = env->GetMethodID(clsArrayList, "<init>", "()V");
        jobject objArrayList = env->NewObject(clsArrayList, constructor, "");
        jmethodID arrayListAdd = env->GetMethodID(clsArrayList, "add", "(Ljava/lang/Object;)Z");
        
        Matrix4dClassInfo *matrixClassInfo = Matrix4dClassInfo::getClassInfo(env, "com/mousebird/maply/Matrix4d");
        
        for (auto item : inst->offsetMatrices) {
            jobject objMtx = matrixClassInfo->makeWrapperObject(env, item);
            env->CallObjectMethod(objArrayList, arrayListAdd, objMtx);
        }
        env->DeleteLocalRef(clsArrayList);
        return objArrayList;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::getOffSetMatrices()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setScene
(JNIEnv *env, jobject obj, jobject sceneObj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!inst || !scene)
            return;
        inst->scene = scene;set
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::setScene()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getScene
(JNIEnv *env, jobject obj);
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        SceneClassInfo *sceneClassInfo = SceneClassInfo::getClassInfo(env, "com/mousebird/maply/Scene");
        return sceneClassInfo->makeWrapperObject(env, inst->scene);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::getScene()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setFrameLen
(JNIEnv *env, jobject obj, jfloat frameLen)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->frameLen = frameLen;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::setFrameLen()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_RendererFrameInfo_getFrameLen
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        return inst->frameLen;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::getFrameLen()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setCurrentTime
(JNIEnv *env, jobject obj, jdouble currentTime)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->currentTime = currentTime;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::setCurrentTime()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_RendererFrameInfo_getCurrentTime
(JNIEnv *env, jobject obj)
{
    try
    {
        RendererFrameInfoClassInfo *classInfo = RendererFrameInfoClassInfo::getClassInfo();
        RendererFrameInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        return inst->currentTime;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RendererFrameInfo::getCurrentTime()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setEyeVec
(JNIEnv *, jobject, jobject)
{
    
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getEyeVec
(JNIEnv *, jobject)
{
    
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setFullEyeVec
(JNIEnv *, jobject, jobject)
{
    
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getFullEyeVec
(JNIEnv *, jobject)
{
    
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setEyePos
(JNIEnv *, jobject, jobject)
{
    
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getEyePos
(JNIEnv *, jobject)
{
    
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setDispCenter
(JNIEnv *, jobject, jobject)
{
    
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getDispCenter
(JNIEnv *, jobject)
{
    
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setHeightAboveSurface
(JNIEnv *, jobject, jfloat)
{
    
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_RendererFrameInfo_getHeightAboveSurface
(JNIEnv *, jobject)
{
    
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setScreenSizeInDisplaysCoords
(JNIEnv *, jobject, jobject)
{
    
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getScreenSizeInDisplaysCoords
(JNIEnv *, jobject)
{
    
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setOpenGLES2Program
(JNIEnv *, jobject, jobject)
{
    
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getOpenGLES2Program
(JNIEnv *, jobject)
{
    
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RendererFrameInfo_setStateOptimizer
(JNIEnv *, jobject, jobject)
{
    
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_RendererFrameInfo_getStateOptimizer
(JNIEnv *, jobject)
{
    
}
