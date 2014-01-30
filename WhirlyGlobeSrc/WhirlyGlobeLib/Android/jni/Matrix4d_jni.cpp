#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_Matrix4d.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_Matrix4d_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		Matrix4d *mat = new Matrix4d();
		*mat = Matrix4d::Identity();
		setHandle(env,obj,mat);
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
		Matrix4d *inst = getHandle<Matrix4d>(env,obj);
		if (!inst)
			return;
		delete inst;

		clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Matrix4d::dispose()");
	}
}

jobject MakeMatrix4d(JNIEnv *env,const Eigen::Matrix4d &mat)
{
	// Make a Java Matrix4d
	jclass cls = env->FindClass("com/mousebirdconsulting/maply/Matrix4d");
	jmethodID methodID = env->GetMethodID(cls, "<init>", "()V");
	if (!methodID)
		throw 1;
	jobject jMat = env->NewObject(cls,methodID);
	Matrix4d *newMat = new Matrix4d(mat);
	setHandle(env,jMat,newMat);

	return jMat;
}

