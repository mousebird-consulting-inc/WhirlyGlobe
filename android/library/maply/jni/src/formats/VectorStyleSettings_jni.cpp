/*  VectorStyleSettings_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg
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

#import <Formats_jni.h>
#import "com_mousebird_maply_VectorStyleSettings.h"

using namespace WhirlyKit;
using namespace Eigen;

template<> VectorStyleSettingsClassInfo *VectorStyleSettingsClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_nativeInit
    (JNIEnv *env, jclass cls)
{
    VectorStyleSettingsClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_initialise
    (JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        auto inst = new VectorStyleSettingsImplRef(std::make_shared<VectorStyleSettingsImpl>(scale));
        // Performance wide vectors aren't yet supported on OpenGL
        (*inst)->perfWideVec = false;
        VectorStyleSettingsClassInfo::set(env,obj,inst);
    }
    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_dispose
    (JNIEnv *env, jobject obj)
{
    try
    {
        const auto classInfo = VectorStyleSettingsClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        delete classInfo->getObject(env,obj);
        classInfo->clearHandle(env,obj);
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getLineScale
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->lineScale;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setLineScale
    (JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->lineScale = (float)scale;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getTextScale
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->textScale;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setTextScale
    (JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->textScale = (float)scale;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getMarkerScale
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->markerScale;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setMarkerScale
    (JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->markerScale = (float)scale;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getSymbolScale
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->symbolScale;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setSymbolScale
    (JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->symbolScale = (float)scale;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getCircleScale
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->circleScale;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setCircleScale
    (JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->circleScale = (float)scale;
        }
    }
    MAPLY_STD_JNI_CATCH()
}


extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getMarkerImportance
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->markerImportance;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setMarkerImportance
    (JNIEnv *env, jobject obj, jdouble import)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->markerImportance = (float)import;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getMarkerSize
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->markerScale;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setMarkerSize
    (JNIEnv *env, jobject obj, jdouble size)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->markerSize = (float)size;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getLabelImportance
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->labelImportance;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setLabelImportance
    (JNIEnv *env, jobject obj, jdouble import)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->labelImportance = (float)import;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorStyleSettings_getUseZoomLevels
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->useZoomLevels;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setUseZoomLevels
    (JNIEnv *env, jobject obj, jboolean zoomLevels)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->useZoomLevels = zoomLevels;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jstring JNICALL Java_com_mousebird_maply_VectorStyleSettings_getUuidField
    (JNIEnv *env, jobject obj)
{
    try
    {
        auto inst = VectorStyleSettingsClassInfo::get(env,obj);
        return inst ? env->NewStringUTF((*inst)->uuidField.c_str()) : nullptr;
    }
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setUuidField
    (JNIEnv *env, jobject obj, jstring fieldStr)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            if (const auto jStr = JavaString(env, fieldStr))
            {
                (*inst)->uuidField = jStr.getCString();
            }
            else
            {
                (*inst)->uuidField.clear();
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_VectorStyleSettings_getBaseDrawPriority
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->baseDrawPriority;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setBaseDrawPriority
    (JNIEnv *env, jobject obj, jint drawPriority)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->baseDrawPriority = drawPriority;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_VectorStyleSettings_getDrawPriorityPerLevel
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->drawPriorityPerLevel;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setDrawPriorityPerLevel
    (JNIEnv *env, jobject obj, jint drawPriority)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->drawPriorityPerLevel = drawPriority;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getMapScaleScale
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->mapScaleScale;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setMapScaleScale
    (JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->mapScaleScale = (float)scale;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getDashPatternScale
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->dashPatternScale;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setDashPatternScale
    (JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->dashPatternScale = (float)scale;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorStyleSettings_getUseWideVectors
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->useWideVectors;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setUseWideVectors
    (JNIEnv *env, jobject obj, jboolean useWideVectors)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->useWideVectors = useWideVectors;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getOldVecWidthScale
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->oldVecWidthScale;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setOldVecWidthScale
    (JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->oldVecWidthScale = (float)scale;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getWideVecCutoff
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->wideVecCuttoff;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setWideVecCutoff
    (JNIEnv *env, jobject obj, jdouble cutoff)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->wideVecCuttoff = (float)cutoff;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorStyleSettings_getSelectable
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->selectable;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setSelectable
    (JNIEnv *env, jobject obj, jboolean selectable)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->selectable = selectable;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jstring JNICALL Java_com_mousebird_maply_VectorStyleSettings_getIconDirectory
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return env->NewStringUTF((*inst)->iconDirectory.c_str());
        }
    }
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setIconDirectory
    (JNIEnv *env, jobject obj, jstring iconDirStr)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            const JavaString jStr(env,iconDirStr);
            (*inst)->iconDirectory = jStr.getCString();
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jstring JNICALL Java_com_mousebird_maply_VectorStyleSettings_getFontName
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return env->NewStringUTF((*inst)->fontName.c_str());
        }
    }
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setFontName
    (JNIEnv *env, jobject obj, jstring fontNameStr)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            const JavaString jStr(env,fontNameStr);
            (*inst)->fontName = jStr.getCString();
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setZBufferRead
    (JNIEnv *env, jobject obj, jboolean val)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->zBufferRead = val;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setZBufferWrite
    (JNIEnv *env, jobject obj, jboolean val)
{
    try
    {
        if (const auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->zBufferWrite = val;
        }
    }
    MAPLY_STD_JNI_CATCH()
}
