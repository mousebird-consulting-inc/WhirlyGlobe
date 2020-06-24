/*
 *  Scene_jni.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/7/19.
 *  Copyright 2011-2019 mousebird consulting
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

#import "Maply_jni.h"
#import "WhirlyGlobe_Android.h"

typedef JavaClassInfo<WhirlyKit::ChangeSetRef> ChangeSetClassInfo;
typedef JavaClassInfo<WhirlyKit::DirectionalLight> DirectionalLightClassInfo;
typedef JavaClassInfo<WhirlyKit::Scene> SceneClassInfo;
typedef JavaClassInfo<WhirlyKit::Texture> TextureClassInfo;
typedef JavaClassInfo<WhirlyKit::Material> MaterialClassInfo;
typedef JavaClassInfo<WhirlyKit::Shader_AndroidRef> ShaderClassInfo;

// Construct a Java-side Shader object around an existing Shader_Android.
jobject MakeShader(JNIEnv *env,WhirlyKit::Shader_AndroidRef shader);

// Construct a Java-side ChangeSet object
jobject MakeChangeSet(JNIEnv *env,const WhirlyKit::ChangeSet &changeSet);
