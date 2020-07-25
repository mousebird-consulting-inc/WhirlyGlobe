/*
 *  VectorStyleSettings_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg
 *  Copyright 2011-2020 mousebird consulting
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

#import <Formats_jni.h>
#import "com_mousebird_maply_VectorStyleSettings.h"

using namespace WhirlyKit;
using namespace Eigen;

template<> VectorStyleSettingsClassInfo *VectorStyleSettingsClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_nativeInit
        (JNIEnv *env, jclass cls)
{
    VectorStyleSettingsClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_initialise
        (JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSettingsImplRef *inst = new VectorStyleSettingsImplRef(new VectorStyleSettingsImpl(1.0));
        VectorStyleSettingsClassInfo::getClassInfo()->setHandle(env,obj,inst);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_dispose
        (JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSettingsClassInfo *classInfo = VectorStyleSettingsClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            VectorStyleSettingsImplRef *inst = classInfo->getObject(env,obj);
            if (!inst)
                return;
            delete inst;
        }

        classInfo->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::dispose()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getLineScale
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            return (*inst)->lineScale;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::getLineScale()");
    }

    return 1.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setLineScale
(JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            (*inst)->lineScale = scale;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::setLineScale()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getTextScale
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            return (*inst)->textScale;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::getTextScale()");
    }

    return 1.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setTextScale
(JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            (*inst)->textScale = scale;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::setTextScale()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getMarkerScale
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            return (*inst)->markerScale;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::getMarkerScale()");
    }

    return 1.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setMarkerScale
(JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            (*inst)->markerScale = scale;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::setMarkerScale()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getMarkerImportance
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            return (*inst)->markerImportance;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::getMarkerImportance()");
    }

    return 1.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setMarkerImportance
(JNIEnv *env, jobject obj, jdouble import)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            (*inst)->markerImportance = import;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::setMarkerImportance()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getMarkerSize
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            return (*inst)->markerScale;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::getMarkerSize()");
    }

    return 1.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setMarkerSize
(JNIEnv *env, jobject obj, jdouble size)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            (*inst)->markerSize = size;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::setMarkerSize()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getLabelImportance
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            return (*inst)->labelImportance;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::getLabelImportance()");
    }

    return 1.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setLabelImportance
(JNIEnv *env, jobject obj, jdouble import)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            (*inst)->labelImportance = import;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::setMarkerSize()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorStyleSettings_getUseZoomLevels
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            return (*inst)->useZoomLevels;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::getUseZoomLevels()");
    }

    return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setUseZoomLabels
(JNIEnv *env, jobject obj, jboolean zoomLevels)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            (*inst)->useZoomLevels = zoomLevels;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::setUseZoomLabels()");
    }
}

JNIEXPORT jstring JNICALL Java_com_mousebird_maply_VectorStyleSettings_getUuidField
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            return env->NewStringUTF((*inst)->uuidField.c_str());
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::getUuidField()");
    }

    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setUuidField
(JNIEnv *env, jobject obj, jstring fieldStr)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst) {
            JavaString jStr(env,fieldStr);
            (*inst)->uuidField = jStr.cStr;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::setUuidField()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_VectorStyleSettings_getBaseDrawPriority
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            return (*inst)->baseDrawPriority;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::getBaseDrawPriority()");
    }

    return 0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setBaseDrawPriority
(JNIEnv *env, jobject obj, jint drawPriority)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            (*inst)->baseDrawPriority = drawPriority;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::setBaseDrawPriority()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_VectorStyleSettings_getDrawPriorityPerLevel
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            return (*inst)->drawPriorityPerLevel;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::getDrawPriorityPerLevel()");
    }

    return 0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setDrawPriorityPerLevel
(JNIEnv *env, jobject obj, jint drawPriority)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            (*inst)->drawPriorityPerLevel = drawPriority;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::setDrawPriorityPerLevel()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getMapScaleScale
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            return (*inst)->mapScaleScale;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::getMapScaleScale()");
    }

    return 1.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setMapScaleScale
(JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            (*inst)->mapScaleScale = scale;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::setMapScaleScale()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getDashPatternScale
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            return (*inst)->dashPatternScale;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::getDashPatternScale()");
    }

    return 1.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setDashPatternScale
(JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            (*inst)->dashPatternScale = scale;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::setDashPatternScale()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorStyleSettings_getUseWideVectors
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            return (*inst)->useWideVectors;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::getUseWideVectors()");
    }

    return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setUseWideVectors
(JNIEnv *env, jobject obj, jboolean useWideVectors)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            (*inst)->useWideVectors = useWideVectors;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::setUseWideVectors()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getOldVecWidthScale
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            return (*inst)->oldVecWidthScale;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::getOldVecWidthScale()");
    }

    return 1.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setOldVecWidthScale
(JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            (*inst)->oldVecWidthScale = scale;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::setOldVecWidthScale()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorStyleSettings_getWideVecCutoff
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            return (*inst)->wideVecCuttoff;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::getWideVecCutoff()");
    }

    return 1.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setWideVecCutoff
(JNIEnv *env, jobject obj, jdouble cutoff)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            (*inst)->wideVecCuttoff = cutoff;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::setWideVecCutoff()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorStyleSettings_getSelectable
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            return (*inst)->selectable;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::getSelectable()");
    }

    return 1.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setSelectable
(JNIEnv *env, jobject obj, jboolean selectable)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            (*inst)->selectable = selectable;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::setSelectable()");
    }
}

JNIEXPORT jstring JNICALL Java_com_mousebird_maply_VectorStyleSettings_getIconDirectory
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            return env->NewStringUTF((*inst)->iconDirectory.c_str());
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::getIconDirectory()");
    }

    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setIconDirectory
(JNIEnv *env, jobject obj, jstring iconDirStr)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst) {
            JavaString jStr(env,iconDirStr);
            (*inst)->iconDirectory = jStr.cStr;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::setIconDirectory()");
    }
}

JNIEXPORT jstring JNICALL Java_com_mousebird_maply_VectorStyleSettings_getFontName
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst)
            return env->NewStringUTF((*inst)->fontName.c_str());
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::getFontName()");
    }

    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorStyleSettings_setFontName
(JNIEnv *env, jobject obj, jstring fontNameStr)
{
    try
    {
        VectorStyleSettingsImplRef *inst = VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,obj);
        if (inst) {
            JavaString jStr(env,fontNameStr);
            (*inst)->fontName = jStr.cStr;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorStyleSettings::setFontName()");
    }
}
