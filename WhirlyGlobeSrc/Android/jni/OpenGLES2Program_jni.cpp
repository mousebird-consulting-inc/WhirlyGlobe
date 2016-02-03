/*
 *  OpenGLES2Program_jni.cpp
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
#import "com_mousebird_maply_OpenGLES2Program.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;



JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLES2Program_nativeInit
(JNIEnv *env, jclass cls)
{
    OpenGLES2ProgramClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLES2Program_initialise__
(JNIEnv *env, jobject obj)
{
    try {
        OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
        OpenGLES2Program *inst = new OpenGLES2Program();
        classInfo->setHandle(env, obj, inst);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLES2Program::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLES2Program_initialise__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2
(JNIEnv *env, jobject obj, jstring name, jstring vShaderString, jstring fShaderString)
{
    try {
        OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
        JavaString jstrName(env, name);
        JavaString jstrVShader(env, vShaderString);
        JavaString jstrFShader(env, fShaderString);
        OpenGLES2Program *inst = new OpenGLES2Program(jstrName.cStr, jstrVShader.cStr, jstrFShader.cStr);
        classInfo->setHandle(env, obj, inst);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLES2Program::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLES2Program_dispose
(JNIEnv *env, jobject obj)
{
    try {
        OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
        OpenGLES2Program *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        delete inst;
        classInfo->clearHandle(env, obj);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLES2Program::dispose()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_OpenGLES2Program_isValid
(JNIEnv *env, jobject obj)
{
    try {
        OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
        OpenGLES2Program *inst = classInfo->getObject(env, obj);
        if (!inst)
            return false;
        
        return inst->isValid();
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLES2Program::isValid()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_OpenGLES2Program_setUniform__Ljava_lang_String_2F
(JNIEnv *env, jobject obj, jstring name, jfloat val)
{
    try {
        OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
        OpenGLES2Program *inst = classInfo->getObject(env, obj);
        if (!inst)
            return false;
        
        JavaString jstr(env, name);
        return inst->setUniform(jstr.cStr, val);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLES2Program::setUniform()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_OpenGLES2Program_setUniform__Ljava_lang_String_2Lcom_mousebird_maply_Point2f_2
(JNIEnv *env, jobject obj, jstring name, jobject ptObj)
{
    try {
        OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
        OpenGLES2Program *inst = classInfo->getObject(env, obj);
        Point2f *pt = Point2fClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !pt)
            return false;
        
        JavaString jstr(env, name);
        return inst->setUniform(jstr.cStr, *pt);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLES2Program::setUniform()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_OpenGLES2Program_setUniform__Ljava_lang_String_2Lcom_mousebird_maply_Point3f_2
(JNIEnv *env, jobject obj, jstring name, jobject ptObj)
{
    try {
        OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
        OpenGLES2Program *inst = classInfo->getObject(env, obj);
        Point3f *pt = Point3fClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !pt)
            return false;
        
        JavaString jstr(env, name);
        return inst->setUniform(jstr.cStr, *pt);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLES2Program::setUniform()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_OpenGLES2Program_setUniform__Ljava_lang_String_2Lcom_mousebird_maply_Point4f_2
(JNIEnv *env, jobject obj, jstring name, jobject ptObj)
{
    try {
        OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
        OpenGLES2Program *inst = classInfo->getObject(env, obj);
        Point4f *pt = Point4fClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !pt)
            return false;
        
        JavaString jstr(env, name);
        return inst->setUniform(jstr.cStr, *pt);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLES2Program::setUniform()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_OpenGLES2Program_setUniform__Ljava_lang_String_2Lcom_mousebird_maply_Matrix4f_2
(JNIEnv *env, jobject obj, jstring name, jobject mtxObj)
{
    try {
        OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
        OpenGLES2Program *inst = classInfo->getObject(env, obj);
        Eigen::Matrix4f *mtx = Matrix4fClassInfo::getClassInfo()->getObject(env, mtxObj);
        if (!inst || !mtx)
            return false;
        
        JavaString jstr(env, name);
        return inst->setUniform(jstr.cStr, *mtx);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLES2Program::setUniform()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_OpenGLES2Program_setUniform__Ljava_lang_String_2I
(JNIEnv *env, jobject obj, jstring name, jint val)
{
    try {
        OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
        OpenGLES2Program *inst = classInfo->getObject(env, obj);
        if (!inst)
            return false;
        
        JavaString jstr(env, name);
        return inst->setUniform(jstr.cStr, val);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLES2Program::setUniform()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_OpenGLES2Program_setTexture
(JNIEnv *env, jobject obj, jstring name, jint val)
{
    try {
        OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
        OpenGLES2Program *inst = classInfo->getObject(env, obj);
        if (!inst)
            return false;
        
        JavaString jstr(env, name);
        return inst->setTexture(jstr.cStr, val);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLES2Program::setTexture()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_OpenGLES2Program_hasLights
(JNIEnv *env, jobject obj)
{
    try {
        OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
        OpenGLES2Program *inst = classInfo->getObject(env, obj);
        if (!inst)
            return false;
        
        return inst->hasLights();
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLES2Program::hasLights()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_OpenGLES2Program_findAttribute
(JNIEnv *env, jobject obj, jstring attrName)
{
    try {
        OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
        OpenGLES2Program *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        JavaString jstr(env, attrName);
        OpenGLESAttribute *attr = (OpenGLESAttribute *)inst->findAttribute(jstr.cStr);
        if (!attr) {
            return NULL;
        }
        else{
            OpenGLESAttributeClassInfo *openGLESAttributeClassInfo = OpenGLESAttributeClassInfo::getClassInfo(env,"com/mousebird/maply/OpenGLESAttribute");
            
            return openGLESAttributeClassInfo->makeWrapperObject(env, attr);
        }
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLES2Program::findAttribute()");
    }
}

JNIEXPORT jstring JNICALL Java_com_mousebird_maply_OpenGLES2Program_getName
(JNIEnv *env, jobject obj)
{
    try {
        OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
        OpenGLES2Program *inst = classInfo->getObject(env, obj);
        std::string empty = "";
        if (!inst)
            return env->NewStringUTF(empty.c_str());
        
        return env->NewStringUTF(inst->getName().c_str());
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLES2Program::getName()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_OpenGLES2Program_getProgram
(JNIEnv *env, jobject obj)
{
    try {
        OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
        OpenGLES2Program *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        
        return inst->getProgram();
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLES2Program::getProgram()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_OpenGLES2Program_bindTextures
(JNIEnv *env, jobject obj)
{
    try {
        OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
        OpenGLES2Program *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        
        return inst->bindTextures();
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLES2Program::bindTextures()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLES2Program_cleanUp
(JNIEnv *env, jobject obj)
{
    try {
        OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
        OpenGLES2Program *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        return inst->cleanUp();
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLES2Program::cleanUp()");
    }
}
