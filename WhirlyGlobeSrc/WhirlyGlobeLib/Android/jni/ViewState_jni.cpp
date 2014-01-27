#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_ViewState.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_ViewState_initialise
  (JNIEnv *env, jobject obj, jobject viewObj, jobject rendererObj)
{
	try
	{
		View *view = getHandle<View>(env,viewObj);
		SceneRendererES *renderer = getHandle<SceneRendererES>(env,rendererObj);
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
			setHandle(env,obj,viewState);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_ViewState_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		ViewState *viewState = getHandle<ViewState>(env,obj);
		if (!viewState)
			return;
		delete viewState;

		clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::dispose()");
	}
}
