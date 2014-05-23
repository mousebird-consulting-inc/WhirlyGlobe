#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_ViewState.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ViewState_nativeInit
  (JNIEnv *env, jclass cls)
{
	ViewStateClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ViewState_initialise
  (JNIEnv *env, jobject obj, jobject viewObj, jobject rendererObj)
{
	try
	{
		View *view = MapViewClassInfo::getClassInfo()->getObject(env,viewObj);
		SceneRendererES *renderer = (SceneRendererES *)MaplySceneRendererInfo::getClassInfo()->getObject(env,rendererObj);
		if (!view || !renderer)
			return;

		ViewState *viewState = NULL;
		if (dynamic_cast<WhirlyGlobe::GlobeView *>(view))
		{
			viewState = new WhirlyGlobe::GlobeViewState((WhirlyGlobe::GlobeView *)view,renderer);
		} else if (dynamic_cast<Maply::MapView *>(view))
		{
			viewState = new Maply::MapViewState((Maply::MapView *)view,renderer);
		}

		if (viewState)
			ViewStateClassInfo::getClassInfo()->setHandle(env,obj,viewState);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ViewState_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		ViewStateClassInfo *classInfo = ViewStateClassInfo::getClassInfo();
		ViewState *viewState = classInfo->getObject(env,obj);
		if (!viewState)
			return;
		delete viewState;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::dispose()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_ViewState_isEqual
  (JNIEnv *env, jobject obj, jobject otherObj)
{
	try
	{
		ViewStateClassInfo *classInfo = ViewStateClassInfo::getClassInfo();
		ViewState *viewState = classInfo->getObject(env,obj);
		ViewState *otherViewState = classInfo->getObject(env,obj);
		if (!viewState || !otherViewState)
			return false;

		return viewState->isSameAs(otherViewState);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::isEqual()");
	}
}
