#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_VectorIterator.h"
#import "Maply_utils_jni.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

// Used to store position in a set of shapes
class VectorIterator
{
public:
	VectorIterator(VectorObject *vecObj)
		: vecObj(vecObj)
	{
		it = vecObj->shapes.begin();
	}

	VectorObject *vecObj;
	ShapeSet::iterator it;
};

typedef JavaClassInfo<VectorIterator> VectorIteratorClassInfo;
template<> VectorIteratorClassInfo *VectorIteratorClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorIterator_nativeInit
  (JNIEnv *env, jclass cls)
{
	VectorIteratorClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorIterator_initialise
  (JNIEnv *env, jobject obj, jobject vecObj)
{
	try
	{
		VectorObject *vec = VectorObjectClassInfo::getClassInfo()->getObject(env,vecObj);
		VectorIterator *vecIter = new VectorIterator(vec);
		VectorIteratorClassInfo::getClassInfo()->setHandle(env,obj,vecIter);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorIterator::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorIterator_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorIteratorClassInfo *classInfo = VectorIteratorClassInfo::getClassInfo();
		VectorIterator *vecIter = classInfo->getObject(env,obj);
		if (!vecIter)
			return;
		delete vecIter;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorIterator::dispose()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorIterator_hasNext
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorIteratorClassInfo *classInfo = VectorIteratorClassInfo::getClassInfo();
		VectorIterator *vecIter = classInfo->getObject(env,obj);
		if (!vecIter)
			return false;

		return vecIter->it != vecIter->vecObj->shapes.end();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorIterator::hasNext()");
	}
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorIterator_next
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorIteratorClassInfo *classInfo = VectorIteratorClassInfo::getClassInfo();
		VectorIterator *vecIter = classInfo->getObject(env,obj);
		if (!vecIter)
			return NULL;

		if (vecIter->it == vecIter->vecObj->shapes.end())
			return NULL;
		VectorObject *vec = new VectorObject();
		vec->shapes.insert(*(vecIter->it));
		jobject retObj = MakeVectorObject(env,vec);
		vecIter->it++;
		return retObj;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorIterator::next()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorIterator_remove
  (JNIEnv *env, jobject obj)
{
	// Note: Not bothering to implement
}
