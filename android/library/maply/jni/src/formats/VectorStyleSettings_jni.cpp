/*  VectorStyleSettings_jni.cpp
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
#import "com_mousebird_maply_VectorStyleSettings.h"

using namespace WhirlyKit;
using namespace Eigen;

template<> VectorStyleSettingsClassInfo *VectorStyleSettingsClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_nativeInit(JNIEnv *env, jclass cls)
{
    VectorStyleSettingsClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_initialise(JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        auto inst = new VectorStyleSettingsImplRef(new VectorStyleSettingsImpl(scale));
        VectorStyleSettingsClassInfo::getClassInfo()->setHandle(env,obj,inst);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::initialise()");
    }
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_dispose(JNIEnv *env, jobject obj)
{
    try
    {
        const auto classInfo = VectorStyleSettingsClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            auto inst = classInfo->getObject(env,obj);
            delete inst;
        }
        classInfo->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::dispose()");
    }
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getLineScale(JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->lineScale;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::getLineScale()");
    }

    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setLineScale(JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->lineScale = scale;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::setLineScale()");
    }
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getTextScale(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->textScale;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::getTextScale()");
    }

    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setTextScale(JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj)) {
            (*inst)->textScale = scale;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::setTextScale()");
    }
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getMarkerScale(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->markerScale;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::getMarkerScale()");
    }

    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setMarkerScale(JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->markerScale = scale;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::setMarkerScale()");
    }
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getMarkerImportance(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->markerImportance;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::getMarkerImportance()");
    }

    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setMarkerImportance(JNIEnv *env, jobject obj, jdouble import)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->markerImportance = import;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::setMarkerImportance()");
    }
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getMarkerSize(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->markerScale;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::getMarkerSize()");
    }
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setMarkerSize(JNIEnv *env, jobject obj, jdouble size)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->markerSize = size;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::setMarkerSize()");
    }
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getLabelImportance(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->labelImportance;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::getLabelImportance()");
    }
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setLabelImportance(JNIEnv *env, jobject obj, jdouble import)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->labelImportance = import;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::setMarkerSize()");
    }
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorStyleSettings_getUseZoomLevels(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->useZoomLevels;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::getUseZoomLevels()");
    }
    return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setUseZoomLevels(JNIEnv *env, jobject obj, jboolean zoomLevels)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->useZoomLevels = zoomLevels;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::setUseZoomLevels()");
    }
}

extern "C"
JNIEXPORT jstring JNICALL Java_com_mousebird_maply_VectorStyleSettings_getUuidField(JNIEnv *env, jobject obj)
{
    try
    {
        auto inst = VectorStyleSettingsClassInfo::get(env,obj);
        return inst ? env->NewStringUTF((*inst)->uuidField.c_str()) : nullptr;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::getUuidField()");
    }
    return nullptr;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setUuidField(JNIEnv *env, jobject obj, jstring fieldStr)
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
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::setUuidField()");
    }
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_VectorStyleSettings_getBaseDrawPriority(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->baseDrawPriority;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::getBaseDrawPriority()");
    }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setBaseDrawPriority(JNIEnv *env, jobject obj, jint drawPriority)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->baseDrawPriority = drawPriority;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::setBaseDrawPriority()");
    }
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_VectorStyleSettings_getDrawPriorityPerLevel(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->drawPriorityPerLevel;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::getDrawPriorityPerLevel()");
    }

    return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setDrawPriorityPerLevel(JNIEnv *env, jobject obj, jint drawPriority)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->drawPriorityPerLevel = drawPriority;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::setDrawPriorityPerLevel()");
    }
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getMapScaleScale(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->mapScaleScale;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::getMapScaleScale()");
    }
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setMapScaleScale(JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        auto inst = VectorStyleSettingsClassInfo::get(env,obj);
        if (inst)
            (*inst)->mapScaleScale = scale;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::setMapScaleScale()");
    }
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getDashPatternScale(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->dashPatternScale;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::getDashPatternScale()");
    }
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setDashPatternScale(JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->dashPatternScale = scale;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::setDashPatternScale()");
    }
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorStyleSettings_getUseWideVectors(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->useWideVectors;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::getUseWideVectors()");
    }
    return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setUseWideVectors(JNIEnv *env, jobject obj, jboolean useWideVectors)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->useWideVectors = useWideVectors;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::setUseWideVectors()");
    }
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getOldVecWidthScale(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->oldVecWidthScale;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::getOldVecWidthScale()");
    }
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setOldVecWidthScale(JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->oldVecWidthScale = scale;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::setOldVecWidthScale()");
    }
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getWideVecCutoff(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->wideVecCuttoff;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::getWideVecCutoff()");
    }
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setWideVecCutoff(JNIEnv *env, jobject obj, jdouble cutoff)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->wideVecCuttoff = cutoff;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::setWideVecCutoff()");
    }
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorStyleSettings_getSelectable(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return (*inst)->selectable;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::getSelectable()");
    }
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setSelectable(JNIEnv *env, jobject obj, jboolean selectable)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->selectable = selectable;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::setSelectable()");
    }
}

extern "C"
JNIEXPORT jstring JNICALL Java_com_mousebird_maply_VectorStyleSettings_getIconDirectory(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return env->NewStringUTF((*inst)->iconDirectory.c_str());
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::getIconDirectory()");
    }
    return nullptr;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setIconDirectory(JNIEnv *env, jobject obj, jstring iconDirStr)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            const JavaString jStr(env,iconDirStr);
            (*inst)->iconDirectory = jStr.getCString();
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::setIconDirectory()");
    }
}

extern "C"
JNIEXPORT jstring JNICALL Java_com_mousebird_maply_VectorStyleSettings_getFontName(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            return env->NewStringUTF((*inst)->fontName.c_str());
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::getFontName()");
    }
    return nullptr;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setFontName(JNIEnv *env, jobject obj, jstring fontNameStr)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            const JavaString jStr(env,fontNameStr);
            (*inst)->fontName = jStr.getCString();
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::setFontName()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setZBufferRead(JNIEnv *env, jobject obj, jboolean val)
{
    try
    {
        if (auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->zBufferRead = val;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::setZBufferRead()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setZBufferWrite(JNIEnv *env, jobject obj, jboolean val)
{
    try
    {
        if (const auto inst = VectorStyleSettingsClassInfo::get(env,obj))
        {
            (*inst)->zBufferWrite = val;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in VectorStyleSettings::setZBufferWrite()");
    }
}
