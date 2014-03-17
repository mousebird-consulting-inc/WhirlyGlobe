/*
 * VectorObject_jni.cpp
 *
 *  Created on: Dec 19, 2013
 *      Author: sjg
 */

#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_VectorObject.h"
#import "WhirlyGlobe.h"
#import "Maply_utils_jni.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_nativeInit
  (JNIEnv *env, jclass cls)
{
	VectorObjectClassInfo::getClassInfo(env,cls);
}

JNIEXPORT jobject JNICALL MakeVectorObject(JNIEnv *env,VectorObject *vec)
{
	VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo(env,"com/mousebird/maply/VectorObject");
	return classInfo->makeWrapperObject(env,vec);
}

void Java_com_mousebird_maply_VectorObject_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
		VectorObject *inst = new VectorObject();
		classInfo->setHandle(env,obj,inst);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::initialise()");
	}
}

void Java_com_mousebird_maply_VectorObject_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
		VectorObject *inst = classInfo->getObject(env,obj);
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_addPoint
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
		VectorObject *vecObj = classInfo->getObject(env,obj);
		Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env,ptObj);
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_addLinear
  (JNIEnv *env, jobject obj, jobjectArray ptsObj)
{
	try
	{
		VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
		VectorObject *vecObj = classInfo->getObject(env,obj);
		if (!vecObj)
			return;

		VectorLinearRef lin = VectorLinear::createLinear();

		int count = env->GetArrayLength(ptsObj);
		if (count == 0)
			return;
		for (int ii=0;ii<count;ii++)
		{
			jobject ptObj = env->GetObjectArrayElement(ptsObj,ii);
			Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env,ptObj);
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_addAreal
  (JNIEnv *env, jobject obj, jobjectArray ptsObj)
{
	try
	{
		VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
		VectorObject *vecObj = classInfo->getObject(env,obj);
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
			Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env,ptObj);
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

jboolean Java_com_mousebird_maply_VectorObject_fromGeoJSON
  (JNIEnv *env, jobject obj, jstring jstr)
{
	try
	{
		VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
		VectorObject *vecObj = classInfo->getObject(env,obj);
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

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_FromGeoJSONAssembly
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
			JavaHashMapInfo *hashMapClassInfo = JavaHashMapInfo::getClassInfo(env);
			jobject hashMap = hashMapClassInfo->makeHashMap(env);
			for (std::map<std::string,VectorObject *>::iterator it = vecData.begin();
					it != vecData.end(); ++it)
			{
				jstring key = env->NewStringUTF(it->first.c_str());
				jobject vecObj = MakeVectorObject(env,it->second);
				hashMapClassInfo->addObject(env, hashMap, key, vecObj);
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

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_getAttributes
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
		VectorObject *vecObj = classInfo->getObject(env,obj);
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

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_readFromFile
  (JNIEnv *env, jobject obj, jstring fileNameStr)
{
	try
	{
		VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
		VectorObject *vecObj = classInfo->getObject(env,obj);
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

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_writeToFile
  (JNIEnv *env, jobject obj, jstring fileNameStr)
{
	try
	{
		VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
		VectorObject *vecObj = classInfo->getObject(env,obj);
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
