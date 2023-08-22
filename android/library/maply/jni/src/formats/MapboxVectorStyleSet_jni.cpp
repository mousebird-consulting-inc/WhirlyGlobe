/*  MapboxVectorStyleSet_jni.cpp
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
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_initialise
    (JNIEnv *env, jobject obj, jobject sceneObj, jobject coordSysObj,
     jobject settingsObj, jobject attrObj)
{
    try
    {
        Scene *scene = SceneClassInfo::get(env,sceneObj);
        CoordSystemRef *coordSystem = CoordSystemRefClassInfo::get(env,coordSysObj);
        MutableDictionary_AndroidRef *attrDict = AttrDictClassInfo::get(env,attrObj);
        if (!scene || !coordSystem || !attrDict)
            return false;

        // Use settings or provide a default
        VectorStyleSettingsImplRef settings;
        if (settingsObj) {
            settings = *(VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,settingsObj));
        } else {
            settings = std::make_shared<VectorStyleSettingsImpl>(1.0);
        }

        // Performance wide vectors aren't yet supported on OpenGL
        settings->perfWideVec = false;

        auto inst = new MapboxVectorStyleSetImpl_AndroidRef(
                std::make_shared<MapboxVectorStyleSetImpl_Android>(scene,coordSystem->get(),settings));

        // Need a pointer to this JNIEnv for low level parsing callbacks
        PlatformInfo_Android threadInst(env);

        (*inst)->thisObj = env->NewWeakGlobalRef(obj);
        MapboxVectorStyleSetClassInfo::getClassInfo()->setHandle(env,obj,inst);

        const bool success = (*inst)->parse(&threadInst,*attrDict);
        if (!success)
        {
            __android_log_print(ANDROID_LOG_WARN, "Maply", "Failed to parse attrs in MapboxVectorStyleSet::initialise()");
        }
        return success;
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_dispose
    (JNIEnv *env, jobject obj)
{
    try
    {
        MapboxVectorStyleSetClassInfo *classInfo = MapboxVectorStyleSetClassInfo::getClassInfo();

        std::lock_guard<std::mutex> lock(disposeMutex);
        if (auto pinst = classInfo->getObject(env,obj))
        {
            if (auto* inst = pinst->get())
            {
                inst->cleanup(env);
                if (inst->thisObj)
                {
                    env->DeleteWeakGlobalRef(inst->thisObj);
                    inst->thisObj = nullptr;
                }
            }
            classInfo->clearHandle(env,obj);
            delete pinst;
        }
    }
    MAPLY_STD_JNI_CATCH()
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
            if (/*const auto style = */(*inst)->backgroundStyle(&platformInfo))
            {
                return true;
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
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
    MAPLY_STD_JNI_CATCH()
    return 0;
}

extern "C"
JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_stylesForFeature
        (JNIEnv *, jobject /*obj*/, jobject /*attrs*/,
         jobject /*tileID*/, jstring /*featureName*/, jobject /*control*/)
{
    try
    {
        // not implemented
        wkLogLevel(Warn, "MapboxVectorStyleSet.stylesForFeature not implemented");
    }
    MAPLY_STD_JNI_CATCH()
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
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_layerShouldDisplay
        (JNIEnv *, jobject /*obj*/, jstring /*name*/, jobject /*tileID*/)
{
    try
    {
        // not implemented
        wkLogLevel(Warn, "MapboxVectorStyleSet.layerShouldDisplay not implemented");
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_styleForUUID
        (JNIEnv *, jobject /*obj*/, jlong /*uuid*/, jobject /*control*/)
{
    try
    {
        wkLogLevel(Warn, "MapboxVectorStyleSet.styleForUUID not implemented");
    }
    MAPLY_STD_JNI_CATCH()
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
    MAPLY_STD_JNI_CATCH()
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
    MAPLY_STD_JNI_CATCH()
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
    MAPLY_STD_JNI_CATCH()
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
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_setLayerVisible
  (JNIEnv *env, jobject obj, jstring layerNameJava, jboolean visible)
{
    try
    {
        if (const auto styleSetRef = MapboxVectorStyleSetClassInfo::get(env,obj))
        {
            JavaString layerName(env,layerNameJava);
            for (auto &layer : (*styleSetRef)->layers)
            {
                if (layer->ident == layerName.getCString())
                {
                    layer->visible = visible;
                }
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_addRepsNative(
  JNIEnv *env, jobject obj, jstring uuidAttrStr,
  jobjectArray srcArr, jobjectArray repArr,
  jobjectArray sizeArr, jobjectArray colorArr)
{
    try
    {
        if (const auto styleSetRef = MapboxVectorStyleSetClassInfo::get(env,obj))
        {
            const JavaString uuidAttr(env, uuidAttrStr);
            const auto sources = ConvertStringArray(env, srcArr);
            const auto reps = ConvertStringArray(env, repArr);
            const auto sizes = ConvertFloatObjArray(env, sizeArr, -1.0f);
            const auto colors = ConvertStringArray(env, colorArr);
            if (reps.size() == sizes.size() && sizes.size() == colors.size())
            {
                PlatformInfo_Android inst(env);
                return (*styleSetRef)->addRepresentations(&inst, uuidAttr.getCString(),
                                                          sources, reps, sizes, colors);
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_hasRepresentations(
  JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto styleSetRef = MapboxVectorStyleSetClassInfo::get(env,obj))
        {
            return (*styleSetRef)->hasRepresentations();
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
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
    MAPLY_STD_JNI_CATCH()
    return true;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_getSpriteInfoNative
    (JNIEnv *env, jobject obj, jstring nameStr, jintArray xywh)
{
    try
    {
        if (!nameStr || !xywh || env->GetArrayLength(xywh) != 4) {
            return false;
        }

        const auto styleSetRef = MapboxVectorStyleSetClassInfo::get(env,obj);
        const auto sprites = (styleSetRef && *styleSetRef) ? (*styleSetRef)->sprites : nullptr;
        if (!sprites) {
            return false;
        }

        const JavaString name(env,nameStr);
        const auto entry = sprites->getSprite(name.getCString());
        if (entry.width == 0 || entry.height == 0)
        {
            return false;
        }

        const int v[] = { entry.x, entry.y, entry.width, entry.height };
        env->SetIntArrayRegion(xywh, 0, 4, &v[0]);
        return true;
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}
