/*
 *  MapboxVectorStyleSet_Android.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/29/20.
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

#import "MapboxVectorStyleSet_Android.h"
#import <string>
#import <iostream>
#import <sstream>
#import <locale>
#import <codecvt>
#import "Formats_jni.h"
#import "LabelsAndMarkers_jni.h"

namespace WhirlyKit
{

MapboxVectorStyleSetImpl_Android::MapboxVectorStyleSetImpl_Android(Scene *scene,CoordSystem *coordSys,VectorStyleSettingsImplRef settings)
: MapboxVectorStyleSetImpl(scene,coordSys,settings), thisObj(NULL),
    makeLabelInfoMethod(NULL), makeCircleTextureMethod(NULL), makeLineTextureMethod(NULL)
{
}

void MapboxVectorStyleSetImpl_Android::setupMethods(JNIEnv *env)
{
    // TODO: Turn global reference into local reference for this object

    if (!makeLabelInfoMethod) {
        jclass thisClass = MapboxVectorStyleSetClassInfo::getClassInfo()->getClass();
        // TODO: Get the right signatures
        makeLabelInfoMethod = env->GetMethodID(thisClass, "labelInfoForFont",
                                               "(Ljava/lang/String;F)Lcom/mousebird/maply/LabelInfo;");
        calculateTextWidthMethod = env->GetMethodID(thisClass, "calculateTextWidth",
                "(Ljava/lang/String;Lcom/mousebird/maply/LabelInfo;)D");
//        makeCircleTextureMethod = env->GetMethodID(thisClass, "makeLabelInfo",
//                                                   "");
//        makeLineTextureMethod = env->GetMethodID(thisClass, "makeLabelInfo",
//                                                 "");
    }
}

void MapboxVectorStyleSetImpl_Android::cleanup(JNIEnv *env)
{
    for (auto labelInfo : labelInfos)
        env->DeleteGlobalRef(labelInfo.second->labelInfoObj);
    labelInfos.clear();
}

MapboxVectorStyleSetImpl_Android::~MapboxVectorStyleSetImpl_Android()
{
}

SimpleIdentity MapboxVectorStyleSetImpl_Android::makeCircleTexture(PlatformThreadInfo *inInst,
        double radius,
        const RGBAColor &fillColor,
        const RGBAColor &strokeColor,
        float strokeWidth,
        Point2f *circleSize)
{
    // TODO: Implement
    return EmptyIdentity;
}

SimpleIdentity MapboxVectorStyleSetImpl_Android::makeLineTexture(PlatformThreadInfo *inInst,const std::vector<double> &dashComponents)
{
    // TODO: Implement
    return EmptyIdentity;
}

LabelInfoRef MapboxVectorStyleSetImpl_Android::makeLabelInfo(PlatformThreadInfo *inInst,const std::string &fontName,float fontSize)
{
    PlatformInfo_Android *inst = (PlatformInfo_Android *)inInst;
    std::pair<std::string, float> entry(fontName,fontSize);

    LabelInfoAndroidRef refLabelInfo = NULL;
    auto it = labelInfos.find(entry);
    if (it != labelInfos.end())
        refLabelInfo = it->second;
    else {
        jstring jFontNameStr = inst->env->NewStringUTF(fontName.c_str());
        jobject labelInfoGlobeObj = inst->env->NewGlobalRef(inst->env->CallObjectMethod(thisObj,makeLabelInfoMethod,jFontNameStr,2.0*fontSize));
        inst->env->DeleteLocalRef(jFontNameStr);
        LabelInfoAndroidRef *newRef = LabelInfoClassInfo::getClassInfo()->getObject(inst->env,labelInfoGlobeObj);
        refLabelInfo = *newRef;
        refLabelInfo->labelInfoObj = labelInfoGlobeObj;
        refLabelInfo->programID = screenMarkerProgramID;
        labelInfos[entry] = refLabelInfo;
    }

    return refLabelInfo;
}

SingleLabelRef MapboxVectorStyleSetImpl_Android::makeSingleLabel(PlatformThreadInfo *inInst,const std::string &text)
{
    SingleLabelAndroid *label = new SingleLabelAndroid();

    // Break up the lines and turn the characters into code points for Java
    std::istringstream ss(text);
    std::string line;
    while (std::getline(ss, line, ss.widen('\n'))) {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
        std::u16string utf16 = utf16conv.from_bytes(line);

        std::vector<int> codePoints;
        for (auto it = utf16.begin(); it != utf16.end(); ++it)
            codePoints.push_back(*it);
        label->codePointsLines.push_back(codePoints);
    }

    return SingleLabelRef(label);
}

ComponentObjectRef MapboxVectorStyleSetImpl_Android::makeComponentObject(PlatformThreadInfo *inInst)
{
    return ComponentObjectRef(new ComponentObject());
}

double MapboxVectorStyleSetImpl_Android::calculateTextWidth(PlatformThreadInfo *inInst,LabelInfoRef inLabelInfo,const std::string &text)
{
    PlatformInfo_Android *inst = (PlatformInfo_Android *)inInst;
    LabelInfoAndroidRef labelInfo = std::dynamic_pointer_cast<LabelInfoAndroid>(inLabelInfo);

    jstring jText = inst->env->NewStringUTF(text.c_str());
    jdouble width = inst->env->CallDoubleMethod(thisObj,calculateTextWidthMethod,jText,labelInfo->labelInfoObj);
    inst->env->DeleteLocalRef(jText);

    return width;
}

void MapboxVectorStyleSetImpl_Android::addSelectionObject(SimpleIdentity selectID,VectorObjectRef vecObj,ComponentObjectRef compObj)
{
    if (!compManage)
        return;

    // TODO: Fill this in
    // We need to associate the given VectorObjectRef with the given selectID
    // There are created on the C++ side by the mapbox style parser and so have no representation on the Java side.
}

}
