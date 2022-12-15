/*  QuadImageLoaderBase_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg on 3/25/19.
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

#import "QuadLoading_jni.h"
#import "Geometry_jni.h"
#import "Scene_jni.h"
#import "Renderer_jni.h"
#import "com_mousebird_maply_QuadImageLoaderBase.h"

using namespace Eigen;
using namespace WhirlyKit;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageLoaderBase_delayedInitNative
  (JNIEnv *env, jobject obj, jobject sceneObj)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env,sceneObj);
        if (!loader || !scene)
            return;

        // Resolve the shader if it's not set
        for (unsigned int ii=0;ii<loader->get()->getNumFocus();ii++) {
            if (loader->get()->getShaderID(ii) == EmptyIdentity) {
                ProgramGLES *prog = (ProgramGLES *) scene->findProgramByName(
                        MaplyDefaultTriMultiTexShader);
                if (prog)
                    loader->get()->setShaderID(ii,prog->getId());
            }
        }

        if (loader->get()->getMode() == QuadImageFrameLoader::Mode::MultiFrame) {
            scene->addActiveModel(*loader);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadImageLoaderBase_getBaseDrawPriority
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            return (*loader)->getBaseDrawPriority();
        }
    }
    MAPLY_STD_JNI_CATCH()
    return -1;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageLoaderBase_setBaseDrawPriority
  (JNIEnv *env, jobject obj, jint baseDrawPriority)
{
    try
    {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            (*loader)->setBaseDrawPriority(baseDrawPriority);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadImageLoaderBase_getDrawPriorityPerLevel
        (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            return (*loader)->getDrawPriorityPerLevel();
        }
    }
    MAPLY_STD_JNI_CATCH()
    return -1;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageLoaderBase_setDrawPriorityPerLevel
  (JNIEnv *env, jobject obj, jint perLevel)
{
    try
    {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            (*loader)->setDrawPriorityPerLevel(perLevel);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageLoaderBase_setColor
  (JNIEnv *env, jobject obj, jfloat red, jfloat green, jfloat blue, jfloat alpha, jobject changeSetObj)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!loader)
            return;
        RGBAColor color(red*255,green*255,blue*255,alpha*255);
        (*loader)->setColor(color,changeSet ? changeSet->get() : nullptr);
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageLoaderBase_setZBufferWrite
  (JNIEnv *env, jobject obj, jboolean zBufferWrite)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (!loader)
            return;
        // TODO: Hook this up
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageLoaderBase_setZBufferRead
  (JNIEnv *env, jobject obj, jboolean)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (!loader)
            return;
        // TODO: Hook this up
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageLoaderBase_setShaderID
  (JNIEnv *env, jobject obj, jlong shaderID)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (!loader)
            return;
        (*loader)->setShaderID(0,shaderID);
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageLoaderBase_setRenderTargetID
  (JNIEnv *env, jobject obj, jlong targetID)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (!loader)
            return;
        (*loader)->setRenderTarget(0,targetID);
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadImageLoaderBase_getImageFormatNative
  (JNIEnv *env, jobject obj)
{
    try {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            return TexTypeToImageFormat((*loader)->getTexType());
        }
    }
    MAPLY_STD_JNI_CATCH()
    return -1;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageLoaderBase_setImageFormatNative
  (JNIEnv *env, jobject obj, jint imageFormat)
{
    try
    {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            (*loader)->setTexType(ImageFormatToTexType((MaplyImageType) imageFormat));
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadImageLoaderBase_getTexInterpNative
  (JNIEnv *env, jobject obj)
{
    try {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            return (*loader)->getTexInterp();
        }
    }
    MAPLY_STD_JNI_CATCH()
    return -1;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageLoaderBase_setTexInterpNative
  (JNIEnv *env, jobject obj, jint type)
{
    try
    {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            (*loader)->setTexInterp((TextureInterpType)type);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageLoaderBase_setBorderTexel
  (JNIEnv *env, jobject obj, jint borderTexel)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (!loader)
            return;
        // TODO: Do something with this
    }
    MAPLY_STD_JNI_CATCH()
}
