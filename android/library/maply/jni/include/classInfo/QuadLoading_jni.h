/*
 *  QuadLoading_jni.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/20/19.
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
#import "../../../WhirlyGlobeLib/include/QuadImageFrameLoader_Android.h"

typedef JavaClassInfo<WhirlyKit::SamplingParams> SamplingParamsClassInfo;
typedef JavaClassInfo<WhirlyKit::QuadLoaderReturnRef> LoaderReturnClassInfo;
typedef JavaClassInfo<WhirlyKit::ImageTile_AndroidRef> ImageTileClassInfo;
typedef JavaClassInfo<WhirlyKit::QuadImageFrameLoader_AndroidRef> QuadImageFrameLoaderClassInfo;
typedef JavaClassInfo<WhirlyKit::QuadSamplingController_Android> QuadSamplingControllerInfo;
typedef JavaClassInfo<WhirlyKit::QIFBatchOps_Android> QIFBatchOpsClassInfo;
typedef JavaClassInfo<WhirlyKit::QIFFrameAsset_Android> QIFFrameAssetClassInfo;

JNIEXPORT jobject JNICALL MakeImageTile(JNIEnv *env,WhirlyKit::ImageTile_AndroidRef imgTile);
JNIEXPORT jobject JNICALL MakeQIFBatchOps(JNIEnv *env,WhirlyKit::QIFBatchOps_Android *batchOps);
JNIEXPORT jobject JNICALL MakeQIFFrameAsset(JNIEnv *env,WhirlyKit::QIFFrameAsset_Android *frame);
