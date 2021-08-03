/*  Maply_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2021 mousebird consulting
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

using namespace Eigen;
using namespace WhirlyKit;

JavaObjectArrayHelper::JavaObjectArrayHelper(JNIEnv *env,jobjectArray objArray) :
    env(env),
    objArray(objArray),
    nextIndex(0),
    curObj(nullptr),
    count((env && objArray) ? env->GetArrayLength(objArray) : 0)
{
}

JavaObjectArrayHelper::~JavaObjectArrayHelper()
{
    if (curObj)
    {
        env->DeleteLocalRef(curObj);
    }
}

jobject JavaObjectArrayHelper::getNextObject()
{
    if (curObj)
    {
        env->DeleteLocalRef(curObj);
        curObj = nullptr;
    }
    if (nextIndex >= count)
    {
        return nullptr;
    }
    return curObj = env->GetObjectArrayElement(objArray,nextIndex++);
}

// Have to instantiate the static members somewhere
// But just some of the general ones.  The rest are in their own modules.

JavaDoubleClassInfo *JavaDoubleClassInfo::classInfoObj = nullptr;
JavaIntegerClassInfo *JavaIntegerClassInfo::classInfoObj = nullptr;
JavaLongClassInfo *JavaLongClassInfo::classInfoObj = nullptr;
JavaHashMapInfo *JavaHashMapInfo::classInfoObj = nullptr;
JavaListInfo *JavaListInfo::classInfoObj = nullptr;

template <typename T>
static inline void copy(std::vector<T> &vec, const T *elements, int len)
{
    vec.reserve(len);
    vec.assign(&elements[0], &elements[len]);
}

void ConvertIntArray(JNIEnv *env,jintArray &intArray,std::vector<int> &intVec)
{
    if (const int len = env->GetArrayLength(intArray))
	if (int *ints = env->GetIntArrayElements(intArray, nullptr))
	{
        copy(intVec, ints, len);
        env->ReleaseIntArrayElements(intArray, ints, JNI_ABORT);
    }
}

void ConvertLongLongArray(JNIEnv *env,jlongArray &longArray,std::vector<WhirlyKit::SimpleIdentity> &longVec)
{
    if (const int len = env->GetArrayLength(longArray))
    if (jlong *longs = env->GetLongArrayElements(longArray, nullptr))
    {
        copy(longVec, (WhirlyKit::SimpleIdentity*)longs, len);
        env->ReleaseLongArrayElements(longArray, longs, JNI_ABORT);
    }
}

void ConvertFloatArray(JNIEnv *env,jfloatArray &floatArray,std::vector<float> &floatVec)
{
    if (const int len = env->GetArrayLength(floatArray))
    if (float *floats = env->GetFloatArrayElements(floatArray, nullptr))
    {
        copy(floatVec, floats, len);
        env->ReleaseFloatArrayElements(floatArray, floats, JNI_ABORT);
    }
}

void ConvertDoubleArray(JNIEnv *env,jdoubleArray &doubleArray,std::vector<double> &doubleVec)
{
    if (const int len = env->GetArrayLength(doubleArray))
    if (double *doubles = env->GetDoubleArrayElements(doubleArray, nullptr))
    {
        copy(doubleVec, doubles, len);
        env->ReleaseDoubleArrayElements(doubleArray, doubles, JNI_ABORT);
    }
}

void ConvertBoolArray(JNIEnv *env,jbooleanArray &boolArray,std::vector<bool> &boolVec)
{
    const int len = env->GetArrayLength(boolArray);
    if (jboolean *bools = env->GetBooleanArrayElements(boolArray, nullptr))
    {
        copy(boolVec, (bool*)bools, len);
        env->ReleaseBooleanArrayElements(boolArray, bools, JNI_ABORT);
    }
}

void ConvertStringArray(JNIEnv *env,jobjectArray &objArray,std::vector<std::string> &strVec)
{
    const jsize count = env->GetArrayLength(objArray);
    strVec.reserve(strVec.size() + count);
    for (unsigned int ii=0;ii<count;ii++)
    {
        const auto string = (jstring)env->GetObjectArrayElement(objArray,ii);
        strVec.emplace_back(string ? env->GetStringUTFChars(string, nullptr) : std::string());
    }
}

void ConvertFloat2fArray(JNIEnv *env,jfloatArray &floatArray,Point2fVector &ptVec)
{
    if (const int len = env->GetArrayLength(floatArray) / 2)
    if (float *floats = env->GetFloatArrayElements(floatArray, nullptr))
    {
        ptVec.resize(len);
        for (int ii = 0; ii < len; ii++) {
            ptVec[ii] = Eigen::Vector2f(floats[2 * ii],
                                        floats[2 * ii + 1]);
        }
        env->ReleaseFloatArrayElements(floatArray, floats, JNI_ABORT);
    }
}

void ConvertFloat3fArray(JNIEnv *env,jfloatArray &floatArray,Point3fVector &ptVec)
{
    if (const int len = env->GetArrayLength(floatArray) / 3)
    if (float *floats = env->GetFloatArrayElements(floatArray, nullptr))
    {
        ptVec.resize(len);
        for (int ii = 0; ii < len; ii++)
        {
            ptVec[ii] = Eigen::Vector3f(floats[3 * ii],
                                        floats[3 * ii + 1],
                                        floats[3 * ii + 2]);
        }
        env->ReleaseFloatArrayElements(floatArray, floats, JNI_ABORT);
    }
}

void ConvertFloat3dArray(JNIEnv *env,jdoubleArray &doubleArray,Point3dVector &ptVec)
{
    if (const int len = env->GetArrayLength(doubleArray) / 3)
    if (double *doubles = env->GetDoubleArrayElements(doubleArray, nullptr))
    {
        ptVec.resize(len);
        for (int ii = 0; ii < len; ii++)
        {
            ptVec[ii] = Eigen::Vector3d(doubles[3 * ii],
                                        doubles[3 * ii + 1],
                                        doubles[3 * ii + 2]);
        }
        env->ReleaseDoubleArrayElements(doubleArray, doubles, JNI_ABORT);
    }
}

void ConvertFloat4fArray(JNIEnv *env,jfloatArray &floatArray,Vector4fVector &ptVec)
{
    if (const int len = env->GetArrayLength(floatArray) / 4)
    if (float *floats = env->GetFloatArrayElements(floatArray, nullptr))
    {
        ptVec.resize(len);
        for (int ii = 0; ii < len; ii++)
        {
            ptVec[ii] = Eigen::Vector4f(floats[4 * ii],
                                        floats[4 * ii + 1],
                                        floats[4 * ii + 2],
                                        floats[4 * ii + 3]);
        }
        env->ReleaseFloatArrayElements(floatArray, floats, JNI_ABORT);
    }
}

void ConvertLongArrayToSet(JNIEnv *env,const jlongArray &idArrayObj,std::set<WhirlyKit::SimpleIdentity> &intSet)
{
    if (const int idCount = env->GetArrayLength(idArrayObj))
    if (jlong *ids = env->GetLongArrayElements(idArrayObj, nullptr))
    {
        intSet.insert(&ids[0], &ids[idCount]);
        env->ReleaseLongArrayElements(idArrayObj, ids, JNI_ABORT);
    }
}

std::set<WhirlyKit::SimpleIdentity> ConvertLongArrayToSet(JNIEnv *env,const jlongArray &idArrayObj)
{
    std::set<WhirlyKit::SimpleIdentity> idSet;
    ConvertLongArrayToSet(env, idArrayObj, idSet);
    return idSet;
}

void ConvertLongArrayToSet(JNIEnv *env,const jlongArray &idArrayObj,std::unordered_set<WhirlyKit::SimpleIdentity> &intSet)
{
    if (const int idCount = env->GetArrayLength(idArrayObj))
    if (jlong *ids = env->GetLongArrayElements(idArrayObj, nullptr))
    {
        intSet.reserve(intSet.size() + idCount);
        intSet.insert(&ids[0], &ids[idCount]);
        env->ReleaseLongArrayElements(idArrayObj, ids, JNI_ABORT);
    }
}

JavaString::JavaString(JNIEnv *env,jstring str) : str(str), env(env)
{
    cStr = str ? env->GetStringUTFChars(str,nullptr) : nullptr;
}

JavaString::JavaString(JavaString &&other) noexcept :
    env(other.env),
    str(other.str),
    cStr(other.cStr)
{
    other.env = nullptr;
    other.cStr = nullptr;
    other.str = nullptr;
}

JavaString::~JavaString()
{
    if (cStr)
    {
        env->ReleaseStringUTFChars(str, cStr);
        cStr = nullptr;
    }
}

JavaBooleanArray::JavaBooleanArray(JNIEnv *env,jbooleanArray &array,bool save)
    : array(array), env(env), save(save)
{
    rawBool = env->GetBooleanArrayElements(array, nullptr);
    len = env->GetArrayLength(array);
}

JavaBooleanArray::~JavaBooleanArray()
{
    env->ReleaseBooleanArrayElements(array,rawBool, save ? 0 : JNI_ABORT);
}

JavaIntArray::JavaIntArray(JNIEnv *env,jintArray &array,bool save)
    : array(array), env(env), save(save)
{
    rawInt = env->GetIntArrayElements(array, nullptr);
    len = env->GetArrayLength(array);
}

JavaIntArray::~JavaIntArray()
{
    env->ReleaseIntArrayElements(array,rawInt, save ? 0 : JNI_ABORT);
}

JavaLongArray::JavaLongArray(JNIEnv *env,jlongArray &array,bool save)
    : array(array), env(env), save(save)
{
    rawLong = env->GetLongArrayElements(array, nullptr);
    len = env->GetArrayLength(array);
}

JavaLongArray::~JavaLongArray()
{
    env->ReleaseLongArrayElements(array,rawLong, save ? 0 : JNI_ABORT);
}

JavaFloatArray::JavaFloatArray(JNIEnv *env,jfloatArray &array,bool save)
    : array(array), env(env), save(save)
{
    rawFloat = env->GetFloatArrayElements(array, nullptr);
    len = env->GetArrayLength(array);
}

JavaFloatArray::~JavaFloatArray()
{
    env->ReleaseFloatArrayElements(array,rawFloat, save ? 0 : JNI_ABORT);
}

JavaDoubleArray::JavaDoubleArray(JNIEnv *env,jdoubleArray &array,bool save)
    : array(array), env(env), save(save)
{
    rawDouble = env->GetDoubleArrayElements(array, nullptr);
    len = env->GetArrayLength(array);
}

JavaDoubleArray::~JavaDoubleArray()
{
    env->ReleaseDoubleArrayElements(array,rawDouble, save ? 0 : JNI_ABORT);
}

jlongArray BuildLongArray(JNIEnv *env,const std::vector<SimpleIdentity> &longVec)
{
    if (longVec.empty())
        return nullptr;

    if (jlongArray newArray = env->NewLongArray(longVec.size()))
    {
        env->SetLongArrayRegion(newArray, 0, longVec.size(), (jlong *)&longVec[0]);
        return newArray;
    }
    return nullptr;
}

jdoubleArray BuildDoubleArray(JNIEnv *env,const std::vector<double> &doubleVec)
{
    if (doubleVec.empty())
        return nullptr;

    if (jdoubleArray newArray = env->NewDoubleArray(doubleVec.size()))
    {
        env->SetDoubleArrayRegion(newArray, 0, doubleVec.size(), (jdouble *)&doubleVec[0]);
        return newArray;
    }
    return nullptr;
}

jintArray BuildIntArray(JNIEnv *env,const std::vector<int> &intVec)
{
    if (intVec.empty())
        return nullptr;

    if (jintArray newArray = env->NewIntArray(intVec.size()))
    {
        env->SetIntArrayRegion(newArray, 0, intVec.size(), (jint *)&intVec[0]);
        return newArray;
    }
    return nullptr;
}

jobjectArray BuildObjectArray(JNIEnv *env,jclass cls,jobject singleObj)
{
    if (auto newArray = env->NewObjectArray(1,cls,nullptr)) {
        env->SetObjectArrayElement(newArray,0,singleObj);
        return newArray;
    }
    return nullptr;
}

jobjectArray BuildObjectArray(JNIEnv *env,jclass cls,const std::vector<jobject> &objVec)
{
    if (objVec.empty())
        return nullptr;

    if (jobjectArray newArray = env->NewObjectArray(objVec.size(),cls,nullptr))
    {
        for (unsigned int ii=0;ii<objVec.size();ii++)
            env->SetObjectArrayElement(newArray,ii,objVec[ii]);
        return newArray;
    }
    return nullptr;
}

jobjectArray BuildStringArray(JNIEnv *env,const std::vector<std::string> &objVec)
{
    if (objVec.empty())
        return nullptr;

    if (jobjectArray newArray = env->NewObjectArray(objVec.size(),env->FindClass("java/lang/String"),nullptr))
    {
        for (unsigned int ii=0;ii<objVec.size();ii++)
            env->SetObjectArrayElement(newArray,ii,env->NewStringUTF(objVec[ii].c_str()));
        return newArray;
    }
    return nullptr;
}

