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
#import "Maply_utils_jni.h"
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

		//Color
        
        float primaryColors[4];
        poly.color->asUnitFloats(primaryColors);
        jfloatArray color;
        color = env->NewFloatArray(4);
        env->SetFloatArrayRegion(color, 0, 4, primaryColors);

        //Texture

        //Create BitMapObject
        RawDataRef data = poly.texture.texData;
        RawData *rawData = data.get();
        jclass bitmapConfig = env->FindClass("android/graphics/Bitmap$Config");
        jfieldID rgba8888FieldID = env->GetStaticFieldID(bitmapConfig, "ARGB_8888", "Landroid/graphics/Bitmap$Config;");
        jobject rgba8888Obj = env->GetStaticObjectField(bitmapConfig, rgba8888FieldID);
        
        jclass bitmapClass = env->FindClass("android/graphics/Bitmap");
        jmethodID createBitmapMethodID = env->GetStaticMethodID(bitmapClass,"createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
        jobject bitmapObj = env->CallStaticObjectMethod(bitmapClass, createBitmapMethodID, poly.texture.getWidth(), poly.texture.getHeight(), rgba8888Obj);

        jintArray pixels = env->NewIntArray(poly.texture.getWidth() * poly.texture.getHeight());
        const unsigned char * bitmap = rawData->getRawData();
        for (int i = 0; i <poly.texture.getWidth() * poly.texture.getHeight() ; i++)
        {
            unsigned char red = bitmap[i*4];
            unsigned char green = bitmap[i*4 + 1];
            unsigned char blue = bitmap[i*4 + 2];
            unsigned char alpha = bitmap[i*4 + 3];
            int currentPixel = (alpha << 24) | (red << 16) | (green << 8) | (blue);
            env->SetIntArrayRegion(pixels, i, 1, &currentPixel);
        }

        jmethodID setPixelsMid = env->GetMethodID(bitmapClass, "setPixels", "([IIIIIII)V");
        env->CallVoidMethod(bitmapObj, setPixelsMid, pixels, 0, poly.texture.getWidth(), 0, 0, poly.texture.getWidth(), poly.texture.getHeight() );

        jclass textureCls = env->FindClass("com/mousebird/maply/Texture");
        jmethodID textureConstructor = env->GetMethodID(textureCls, "<init>", "(Landroid/graphics/Bitmap;)V");
        jobject texture = env->NewObject(textureCls, textureConstructor, bitmapObj);


        //Pts List object

        jclass listCls = env->FindClass("java/util/ArrayList");
        jmethodID listConstructor = env->GetMethodID(listCls, "<init>", "(I)V");
        jmethodID listAdd = env->GetMethodID(listCls, "add", "(Ljava/lang/Object;)Z");

        jobject listPtObj = env->NewObject(listCls, listConstructor, poly.pts.size());
        for (WhirlyKit::Point2d pt : poly.pts) {
            jobject ptObject = MakePoint2d(env, pt);
            env->CallBooleanMethod(listPtObj, listAdd, ptObject);
            env->DeleteLocalRef(ptObject);
        }

        //TexCoord List Object

        jobject listTCObj = env->NewObject(listCls, listConstructor, poly.texCoords.size());
        for (WhirlyKit::TexCoord tc : poly.texCoords) {
            Point2d pt(tc.u(), tc.v());
            jobject tcObject = MakePoint2d(env, pt);
            env->CallBooleanMethod(listTCObj, listAdd, tcObject);
            env->DeleteLocalRef(tcObject);
        }
        jclass cls = env->FindClass("com/mousebird/maply/SimplePoly");
        jmethodID constructor = env->GetMethodID(cls, "<init>", "(Lcom/mousebird/maply/Texture;[FLjava/util/List;Ljava/util/List;)V");
        jobject result = env->NewObject(cls, constructor, texture, color, listPtObj, listTCObj );
        return result;
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
        
        //Matrix3d Mat
        
        Eigen::Matrix3d mat = string.mat;
        jobject matrixObj = MakeMatrix3d(env, mat);
        
        //StringWrapper
        
        jclass cls = env->FindClass("com/mousebird/maply/StringWrapper");
        jmethodID constructor = env->GetMethodID(cls, "<init>", "(IILcom/mousebird/maply/Matrix3d;)V");
        jobject result = env->NewObject(cls, constructor, string.size.height, string.size.width, matrixObj  );
        
        return result;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ScreenObject::getString()");
    }
    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ScreenObject_addImage
(JNIEnv *env, jobject obj, jobject bitmapObj, jfloatArray colorArray, jfloat width, jfloat height)
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
        
        jsize len = 0;
        jfloat *colors;
        if (colorArray != NULL){
            colors = env->GetFloatArrayElements(colorArray, 0);
            len = env->GetArrayLength(colorArray);
        }
        RGBAColor *color;
        if (len < 4) {
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
