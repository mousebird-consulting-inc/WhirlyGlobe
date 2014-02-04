/*
 * VectorObject_jni.cpp
 *
 *  Created on: Dec 19, 2013
 *      Author: sjg
 */

#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_VectorObject.h"
#import "WhirlyGlobe.h"
#import "Maply_utils_jni.h"

using namespace WhirlyKit;

// Caching class and method pointers
class VectorClassInfo : public JavaClassInfo<VectorObject>
{
public:
	VectorClassInfo(JNIEnv *env, jclass theClass)
		: JavaClassInfo<VectorObject>(env,theClass)
	{
		initMethodID = env->GetMethodID(theClass, "<init>", "()V");

		jclass localMapClass = env->FindClass("java/util/HashMap");
		mapClass = (jclass)env->NewGlobalRef(localMapClass);
		mapInitMethodID = env->GetMethodID(mapClass, "<init>", "(I)V");
		putMethodID = env->GetMethodID(mapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
	}

	jmethodID initMethodID;
	jclass mapClass;
	jmethodID mapInitMethodID;
	jmethodID putMethodID;
};

static VectorClassInfo *classInfo = NULL;

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorObject_nativeInit
  (JNIEnv *env, jclass theClass)
{
	if (classInfo)
		delete classInfo;
	classInfo = new VectorClassInfo(env,theClass);
}

JNIEXPORT jobject JNICALL MakeVectorObject(JNIEnv *env,VectorObject *vec)
{
	if (!classInfo)
	{
		jclass cls = env->FindClass("com/mousebirdconsulting/maply/VectorObject");
		classInfo = new VectorClassInfo(env,cls);
		env->DeleteLocalRef(cls);
	}

	jobject vecObj = env->NewObject(classInfo->theClass, classInfo->initMethodID);
	setHandle(env,vecObj,vec);

	return vecObj;
}

void Java_com_mousebirdconsulting_maply_VectorObject_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorObject *inst = new VectorObject();
		classInfo->setHandle(env,obj,inst);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::initialise()");
	}
}

void Java_com_mousebirdconsulting_maply_VectorObject_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorObject *inst = classInfo->getHandle(env,obj);
		if (!inst)
			return;
		delete inst;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorObject_addPoint
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		VectorObject *vecObj = classInfo->getHandle(env,obj);
		Point2d *pt = getHandle<Point2d>(env,ptObj);
		if (!vecObj)
			return;

		VectorPointsRef pts = VectorPoints::createPoints();
		pts->pts.push_back(GeoCoord(pt->x(),pt->y()));
		pts->initGeoMbr();
		vecObj->shapes.insert(pts);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::addPoint()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorObject_addLinear
  (JNIEnv *env, jobject obj, jobjectArray ptsObj)
{
	try
	{
		VectorObject *vecObj = classInfo->getHandle(env,obj);
		if (!vecObj)
			return;

		VectorLinearRef lin = VectorLinear::createLinear();

		int count = env->GetArrayLength(ptsObj);
		if (count == 0)
			return;
		for (int ii=0;ii<count;ii++)
		{
			jobject ptObj = env->GetObjectArrayElement(ptsObj,ii);
			Point2d *pt = getHandle<Point2d>(env,ptObj);
			lin->pts.push_back(GeoCoord(pt->x(),pt->y()));
		}
		lin->initGeoMbr();
		vecObj->shapes.insert(lin);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::addLinear()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorObject_addAreal
  (JNIEnv *env, jobject obj, jobjectArray ptsObj)
{
	try
	{
		VectorObject *vecObj = classInfo->getHandle(env,obj);
		if (!vecObj)
			return;

		VectorArealRef ar = VectorAreal::createAreal();
		ar->loops.resize(1);

		int count = env->GetArrayLength(ptsObj);
		if (count == 0)
			return;
		for (int ii=0;ii<count;ii++)
		{
			jobject ptObj = env->GetObjectArrayElement(ptsObj,ii);
			Point2d *pt = getHandle<Point2d>(env,ptObj);
			ar->loops[0].push_back(GeoCoord(pt->x(),pt->y()));
		}
		ar->initGeoMbr();
		vecObj->shapes.insert(ar);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::addAreal()");
	}
}

jboolean Java_com_mousebirdconsulting_maply_VectorObject_fromGeoJSON
  (JNIEnv *env, jobject obj, jstring jstr)
{
	try
	{
		VectorObject *vecObj = classInfo->getHandle(env,obj);
		if (!vecObj)
			return false;

		const char *cStr = env->GetStringUTFChars(jstr,0);
		if (!cStr)
			return false;
		std::string jsonStr(cStr);
		env->ReleaseStringUTFChars(jstr, cStr);

		bool ret = vecObj->fromGeoJSON(jsonStr);

		return ret;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::fromGeoJSON()");
	}
}

JNIEXPORT jobject JNICALL Java_com_mousebirdconsulting_maply_VectorObject_FromGeoJSONAssembly
  (JNIEnv *env, jclass vecObjClass, jstring jstr)
{
	try
	{
		const char *cStr = env->GetStringUTFChars(jstr,0);
		if (!cStr)
			return NULL;
		std::string jsonStr(cStr);
		env->ReleaseStringUTFChars(jstr, cStr);

		std::map<std::string,VectorObject *> vecData;
		bool ret = VectorObject::FromGeoJSONAssembly(jsonStr,vecData);

		if (ret)
		{
			jobject hashMap = env->NewObject(classInfo->mapClass, classInfo->mapInitMethodID, 1);
			for (std::map<std::string,VectorObject *>::iterator it = vecData.begin();
					it != vecData.end(); ++it)
			{
				jstring key = env->NewStringUTF(it->first.c_str());
				jobject vecObj = MakeVectorObject(env,it->second);
				env->CallObjectMethod(hashMap, classInfo->putMethodID, key, vecObj);
			}

			return hashMap;
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::FromGeoJSONAssembly()");
	}

	return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebirdconsulting_maply_VectorObject_getAttributes
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorObject *vecObj = classInfo->getHandle(env,obj);
		if (!vecObj)
			return NULL;

		jobject dictObj = MakeAttrDictionary(env,vecObj->getAttributes());

		return dictObj;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::getAttributes()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebirdconsulting_maply_VectorObject_readFromFile
  (JNIEnv *env, jobject obj, jstring fileNameStr)
{
	try
	{
		VectorObject *vecObj = classInfo->getHandle(env,obj);
		if (!vecObj)
			return false;
		const char *cStr = env->GetStringUTFChars(fileNameStr,0);
		if (!cStr)
			return false;
		std::string fileName(cStr);
		env->ReleaseStringUTFChars(fileNameStr, cStr);

		return vecObj->fromFile(fileName);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::readFromFile()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebirdconsulting_maply_VectorObject_writeToFile
  (JNIEnv *env, jobject obj, jstring fileNameStr)
{
	try
	{
		VectorObject *vecObj = classInfo->getHandle(env,obj);
		if (!vecObj)
			return false;
		const char *cStr = env->GetStringUTFChars(fileNameStr,0);
		if (!cStr)
			return false;
		std::string fileName(cStr);
		env->ReleaseStringUTFChars(fileNameStr, cStr);

		return vecObj->toFile(fileName);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::writeToFile()");
	}
}
