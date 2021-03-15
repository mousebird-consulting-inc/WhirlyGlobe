/*  VectorTileData_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg
 *  Copyright 2011-2021 mousebird consulting
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

#import <Formats_jni.h>
#import <Geometry_jni.h>
#import <Vectors_jni.h>
#import <Components_jni.h>
#import <Scene_jni.h>
#import "com_mousebird_maply_VectorTileData.h"

using namespace WhirlyKit;
using namespace Eigen;

template<> VectorTileDataClassInfo *VectorTileDataClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorTileData_nativeInit(JNIEnv *env, jclass cls)
{
    VectorTileDataClassInfo::getClassInfo(env, cls);
}

JNIEXPORT jobject JNICALL MakeVectorTileDataObject(JNIEnv *env,const VectorTileDataRef &tileData)
{
    const auto classInfo = VectorTileDataClassInfo::getClassInfo(env,"com/mousebird/maply/VectorTileData");
    return classInfo->makeWrapperObject(env,new VectorTileDataRef(tileData));
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorTileData_initialise__(JNIEnv *env, jobject obj)
{
    try
    {
        VectorTileDataRef *tileData = new VectorTileDataRef(new VectorTileData());
        VectorTileDataClassInfo::getClassInfo()->setHandle(env,obj,tileData);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorTileData::initialise()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorTileData_initialise__IIILcom_mousebird_maply_Point2d_2Lcom_mousebird_maply_Point2d_2Lcom_mousebird_maply_Point2d_2Lcom_mousebird_maply_Point2d_2
    (JNIEnv *env, jobject obj, jint x, jint y, jint level, jobject bllobj, jobject burobj, jobject geollobj, jobject geourobj)
{
    try
    {
        Point2dClassInfo *pt2dClassInfo = Point2dClassInfo::getClassInfo();
        Point2d *boundLL = pt2dClassInfo->getObject(env,bllobj);
        Point2d *boundUR = pt2dClassInfo->getObject(env,burobj);
        Point2d *geoLL = pt2dClassInfo->getObject(env,geollobj);
        Point2d *geoUR = pt2dClassInfo->getObject(env,geourobj);
        if (!boundLL || !boundUR || !geoLL || !geoUR)
            return;
        VectorTileDataRef *tileData = new VectorTileDataRef(new VectorTileData());
        (*tileData)->ident = QuadTreeIdentifier(x,y,level);
        (*tileData)->bbox.ll() = *boundLL;
        (*tileData)->bbox.ur() = *boundUR;
        (*tileData)->geoBBox.ll() = *geoLL;
        (*tileData)->geoBBox.ur() = *geoUR;
        VectorTileDataClassInfo::getClassInfo()->setHandle(env,obj,tileData);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorTileData::initialise()");
    }
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorTileData_dispose(JNIEnv *env, jobject obj)
{
    try
    {
        const auto classInfo = VectorTileDataClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            auto inst = classInfo->getObject(env,obj);
            delete inst;
        }

        classInfo->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorTileData::dispose()");
    }
}

extern "C"
JNIEXPORT jintArray JNICALL Java_com_mousebird_maply_VectorTileData_getTileIDNative(JNIEnv *env, jobject obj)
{
    try
    {
        const auto tileData = VectorTileDataClassInfo::get(env,obj);
        if (tileData)
        {
            const auto tileID = (*tileData)->ident;
            const std::vector<int> ints{tileID.x, tileID.y, tileID.level};
            return BuildIntArray(env, ints);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorTileData::getTileIDNative");
    }
    return nullptr;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorTileData_getBoundsNative(JNIEnv *env, jobject obj, jobject llObj, jobject urObj)
{
    try
    {
        if (const auto tileData = VectorTileDataClassInfo::get(env,obj))
        {
            if (auto ll = Point2dClassInfo::get(env, llObj))
            {
                if (auto ur = Point2dClassInfo::get(env, urObj))
                {
                    *ll = (*tileData)->bbox.ll();
                    *ur = (*tileData)->bbox.ur();
                }
            }
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply","Crash in VectorTileData::getBoundsNative");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorTileData_getGeoBoundsNative(JNIEnv *env, jobject obj, jobject llObj, jobject urObj)
{
    try
    {
        if (const auto tileData = VectorTileDataClassInfo::get(env,obj))
        {
            if (auto ll = Point2dClassInfo::get(env, llObj))
            {
                if (auto ur = Point2dClassInfo::get(env, urObj))
                {
                    *ll = (*tileData)->geoBBox.ll();
                    *ur = (*tileData)->geoBBox.ur();
                }
            }
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply","Crash in VectorTileData::getGeoBoundsNative");
    }
}

extern "C"
JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_VectorTileData_getComponentObjects__(JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto tileData = VectorTileDataClassInfo::get(env,obj))
        {
            const auto classInfo = ComponentObjectRefClassInfo::getClassInfo();

            std::vector<jobject> compObjs;
            compObjs.reserve((*tileData)->compObjs.size());
            for (const auto &compObj : (*tileData)->compObjs)
            {
                compObjs.push_back(MakeComponentObjectWrapper(env, classInfo, compObj));
            }

            jobjectArray retArray = BuildObjectArray(env, classInfo->getClass(), compObjs);
            for (auto objRef : compObjs)
            {
                env->DeleteLocalRef(objRef);
            }

            return retArray;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorTileData::getComponentObjects");
    }
    return nullptr;
}

extern "C"
JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_VectorTileData_getComponentObjects__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring jStr)
{
    try
    {
        if (const auto tileData = VectorTileDataClassInfo::get(env,obj))
        {
            const JavaString str(env, jStr);
            const auto it = (*tileData)->categories.find(str.getCString());
            if (it == (*tileData)->categories.end())
            {
                return nullptr;
            }
            
            const auto &compObjs = it->second;

            const auto classInfo = ComponentObjectRefClassInfo::getClassInfo();

            std::vector<jobject> outCompObjs;
            outCompObjs.reserve(compObjs.size());
            for (const auto &compObj : compObjs)
            {
                outCompObjs.push_back(MakeComponentObjectWrapper(env, classInfo, compObj));
            }

            return BuildObjectArray(env, classInfo->getClass(), outCompObjs);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply","Crash in VectorTileData::getComponentObjects (by category)");
    }
    return nullptr;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorTileData_addComponentObject(JNIEnv *env, jobject obj, jobject compObjObj)
{
    try
    {
        if (const auto tileData = VectorTileDataClassInfo::get(env,obj))
        {
            if (const auto compObj = ComponentObjectRefClassInfo::get(env,compObjObj))
            {
                (*tileData)->compObjs.push_back(*compObj);
            }
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorTileData::addComponentObject");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorTileData_addComponentObjects(JNIEnv *env, jobject obj, jobjectArray compObjArray)
{
    try
    {
        if (const auto tileData = VectorTileDataClassInfo::get(env,obj))
        {
            JavaObjectArrayHelper compObjHelp(env, compObjArray);
            while (jobject compObjObj = compObjHelp.getNextObject())
            {
                const auto compObj = ComponentObjectRefClassInfo::get(env, compObjObj);
                (*tileData)->compObjs.push_back(*compObj);
            }
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorTileData::addComponentObjects");
    }
}

extern "C"
JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_VectorTileData_getVectors(JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto tileData = VectorTileDataClassInfo::get(env,obj))
        {
            const auto vecObjClassInfo = VectorObjectClassInfo::getClassInfo();

            std::vector<jobject> outVecs;
            outVecs.reserve((*tileData)->vecObjs.size());
            for (const auto &vecObjRef : (*tileData)->vecObjs)
            {
                outVecs.push_back(MakeVectorObjectWrapper(env, vecObjClassInfo, vecObjRef));
            }
            return BuildObjectArray(env, vecObjClassInfo->getClass(), outVecs);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorTileData::getVectors");
    }
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorTileData_getChangeSet(JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto tileData = VectorTileDataClassInfo::get(env,obj))
        {
            const jobject newObj = MakeChangeSet(env, (*tileData)->changes);
            (*tileData)->changes.clear();
            return newObj;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorTileData::getChangeSet");
    }
    return nullptr;
}
