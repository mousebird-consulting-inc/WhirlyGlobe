/*  ColorExpressionInfo_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Tim Sylvester on 03/22/2022
 *  Copyright 2022-2022 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this
 *  file except in compliance with the License. You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software distributed under
 *  the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *  KIND, either express or implied. See the License for the specific language governing
 *  permissions and limitations under the License.
 */

#import <Expressions_jni.h>

using namespace WhirlyKit;
using namespace Eigen;

template<> ColorExpressionClassInfo *ColorExpressionClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ColorExpressionInfo_nativeInit(
  JNIEnv *env, jclass cls)
{
    ColorExpressionClassInfo::getClassInfo(env, cls);
}

jobject WhirlyKit::MakeWrapper(JNIEnv *env, ColorExpressionInfoRef exp)
{
    if (exp)
    {
        constexpr char const *className = "com/mousebird/maply/ColorExpressionInfo";
        if (const auto expInfo = ColorExpressionClassInfo::getClassInfo(env, className))
        {
            if (jobject newObj = expInfo->makeWrapperObject(env, nullptr))
            {
                expInfo->setHandle(env, newObj, new ColorExpressionInfoRef(std::move(exp)));
                return newObj;
            }
        }
    }
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_ColorExpressionInfo_createLinear(
  JNIEnv *env, jclass cls,
  jfloat minZoom, jint minColor,
  jfloat maxZoom, jint maxColor)
{
    try
    {
        if (jobject newJObj = MakeWrapper(env, std::make_shared<ColorExpressionInfo>()))
        {
            if (const auto &obj = *ColorExpressionClassInfo::get(env, newJObj))
            {
                obj->base = 1.0;
                obj->type = ExpressionInfoType::ExpressionLinear;
                obj->stopInputs = { minZoom, maxZoom };
                obj->stopOutputs = { RGBAColor::FromARGBInt(minColor),
                                     RGBAColor::FromARGBInt(maxColor) };
                return newJObj;
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ColorExpressionInfo_dispose(
  JNIEnv *env, jobject obj)
{
    try
    {
        const auto classInfo = ColorExpressionClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        delete classInfo->getObject(env,obj);
        classInfo->clearHandle(env,obj);
    }
    MAPLY_STD_JNI_CATCH()
}

