#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_QuadPagingLayer.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

// Adapter between the Java QuadPagingLayer and the loader and dataStructure callbacks
class QuadPagingLayerAdapter : public QuadLoader, public QuadDataStructure, public QuadDisplayControllerAdapter
{
public:
	QuadDisplayController *displayControl;
	CoordSystem *coordSys;
	jobject delegateObj;

	QuadPagingLayerAdapter(CoordSystem *coordSys,jobject delegateObj)
		: coordSys(coordSys), delegateObj(delegateObj), displayControl(NULL)
	{
	}

	virtual ~QuadPagingLayerAdapter()
	{
		if (coordSys)
			delete coordSys;
	}

	// Called after a tile unloads.  Don't care here.
    virtual void adapterTileDidLoad(const Quadtree::Identifier &tileIdent)
    {
    }

    // Called right after a tile unloaded.  Don't care here.
    virtual void adapterTileDidNotLoad(const Quadtree::Identifier &tileIdent)
    {
    }

    /** QuadDataStructure Calls **/
    virtual CoordSystem *getCoordSystem()
    {
    	return coordSys;
    }

    /// Bounding box used to calculate quad tree nodes.  In local coordinate system.
    virtual Mbr getTotalExtents()
    {
    }

    /// Bounding box of data you actually want to display.  In local coordinate system.
    /// Unless you're being clever, make this the same as totalExtents.
    virtual Mbr getValidExtents()
    {
        return getTotalExtents();
    }

    /// Return the minimum quad tree zoom level (usually 0)
    virtual int getMinZoom()
    {
    }

    /// Return the maximum quad tree zoom level.  Must be at least minZoom
    virtual int getMaxZoom()
    {
    }

    /// Return an importance value for the given tile
    virtual double importanceForTile(const Quadtree::Identifier &ident,const Mbr &mbr,ViewState *viewState,const Point2f &frameSize,Dictionary *attrs)
    {
    }

    /// Called when the view state changes.  If you're caching info, do it here.
    virtual void newViewState(ViewState *viewState)
    {
    }

    /// Called when the layer is shutting down.  Clean up any drawable data and clear out caches.
    virtual void shutdown()
    {
    }

    /** QuadLoader Calls **/
    virtual bool isReady()
    {
    }

    virtual void startUpdates()
    {
    }

    virtual void endUpdates()
    {
    }

    virtual void loadTile(const Quadtree::NodeInfo &tileInfo)
    {
    }

    virtual void unloadTile(const Quadtree::NodeInfo &tileInfo)
    {
    }

    virtual bool canLoadChildrenOfTile(const Quadtree::NodeInfo &tileInfo)
    {
    }

    virtual void shutdownLayer()
    {
    }

    virtual bool shouldUpdate(ViewState *viewState,bool isInitial)
    {
        return true;
    }

    virtual void updateWithoutFlush()
    {
    }

    virtual int networkFetches() { return -1; }

    virtual int localFetches() { return -1; }

    virtual void log()
    {
    }
};

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_QuadPagingLayer_initialise
  (JNIEnv *env, jobject obj, jobject coordSysObj, jobject delegateObj)
{
	try
	{
		CoordSystem *coordSys = getHandle<CoordSystem>(env,coordSysObj);
		if (!coordSys)
			return;

		QuadPagingLayerAdapter *adapter = new QuadPagingLayerAdapter(coordSys,delegateObj);
		setHandle(env,obj,adapter);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_QuadPagingLayer_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		QuadPagingLayerAdapter *adapter = getHandle<QuadPagingLayerAdapter>(env,obj);
		if (!adapter)
			return;
		delete adapter;

		clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_QuadPagingLayer_nativeStartLayer
  (JNIEnv *env, jobject obj)
{
	try
	{
		QuadPagingLayerAdapter *adapter = getHandle<QuadPagingLayerAdapter>(env,obj);
		if (!adapter)
			return;

		// Set up the display controller
		adapter->displayControl = new QuadDisplayController(adapter,adapter,adapter);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::nativeStartLayer()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_QuadPagingLayer_nativeShutdown
  (JNIEnv *env, jobject obj)
{
	try
	{
		QuadPagingLayerAdapter *adapter = getHandle<QuadPagingLayerAdapter>(env,obj);
		if (!adapter)
			return;

		adapter->displayControl->shutdown();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::nativeShutdown()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_QuadPagingLayer_nativeViewUpdate
  (JNIEnv *env, jobject obj, jobject viewStateObj)
{
	try
	{
		QuadPagingLayerAdapter *adapter = getHandle<QuadPagingLayerAdapter>(env,obj);
		ViewState *viewState = getHandle<ViewState>(env,viewStateObj);
		if (!adapter || !viewState)
			return;

		adapter->displayControl->viewUpdate(viewState);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::nativeViewUpdate()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebirdconsulting_maply_QuadPagingLayer_nativeEvalStep
  (JNIEnv *env, jobject obj)
{
	try
	{
		QuadPagingLayerAdapter *adapter = getHandle<QuadPagingLayerAdapter>(env,obj);
		if (!adapter)
			return false;

		// Note: Not passing in frame boundary info
		return adapter->displayControl->evalStep(0.0,0.0,0.0);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::nativeEvalStep()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebirdconsulting_maply_QuadPagingLayer_nativeRefresh
  (JNIEnv *env, jobject obj)
{
	try
	{
		QuadPagingLayerAdapter *adapter = getHandle<QuadPagingLayerAdapter>(env,obj);
		if (!adapter)
			return false;

		if (adapter->displayControl->getWaitForLocalLoads())
			return false;

		adapter->displayControl->refresh();
		return true;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::nativeRefresh()");
	}
}
