/*
 *  Maply_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2015 mousebird consulting
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
template<> Point4dClassInfo *Point4dClassInfo::classInfoObj = NULL;
template<> Matrix4dClassInfo *Matrix4dClassInfo::classInfoObj = NULL;
template<> QuaternionClassInfo *QuaternionClassInfo::classInfoObj = NULL;
template<> AngleAxisClassInfo *AngleAxisClassInfo::classInfoObj = NULL;
template<> CoordSystemDisplayAdapterInfo *CoordSystemDisplayAdapterInfo::classInfoObj = NULL;
template<> FakeGeocentricDisplayAdapterInfo *FakeGeocentricDisplayAdapterInfo::classInfoObj = NULL;
template<> GeneralDisplayAdapterInfo *GeneralDisplayAdapterInfo::classInfoObj = NULL;
template<> MaplySceneRendererInfo *MaplySceneRendererInfo::classInfoObj = NULL;
template<> MapSceneClassInfo *MapSceneClassInfo::classInfoObj = NULL;
template<> GlobeSceneClassInfo *GlobeSceneClassInfo::classInfoObj = NULL;
template<> MapViewClassInfo *MapViewClassInfo::classInfoObj = NULL;
template<> GlobeViewClassInfo *GlobeViewClassInfo::classInfoObj = NULL;
template<> SceneClassInfo *SceneClassInfo::classInfoObj = NULL;
template<> ViewClassInfo *ViewClassInfo::classInfoObj = NULL;
template<> BaseInfoClassInfo *BaseInfoClassInfo::classInfoObj = NULL;
template<> VectorInfoClassInfo *VectorInfoClassInfo::classInfoObj = NULL;
template<> VectorObjectClassInfo *VectorObjectClassInfo::classInfoObj = NULL;
template<> MarkerInfoClassInfo *MarkerInfoClassInfo::classInfoObj = NULL;
template<> SphericalChunkInfoClassInfo *SphericalChunkInfoClassInfo::classInfoObj = NULL;
template<> SphericalChunkClassInfo *SphericalChunkClassInfo::classInfoObj = NULL;
template<> ViewStateClassInfo *ViewStateClassInfo::classInfoObj = NULL;
template<> MapViewStateClassInfo *MapViewStateClassInfo::classInfoObj = NULL;
template<> GlobeViewStateClassInfo *GlobeViewStateClassInfo::classInfoObj = NULL;
template<> SingleVertexAttributeInfoClassInfo *SingleVertexAttributeInfoClassInfo::classInfoObj = NULL;
template<> ParticleBatchClassInfo *ParticleBatchClassInfo::classInfoObj = NULL;
template<> ParticleSystemClassInfo *ParticleSystemClassInfo::classInfoObj = NULL;
template<> ParticleSystemManagerClassInfo * ParticleSystemManagerClassInfo::classInfoObj = NULL;
template<> OpenGLES2ProgramClassInfo *OpenGLES2ProgramClassInfo::classInfoObj = NULL;
template<> QuadTrackerPointReturnClassInfo *QuadTrackerPointReturnClassInfo::classInfoObj = NULL;
template<> QuadTrackerClassInfo *QuadTrackerClassInfo::classInfoObj = NULL;
template<> ShapeInfoClassInfo *ShapeInfoClassInfo::classInfoObj = NULL;
template<> ShapeClassInfo *ShapeClassInfo::classInfoObj = NULL;
template<> ShapeSphereClassInfo *ShapeSphereClassInfo::classInfoObj = NULL;
template<> ShapeManagerClassInfo *ShapeManagerClassInfo::classInfoObj = NULL;
template<> SingleVertexAttributeClassInfo *SingleVertexAttributeClassInfo::classInfoObj = NULL;
template<> DirectionalLightClassInfo *DirectionalLightClassInfo::classInfoObj = NULL;
template<> MaterialClassInfo *MaterialClassInfo::classInfoObj = NULL;

void ConvertIntArray(JNIEnv *env,jintArray &intArray,std::vector<int> &intVec)
{
	int *ints = env->GetIntArrayElements(intArray, NULL);
	int len = env->GetArrayLength(intArray);
	intVec.resize(len);
	for (int ii=0;ii<len;ii++)
		intVec[ii] = ints[ii];
	env->ReleaseIntArrayElements(intArray,ints, 0);
}

JavaString::JavaString(JNIEnv *env,jstring &str)
: str(str), env(env)
{
    cStr = env->GetStringUTFChars(str,0);
}

JavaString::~JavaString()
{
    env->ReleaseStringUTFChars(str, cStr);
}

JavaBooleanArray::JavaBooleanArray(JNIEnv *env,jbooleanArray &array)
: array(array), env(env)
{
    rawBool = env->GetBooleanArrayElements(array, NULL);
    len = env->GetArrayLength(array);
}

JavaBooleanArray::~JavaBooleanArray()
{
    env->ReleaseBooleanArrayElements(array,rawBool, 0);
}

JavaIntArray::JavaIntArray(JNIEnv *env,jintArray &array)
: array(array), env(env)
{
    rawInt = env->GetIntArrayElements(array, NULL);
    len = env->GetArrayLength(array);
}

JavaIntArray::~JavaIntArray()
{
    env->ReleaseIntArrayElements(array,rawInt, 0);
}

JavaLongArray::JavaLongArray(JNIEnv *env,jlongArray &array)
: array(array), env(env)
{
    rawLong = env->GetLongArrayElements(array, NULL);
    len = env->GetArrayLength(array);
}

JavaLongArray::~JavaLongArray()
{
    env->ReleaseLongArrayElements(array,rawLong, 0);
}

JavaFloatArray::JavaFloatArray(JNIEnv *env,jfloatArray &array)
: array(array), env(env)
{
    rawFloat = env->GetFloatArrayElements(array, NULL);
    len = env->GetArrayLength(array);
}

JavaFloatArray::~JavaFloatArray()
{
    env->ReleaseFloatArrayElements(array,rawFloat, 0);
}

JavaDoubleArray::JavaDoubleArray(JNIEnv *env,jdoubleArray &array)
: array(array), env(env)
{
    rawDouble = env->GetDoubleArrayElements(array, NULL);
    len = env->GetArrayLength(array);
}

JavaDoubleArray::~JavaDoubleArray()
{
    env->ReleaseDoubleArrayElements(array,rawDouble, 0);
}
