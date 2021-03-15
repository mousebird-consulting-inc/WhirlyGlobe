/*  Maply_jni.h
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
 */

#ifndef Maply_JNI_h_
#define Maply_JNI_h_

#include <stdlib.h>
#import <vector>
#import <android/log.h>
#import <jni.h>
#import "WhirlyGlobe.h"

/* Java Class Info
 * This tracks JNI info about classes we implement.
 * We use this rather than hitting the JNI class and method routines
 * all the time.
 * Each object has one of these in its own file.
 */
template<typename T> class JavaClassInfo
{
public:
	// Construct with environment and class
	JavaClassInfo(JNIEnv *env,jclass inClass) :
		theClass((jclass)env->NewGlobalRef(inClass)),
		nativeHandleField(nullptr)
	{
		initMethodID = env->GetMethodID(theClass, "<init>", "()V");
	}
	virtual ~JavaClassInfo() {
	    if (theClass) {
	        wkLogLevel(Warn, "JavaClassInfo not cleaned up");
	    }
	}

	// Clear references to JNI data
	virtual void clear(JNIEnv *env)
	{
		env->DeleteGlobalRef(theClass);
		theClass = nullptr;
	}

	// Make an object of the type and point it to the C++ object given
	virtual jobject makeWrapperObject(JNIEnv *env,T *cObj)
	{
		jobject obj = env->NewObject(theClass,initMethodID);
		T *oldRef = getObject(env,obj);
		setHandle(env, obj, cObj);
		if (oldRef && cObj)
			delete oldRef;
		return obj;
	}

	// Make a wrapper object, but don't set the handle
	virtual jobject makeWrapperObject(JNIEnv *env) {
		jobject obj = env->NewObject(theClass,initMethodID);
		return obj;
	}

	// Return the field ID for the handle we use
	jfieldID getHandleField(JNIEnv *env)
	{
		if (!nativeHandleField)
		{
			nativeHandleField = env->GetFieldID(theClass, "nativeHandle", "J");
		}
		return nativeHandleField;
	}

	// Return the C++ object, given a Java object
	T *getObject(JNIEnv *env,jobject obj)
	{
		if (!obj)
		{
			// wkLog doesn't work from here? why?
			__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Null object handle in getHandle() for '%s'", typeid(T).name());
			return nullptr;
		}
	    jlong handle = env->GetLongField(obj, getHandleField(env));
	    return reinterpret_cast<T *>(handle);
	}

	static T* get(JNIEnv *env,jobject obj)
	{
		return getClassInfo()->getObject(env,obj);
	}

	// Set the handle for a Java wrapper to its C++ object
	void setHandle(JNIEnv *env, jobject obj, T *t)
	{
		if (!t)
		{
//			__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Null handle in setHandle()");
			return;
		}
	    jlong handle = reinterpret_cast<jlong>(t);
	    env->SetLongField(obj, getHandleField(env), handle);
	}

	// Clear the handle out of a Java wrapper object
	void clearHandle(JNIEnv *env, jobject obj)
	{
	    env->SetLongField(obj, getHandleField(env), 0);
	}

	// Just one class info object for the whole class
	static JavaClassInfo *classInfoObj;

	// Return the class info without getting clever
	static JavaClassInfo<T> *getClassInfo()
	{
		return classInfoObj;
	}

	// Return the class info, just once, we know the name
	static JavaClassInfo<T> *getClassInfo(JNIEnv *env,const char *className)
	{
		if (!classInfoObj)
		{
			jclass cls = env->FindClass(className);
			classInfoObj = new JavaClassInfo(env,cls);
			env->DeleteLocalRef(cls);
		}

		return classInfoObj;
	}

	// Return the class info, just once, but we know the class itself
	static JavaClassInfo<T> *getClassInfo(JNIEnv *env,jclass cls)
	{
		if (!classInfoObj)
			classInfoObj = new JavaClassInfo(env,cls);

		return classInfoObj;
	}
    
    // Return the Java class
    jclass getClass() const { return theClass; }

protected:
	JavaClassInfo() = default;

	jclass theClass;
	jfieldID nativeHandleField;
	jmethodID initMethodID;
};

// Wrapper for creating Java Integer objects
class JavaIntegerClassInfo
{
private:
	JavaIntegerClassInfo(JNIEnv *env)
	{
		jclass intLocalClass = env->FindClass("java/lang/Integer");
		integerClass = (jclass)env->NewGlobalRef(intLocalClass);
	    integerClassInitID = env->GetMethodID(integerClass, "<init>", "(I)V");
	    integerGetID = env->GetMethodID(integerClass,"intValue","()I");
        env->DeleteLocalRef(intLocalClass);
	}

    ~JavaIntegerClassInfo()
    {
        // We don't expect this because this should only be used by static instances
        wkLogLevel(Warn, "Global ref not cleaned up");
    }

public:
	// Make an Integer with the given value
	jobject makeInteger(JNIEnv *env,int iVal) { return env->NewObject(integerClass, integerClassInitID, iVal); }
	static jobject make(JNIEnv *env,int iVal) { return getClassInfo(env)->makeInteger(env,iVal); }

	// Return the value of an Integer object
	int getInteger(JNIEnv *env,jobject intObj) { return env->CallIntMethod(intObj,integerGetID); }
	static int get(JNIEnv *env,jobject intObj) { return getClassInfo(env)->getInteger(env,intObj); }

	static JavaIntegerClassInfo *getClassInfo(JNIEnv *env)
	{
		if (!classInfoObj)
			classInfoObj = new JavaIntegerClassInfo(env);
		return classInfoObj;
	}

protected:
	jclass integerClass;
	jmethodID integerClassInitID,integerGetID;

	static JavaIntegerClassInfo *classInfoObj;
};

// Wrapper for creating Java Long objects
class JavaLongClassInfo
{
private:
	JavaLongClassInfo(JNIEnv *env)
	{
		jclass longLocalClass = env->FindClass("java/lang/Long");
		longClass = (jclass)env->NewGlobalRef(longLocalClass);
	    longClassInitID = env->GetMethodID(longClass, "<init>", "(L)V");
	    longGetID = env->GetMethodID(longClass,"longValue","()L");
        env->DeleteLocalRef(longLocalClass);
	}

    ~JavaLongClassInfo()
    {
        // We don't expect this because this should only be used by static instances
        wkLogLevel(Warn, "Global ref not cleaned up");
    }

public:

	// Make an Integer with the given value
	jobject makeLong(JNIEnv *env,long long lVal) { return env->NewObject(longClass, longClassInitID, lVal); }
	static jobject make(JNIEnv *env,long long lVal) { return getClassInfo(env)->makeLong(env,lVal); }

	// Return the value of an Integer object
	int getLong(JNIEnv *env,jobject longObj) { return env->CallLongMethod(longObj,longGetID); }
	static int get(JNIEnv *env,jobject longObj) { return getClassInfo(env)->getLong(env,longObj); }

	static JavaLongClassInfo *getClassInfo(JNIEnv *env)
	{
		if (!classInfoObj)
			classInfoObj = new JavaLongClassInfo(env);
		return classInfoObj;
	}

protected:
	jclass longClass;
	jmethodID longClassInitID,longGetID;

	static JavaLongClassInfo *classInfoObj;
};

// Wrapper for creating Java Double objects
class JavaDoubleClassInfo
{
private:
	JavaDoubleClassInfo(JNIEnv *env)
	{
	    jclass doubleLocalClass = env->FindClass("java/lang/Double");
	    doubleClass = (jclass)env->NewGlobalRef(doubleLocalClass);
	    doubleClassInitID = env->GetMethodID(doubleClass, "<init>", "(D)V");
		doubleGetID = env->GetMethodID(doubleClass,"doubleValue","()D");
        env->DeleteLocalRef(doubleLocalClass);
	}

	~JavaDoubleClassInfo()
    {
	    // We don't expect this because this should only be used by static instances
	    wkLogLevel(Warn, "Global ref not cleaned up");
    }

public:
	// Make a Double with the given value
	jobject makeDouble(JNIEnv *env,int iVal) { return env->NewObject(doubleClass, doubleClassInitID, iVal); }
	static jobject make(JNIEnv *env,int iVal) { return getClassInfo(env)->makeDouble(env,iVal); }

	// Return the value of an Double object
	double getDouble(JNIEnv *env,jobject dblObj) { return env->CallDoubleMethod(dblObj,doubleGetID); }
	static double get(JNIEnv *env,jobject dblObj) { return getClassInfo(env)->getDouble(env,dblObj); }

	static JavaDoubleClassInfo *getClassInfo(JNIEnv *env)
	{
		if (!classInfoObj)
			classInfoObj = new JavaDoubleClassInfo(env);
		return classInfoObj;
	}

protected:
	jclass doubleClass;
	jmethodID doubleClassInitID;
	jmethodID doubleGetID;
	static JavaDoubleClassInfo *classInfoObj;
};

// Wrapper for HashMap
class JavaHashMapInfo
{
private:
	JavaHashMapInfo(JNIEnv *env)
	{
		jclass localMapClass = env->FindClass("java/util/HashMap");
		mapClass = (jclass)env->NewGlobalRef(localMapClass);
		mapInitMethodID = env->GetMethodID(mapClass, "<init>", "(I)V");
		putMethodID = env->GetMethodID(mapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
        env->DeleteLocalRef(localMapClass);
	}

    ~JavaHashMapInfo()
    {
		// singleton pattern, so we don't expect this except at process stop
	    if (mapClass) {
            wkLogLevel(Warn, "Global ref not cleaned up");
        }
    }

public:
	// Create a new hash map
	jobject makeHashMap(JNIEnv *env) { return env->NewObject(mapClass, mapInitMethodID, 1); }
	static jobject make(JNIEnv *env) { return getClassInfo(env)->makeHashMap(env); }

	void teardown(JNIEnv *env) {
		if (mapClass) {
			env->DeleteGlobalRef(mapClass);
			mapClass = nullptr;
		}
	}

	// Add an object to an existing hash map
	void addObject(JNIEnv *env,jobject hashMap, jobject key, jobject val)
	{
		env->CallObjectMethod(hashMap, putMethodID, key, val);
	}

	static JavaHashMapInfo *getClassInfo(JNIEnv *env)
	{
		if (!classInfoObj)
			classInfoObj = new JavaHashMapInfo(env);
		return classInfoObj;
	}

protected:
	jclass mapClass;
	jmethodID mapInitMethodID;
	jmethodID putMethodID;
	static JavaHashMapInfo *classInfoObj;
};

// Wrapper for List
class JavaListInfo
{
private:
	JavaListInfo(JNIEnv *env)
	{
		jclass localListClass = env->FindClass("java/util/List");
		listClass = (jclass)env->NewGlobalRef(localListClass);
		jclass localIterClass = env->FindClass("java/util/Iterator");
		iterClass = (jclass)env->NewGlobalRef(localIterClass);
		literMethodID = env->GetMethodID(listClass,"iterator","()Ljava/util/Iterator;");
		hasNextID = env->GetMethodID(iterClass,"hasNext","()Z");
		nextID = env->GetMethodID(iterClass,"next","()Ljava/lang/Object;");
        env->DeleteLocalRef(localIterClass);
        env->DeleteLocalRef(localListClass);
	}

	~JavaListInfo()
    {
		// singleton pattern, so we don't expect this except at process stop
		if (iterClass || listClass) {
			wkLogLevel(Warn, "JavaListInfo not cleaned up");
		}
    }

public:
	// Return the iterator for a given list object
	jobject getIter(JNIEnv *env,jobject listObj) const { return env->CallObjectMethod(listObj,literMethodID); }

	// See if there's a next object
	bool hasNext(JNIEnv *env,jobject listObj,jobject iterObj) const { return env->CallBooleanMethod(iterObj, hasNextID); }

	// Get the next object with an iterator
	jobject getNext(JNIEnv *env,jobject listObj,jobject iterObj) { return env->CallObjectMethod(iterObj, nextID); }

	void teardown(JNIEnv *env) {
		if (iterClass) {
			env->DeleteGlobalRef(iterClass);
			env->DeleteGlobalRef(listClass);
			iterClass = nullptr;
			listClass = nullptr;
		}
	}
	static JavaListInfo *getClassInfo(JNIEnv *env)
	{
		if (!classInfoObj)
			classInfoObj = new JavaListInfo(env);
		return classInfoObj;
	}

protected:
	jclass listClass,iterClass;
	jmethodID literMethodID,hasNextID,nextID;
	static JavaListInfo *classInfoObj;
};

// Wrapper for Java string.  Destructor releases.
// These are not meant to be long-lived, and should not be composed into other objects.
struct JavaString
{
    // Construct with the string we want to wrap
    JavaString(JNIEnv *env,jstring str);
    JavaString(const JavaString &other) = delete;
    JavaString(JavaString &&other) noexcept;

    // This cleans up the reference.
    ~JavaString();

    operator const char*() const { return cStr; }
    operator bool() const { return cStr != nullptr; }

    const char *getCString() const { return cStr; }

private:
	const char *cStr;
	JNIEnv *env;
	jstring str;
};

// Wrapper for Java boolean array.  Destructor cleans up.
class JavaBooleanArray
{
public:
    JavaBooleanArray(JNIEnv *env,jbooleanArray &array);
    ~JavaBooleanArray();
    
    JNIEnv *env;
    jbooleanArray &array;
    int len;
    jboolean *rawBool;
};

// Wrapper for Java int array.  Destructor cleans up.
class JavaIntArray
{
public:
    JavaIntArray(JNIEnv *env,jintArray &array);
    ~JavaIntArray();
    
    JNIEnv *env;
    jintArray &array;
    int len;
    jint *rawInt;
};

// Wrapper for Java long array.  Destructor cleans up.
class JavaLongArray
{
public:
    JavaLongArray(JNIEnv *env,jlongArray &array);
    ~JavaLongArray();
    
    JNIEnv *env;
    jlongArray &array;
    int len;
    jlong *rawLong;
};

// Wrapper for Java float array.  Destructor cleans up.
class JavaFloatArray
{
public:
    JavaFloatArray(JNIEnv *env,jfloatArray &array);
    ~JavaFloatArray();
    
    JNIEnv *env;
    jfloatArray &array;
    int len;
    jfloat *rawFloat;
};

// Wrapper for Java double array.  Destructor cleans up.
class JavaDoubleArray
{
public:
    JavaDoubleArray(JNIEnv *env,jdoubleArray &array);
    ~JavaDoubleArray();
    
    JNIEnv *env;
    jdoubleArray &array;
    int len;
    jdouble *rawDouble;
};

/**
 * Used to iterate over the elements of an object array.
 * This cleans up the previous object when you get the next
 * and cleans up the last one on destruction.
 *
 * Retains the JNIEnv, and so must not be held across JNI calls.
 */
class JavaObjectArrayHelper
{
public:
	JavaObjectArrayHelper(JNIEnv *env,jobjectArray objArray);
	JavaObjectArrayHelper(const JavaObjectArrayHelper &) = delete;
	~JavaObjectArrayHelper();

	JavaObjectArrayHelper& operator=(const JavaObjectArrayHelper&) = delete;

	// Total number of objects
	int numObjects() const { return count; }

	operator bool() const { return count > 0; }

	bool hasNextObject() const { return nextIndex < count; }

	jobject getCurrentObject() const { return curObj; }

	// Return the next object, if there is one.  NULL otherwise.
	jobject getNextObject();

protected:
	JNIEnv *env;
	jobjectArray objArray;
	int count;
	int nextIndex;
	jobject curObj;
};

namespace WhirlyKit {
/**
 * For more complex parts of the system we need the JNIEnv associated
 * with the thread we're current on.  But we really like to reuse
 * objects between threads, so this thing has to be passed way, way down.
 */
class PlatformInfo_Android : public WhirlyKit::PlatformThreadInfo {
public:
    PlatformInfo_Android(JNIEnv *env) : env(env) {}
	JNIEnv *env;
};
}

// Convert a Java int array into a std::vector of ints
void ConvertIntArray(JNIEnv *env,jintArray &intArray,std::vector<int> &intVec);
// Convert a Java long array into a std::vector of longs
void ConvertLongLongArray(JNIEnv *env,jlongArray &longArray,std::vector<WhirlyKit::SimpleIdentity> &longVec);
// Convert a Java float array into a std::vector of floats
void ConvertFloatArray(JNIEnv *env,jfloatArray &floatArray,std::vector<float> &floatVec);
// Convert a Java double array into a std::vector of doubles
void ConvertDoubleArray(JNIEnv *env,jdoubleArray &doubleArray,std::vector<double> &doubleVec);
// Convert a Java boolean array into a std::vector of bools
void ConvertBoolArray(JNIEnv *env,jbooleanArray &boolArray,std::vector<bool> &boolVec);
// Convert a Java float array into a std::vector of Point2f
void ConvertFloat2fArray(JNIEnv *env,jfloatArray &floatArray,WhirlyKit::Point2fVector &ptVec);
// Convert a Java float array into a std::vector of Point3f
void ConvertFloat3fArray(JNIEnv *env,jfloatArray &floatArray,WhirlyKit::Point3fVector &ptVec);
// Convert a Java float array into a std::vector of Point3d
void ConvertFloat3dArray(JNIEnv *env,jdoubleArray &floatArray,WhirlyKit::Point3dVector &ptVec);
// Convert a Java float array into a std::vector of Point4f
void ConvertFloat4fArray(JNIEnv *env,jfloatArray &floatArray,WhirlyKit::Vector4fVector &ptVec);
// Convert a Java long long array into a set of SimpleIdentity values
void ConvertLongArrayToSet(JNIEnv *env,jlongArray &longArray,std::set<WhirlyKit::SimpleIdentity> &intSet);
// Convert a Java String object array into a std::vector of std::strings
void ConvertStringArray(JNIEnv *env,jobjectArray &objArray,std::vector<std::string> &strVec);

// Return a Java long array
jlongArray BuildLongArray(JNIEnv *env,const std::vector<WhirlyKit::SimpleIdentity> &longVec);
// Return a Java double array
jdoubleArray BuildDoubleArray(JNIEnv *env,const std::vector<double> &doubleVec);
// Return a Java int array
jintArray BuildIntArray(JNIEnv *env,const std::vector<int> &longVec);
// Return a new Java object array
jobjectArray BuildObjectArray(JNIEnv *env,jclass cls,jobject singleObj);
jobjectArray BuildObjectArray(JNIEnv *env,jclass cls,const std::vector<jobject> &objVec);
// Return new Java string array
jobjectArray BuildStringArray(JNIEnv *env,const std::vector<std::string> &objVec);

#endif /* Maply_JNI_h_ */
