/*
 *  Drawable_jni.cpp
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
#import "com_mousebird_maply_Drawable.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;


JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_nativeInit
(JNIEnv *env, jclass cls)
{
    DrawableClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_initialise__Ljava_lang_String_2
(JNIEnv *env, jobject obj, jstring name)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *drawable = new Drawable(name);
        classInfo->setHandle(env, obj, drawable);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_initialise__Ljava_lang_String_2II
(JNIEnv *env, jobject obj, jstring name, jint numVert, jint numTri)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *drawable = new Drawable(name, numVert, numTri);
        classInfo->setHandle(env, obj, drawable);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_dispose
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        delete inst;
        classInfo->clearHandle(env, obj);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::dispose()");
    }
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_Drawable_getProgram
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        return inst->getProgram();
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getProgram()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setProgram
(JNIEnv *env, jobject obj, jint programID)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->setProgram(programID);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setProgram()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setupGL__Lcom_mousebird_maply_WhirlyKitGLSetupInfo_2Lcom_mousebird_maply_OpenGLMemManager_2
(JNIEnv *env, jobject obj, jobject setupInfoObj, jobject glMemManObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        WhirlyKitGLSetupInfo *setupInfo = WhirlyKitGLSetupInfoClassInfo::getClassInfo()->getObject(env, setupInfoObj);
        OpenGLMemManager *manager = OpenGLMemManagerClassInfo::getClassInfo()->getObject(env, glMemManObj);
        if (!inst || !setupInfo || !manager)
            return;
        inst->setupGL(setupInfo, manager, 0, 0);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setupGL()");
    }
    
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setupGL__Lcom_mousebird_maply_WhirlyKitGLSetupInfo_2Lcom_mousebird_maply_OpenGLMemManager_2II
(JNIEnv *env, jobject obj, jobject setupInfoObj, jobject glMemManObj, jint sharedBuf, jint sharedBufOffset)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        WhirlyKitGLSetupInfo *setupInfo = WhirlyKitGLSetupInfoClassInfo::getClassInfo()->getObject(env, setupInfoObj);
        OpenGLMemManager *manager = OpenGLMemManagerClassInfo::getClassInfo()->getObject(env, glMemManObj);
        if (!inst || !setupInfo || !manager)
            return;
        inst->setupGL(setupInfo, manager, sharedBuf, sharedBufOffset);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setupGL()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_tearDownGL
(JNIEnv *env, jobject obj, jobject glMemManObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        OpenGLMemManager *manager = OpenGLMemManagerClassInfo::getClassInfo()->getObject(env, glMemManObj);
        if (!inst || !manager)
            return;
        inst->tearDownGL(manager);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::tearDownGL()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_Drawable_singleVertexSize
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        return inst->singleVertexSize();
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::singleVertexSize()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_Drawable_setupVAO
(JNIEnv *env, jobject obj, jobject programObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        OpenGLES2ProgramClassInfo *program = OpenGLES2ProgramClassInfo::getClassInfo()->getObject(env, programObj);
        if (!inst || !program)
            return NULL;
        return inst->setupVAO(program);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setupVAO()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_draw
(JNIEnv *env, jobject obj, jobject frameInfoObj, jobject sceneObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        RendererFrameInfo *frameInfo = RendererFrameInfoClassInfo::getClassInfo()->getObject(env, frameInfoObj);
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!inst || !frameInfo || !scene)
            return;
        inst->draw(frameInfo, scene);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::draw()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_Drawable_getDrawPriority
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        return inst->getDrawPriority();
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getDrawPriority()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Drawable_isOn
(JNIEnv *env, jobject obj, jobject frameInfoObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        RendererFrameInfo *frameInfo = RendererFrameInfoClassInfo::getClassInfo()->getObject(env, frameInfoObj);
        if (!inst || !frameInfo)
            return NULL;
        return inst->isOn(frameInfo);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::isOn()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setOnOff
(JNIEnv *env, jobject obj, jboolean onOff)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->setOnOff(onOff);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setOnOff()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setEnableTimeRange
(JNIEnv *env, jobject obj, jdouble inStartEnable, jdouble inEndEnable)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->setEnableTimeRange(inStartEnable, inEndEnable);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setEnableTimeRange()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Drawable_hasAlpha
(JNIEnv *env, jobject obj, jobject frameObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        RendererFrameInfo *frameInfo = RendererFrameInfoClassInfo::getClassInfo()->getObject(env, frameObj);
        if (!inst || !frameInfo)
            return NULL;
        return inst->hasAlpha(frameInfo);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::hasAlpha()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Drawable_setAlpha
(JNIEnv *env, jobject obj, jboolean onOff)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        return inst->setAlpha(onOff);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setAlpha()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Drawable_getLocalMbrPoint2dLL
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        Point2dClassInfo *classInfoPoint2d = Point2dClassInfo::getClassInfo(env,"com/mousebird/maply/Point2d");
        return classInfoPoint2d->makeWrapperObject(env, inst->getLocalMbr()->ll());
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getLocalMbrPoint2dLL()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Drawable_getLocalMbrPoint2dUr
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        Point2dClassInfo *classInfoPoint2d = Point2dClassInfo::getClassInfo(env,"com/mousebird/maply/Point2d");
        return classInfoPoint2d->makeWrapperObject(env, inst->getLocalMbr()->ur());
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getLocalMbrPoint2dUr()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setLocalMbr
(JNIEnv *env, jobject obj, jobject point2dLlObj, jobject point2dUrObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        Point2d *ll = Point2dClassInfo::getClassInfo()->getObject(env, point2dLlObj);
        Point2d *ur = Point2dClassInfo::getClassInfo()->getObject(env, point2dUrObj);
        if (!inst || !ll || !ur)
            return;
        Mbr *mbr = new Mbr(ll, ur);
        inst->setLocalMbr(mbr);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setLocalMbr()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setDrawPriority
(JNIEnv *env, jobject obj, jint newPriority)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->setDrawPriority(newPriority);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setDrawPriority()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setDrawOffset
(JNIEnv *env, jobject obj, jfloat drawOffSet)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->setDrawOffset(drawOffSet);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setDrawOffset()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_Drawable_getDrawOffset
(JNIEnv * env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        return inst->getDrawOffset();
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getDrawOffset()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setType
(JNIEnv * env, jobject obj, jint type)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->type = type;
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setType()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_Drawable_getType
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        return inst->type;
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getType()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setTexId
(JNIEnv *env, jobject obj, jint which, jlong inID)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->setTexID(which, inID);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setTexID()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setTexIds
(JNIEnv *env, jobject obj, jobject listObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        jclass listClass = env->GetObjectClass(vectObjList);
        jclass iterClass = env->FindClass("java/util/Iterator");
        jmethodID literMethod = env->GetMethodID(listClass, "iterator", "()Ljava/util/Iterator;");
        jobject liter = env->CallObjectMethod(listObj, literMethod);
        jmethodID hasNext = env->GetMethodID(iterClass, "hasNext", "()Z");
        jmethodID next = env->GetMethodID(iterClass, "next", "()Ljava/lang/Object");
        int i = 0;
        while (env->CallBooleanMethod(liter, hasNexr)) {
            jlong item = env->CallLongMethod(liter, next);
            if (item) {
                inst->setTexID(i, item);
                i++;
            }
        }
        env->DeleteLocalRef(liter);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setTexIDs()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_Drawable_getColorPartR
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;

        return inst->color.r;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getColorPartR()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_Drawable_getColorPartG
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        return inst->color.g;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getColorPartG()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_Drawable_getColorPartB
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        return inst->color.b;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getColorPartB()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_Drawable_getColorPartA
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        return inst->color.a;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getColorPartA()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setColor
(JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        RGBAColor *color = new RGBAColor(r, g, b, a);
        inst->setColor(color);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setColor()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setVisibleRange
(JNIEnv *env, jobject obj, jfloat minVis, jfloat maxVis, jfloat minVisBand, jfloat maxVisBand)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->setVisibleRange(minVis, maxVis, minVisBand, maxVisBand);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setVisibleRange()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_Drawable_getVisibleRangeMinVis
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        float minVis, maxVis;
        inst->getVisibleRange(minVis, maxVis);
        return minVis;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getVisibleRangeMinVis()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_Drawable_getVisibleRangeMaxVis
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        float minVis, maxVis;
        inst->getVisibleRange(minVis, maxVis);
        return maxVis;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getVisibleRangeMaxVis()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_Drawable_getVisibleRangeMinVisBand
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        float minVis, maxVis, minVisBand, maxVisBand;
        inst->getVisibleRange(minVis, maxVis, minVisBand, maxVisBand);
        return minVisBand;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getVisibleRangeMinVisBand()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_Drawable_getVisibleRangeMaxVisBand
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        float minVis, maxVis, minVisBand, maxVisBand;
        inst->getVisibleRange(minVis, maxVis, minVisBand, maxVisBand);
        return maxVisBand;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getVisibleRangeMaxVisBand()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setViewerVisibility
(JNIEnv *env, jobject obj, jdouble minViewerDis, jdouble maxViewerDis, jobject point3dObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        Point3d *viewerCenter = Point3dClassInfo::getClassInfo()->getObject(env, point3dObj);
        if (!inst | !viewerCenter)
            return;
        
        inst->setViewerVisibility(minVieweDis, maxViewerDis, viewerCenter);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setViewerVisibility()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Drawable_getViewerVisibilityMinDist
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        double outMinViewerDist, outMaxViewerDist;
        Point3d viewerCenter;
        inst->getViewerVisibility(outMinViewerDist, outMaxViewerDist, viewerCenter);
        return outMinViewerDist;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getViewerVisibilityMinDist()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Drawable_getViewerVisibilityMaxdist
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        double outMinViewerDist, outMaxViewerDist;
        Point3d viewerCenter;
        inst->getViewerVisibility(outMinViewerDist, outMaxViewerDist, viewerCenter);
        return outMaxViewerDist;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getViewerVisibilityMaxDist()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Drawable_getViewerVisibilityCenter
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        double outMinViewerDist, outMaxViewerDist;
        Point3d viewerCenter;
        inst->getViewerVisibility(outMinViewerDist, outMaxViewerDist, viewerCenter);
        Point3dClassInfo *classInfoPoint3d = Point3dClassInfo::getClassInfo(env,"com/mousebird/maply/Point3d");
        return classInfoPoint3d->makeWrapperObject(env, viewerCenter);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getViewerVisibilityCenter()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setFade
(JNIEnv *env, jobject obj, jdouble inFadeDown, jdouble inFadeUp)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->setFade (inFadeDown, inFadeUp);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setFade()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setLineWidth
(JNIEnv *env, jobject obj, jfloat inWidth)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->setLineWidtg (inWidth);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setLineWidth()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_Drawable_getLineWidth
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        return  inst->getLineWidth();
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getLineWidth()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setRequestZBuffer
(JNIEnv *env, jobject obj, jboolean val)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->setRequestZBuffer (val);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setRequestZBuffer()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Drawable_getRequestZBuffer
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        return inst->getRequestZBuffer ();
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getRequestZBuffer()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setWriteZBuffer
(JNIEnv *env, jobject obj, jboolean val)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->setWriteZBuffer (val);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setWriteZBuffer()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Drawable_getWriteZBuffer
(JNIEnv * env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        return inst->getWriteZBuffer ();
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getWriteZBuffer()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_Drawable_addPoint__Lcom_mousebird_maply_Point3f_2
(JNIEnv * env, jobject obj, jobject ptObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        Point3f *pt = Point3fClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !pt)
            return;
        
        inst->addPoint(pt);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::addPoint()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_Drawable_addPoint__Lcom_mousebird_maply_Point3d_2
(JNIEnv *env, jobject obj, jobject ptObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        Point3d *pt = Point3dClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !pt)
            return;
        
        inst->addPoint(pt);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::addPoint()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Drawable_getPoint
(JNIEnv *env, jobject obj, jint which)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        Point3f point = inst->getPoint(which);

        Point3fClassInfo *classInfoPoint3f = Point3fClassInfo::getClassInfo(env,"com/mousebird/maply/Point3f");
        return classInfoPoint3f->makeWrapperObject(env, point);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getPoint()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_addTexCoord
(JNIEnv *env, jobject obj, jint witch, jobject coordObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        Point2f *coord = Point3fClassInfo::getClassInfo()->getObject(env, coordObj);
        if (!inst || !coord)
            return;
        
        inst->addTexCoord(witch, coord);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::addTexCoord()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_addColor
(JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        RGBAColor *color = new RGBAColor(r,g,b,a);
        
        inst->addColor(color);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::addColor()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_addNormal__Lcom_mousebird_maply_Point3f_2
(JNIEnv *env, jobject obj, jobject ptObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        Point3f *norm = Point3fClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !norm)
            return;
        
        inst->addNormal(norm);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::addNormal()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_addNormal__Lcom_mousebird_maply_Point3d_2
(JNIEnv *env, jobject obj, jobject ptObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        Point3d *norm = Point3dClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !norm)
            return;
        
        inst->addNormal(norm);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::addNormal()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Drawable_compareVertexAttributes
(JNIEnv *env, jobject obj, jobject objList)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        jclass listClass = env->GetObjectClass(vectObjList);
        jclass iterClass = env->FindClass("java/util/Iterator");
        jmethodID literMethod = env->GetMethodID(listClass, "iterator", "()Ljava/util/Iterator;");
        jobject liter = env->CallObjectMethod(objList, literMethod);
        jmethodID hasNext = env->GetMethodID(iterClass, "hasNext", "()Z");
        jmethodID next = env->GetMethodID(iterClass, "next", "()Ljava/lang/Object");
        
        SingleVertexAttributeClassInfo *classInfoAttr = SingleVertexAttributeClassInfo::getClassInfo();
        
        std:set<SingleVertexAttribute> inSet;
        
        while (env->CallBooleanMethod(liter, hasNext)) {
            jobject attrObj = env->CallObjectMethod(liter, next);
            SingleVertexAttribute *attr = classInfoAttr->getObject(env, attrObj);
            if (attr) {
                inSet.insert(attr);
            }
            env->DeleteLocalRef(attrObj);
        }
        env->DeleteLocalRef(liter);
        return inst->compareVertexAttributes(inSet);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::compareVertexAttributes()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setVertexAttributes
(JNIEnv *env, jobject obj, jobject objList)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        jclass listClass = env->GetObjectClass(vectObjList);
        jclass iterClass = env->FindClass("java/util/Iterator");
        jmethodID literMethod = env->GetMethodID(listClass, "iterator", "()Ljava/util/Iterator;");
        jobject liter = env->CallObjectMethod(objList, literMethod);
        jmethodID hasNext = env->GetMethodID(iterClass, "hasNext", "()Z");
        jmethodID next = env->GetMethodID(iterClass, "next", "()Ljava/lang/Object");
        
        SingleVertexAttributeInfoClassInfo *classInfoAttr = SingleVertexAttributeInfoClassInfo::getClassInfo();
        
    std:set<SingleVertexAttributeInfo> inSet;
        
        while (env->CallBooleanMethod(liter, hasNext)) {
            jobject attrObj = env->CallObjectMethod(liter, next);
            SingleVertexAttributeInfo *attr = classInfoAttr->getObject(env, attrObj);
            if (attr) {
                inSet.insert(attr);
            }
            env->DeleteLocalRef(attrObj);
        }
        env->DeleteLocalRef(liter);
        inst->setVertexAttributes(inSet);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setVertexAttributes()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_addVertexAttributes
(JNIEnv *env, jobject obj, jobject objList)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        jclass listClass = env->GetObjectClass(vectObjList);
        jclass iterClass = env->FindClass("java/util/Iterator");
        jmethodID literMethod = env->GetMethodID(listClass, "iterator", "()Ljava/util/Iterator;");
        jobject liter = env->CallObjectMethod(objList, literMethod);
        jmethodID hasNext = env->GetMethodID(iterClass, "hasNext", "()Z");
        jmethodID next = env->GetMethodID(iterClass, "next", "()Ljava/lang/Object");
        
        SingleVertexAttributeClassInfo *classInfoAttr = SingleVertexAttributeClassInfo::getClassInfo();
        
        std:set<SingleVertexAttribute> inSet;
        
        while (env->CallBooleanMethod(liter, hasNext)) {
            jobject attrObj = env->CallObjectMethod(liter, next);
            SingleVertexAttribute *attr = classInfoAttr->getObject(env, attrObj);
            if (attr) {
                inSet.insert(attr);
            }
            env->DeleteLocalRef(attrObj);
        }
        env->DeleteLocalRef(liter);
        inst->addVertexAttributes(inSet);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::addVertexAttributes()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_addAttributeValue__ILcom_mousebird_maply_Point2f_2
(JNIEnv *env, jobject obj, jint attrID, jobject ptObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        Point2f *pt = Point2fClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !pt)
            return;
        
        inst->addAttributeValue(attrID, pt);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::addAttributeValue()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_addAttributeValue__ILcom_mousebird_maply_Point3f_2
(JNIEnv *env, jobject obj, jint attrID, jobject ptObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        Point3f *pt = Point3fClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !pt)
            return;
        
        inst->addAttributeValue(attrID, pt);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::addAttributeValue()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_addAttributeValue__ILcom_mousebird_maply_Point4f_2
(JNIEnv *env, jobject obj, jint attrID, jobject ptObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        Point4f *pt = Point4fClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !pt)
            return;
        
        inst->addAttributeValue(attrID, pt);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::addAttributeValue()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_addAttributeValue__IFFFF
(JNIEnv *env, jobject obj, jint attrID, jfloat r, jfloat g, jfloat b, jfloat a)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        RGBAColor *color = new RGBAColor(r,g,b,a);
        inst->addAttributeValue(attrID, color);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::addAttributeValue()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_addAttributeValue__IF
(JNIEnv *env, jobject obj, jint attrID, jfloat val)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->addAttributeValue(attrID, val);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::addAttributeValue()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_addTriangle
(JNIEnv *env, jobject obj, jobject objTri)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        Triangle *tri = TriangleClassInfo::getClassInfo()->getObject(env, objTri);
        if (!inst || !tri)
            return;
        
        inst->addTriangle(tri);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::addTriangle()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_Drawable_getTexID
(JNIEnv *env, jobject obj, jint which)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        return inst->getTexID(which);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getTexID()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Drawable_getTexInfo
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        jclass clsArrayList = env->FindClass("java/util/List");
        jmethodID constructor = env->GetMethodID(clsArrayList, "<init>", "()V");
        jobject objArrayList = env->NewObject(clsArrayList, constructor, "");
        jmethodID arrayListAdd = env->GetMethodID(clsArrayList, "add", "(Ljava/lang/Object;)Z");
        
        TexInfoClassInfo *texInfo = TexInfoClassInfo::getClassInfo(env, "com/mousebird/maply/TexInfo");
        
        for (auto item : inst->getTexInfo()) {
            jobject objTexInfo = texInfo->makeWrapperObject(env, item);
            env->CallObjectMethod(objArrayList, arrayListAdd, objTexInfo);
        }
        env->DeleteLocalRef(clsArrayList);
        return objArrayList;
        
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getTexInfo()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_Drawable_addAttribute
(JNIEnv *env, jobject obj, jint dataType, jstring name)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        return inst->addAttribute(dataType, name);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::addAttribute()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_Drawable_getNumPoint
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        return inst->getNumPoint();
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getNumPoint()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_Drawable_getNumTris
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        return inst->getNumTris();
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getNumTris()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_reserveNumPoint
(JNIEnv *env, jobject obj, jint numPoints)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->reserveNumPoint(numPoints);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::reserveNumPoint()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_reserveNumTris
(JNIEnv *env, jobject obj, jint numTris)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->reserveNumTris(numTris);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::reserveNumTris()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_reserveNumTexCoords
(JNIEnv *env, jobject obj, jint which, jint numCoords)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->reserveNumTexCoords(which, numCoords);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::reserveNumTexCoords()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_reserveNumNorms
(JNIEnv *env, jobject obj, jint numNorms)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->reserveNumNorms(numNorms);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::reserveNumNorms()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_reserveNumColors
(JNIEnv *env, jobject obj, jint numColors)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->reserveNumColors(numColors);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::reserveNumColors()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setMatrix
(JNIEnv *env, jobject obj, jobject objMtx)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        Matrix4d *matrix = Matrix4dClassInfo::getClassInfo()->getObject(objMtx);
        if (!inst || !matrix)
            return;
        
        inst->setMatrix(matrix);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setMatrix()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Drawable_getMatrix
(JNIEnv *env, jobject obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        Matrix4d *matrix = inst->getMatrix();
        Matrix4dClassInfo *classInfoMatrix4d = Matrix4dClassInfo::getClassInfo(env,"com/mousebird/maply/Matrix4d");
        
        return classInfoMatrix4d->makeWrapperObject(env, matrix);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getMatrix()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_applySubTexture
(JNIEnv *env, jobject obj, jint which, jobject subObj, jint startingAt)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        SubTexture *subTex = SubTextureClassInfo::getClassInfo()->getObject(env, subObj);
        if (!inst || !subTex)
            return;
        
        inst->applySubTexture(which, subTex, startingAt);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::applySubTexture()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_updateRenderer
(JNIEnv *env, jobject obj, jobject renObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        SceneRendererES *sceneRenderer = SceneRendererESClassInfo::getClassInfo()->getObject(env, renObj);
        if (!inst || !sceneRenderer)
            return;
        
        inst->updateRenderer(sceneRenderer);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::updateRenderer()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Drawable_asData
(JNIEnv *env, jobject obj, jboolean dupStart, jboolean dupEnd)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        RawData * data = inst->asData(dupStart, dupEnd);
        RawDataClassInfo *rawDataClassInfo = RawDataClassInfo::getClassInfo(env,"com/mousebird/maply/RawData");
        
        return rawDataClassInfo->makeWrapperObject(env, data);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::asData()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_asVextexAndElementData
(JNIEnv *env, jobject obj, jobject vertDataObj, jobject elementDataObj, jint singleElementSize, jobject ptObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        MutableRawDataClassInfo *mdClassInfo = MutableRawDataClassInfo::getClassInfo();
        MutableRawData *vertData = mdClassInfo->getObject(env, vertDataObj);
        MutableRawData *elementData = mdClassInfo->getObject(env, elementDataObj);
        Point3d *center = Point3dClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !vertData || !elementData || !center)
            return;
        inst->asVextexAndElementData(vertData, elementData, singleElementSize, center);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::asVextexAndElementData()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Drawable_getVertexAttributes
(JNIEnv *env, jobject  obj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        jclass clsArrayList = env->FindClass("java/util/List");
        jmethodID constructor = env->GetMethodID(clsArrayList, "<init>", "()V");
        jobject objArrayList = env->NewObject(clsArrayList, constructor, "");
        jmethodID arrayListAdd = env->GetMethodID(clsArrayList, "add", "(Ljava/lang/Object;)Z");
        
        VertexAttributeClassInfo *vertexAttributeClassInfo = VertexAttributeClassInfo::getClassInfo(env, "com/mousebird/maply/VertexAttribute");
        
        for (auto item: inst->getVertexAttributes()) {
            jobject objVerAttr = vertexAttributeClassInfo->makeWrapperObject(env, item);
            env->CallObjectMethod(objArrayList, arrayListAdd, objVerAttr);
        }
        env->DeleteLocalRef(clsArrayList);
        return objArrayList;
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::getVertexAttributes()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setupTexCoordEntry
(JNIEnv *env, jobject obj, jint which, jint numReserve)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->setupTexCoordEntry(which, numReserve);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setupTexCoordEntry()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_drawOGL2
(JNIEnv *env, jobject obj, jobject frameObj, jobject sceneObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        RendererFrameInfo *frame = RendererFrameInfoClassInfo::getClassInfo()->getObject(env, frameObj);
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!inst || !frame || !scene)
            return;
        
        inst->drawOGL2(frame, scene);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::drawOGL2()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_addPointToBuffer
(JNIEnv *env, jobject obj, jstring basePtr, jint which, jobject ptObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        Point3d *center = Point3dClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !center)
            return;
        JavaString jstr(env, basePtr);
        inst->AddPointToBuffer(jstr.cStr, which, center);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::addPointToBuffer()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setupAdditionalVAO
(JNIEnv *env, jobject obj, jobject progObj, jint vertArrayObj);
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        OpenGLES2Program *program = OpenGLES2ProgramClassInfo::getClassInfo()->getObject(env, progObj);
        if (!inst || !program)
            return;
        inst->setupAdditionalVAO(program, vertArrayObj);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setupAdditionalVAO()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_bindAdditionalRenderObjects
(JNIEnv *env, jobject obj, jobject frameObj, jobject sceneObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        RendererFrameInfo *frameInfo = RendererFrameInfoClassInfo::getClassInfo()->getObject(env, frameObj);
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!inst || !frameInfo || !scene)
            return;
        inst->bindAdditionalRenderObjects(frameInfo, scene);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::bindAdditionalRenderObjects()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_postDrawCallBack
(JNIEnv *env, jobject obj, jobject frameObj, jobject sceneObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        RendererFrameInfo *frameInfo = RendererFrameInfoClassInfo::getClassInfo()->getObject(env, frameObj);
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!inst || !frameInfo || !scene)
            return;
        inst->postDrawCallBack(frameInfo, scene);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::postDrawCallBack()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_setupStandardAttributes
(JNIEnv *env, jobject obj, jint numReserve)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->setupStandardAttributes(numReserve);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::setupStandardAttributes()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_addTweaker
(JNIEnv *env, jobject obj, jobject tweakerObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        DrawableTweaker *tweaker = DrawableTweakerClassInfo::getClassInfo()->getObject(env, tweakerObj);
        if (!inst || !tweaker)
            return;
        inst->addTweaker(tweaker);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::addTweaker()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_removeTweaker
(JNIEnv *env, jobject obj, jobject tweakerObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        DrawableTweaker *tweaker = DrawableTweakerClassInfo::getClassInfo()->getObject(env, tweakerObj);
        if (!inst || !tweaker)
            return;
        inst->removeTweaker(tweaker);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::removeTweaker()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Drawable_runTweakers
(JNIEnv *env, jobject obj, jobject frameInfoObj)
{
    try {
        DrawableClassInfo *classInfo = DrawableClassInfo::getClassInfo();
        Drawable *inst = classInfo->getObject(env, obj);
        RendererFrameInfo *frameInfo = RendererFrameInfoClassInfo::getClassInfo()->getObject(env, frameInfoObj);
        if (!inst || !frameInfo)
            return;
        inst->runTweakers(frameInfo);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Drawable::runTweakers()");
    }
}