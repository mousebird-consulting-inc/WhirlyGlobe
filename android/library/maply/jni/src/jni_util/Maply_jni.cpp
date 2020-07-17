/*
 *  Maply_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2016 mousebird consulting
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

using namespace Eigen;
using namespace WhirlyKit;

JavaObjectArrayHelper::JavaObjectArrayHelper(JNIEnv *env,jobjectArray objArray)
: env(env), objArray(objArray), which(0), curObj(NULL)
{
    count = env->GetArrayLength(objArray);
}

JavaObjectArrayHelper::~JavaObjectArrayHelper()
{
    if (curObj)
    {
        env->DeleteLocalRef(curObj);
    }
}

int JavaObjectArrayHelper::numObjects()
{
    return count;
}

jobject JavaObjectArrayHelper::getNextObject()
{
    if (which >= count)
        return NULL;
    if (curObj)
        env->DeleteLocalRef(curObj);
    curObj = env->GetObjectArrayElement(objArray,which);
    which++;

    return curObj;
}

// Have to instantiate the static members somewhere
// But just some of the general ones.  The rest are in their own modules.

JavaDoubleClassInfo *JavaDoubleClassInfo::classInfoObj = NULL;
JavaIntegerClassInfo *JavaIntegerClassInfo::classInfoObj = NULL;
JavaLongClassInfo *JavaLongClassInfo::classInfoObj = NULL;
JavaHashMapInfo *JavaHashMapInfo::classInfoObj = NULL;
JavaListInfo *JavaListInfo::classInfoObj = NULL;

// Note: Move these out
/*
template<> MaplySceneRendererInfo *MaplySceneRendererInfo::classInfoObj = NULL;
template<> MapboxVectorTileParserClassInfo *MapboxVectorTileParserClassInfo::classInfoObj = NULL;
template<> GeoJSONSourceClassInfo *GeoJSONSourceClassInfo::classInfoObj = NULL;
*/

void ConvertIntArray(JNIEnv *env,jintArray &intArray,std::vector<int> &intVec)
{
	int *ints = env->GetIntArrayElements(intArray, NULL);
	int len = env->GetArrayLength(intArray);
	intVec.resize(len);
	for (int ii=0;ii<len;ii++)
		intVec[ii] = ints[ii];
	env->ReleaseIntArrayElements(intArray,ints,0);
}

void ConvertLongLongArray(JNIEnv *env,jlongArray &longArray,std::vector<WhirlyKit::SimpleIdentity> &longVec)
{
    jlong *longs = env->GetLongArrayElements(longArray, NULL);
    int len = env->GetArrayLength(longArray);
    longVec.resize(len);
    for (int ii=0;ii<len;ii++)
        longVec[ii] = longs[ii];
    env->ReleaseLongArrayElements(longArray,longs,0);
}

void ConvertFloatArray(JNIEnv *env,jfloatArray &floatArray,std::vector<float> &floatVec)
{
    float *floats = env->GetFloatArrayElements(floatArray, NULL);
    int len = env->GetArrayLength(floatArray);
    floatVec.resize(len);
    for (int ii=0;ii<len;ii++)
        floatVec[ii] = floats[ii];
    env->ReleaseFloatArrayElements(floatArray,floats,0);
}

void ConvertDoubleArray(JNIEnv *env,jdoubleArray &doubleArray,std::vector<double> &doubleVec)
{
    double *doubles = env->GetDoubleArrayElements(doubleArray, NULL);
    int len = env->GetArrayLength(doubleArray);
    doubleVec.resize(len);
    for (int ii=0;ii<len;ii++)
        doubleVec[ii] = doubles[ii];
    env->ReleaseDoubleArrayElements(doubleArray,doubles,0);
}

void ConvertFloat2fArray(JNIEnv *env,jfloatArray &floatArray,Point2fVector &ptVec)
{
    float *floats = env->GetFloatArrayElements(floatArray, NULL);
    int len = env->GetArrayLength(floatArray)/2;
    ptVec.resize(len);
    for (int ii=0;ii<len;ii++)
        ptVec[ii] = Eigen::Vector2f(floats[2*ii],floats[2*ii+1]);
    env->ReleaseFloatArrayElements(floatArray,floats,0);
}

void ConvertFloat3fArray(JNIEnv *env,jfloatArray &floatArray,Point3fVector &ptVec)
{
    float *floats = env->GetFloatArrayElements(floatArray, NULL);
    int len = env->GetArrayLength(floatArray)/3;
    ptVec.resize(len);
    for (int ii=0;ii<len;ii++)
        ptVec[ii] = Eigen::Vector3f(floats[3*ii],floats[3*ii+1],floats[3*ii+2]);
    env->ReleaseFloatArrayElements(floatArray,floats,0);
}

void ConvertFloat3dArray(JNIEnv *env,jdoubleArray &doubleArray,Point3dVector &ptVec)
{
    double *doubles = env->GetDoubleArrayElements(doubleArray, NULL);
    int len = env->GetArrayLength(doubleArray)/34;
    ptVec.resize(len);
    for (int ii=0;ii<len;ii++)
        ptVec[ii] = Eigen::Vector3d(doubles[3*ii],doubles[3*ii+1],doubles[3*ii+2]);
    env->ReleaseDoubleArrayElements(doubleArray,doubles,0);
}

void ConvertFloat4fArray(JNIEnv *env,jfloatArray &floatArray,Vector4fVector &ptVec)
{
    float *floats = env->GetFloatArrayElements(floatArray, NULL);
    int len = env->GetArrayLength(floatArray)/4;
    ptVec.resize(len);
    for (int ii=0;ii<len;ii++)
        ptVec[ii] = Eigen::Vector4f(floats[4*ii],floats[4*ii+1],floats[4*ii+2],floats[4*ii+3]);
    env->ReleaseFloatArrayElements(floatArray,floats,0);
}


void ConvertLongArrayToSet(JNIEnv *env,jlongArray &idArrayObj,std::set<WhirlyKit::SimpleIdentity> &intSet)
{
    int idCount = env->GetArrayLength(idArrayObj);
    jlong *ids = env->GetLongArrayElements(idArrayObj, NULL);
    if (idCount == 0)
        return;
    
    for (int ii=0;ii<idCount;ii++)
        intSet.insert((WhirlyKit::SimpleIdentity)ids[ii]);

    env->ReleaseLongArrayElements(idArrayObj,ids, 0);
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

jlongArray BuildLongArray(JNIEnv *env,std::vector<SimpleIdentity> &longVec)
{
    if (longVec.empty())
        return NULL;

    jlongArray newArray = env->NewLongArray(longVec.size());
    if (!newArray)
        return NULL;

    env->SetLongArrayRegion(newArray, 0, longVec.size(), (jlong *)&longVec[0]);
    return newArray;
}

jdoubleArray BuildDoubleArray(JNIEnv *env,std::vector<double> &doubleVec)
{
    if (doubleVec.empty())
        return NULL;

    jdoubleArray newArray = env->NewDoubleArray(doubleVec.size());
    if (!newArray)
        return NULL;

    env->SetDoubleArrayRegion(newArray, 0, doubleVec.size(), (jdouble *)&doubleVec[0]);
    return newArray;
}

jintArray BuildIntArray(JNIEnv *env,std::vector<int> &intVec)
{
    if (intVec.empty())
        return NULL;

    jintArray newArray = env->NewIntArray(intVec.size());
    if (!newArray)
        return NULL;

    env->SetIntArrayRegion(newArray, 0, intVec.size(), (jint *)&intVec[0]);
    return newArray;
}

jobjectArray BuildObjectArray(JNIEnv *env,jclass cls,std::vector<jobject> &objVec)
{
    if (objVec.empty())
        return NULL;

    jobjectArray newArray = env->NewObjectArray(objVec.size(),cls,NULL);
    if (!newArray)
        return NULL;

    for (unsigned int ii=0;ii<objVec.size();ii++)
        env->SetObjectArrayElement(newArray,ii,objVec[ii]);
    return newArray;
}

jobjectArray BuildStringArray(JNIEnv *env,std::vector<std::string> &objVec)
{
    if (objVec.empty())
        return NULL;

    jobjectArray newArray = env->NewObjectArray(objVec.size(),env->FindClass("java/lang/String"),NULL);
    if (!newArray)
        return NULL;

    for (unsigned int ii=0;ii<objVec.size();ii++)
        env->SetObjectArrayElement(newArray,ii,env->NewStringUTF(objVec[ii].c_str()));
    return newArray;
}
