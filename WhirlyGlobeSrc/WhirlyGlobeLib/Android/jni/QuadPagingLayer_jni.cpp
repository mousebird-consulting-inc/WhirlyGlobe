#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_QuadPagingLayer.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

// Adapter between the Java QuadPagingLayer and the loader and dataStructure callbacks
class QuadPagingLayerAdapter : public QuadLoader, public QuadDataStructure, public QuadDisplayControllerAdapter
{
public:
	JNIEnv *env;
	jobject javaObj;
	CoordSystem *coordSys;
	jobject delegateObj;
	Point2d ll,ur;
	int minZoom,maxZoom;
	int numFetches;
	int simultaneousFetches;

	// Methods for Java quad layer
	jmethodID tileLoadJava,tileUnloadJava;

	QuadPagingLayerAdapter(CoordSystem *coordSys,jobject delegateObj)
		: env(NULL), javaObj(NULL), coordSys(coordSys), delegateObj(delegateObj), QuadLoader(),
		  numFetches(0), simultaneousFetches(1)
	{
	}

	virtual ~QuadPagingLayerAdapter()
	{
		if (coordSys)
			delete coordSys;
	}

	QuadDisplayController *getController() { return control; }

	void start(Scene *inScene,SceneRendererES *renderer,const Point2d &inLL,const Point2d &inUR,int inMinZoom,int inMaxZoom)
	{
		scene = inScene;
		ll = inLL;  ur = inUR;  minZoom = inMinZoom;  maxZoom = inMaxZoom;

		// Set up the display controller
		control = new QuadDisplayController(this,this,this);
		control->init(scene,renderer);
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
    	Mbr mbr(Point2f(ll.x(),ll.y()),Point2f(ur.x(),ur.y()));
    	return mbr;
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
    	return minZoom;
    }

    /// Return the maximum quad tree zoom level.  Must be at least minZoom
    virtual int getMaxZoom()
    {
    	return maxZoom;
    }

    /// Return an importance value for the given tile
    virtual double importanceForTile(const Quadtree::Identifier &ident,const Mbr &mbr,ViewState *viewState,const Point2f &frameSize,Dictionary *attrs)
    {
        if (ident.level <= 1)
            return MAXFLOAT;

        // For a child tile, we're taking the size of our parent so all the children load at once
        WhirlyKit::Quadtree::Identifier parentIdent;
        parentIdent.x = ident.x / 2;
        parentIdent.y = ident.y / 2;
        parentIdent.level = ident.level - 1;

        Mbr parentMbr = control->getQuadtree()->generateMbrForNode(parentIdent);

        // This is how much screen real estate we're covering for this tile
        double import = ScreenImportance(viewState, frameSize, viewState->eyeVec, 1, coordSys, scene->getCoordAdapter(), parentMbr, ident, attrs) / 4;
    }

    /// Called when the view state changes.  If you're caching info, do it here.
    virtual void newViewState(ViewState *viewState)
    {
    }

    /// Called when the layer is shutting down.  Clean up any drawable data and clear out caches.
    virtual void shutdown()
    {
    	if (control)
    	{
    		control->shutdown();
    		delete control;
    	}
    }

    /** QuadLoader Calls **/
    virtual bool isReady()
    {
    	return (numFetches < simultaneousFetches);
    }

    virtual void startUpdates()
    {
    }

    virtual void endUpdates()
    {
    }

    // Call loadTile on the java side
    virtual void loadTile(const Quadtree::NodeInfo &tileInfo)
    {
    	env->CallVoidMethod(javaObj, tileLoadJava, tileInfo.ident.x, tileInfo.ident.y, tileInfo.ident.level);
    }

    // Call unloadTile on the java side
    virtual void unloadTile(const Quadtree::NodeInfo &tileInfo)
    {
    	env->CallVoidMethod(javaObj, tileUnloadJava, tileInfo.ident.x, tileInfo.ident.y, tileInfo.ident.level);
    }

    virtual bool canLoadChildrenOfTile(const Quadtree::NodeInfo &tileInfo)
    {
    	return true;
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
  (JNIEnv *env, jobject obj, jobject sceneObj, jobject rendererObj, jobject llObj, jobject urObj, jint minZoom, jint maxZoom)
{
	try
	{
		QuadPagingLayerAdapter *adapter = getHandle<QuadPagingLayerAdapter>(env,obj);
		Point2d *ll = getHandle<Point2d>(env,llObj);
		Point2d *ur = getHandle<Point2d>(env,urObj);
		Scene *scene = getHandle<Scene>(env,sceneObj);
		SceneRendererES *renderer = getHandle<SceneRendererES>(env,rendererObj);
		if (!adapter || !ll || !ur || !scene || !renderer)
			return;

		adapter->env = env;
		adapter->javaObj = obj;

		adapter->start(scene,renderer,*ll,*ur,minZoom,maxZoom);
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

		adapter->getController()->shutdown();
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

		adapter->env = env;
		adapter->javaObj = obj;
		// Look for the wrapper object's methods
		jclass theClass = env->GetObjectClass(obj);
		adapter->tileLoadJava = env->GetMethodID(theClass,"loadTile","(III)V");
		adapter->tileUnloadJava = env->GetMethodID(theClass,"unloadTile","(III)V");

		adapter->getController()->viewUpdate(viewState);
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

		adapter->env = env;
		adapter->javaObj = obj;
		// Look for the wrapper object's methods
		jclass theClass = env->GetObjectClass(obj);
		adapter->tileLoadJava = env->GetMethodID(theClass,"loadTile","(III)V");
		adapter->tileUnloadJava = env->GetMethodID(theClass,"unloadTile","(III)V");

		// Note: Not passing in frame boundary info
		return adapter->getController()->evalStep(0.0,0.0,0.0);
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

		if (adapter->getController()->getWaitForLocalLoads())
			return false;

		adapter->getController()->refresh();
		return true;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::nativeRefresh()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_QuadPagingLayer_nativeTileDidLoad
  (JNIEnv *env, jobject obj, jint x, jint y, jint level)
{
	try
	{
		QuadPagingLayerAdapter *adapter = getHandle<QuadPagingLayerAdapter>(env,obj);
		if (!adapter)
			return;

	    adapter->getController()->tileDidLoad(Quadtree::Identifier(x,y,level));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::nativeTileDidLoad()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_QuadPagingLayer_nativeTileDidNotLoad
  (JNIEnv *env, jobject obj, jint x, jint y, jint level)
{
	try
	{
		QuadPagingLayerAdapter *adapter = getHandle<QuadPagingLayerAdapter>(env,obj);
		if (!adapter)
			return;

	    adapter->getController()->tileDidNotLoad(Quadtree::Identifier(x,y,level));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::nativeTileDidNotLoad()");
	}
}

