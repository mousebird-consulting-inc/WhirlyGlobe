/*
 *  VectorManager_jni.h
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

typedef JavaClassInfo<WhirlyKit::MutableDictionary_AndroidRef> AttrDictClassInfo;
typedef JavaClassInfo<WhirlyKit::DictionaryEntry_AndroidRef> AttrDictEntryClassInfo;
typedef JavaClassInfo<WhirlyKit::VectorInfoRef> VectorInfoClassInfo;
typedef JavaClassInfo<WhirlyKit::WideVectorInfoRef> WideVectorInfoClassInfo;
typedef JavaClassInfo<WhirlyKit::VectorObjectRef> VectorObjectClassInfo;
typedef JavaClassInfo<WhirlyKit::LoftedPolyInfoRef> LoftedPolyInfoClassInfo;
//typedef JavaClassInfo<WhirlyKit::MapboxVectorTileParser> MapboxVectorTileParserClassInfo;

// Construct a Java-side AttrDictionary wrapper
JNIEXPORT jobject JNICALL MakeAttrDictionary(JNIEnv *env,WhirlyKit::MutableDictionary_AndroidRef dict);
JNIEXPORT jobject JNICALL MakeAttrDictionaryEntry(JNIEnv *env,WhirlyKit::DictionaryEntry_AndroidRef dict);

// Construct a Java-side Vector Object
JNIEXPORT jobject JNICALL MakeVectorObject(JNIEnv *env,WhirlyKit::VectorObject *vec);
JNIEXPORT jobject JNICALL MakeVectorObject(JNIEnv *env,WhirlyKit::VectorObjectRef vec);

// This one takes the classInfo object which skips a lookup
JNIEXPORT jobject JNICALL MakeVectorObjectWrapper(JNIEnv *env,VectorObjectClassInfo *classInfo,WhirlyKit::VectorObjectRef vecObj);