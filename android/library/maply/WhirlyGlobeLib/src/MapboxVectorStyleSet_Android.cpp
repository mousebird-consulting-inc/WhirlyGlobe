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
#import "Formats_jni.h"
#import "LabelsAndMarkers_jni.h"

namespace WhirlyKit
{

MapboxVectorStyleSetImpl_Android::MapboxVectorStyleSetImpl_Android(Scene *scene,VectorStyleSettingsImplRef settings)
: MapboxVectorStyleSetImpl(scene,settings), env(NULL), thisObj(NULL),
    makeLabelInfoMethod(NULL), makeCircleTextureMethod(NULL), makeLineTextureMethod(NULL)
{
}

void MapboxVectorStyleSetImpl_Android::setEnv(JNIEnv *inEnv)
{
    env = inEnv;

    // TODO: Turn global reference into local reference for this object

    if (!makeLabelInfoMethod) {
        jclass thisClass = MapboxVectorStyleSetClassInfo::getClassInfo()->getClass();
        // TODO: Get the right signatures
        makeLabelInfoMethod = env->GetMethodID(thisClass, "makeLabelInfo",
                                               "(Ljava/lang/String;F)Lcom/mousebird/maply/LabelInfo;");
        makeCircleTextureMethod = env->GetMethodID(thisClass, "makeLabelInfo",
                                                   "");
        makeLineTextureMethod = env->GetMethodID(thisClass, "makeLabelInfo",
                                                 "");
    }
}

MapboxVectorStyleSetImpl_Android::~MapboxVectorStyleSetImpl_Android()
{
}

SimpleIdentity MapboxVectorStyleSetImpl_Android::makeCircleTexture(double radius,const RGBAColor &fillColor,const RGBAColor &strokeColor,float strokeWidth,Point2f *circleSize)
{
    // TODO: Implement
    return EmptyIdentity;
}

SimpleIdentity MapboxVectorStyleSetImpl_Android::makeLineTexture(const std::vector<double> &dashComponents)
{
    // TODO: Implement
    return EmptyIdentity;
}

LabelInfoRef MapboxVectorStyleSetImpl_Android::makeLabelInfo(const std::string &fontName,float fontSize)
{
    std::pair<std::string, float> entry(fontName,fontSize);

    LabelInfoAndroid *refLabelInfo = NULL;
    auto it = labelInfos.find(entry);
    if (it != labelInfos.end())
        refLabelInfo = it->second;
    else {
        jstring jFontNameStr = env->NewStringUTF(fontName.c_str());
        jobject labelInfoObj = env->CallObjectMethod(thisObj,makeLabelInfoMethod,jFontNameStr,fontSize);
        env->DeleteLocalRef(jFontNameStr);
        refLabelInfo = LabelInfoClassInfo::getClassInfo()->getObject(env,labelInfoObj);
        labelInfos[entry] = refLabelInfo;
    }

    return LabelInfoRef(new LabelInfoAndroid(*refLabelInfo));
}

SingleLabelRef MapboxVectorStyleSetImpl_Android::makeSingleLabel(const std::string &text)
{
    SingleLabelAndroid *label = new SingleLabelAndroid();

    // Break up the lines and turn the characters into code points for Java
    std::stringstream ss(text);
    std::string line;
    while (std::getline(ss,line)) {
        std::u32string str32;
        std::copy(line.begin(),line.end(),str32.begin());
        std::vector<int> codePoints;
        std::copy(str32.begin(),str32.end(),codePoints.begin());

        label->codePointsLines.push_back(codePoints);
    }

    return SingleLabelRef(label);
}

ComponentObjectRef MapboxVectorStyleSetImpl_Android::makeComponentObject()
{
    return ComponentObjectRef(new ComponentObject());
}

}
