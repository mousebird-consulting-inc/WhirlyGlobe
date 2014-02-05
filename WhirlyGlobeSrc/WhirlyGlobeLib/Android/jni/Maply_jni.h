/*
 * handle.h
 *
 *  Created on: Dec 19, 2013
 *      Author: sjg
 */
#ifndef Maply_JNI_h_
#define Maply_JNI_h_

#include <stdlib.h>
#include <android/log.h>
#include <WhirlyGlobe.h>

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
			jclass cls = env->FindClass("com/mousebirdconsulting/maply/AttrDictionary");
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
	}

public:
	// Don't use this constructor
	JavaIntegerClassInfo() { }

	// Make an Integer with the given value
	jobject makeInteger(JNIEnv *env,int iVal)
	{
	    return env->NewObject(integerClass, integerClassInitID, iVal);
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
	jmethodID integerClassInitID;
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

// Wrappers for class info for all the various classes that have presence in Java
typedef JavaClassInfo<WhirlyKit::Dictionary> AttrDictClassInfo;
typedef JavaClassInfo<WhirlyKit::ChangeSet> ChangeSetClassInfo;
typedef JavaClassInfo<WhirlyKit::Texture> TextureClassInfo;
typedef JavaClassInfo<WhirlyKit::CoordSystem> CoordSystemClassInfo;
typedef JavaClassInfo<WhirlyKit::Point2d> Point2dClassInfo;
typedef JavaClassInfo<WhirlyKit::Point3d> Point3dClassInfo;
typedef JavaClassInfo<Eigen::Matrix4d> Matrix4dClassInfo;
typedef JavaClassInfo<WhirlyKit::CoordSystemDisplayAdapter> CoordSystemDisplayAdapterInfo;
class MaplySceneRenderer;
typedef JavaClassInfo<MaplySceneRenderer> MaplySceneRendererInfo;
typedef JavaClassInfo<Maply::MapScene> MapSceneClassInfo;
typedef JavaClassInfo<Maply::MapView> MapViewClassInfo;
typedef JavaClassInfo<WhirlyKit::VectorInfo> VectorInfoClassInfo;
typedef JavaClassInfo<WhirlyKit::VectorObject> VectorObjectClassInfo;
typedef JavaClassInfo<WhirlyKit::MarkerInfo> MarkerInfoClassInfo;
typedef JavaClassInfo<WhirlyKit::ViewState> ViewStateClassInfo;

#endif /* Maply_JNI_h_ */
