/*  MapboxVectorStyleSet_jni.cpp
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
#import <Scene_jni.h>
#import <CoordSystem_jni.h>
#import <Vectors_jni.h>
#import "com_mousebird_maply_MapboxVectorStyleSet.h"

using namespace WhirlyKit;

template<> MapboxVectorStyleSetClassInfo *MapboxVectorStyleSetClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_nativeInit(JNIEnv *env, jclass cls)
{
    MapboxVectorStyleSetClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_initialise
(JNIEnv *env, jobject obj, jobject sceneObj, jobject coordSysObj, jobject settingsObj, jobject attrObj)
{
    try
    {
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env,sceneObj);
        CoordSystemRef *coordSystem = CoordSystemRefClassInfo::getClassInfo()->getObject(env,coordSysObj);
        MutableDictionary_AndroidRef *attrDict = AttrDictClassInfo::getClassInfo()->getObject(env,attrObj);
        if (!scene || !coordSystem || !attrDict)
            return;

        // Use settings or provide a default
        VectorStyleSettingsImplRef settings;
        if (settingsObj) {
            settings = *(VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,settingsObj));
        } else {
            settings = std::make_shared<VectorStyleSettingsImpl>(1.0);
        }

        auto inst = new MapboxVectorStyleSetImpl_AndroidRef(
                new MapboxVectorStyleSetImpl_Android(scene,coordSystem->get(),settings));

        // Need a pointer to this JNIEnv for low level parsing callbacks
        PlatformInfo_Android threadInst(env);

        (*inst)->thisObj = env->NewGlobalRef(obj);
        MapboxVectorStyleSetClassInfo::getClassInfo()->setHandle(env,obj,inst);

        const bool success = (*inst)->parse(&threadInst,*attrDict);
        if (!success)
        {
            __android_log_print(ANDROID_LOG_WARN, "Maply", "Failed to parse attrs in MapboxVectorStyleSet::initialise()");
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in MapboxVectorStyleSet::initialise()");
    }
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        MapboxVectorStyleSetClassInfo *classInfo = MapboxVectorStyleSetClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            MapboxVectorStyleSetImpl_AndroidRef *inst = classInfo->getObject(env,obj);
            if (!inst)
                return;
            (*inst)->cleanup(env);
            env->DeleteGlobalRef((*inst)->thisObj);
            (*inst)->thisObj = nullptr;
            delete inst;
        }

        classInfo->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in MapboxVectorStyleSet::dispose()");
    }
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_hasBackgroundStyle
        (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto inst = MapboxVectorStyleSetClassInfo::get(env,obj))
        {
            PlatformInfo_Android platformInfo(env);
            if (const auto style = (*inst)->backgroundStyle(&platformInfo))
            {
                return true;
            }
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in MapboxVectorStyleSet::hasBackgroundStyle()");
    }
    return false;
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_backgroundColorForZoomNative
        (JNIEnv *env, jobject obj, jdouble zoom)
{
    try
    {
        MapboxVectorStyleSetClassInfo *classInfo = MapboxVectorStyleSetClassInfo::getClassInfo();
        MapboxVectorStyleSetImpl_AndroidRef *inst = classInfo->getObject(env,obj);
        if (!inst)
            return 0;

        PlatformInfo_Android platformInfo(env);

        RGBAColorRef backColor = (*inst)->backgroundColor(&platformInfo,zoom);
        if (!backColor)
            return 0;

        return backColor->asInt();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in MapboxVectorStyleSet::backgroundColorForZoomNative()");
    }

    return 0;
}

extern "C"
JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_stylesForFeature
        (JNIEnv *, jobject, jobject attrs, jobject tileID, jstring featureName, jobject control)
{
    try
    {
        // not implemented
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in MapboxVectorStyleSet::stylesForFeature()");
    }
    return nullptr;
}

extern "C"
JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_allStyles
        (JNIEnv *env, jobject obj)
{
    try
    {
//        const auto styleInfo = VectorStyleClassInfo::getClassInfo();
        const auto styleSetRef = MapboxVectorStyleSetClassInfo::get(env,obj);
        if (const auto styleSet = styleSetRef ? *styleSetRef : nullptr)
        {
            PlatformInfo_Android inst(env);
            const auto styles = styleSet->allStyles(&inst);

//            std::vector<jobject> styleObjs;
//            styleObjs.reserve(styles.size());
//            for (const auto &style : styles)
//            {
//                auto styleObj = styleInfo->makeWrapperObject(env,new VectorStyleImpl_AndroidRef(style));
//                styleObjs.push_back(MakeComponentObjectWrapper(env, styleInfo, styleObj));
//            }
//
//            jobjectArray retArray = BuildObjectArray(env, styleInfo->getClass(), styleObjs);
//            for (auto objRef : styleObjs)
//            {
//                env->DeleteLocalRef(objRef);
//            }
//
//            return retArray;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in MapboxVectorStyleSet::allStyles()");
    }
    return nullptr;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_layerShouldDisplay
        (JNIEnv *, jobject, jstring name, jobject tileID)
{
    try
    {
        // not implemented
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in MapboxVectorStyleSet::layerShouldDisplay()");
    }
    return false;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_styleForUUID
        (JNIEnv *, jobject, jlong uuid, jobject control)
{
    try
    {
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in MapboxVectorStyleSet::styleForUUID()");
    }
    return nullptr;
}

/*
 * Class:     com_mousebird_maply_MapboxVectorStyleSet
 * Method:    getZoomSlot
 * Signature: ()I
 */
extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_getZoomSlot(JNIEnv *env, jobject obj)
{
    try
    {
        auto classInfo = MapboxVectorStyleSetClassInfo::getClassInfo();
        auto instPtr = classInfo->getObject(env,obj);
        if (auto inst = instPtr ? *instPtr : nullptr)
        {
            return inst->getZoomSlot();
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in MapboxVectorStyleSet::getZoomSlot()");
    }
    return -1;
}

/*
 * Class:     com_mousebird_maply_MapboxVectorStyleSet
 * Method:    setZoomSlot
 * Signature: (I)V
 */
extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_setZoomSlot(JNIEnv *env, jobject obj, jint slot)
{
    try
    {
        auto classInfo = MapboxVectorStyleSetClassInfo::getClassInfo();
        auto instPtr = classInfo->getObject(env,obj);
        if (auto inst = instPtr ? *instPtr : nullptr)
        {
            inst->setZoomSlot(slot);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in MapboxVectorStyleSet::setZoomSlot()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_setArealShaderNative
        (JNIEnv *env, jobject obj, jlong shaderID)
{
    try
    {
        MapboxVectorStyleSetClassInfo *classInfo = MapboxVectorStyleSetClassInfo::getClassInfo();
        MapboxVectorStyleSetImpl_AndroidRef *inst = classInfo->getObject(env,obj);
        if (!inst || shaderID == EmptyIdentity)
            return;

        (*inst)->vectorArealProgramID = shaderID;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in MapboxVectorStyleSet::setArealShaderNative()");
    }
}

extern "C"
JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_getStyleInfo
        (JNIEnv *env, jobject obj, jfloat zoom)
{
    try
    {
        const auto attrDictInfo = AttrDictClassInfo::getClassInfo();
        const auto styleSetRef = MapboxVectorStyleSetClassInfo::get(env,obj);
        if (const auto styleSet = styleSetRef ? *styleSetRef : nullptr)
        {
            PlatformInfo_Android inst(env);
            const auto styles = styleSet->allStyles(&inst);

            std::vector<jobject> results;
            results.reserve(styles.size());
            for (const auto &style : styles)
            {
                auto dict = std::make_unique<MutableDictionary_Android>();
                dict->setString("type", style->getType());
                dict->setString("ident", style->getIdent());
                dict->setString("representation", style->getRepresentation());
                const auto text = style->getLegendText(zoom);
                if (!text.empty())
                {
                    dict->setString("legendText", style->getLegendText(zoom));
                }
                auto const color = style->getLegendColor(zoom);
                if (color != RGBAColor::clear())
                {
                    dict->setInt("legendColor", color.asARGBInt());
                }
                auto dictObj = attrDictInfo->makeWrapperObject(env, new MutableDictionary_AndroidRef(dict.release()));
                results.push_back(dictObj);
            }

            auto retArray = BuildObjectArray(env, attrDictInfo->getClass(), results);
            for (auto &objRef : results)
            {
                env->DeleteLocalRef(objRef);
            }

            return retArray;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in MapboxVectorStyleSet::allStyles()");
    }
    return nullptr;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_setLayerVisible
        (JNIEnv *env, jobject obj, jstring layerNameJava, jboolean visible)
{
    try
    {
        const auto styleSetRef = MapboxVectorStyleSetClassInfo::get(env,obj);
        if (!styleSetRef)
            return;

        JavaString layerName(env,layerNameJava);
        for (auto &layer : (*styleSetRef)->layers) {
            if (layer->ident == layerName.getCString()) {
                layer->visible = visible;
            }
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in MapboxVectorStyleSet::setLayerVisible()");
    }
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_addSpritesNative
        (JNIEnv *env, jobject obj, jstring assetJSONJava, jlong texID, int width, int height)
{
    try
    {
        const auto styleSetRef = MapboxVectorStyleSetClassInfo::get(env,obj);
        if (!styleSetRef)
            return false;

        JavaString assetJSON(env,assetJSONJava);
        auto newSprites = std::make_shared<MapboxVectorStyleSprites>(texID,width,height);
        auto dict = std::make_shared<MutableDictionary_Android>();
        if (!dict->parseJSON(assetJSON.getCString()))
            return false;
        if (newSprites->parse(*styleSetRef, dict))
        {
            (*styleSetRef)->addSprites(newSprites);
            return true;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in MapboxVectorStyleSet::addSpritesNative()");
    }

    return true;
}