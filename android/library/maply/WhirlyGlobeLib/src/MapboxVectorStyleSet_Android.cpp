/*  MapboxVectorStyleSet_Android.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/29/20.
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

#import "MapboxVectorStyleSet_Android.h"
#import "Formats_jni.h"
#import "LabelsAndMarkers_jni.h"
#import "Exceptions_jni.h"
#import "classInfo/Geometry_jni.h"
#import <string>
#import <iostream>
#import <sstream>
#import <locale>
#import <codecvt>

namespace WhirlyKit
{

MapboxVectorStyleSetImpl_Android::MapboxVectorStyleSetImpl_Android(Scene *scene,CoordSystem *coordSys,VectorStyleSettingsImplRef settings) :
    MapboxVectorStyleSetImpl(scene,coordSys,std::move(settings))
{
}

void MapboxVectorStyleSetImpl_Android::setupMethods(JNIEnv *env)
{
    if (!makeLabelInfoMethod)
    {
        jclass thisClass = MapboxVectorStyleSetClassInfo::getClassInfo()->getClass();
        makeLabelInfoMethod      = env->GetMethodID(thisClass,"labelInfoForFont",  "(Ljava/lang/String;F)Lcom/mousebird/maply/LabelInfo;");
        calculateTextWidthMethod = env->GetMethodID(thisClass,"calculateTextWidth","(Ljava/lang/String;Lcom/mousebird/maply/LabelInfo;)D");
        makeCircleTextureMethod  = env->GetMethodID(thisClass,"makeCircleTexture", "(DIIFLcom/mousebird/maply/Point2d;)J");
        makeLineTextureMethod    = env->GetMethodID(thisClass,"makeLineTexture",   "([D)J");
    }
}

void MapboxVectorStyleSetImpl_Android::cleanup(JNIEnv *env)
{
    for (auto &labelInfo : labelInfos)
    {
        env->DeleteGlobalRef(labelInfo.second->labelInfoObj);
        labelInfo.second->labelInfoObj = nullptr;
    }
    labelInfos.clear();
}

MapboxVectorStyleSetImpl_Android::~MapboxVectorStyleSetImpl_Android()
{
    if (thisObj)
    {
        wkLogLevel(Warn, "Failed to clean up MapboxVectorStyleSetImpl_Android Java ref");
    }
}

SimpleIdentity MapboxVectorStyleSetImpl_Android::makeCircleTexture(PlatformThreadInfo *inInst,
        double radius,
        const RGBAColor &fillColor,
        const RGBAColor &strokeColor,
        float strokeWidth,
        Point2f *circleSize)
{
    auto inst = (PlatformInfo_Android *)inInst;
    auto env = inst->env;
    try
    {
        setupMethods(env);
        jobject pointObj = nullptr;
        if (circleSize)
        {
            pointObj = MakePoint2d(env);
            if (!pointObj)
            {
                return EmptyIdentity;
            }
        }

        const auto id = env->CallLongMethod(thisObj, makeCircleTextureMethod,
                                            radius, fillColor.asInt(), strokeColor.asInt(), strokeWidth, pointObj);

        if (circleSize)
        {
            auto point = Point2dClassInfo::getClassInfo()->getObject(env,pointObj);
            circleSize->x() = (float)point->x();
            circleSize->y() = (float)point->y();

            // local ref, no need for `finally` style cleanup
            env->DeleteLocalRef(pointObj);
        }

        return id;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in makeCircleTexture()");
    }
    return EmptyIdentity;
}

SimpleIdentity MapboxVectorStyleSetImpl_Android::makeLineTexture(PlatformThreadInfo *inInst,const std::vector<double> &dashComponents)
{
    auto inst = (PlatformInfo_Android *)inInst;
    auto env = inst->env;
    try
    {
        setupMethods(env);
        if (auto arrayObj = BuildDoubleArray(env, dashComponents))
        {
            const auto id = env->CallLongMethod(thisObj, makeLineTextureMethod, arrayObj);
            env->DeleteLocalRef(arrayObj);
            return id;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in makeLineTexture()");
    }
    return EmptyIdentity;
}

LabelInfoRef MapboxVectorStyleSetImpl_Android::makeLabelInfo(PlatformThreadInfo *inInst,const std::vector<std::string> &fontNames,float fontSize)
{
    auto inst = (PlatformInfo_Android *)inInst;

    if (fontNames.empty())
    {
        return LabelInfoRef();
    }

    try
    {
        const auto key = std::make_pair(fontNames[0],fontSize);
        const auto result = labelInfos.insert(std::make_pair(key, LabelInfoAndroidRef()));
        if (!result.second)
        {
            // Already present, return it
            return result.first->second;
        }

        jstring jFontNameStr = inst->env->NewStringUTF(fontNames[0].c_str());
        jobject labelInfo = inst->env->CallObjectMethod(thisObj, makeLabelInfoMethod, jFontNameStr, fontSize);
        logAndClearJVMException(inst->env, "labelInfoForFont");
        inst->env->DeleteLocalRef(jFontNameStr);

        if (jobject labelInfoGlobeObj = inst->env->NewGlobalRef(labelInfo))
        {
            const auto newRef = LabelInfoClassInfo::getClassInfo()->getObject(inst->env,labelInfoGlobeObj);
            const auto refLabelInfo = *newRef;
            refLabelInfo->labelInfoObj = labelInfoGlobeObj;
            refLabelInfo->programID = screenMarkerProgramID;

            // Save it to the cache map
            result.first->second = refLabelInfo;

            return refLabelInfo;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in makeLabelInfo()");
    }
    return LabelInfoRef();
}

SingleLabelRef MapboxVectorStyleSetImpl_Android::makeSingleLabel(PlatformThreadInfo *inInst,const std::string &text)
{
    auto label = std::make_shared<SingleLabelAndroid>();

    // Break up the lines and turn the characters into code points for Java
    std::istringstream ss(text);
    std::string line;
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
    while (std::getline(ss, line, ss.widen('\n')))
    {
        const std::u16string utf16 = utf16conv.from_bytes(line);
        label->codePointsLines.emplace_back(utf16.begin(), utf16.end());
    }

    return std::move(label);
}

// NOLINTNEXTLINE(google-default-arguments)
ComponentObjectRef MapboxVectorStyleSetImpl_Android::makeComponentObject(PlatformThreadInfo *inInst, const Dictionary *desc)
{
    return desc ? std::make_shared<ComponentObject>(false, false, *desc)
                : std::make_shared<ComponentObject>(false, false);
}

double MapboxVectorStyleSetImpl_Android::calculateTextWidth(PlatformThreadInfo *inInst,const LabelInfoRef &inLabelInfo,const std::string &text)
{
    const auto inst = (PlatformInfo_Android *)inInst;
    if (auto labelInfo = dynamic_cast<LabelInfoAndroid*>(inLabelInfo.get()))
    {
        if (jstring jText = inst->env->NewStringUTF(text.c_str()))
        {
            jdouble width = 0;
            width = inst->env->CallDoubleMethod(thisObj, calculateTextWidthMethod, jText,
                                                labelInfo->labelInfoObj);
            inst->env->DeleteLocalRef(jText);
            return width;
        }
    }
    return 0.0;
}

void MapboxVectorStyleSetImpl_Android::addSelectionObject(SimpleIdentity selectID,const VectorObjectRef &vecObj,const ComponentObjectRef &compObj)
{
    if (!compManage)
        return;

    // TODO: Fill this in
    // We need to associate the given VectorObjectRef with the given selectID
    // There are created on the C++ side by the mapbox style parser and so have no representation on the Java side.
}

}
