/*  Expressions_jni.h
 *  WhirlyGlobeLib
 *
 *  Created by Tim Sylvester on 03/22/2022.
 *  Copyright 2022-2022 mousebird consulting
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

#import "Maply_jni.h"
#import "WhirlyGlobe_Android.h"
#import "BaseInfo.h"

namespace WhirlyKit {

typedef JavaClassInfo<WhirlyKit::ColorExpressionInfoRef> ColorExpressionClassInfo;
typedef JavaClassInfo<WhirlyKit::FloatExpressionInfoRef> FloatExpressionClassInfo;

extern jobject MakeWrapper(JNIEnv *env, ColorExpressionInfoRef exp);
extern jobject MakeWrapper(JNIEnv *env, FloatExpressionInfoRef exp);

}