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

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ScreenObject_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        ScreenObjectClassInfo *classInfo = ScreenObjectClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            ScreenObject *inst = classInfo->getObject(env, obj);
            if (!inst)
                return;
            delete inst;
            classInfo->clearHandle(env, obj);
        }
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
        poly.color.asUnitFloats(primaryColors);

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
        jobject result = env->NewObject(cls, constructor, poly.texID, primaryColors[0], primaryColors[1], primaryColors[2], primaryColors[3], listPtObj, listTCObj );
        
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_ScreenObject_addTextureNative
(JNIEnv *env, jobject obj, jlong texID, jfloat r, jfloat g, jfloat b, jfloat a, jfloat width, jfloat height)
{
    try
    {
        ScreenObjectClassInfo *classInfo = ScreenObjectClassInfo::getClassInfo();
        ScreenObject *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        //Create Poly
        SimplePoly *poly = new SimplePoly();
        poly->texID = texID;
        poly->color = RGBAColor(r*255.0,g*255.0,b*255.0,a*255.0);
        poly->texID = texID;
        
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_ScreenObject_getSizeNative
(JNIEnv *env, jobject obj, jobject llObj, jobject urObj)
{
    try
    {
        ScreenObjectClassInfo *classInfo = ScreenObjectClassInfo::getClassInfo();
        Point2dClassInfo *pt2dClassInfo = Point2dClassInfo::getClassInfo();
        ScreenObject *inst = classInfo->getObject(env, obj);
        Point2d *ll = pt2dClassInfo->getObject(env,llObj);
        Point2d *ur = pt2dClassInfo->getObject(env,urObj);
        if (!inst || !ll || !ur)
            return;

        Mbr mbr;
        for (SimplePoly &poly : inst->polys)
        {
            for (auto &pt : poly.pts)
            {
                mbr.addPoint(pt);
            }
        }
        
        *ll = Point2d(mbr.ll().x(),mbr.ll().y());
        *ur = Point2d(mbr.ur().x(),mbr.ur().y());

        // Note: Porting  Do the strings too
//        for (int ii = 0; ii < getStringsSize(); ii++) {
//            StringWrapper str = getString(ii);
//            Point3d p0 = str.getMat().multiply(new Point3d(0,0,1));
//            Point3d p1 = str.getMat().multiply(new Point3d(str.getSize()[0], str.getSize()[1], 1));
//            mbr.addPoint(new Point2d(p0.getX(), p0.getY()));
//            mbr.addPoint(new Point2d(p1.getX(), p1.getY()));
//        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ScreenObject::getSizeNative()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ScreenObject_transform
(JNIEnv *env, jobject obj, jobject matObj)
{
    try
    {
        ScreenObjectClassInfo *classInfo = ScreenObjectClassInfo::getClassInfo();
        Matrix3dClassInfo *matClassInfo = Matrix3dClassInfo::getClassInfo();
        ScreenObject *inst = classInfo->getObject(env, obj);
        Matrix3d *mat = matClassInfo->getObject(env,matObj);
        if (!inst || !mat)
            return;
        
        for (SimplePoly &poly : inst->polys)
        {
            for (Point2d &pt : poly.pts)
            {
                Point3d newPt = (*mat) * Point3d(pt.x(),pt.y(),1.0);
                pt = Point2d(newPt.x(),newPt.y());
            }
        }
        
        for (StringWrapper &str : inst->strings)
        {
            str.mat = str.mat * (*mat);
        }
        
        
        // Note: Porting  Do the strings too
        //        for (int ii = 0; ii < getStringsSize(); ii++) {
        //            StringWrapper str = getString(ii);
        //            Point3d p0 = str.getMat().multiply(new Point3d(0,0,1));
        //            Point3d p1 = str.getMat().multiply(new Point3d(str.getSize()[0], str.getSize()[1], 1));
        //            mbr.addPoint(new Point2d(p0.getX(), p0.getY()));
        //            mbr.addPoint(new Point2d(p1.getX(), p1.getY()));
        //        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ScreenObject::getSizeNative()");
    }
 
}