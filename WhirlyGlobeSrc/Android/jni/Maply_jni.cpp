/*
 *  Maply_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2014 mousebird consulting
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

#import <jni.h>
#import "Maply_jni.h"

// Have to instantiate the static members somewhere

JavaDoubleClassInfo *JavaDoubleClassInfo::classInfoObj = NULL;
JavaIntegerClassInfo *JavaIntegerClassInfo::classInfoObj = NULL;
JavaHashMapInfo *JavaHashMapInfo::classInfoObj = NULL;
JavaListInfo *JavaListInfo::classInfoObj = NULL;
template<> AttrDictClassInfo *AttrDictClassInfo::classInfoObj = NULL;
template<> ChangeSetClassInfo *ChangeSetClassInfo::classInfoObj = NULL;
template<> TextureClassInfo *TextureClassInfo::classInfoObj = NULL;
template<> CoordSystemClassInfo *CoordSystemClassInfo::classInfoObj = NULL;
template<> Point2dClassInfo *Point2dClassInfo::classInfoObj = NULL;
template<> Point3dClassInfo *Point3dClassInfo::classInfoObj = NULL;
template<> Matrix4dClassInfo *Matrix4dClassInfo::classInfoObj = NULL;
template<> CoordSystemDisplayAdapterInfo *CoordSystemDisplayAdapterInfo::classInfoObj = NULL;
template<> MaplySceneRendererInfo *MaplySceneRendererInfo::classInfoObj = NULL;
template<> MapSceneClassInfo *MapSceneClassInfo::classInfoObj = NULL;
template<> MapViewClassInfo *MapViewClassInfo::classInfoObj = NULL;
template<> SceneClassInfo *SceneClassInfo::classInfoObj = NULL;
template<> ViewClassInfo *ViewClassInfo::classInfoObj = NULL;
template<> VectorInfoClassInfo *VectorInfoClassInfo::classInfoObj = NULL;
template<> VectorObjectClassInfo *VectorObjectClassInfo::classInfoObj = NULL;
template<> MarkerInfoClassInfo *MarkerInfoClassInfo::classInfoObj = NULL;
template<> ViewStateClassInfo *ViewStateClassInfo::classInfoObj = NULL;
