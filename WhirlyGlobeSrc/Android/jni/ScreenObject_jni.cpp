/*
 *  ScreenObject_jni.cpp
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
#import "com_mousebird_maply_ScreenObject.h"
#import "WhirlyGlobe.h"
#import <android/bitmap.h>


using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ScreenObject_nativeInit
(JNIEnv *env, jclass cls)
{
    ScreenObjectClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ScreenObject_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        ScreenObjectClassInfo *classInfo = ScreenObjectClassInfo::getClassInfo();
        ScreenObject *inst = new ScreenObject();
        classInfo->setHandle(env, obj, inst);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ScreenObject::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ScreenObject_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        ScreenObjectClassInfo *classInfo = ScreenObjectClassInfo::getClassInfo();
        ScreenObject *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        delete inst;
        classInfo->clearHandle(env, obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ScreenObject::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ScreenObject_addPoly
(JNIEnv *env, jobject obj, jobject polyObj)
{
    try
    {
        ScreenObjectClassInfo *classInfo = ScreenObjectClassInfo::getClassInfo();
        ScreenObject *inst = classInfo->getObject(env, obj);
        SimplePoly *poly = SimplePolyClassInfo::getClassInfo()->getObject(env, polyObj);
        if (!inst || !poly)
            return;
        inst->polys.push_back(*poly);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ScreenObject::addPoly()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_ScreenObject_getPoly
(JNIEnv *env, jobject obj, jint index)
{
    try
    {
        ScreenObjectClassInfo *classInfo = ScreenObjectClassInfo::getClassInfo();
        ScreenObject *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        if (index >= inst->polys.size())
            return NULL;
        SimplePoly poly = inst->polys.at(index);
        SimplePolyClassInfo *polyClassInfo = SimplePolyClassInfo::getClassInfo(env,"com/mousebird/maply/SimplyPoly");
        return polyClassInfo->makeWrapperObject(env,&poly);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ScreenObject::getPoly()");
    }
    return NULL;
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_ScreenObject_getPolysSize
(JNIEnv *env, jobject obj)
{
    try
    {
        ScreenObjectClassInfo *classInfo = ScreenObjectClassInfo::getClassInfo();
        ScreenObject *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        return inst->polys.size();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ScreenObject::getPolysSize()");
    }
    return -1;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ScreenObject_addString
(JNIEnv *env, jobject obj, jobject stringObj)
{
    try
    {
        ScreenObjectClassInfo *classInfo = ScreenObjectClassInfo::getClassInfo();
        ScreenObject *inst = classInfo->getObject(env, obj);
        StringWrapper *string = StringWrapperClassInfo::getClassInfo()->getObject(env, stringObj);
        if (!inst || !string)
            return;
        inst->strings.push_back(*string);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ScreenObject::addString()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_ScreenObject_getString
(JNIEnv *env, jobject obj, jint index)
{
    try
    {
        ScreenObjectClassInfo *classInfo = ScreenObjectClassInfo::getClassInfo();
        ScreenObject *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        if (index >= inst->strings.size())
            return NULL;
        StringWrapper string = inst->strings.at(index);
        StringWrapperClassInfo *stringClassInfo = StringWrapperClassInfo::getClassInfo(env,"com/mousebird/maply/StringWrapper");
        return stringClassInfo->makeWrapperObject(env,&string);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ScreenObject::getPoly()");
    }
    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ScreenObject_addImage
(JNIEnv *env, jobject obj, jobject bitmapObj, jfloatArray colorArray, jint width, jint height)
{
    try
    {
        ScreenObjectClassInfo *classInfo = ScreenObjectClassInfo::getClassInfo();
        ScreenObject *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        //Bitmap
        Texture *tex = new Texture("Image") ;
        
        AndroidBitmapInfo info;
        if (AndroidBitmap_getInfo(env, bitmapObj, &info) < 0)
        {
            return;
        }
        if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
        {
            __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Only dealing with 8888 bitmaps in Texture::setBitmap()");
            return;
        }
        // Copy  raw data over to the texture
        void* bitmapPixels;
        if (AndroidBitmap_lockPixels(env, bitmapObj, &bitmapPixels) < 0)
        {
            return;
        }
        
        uint32_t* src = (uint32_t*) bitmapPixels;
        MutableRawData *rawData = new MutableRawData(bitmapPixels,info.height*info.width*4);
        tex->setRawData(rawData,info.width,info.height);
        AndroidBitmap_unlockPixels(env, bitmapObj);
        
        //Color
        
        jfloat *colors = env->GetFloatArrayElements(colorArray, 0);
        jsize len = env->GetArrayLength(colorArray);
        RGBAColor *color;
        if (len <4){
            color = new RGBAColor(0,0,0,0);
        }
        else{
            color = new RGBAColor(colors[0]*255.0,colors[1]*255.0,colors[2]*255.0,colors[3]*255.0);
        }
        
        //Create Poly
        SimplePoly *poly = new SimplePoly();
        poly->texture = *tex;
        poly->color = color;
        
        
        poly->pts.push_back(Point2d(0,0));
        poly->texCoords.push_back(TexCoord(0,1));
        
        poly->pts.push_back(Point2d(width, 0));
        poly->texCoords.push_back(TexCoord(1,1));
        
        poly->pts.push_back(Point2d(width, height));
        poly->texCoords.push_back(TexCoord(1,0));
        
        poly->pts.push_back(Point2d(0, height));
        poly->texCoords.push_back(TexCoord(0,0));
        
        //Insert Poly
        inst->polys.push_back(*poly);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ScreenObject::addImage()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_ScreenObject_getStringsSize
(JNIEnv *env, jobject obj)
{
    try
    {
        ScreenObjectClassInfo *classInfo = ScreenObjectClassInfo::getClassInfo();
        ScreenObject *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        return inst->strings.size();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ScreenObject::getStringsSize()");
    }
    return -1;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ScreenObject_addScreenObject
(JNIEnv *env, jobject obj, jobject screenObj)
{
    try
    {
        ScreenObjectClassInfo *classInfo = ScreenObjectClassInfo::getClassInfo();
        ScreenObject *inst = classInfo->getObject(env, obj);
        ScreenObject *newScreen = classInfo->getObject(env, screenObj);
        if (!inst || !newScreen)
            return;
        inst = newScreen;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ScreenObject::addScreenObject()");
    }
}
