#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebirdconsulting_maply_Matrix4d.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_Matrix4d_nativeInit
  (JNIEnv *env, jclass cls)
{
	Matrix4dClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_Matrix4d_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		Matrix4d *mat = new Matrix4d();
		*mat = Matrix4d::Identity();
		Matrix4dClassInfo::getClassInfo()->setHandle(env,obj,mat);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Matrix4d::initialise()");
	}

}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_Matrix4d_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		Matrix4dClassInfo *classInfo = Matrix4dClassInfo::getClassInfo();
		Matrix4d *inst = classInfo->getObject(env,obj);
		if (!inst)
			return;
		delete inst;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Matrix4d::dispose()");
	}
}

jobject MakeMatrix4d(JNIEnv *env,const Eigen::Matrix4d &mat)
{
	// Make a Java Matrix4d
	Matrix4dClassInfo *classInfo = Matrix4dClassInfo::getClassInfo(env,"com/mousebirdconsulting/maply/Matrix4d");
	Matrix4d *newMat = new Matrix4d(mat);
	return classInfo->makeWrapperObject(env,newMat);
}

