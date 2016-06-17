/*
 *  Maply_jni.h
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

#ifndef Maply_JNI_h_
#define Maply_JNI_h_

#include <stdlib.h>
#ifdef __ANDROID__
#include <android/log.h>
#endif
#include <WhirlyGlobe.h>
#import "SingleLabelAndroid.h"

/* Java Class Info
 * This tracks JNI info about classes we implement.
 * We use this rather than hitting the JNI class and method routines
 * all the time.
 * Each object has one of these in its own file.
 */
template<typename T> class JavaClassInfo
{
public:
	// Don't use this constructor
	JavaClassInfo() { }
	// Construct with environment and class
	JavaClassInfo(JNIEnv *env,jclass inClass)
		: theClass(NULL), nativeHandleField(NULL)
	{
		theClass = (jclass)env->NewGlobalRef(inClass);
		initMethodID = env->GetMethodID(theClass, "<init>", "()V");
	}

	// Clear references to JNI data
	virtual void clear(JNIEnv *env)
	{
		env->DeleteGlobalRef(theClass);
	}

	// Make an object of the type and point it to the C++ object given
	virtual jobject makeWrapperObject(JNIEnv *env,T *cObj)
	{
		jobject obj = env->NewObject(theClass,initMethodID);
		setHandle(env, obj, cObj);
		return obj;
	}

	// Return the field ID for the handle we use
	jfieldID getHandleField(JNIEnv *env,jobject obj)
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
			__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Null object handle in getHandle().");
			return NULL;
		}
	    jlong handle = env->GetLongField(obj, getHandleField(env, obj));
	    return reinterpret_cast<T *>(handle);
	}

	// Set the handle for a Java wrapper to its C++ object
	void setHandle(JNIEnv *env, jobject obj, T *t)
	{
		if (!t)
		{
			__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Null handle in setHandle()");
			return;
		}
	    jlong handle = reinterpret_cast<jlong>(t);
	    env->SetLongField(obj, getHandleField(env, obj), handle);
	}

	// Clear the handle out of a Java wrapper object
	void clearHandle(JNIEnv *env, jobject obj)
	{
	    env->SetLongField(obj, getHandleField(env, obj), 0);
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
    jclass getClass() { return theClass; }

protected:
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

public:
	// Don't use this constructor
	JavaIntegerClassInfo() { }

	// Make an Integer with the given value
	jobject makeInteger(JNIEnv *env,int iVal)
	{
	    return env->NewObject(integerClass, integerClassInitID, iVal);
	}

	// Return the value of an Integer object
	int getInteger(JNIEnv *env,jobject intObj)
	{
		return env->CallIntMethod(intObj,integerGetID);
	}

	static JavaIntegerClassInfo *classInfoObj;
	static JavaIntegerClassInfo *getClassInfo(JNIEnv *env)
	{
		if (!classInfoObj)
			classInfoObj = new JavaIntegerClassInfo(env);
		return classInfoObj;
	}

protected:
	jclass integerClass;
	jmethodID integerClassInitID,integerGetID;
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
        env->DeleteLocalRef(doubleLocalClass);
	}

public:
	// Don't use this constructor
	JavaDoubleClassInfo() { }

	// Make a Double with the given value
	jobject makeDouble(JNIEnv *env,int iVal)
	{
	    return env->NewObject(doubleClass, doubleClassInitID, iVal);
	}

	static JavaDoubleClassInfo *classInfoObj;
	static JavaDoubleClassInfo *getClassInfo(JNIEnv *env)
	{
		if (!classInfoObj)
			classInfoObj = new JavaDoubleClassInfo(env);
		return classInfoObj;
	}

protected:
	jclass doubleClass;
	jmethodID doubleClassInitID;
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

public:
	// Create a new hash map
	jobject makeHashMap(JNIEnv *env)
	{
		return env->NewObject(mapClass, mapInitMethodID, 1);
	}

	// Add an object to an existing hash map
	void addObject(JNIEnv *env,jobject hashMap, jobject key, jobject val)
	{
		env->CallObjectMethod(hashMap, putMethodID, key, val);
	}

	static JavaHashMapInfo *classInfoObj;
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

public:
	// Return the iterator for a given list object
	jobject getIter(JNIEnv *env,jobject listObj)
	{
		return env->CallObjectMethod(listObj,literMethodID);
	}

	// See if there's a next object
	bool hasNext(JNIEnv *env,jobject listObj,jobject iterObj)
	{
		return env->CallBooleanMethod(iterObj, hasNextID);
	}

	// Get the next object with an iterator
	jobject getNext(JNIEnv *env,jobject listObj,jobject iterObj)
	{
		return env->CallObjectMethod(iterObj, nextID);
	}

public:

	static JavaListInfo *classInfoObj;
	static JavaListInfo *getClassInfo(JNIEnv *env)
	{
		if (!classInfoObj)
			classInfoObj = new JavaListInfo(env);
		return classInfoObj;
	}

protected:
	jclass listClass,iterClass;
	jmethodID literMethodID,hasNextID,nextID;
};

// Wrapper for Java string.  Destructor releases.
class JavaString
{
public:
    // Construct with the string we want to wrap
    // Only allocate these on the heap.
    JavaString(JNIEnv *env,jstring &str);
    
    // This cleans up the reference.
    ~JavaString();
    
    JNIEnv *env;
    jstring &str;
    const char *cStr;
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

namespace WhirlyKit
{
typedef Eigen::Vector4d Point4d;
typedef Eigen::Vector4f Point4f;
}

// Wrapper on top of scene renderer
class MaplySceneRenderer : public WhirlyKit::SceneRendererES2
{
public:
    MaplySceneRenderer();
    
    // Called when the window changes size (or on startup)
    bool resize(int width,int height);
    
    EGLContext context;
};

// Wrappers for class info for all the various classes that have presence in Java
typedef JavaClassInfo<WhirlyKit::Dictionary> AttrDictClassInfo;
typedef JavaClassInfo<WhirlyKit::ChangeSet> ChangeSetClassInfo;
typedef JavaClassInfo<WhirlyKit::Texture> TextureClassInfo;
typedef JavaClassInfo<WhirlyKit::CoordSystem> CoordSystemClassInfo;
typedef JavaClassInfo<WhirlyKit::Point2d> Point2dClassInfo;
typedef JavaClassInfo<WhirlyKit::Point3d> Point3dClassInfo;
typedef JavaClassInfo<WhirlyKit::Point4d> Point4dClassInfo;
typedef JavaClassInfo<Eigen::Matrix3d> Matrix3dClassInfo;
typedef JavaClassInfo<Eigen::Matrix4d> Matrix4dClassInfo;
typedef JavaClassInfo<Eigen::Quaterniond> QuaternionClassInfo;
typedef JavaClassInfo<Eigen::AngleAxisd> AngleAxisClassInfo;
typedef JavaClassInfo<WhirlyKit::CoordSystemDisplayAdapter> CoordSystemDisplayAdapterInfo;
typedef JavaClassInfo<WhirlyKit::FakeGeocentricDisplayAdapter> FakeGeocentricDisplayAdapterInfo;
typedef JavaClassInfo<WhirlyKit::GeneralCoordSystemDisplayAdapter> GeneralDisplayAdapterInfo;
typedef JavaClassInfo<MaplySceneRenderer> MaplySceneRendererInfo;
typedef JavaClassInfo<Maply::MapScene> MapSceneClassInfo;
typedef JavaClassInfo<WhirlyKit::Scene> SceneClassInfo;
typedef JavaClassInfo<WhirlyGlobe::GlobeScene> GlobeSceneClassInfo;
typedef JavaClassInfo<Maply::MapView> MapViewClassInfo;
typedef JavaClassInfo<WhirlyGlobe::GlobeView> GlobeViewClassInfo;
typedef JavaClassInfo<WhirlyKit::View> ViewClassInfo;
typedef JavaClassInfo<WhirlyKit::BaseInfo> BaseInfoClassInfo;
typedef JavaClassInfo<WhirlyKit::VectorInfo> VectorInfoClassInfo;
typedef JavaClassInfo<WhirlyKit::VectorObject> VectorObjectClassInfo;
typedef JavaClassInfo<WhirlyKit::MarkerInfo> MarkerInfoClassInfo;
typedef JavaClassInfo<WhirlyKit::LabelInfo> LabelInfoClassInfo;
typedef JavaClassInfo<WhirlyKit::SphericalChunkInfo> SphericalChunkInfoClassInfo;
typedef JavaClassInfo<WhirlyKit::SphericalChunk> SphericalChunkClassInfo;
typedef JavaClassInfo<WhirlyKit::ViewState> ViewStateClassInfo;
typedef JavaClassInfo<Maply::MapViewState> MapViewStateClassInfo;
typedef JavaClassInfo<WhirlyGlobe::GlobeViewState> GlobeViewStateClassInfo;
typedef JavaClassInfo<WhirlyKit::Marker> MarkerClassInfo;
typedef JavaClassInfo<WhirlyKit::SingleLabelAndroid> LabelClassInfo;
typedef JavaClassInfo<WhirlyKit::SingleVertexAttributeInfo> SingleVertexAttributeInfoClassInfo;
typedef JavaClassInfo<WhirlyKit::ParticleBatch> ParticleBatchClassInfo;
typedef JavaClassInfo<WhirlyKit::ParticleSystem> ParticleSystemClassInfo;
typedef JavaClassInfo<WhirlyKit::ParticleSystemManager> ParticleSystemManagerClassInfo;
typedef JavaClassInfo<WhirlyKit::OpenGLES2Program> OpenGLES2ProgramClassInfo;
typedef JavaClassInfo<WhirlyKit::QuadTracker> QuadTrackerClassInfo;
typedef JavaClassInfo<WhirlyKit::QuadTrackerPointReturn> QuadTrackerPointReturnClassInfo;
typedef JavaClassInfo<WhirlyKit::WhirlyKitShapeInfo> ShapeInfoClassInfo;
typedef JavaClassInfo<WhirlyKit::WhirlyKitShape> ShapeClassInfo;
typedef JavaClassInfo<WhirlyKit::WhirlyKitSphere> ShapeSphereClassInfo;
typedef JavaClassInfo<WhirlyKit::ShapeManager> ShapeManagerClassInfo;
typedef JavaClassInfo<WhirlyKit::SingleVertexAttribute> SingleVertexAttributeClassInfo;
typedef JavaClassInfo<WhirlyKit::WhirlyKitDirectionalLight> DirectionalLightClassInfo;
typedef JavaClassInfo<WhirlyKit::WhirlyKitMaterial> MaterialClassInfo;
typedef JavaClassInfo<WhirlyKit::Moon> MoonClassInfo;
typedef JavaClassInfo<WhirlyKit::Sun> SunClassInfo;
typedef JavaClassInfo<WhirlyKit::Billboard> BillboardClassInfo;
typedef JavaClassInfo<WhirlyKit::BillboardInfo> BillboardInfoClassInfo;
typedef JavaClassInfo<WhirlyKit::BillboardManager> BillboardManagerClassInfo;
typedef JavaClassInfo<WhirlyKit::SimplePoly> SimplePolyClassInfo;
typedef JavaClassInfo<WhirlyKit::StringWrapper> StringWrapperClassInfo;
typedef JavaClassInfo<WhirlyKit::ScreenObject> ScreenObjectClassInfo;
typedef JavaClassInfo<WhirlyKit::MapboxVectorTileParser> MapboxVectorTileParserClassInfo;
typedef JavaClassInfo<WhirlyKit::SelectionManager::SelectedObject> SelectedObjectClassInfo;

// The shared JNIEnv set in the ::render call
extern JNIEnv *maplyCurrentEnv;

// Convert a Java int array into a std::vector of ints
void ConvertIntArray(JNIEnv *env,jintArray &intArray,std::vector<int> &intVec);

#endif /* Maply_JNI_h_ */
