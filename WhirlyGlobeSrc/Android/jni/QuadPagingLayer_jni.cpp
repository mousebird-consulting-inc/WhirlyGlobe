/*
 *  QuadPagingLayer_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2016 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_QuadPagingLayer.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

// Adapter between the Java QuadPagingLayer and the loader and dataStructure callbacks
class QuadPagingLayerAdapter : public QuadLoader, public QuadDataStructure, public QuadDisplayControllerAdapter
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

	Point2d ll,ur;

	JNIEnv *env;
	jobject javaObj;
	SceneRendererES *renderer;
	CoordSystem *coordSys;
	jobject delegateObj;
	int minZoom,maxZoom;
	int numFetches;
	int simultaneousFetches;
	bool useTargetZoomLevel;
	bool singleLevelLoading;
	bool canShortCircuitImportance;
	int maxShortCircuitLevel;

	// Methods for Java quad layer
	jmethodID tileLoadJava,tileUnloadJava;

	QuadPagingLayerAdapter(CoordSystem *coordSys,jobject delegateObj)
		: env(NULL), javaObj(NULL), renderer(NULL), coordSys(coordSys), delegateObj(delegateObj), QuadLoader(),
		  numFetches(0), simultaneousFetches(1)
	{
		useTargetZoomLevel = true;
        canShortCircuitImportance = false;
        singleLevelLoading = false;
        shortCircuitImportance = -1;
        maxShortCircuitLevel = -1;
	}

	virtual ~QuadPagingLayerAdapter()
	{
		if (coordSys)
			delete coordSys;
		if (control)
		{
			delete control;
			control = NULL;
		}
	}

	QuadDisplayController *getController() { return control; }
	float shortCircuitImportance;

	// Called to start the layer
	void start(Scene *inScene,SceneRendererES *inRenderer,const Point2d &inLL,const Point2d &inUR,int inMinZoom,int inMaxZoom)
	{
		renderer = inRenderer;
		scene = inScene;
		ll = inLL;  ur = inUR;  minZoom = inMinZoom;  maxZoom = inMaxZoom;

		// Set up the display controller
		control = new QuadDisplayController(this,this,this);
		control->setMaxTiles(256);
		control->setMeteredMode(false);
		control->init(scene,renderer);

		// Note: Porting
		// Note: Explicitly setting the min importance for a 128*128 tile
//		getController()->setMinImportance(128*128);
		if (shortCircuitImportance == -1)
			shortCircuitImportance = 256*256 * 16;
		getController()->setMinImportance(1.0);
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

    // Shutdown for QuadDataStructure
    virtual void shutdown()
    {
    }

    /// Return an importance value for the given tile
    virtual double importanceForTile(const Quadtree::Identifier &ident,const Mbr &mbr,ViewState *viewState,const Point2f &frameSize,Dictionary *attrs)
    {
        if (ident.level <= 1)
            return MAXFLOAT;

        double import = 0;
		// For a child tile, we're taking the size of our parent so all the children load at once
		WhirlyKit::Quadtree::Identifier parentIdent;
		parentIdent.x = ident.x / 2;
		parentIdent.y = ident.y / 2;
		parentIdent.level = ident.level - 1;

		Mbr parentMbr = control->getQuadtree()->generateMbrForNode(parentIdent);

		if (canShortCircuitImportance && maxShortCircuitLevel != -1)
        {
            if (TileIsOnScreen(viewState, frameSize, coordSys, scene->getCoordAdapter(), (singleLevelLoading ? mbr : parentMbr), ident, attrs))
            {
                import = 1.0/(ident.level+10);
                if (ident.level <= maxShortCircuitLevel)
                	import += 1.0;
            }
        } else {
			// This is how much screen real estate we're covering for this tile
			import = ScreenImportance(viewState, frameSize, viewState->eyeVec, 1, coordSys, scene->getCoordAdapter(), parentMbr, ident, attrs) / 4;
        }
//		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "tile %d: (%d,%d) import = %f",ident.level,ident.x,ident.y,import);

        return import;
    }

    // Calculate a target zoom level for display
    int targetZoomLevel(ViewState *viewState)
    {
        if (!viewState)
            return minZoom;
        Point2f frameSize = renderer->getFramebufferSize();

        int zoomLevel = 0;
        WhirlyKit::Point2f center = Point2f(viewState->eyePos.x(),viewState->eyePos.y());

        // The coordinate adapter might have its own center
        Point3d adaptCenter = scene->getCoordAdapter()->getCenter();
        center.x() += adaptCenter.x();
        center.y() += adaptCenter.y();

        while (zoomLevel <= maxZoom)
        {
            WhirlyKit::Quadtree::Identifier ident;
            ident.x = 0;  ident.y = 0;  ident.level = zoomLevel;
            // Make an MBR right in the middle of where we're looking
            Mbr mbr = getController()->getQuadtree()->generateMbrForNode(ident);
            Point2f span = mbr.ur()-mbr.ll();
            mbr.ll() = center - span/2.0;
            mbr.ur() = center + span/2.0;
            Dictionary attrs;
            float import = ScreenImportance(viewState, frameSize, viewState->eyeVec, 1, coordSys, scene->getCoordAdapter(), mbr, ident, &attrs);
            if (import <= shortCircuitImportance)
            {
                zoomLevel--;
                break;
            }
            zoomLevel++;
        }

        return std::min(zoomLevel,maxZoom);
    }

    /// Called when the view state changes.  We look for an optimal display level here
    virtual void newViewState(ViewState *viewState)
    {
    	if (!useTargetZoomLevel)
    	{
    		canShortCircuitImportance = false;
    		maxShortCircuitLevel = -1;
    		return;
    	}

        CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
        Point3d center = coordAdapter->getCenter();
        if (center.x() == 0.0 && center.y() == 0.0 && center.z() == 0.0)
        {
            canShortCircuitImportance = true;
            // Note: Trying out 3D
//            if (!coordAdapter->isFlat())
//            {
//                canShortCircuitImportance = false;
//                return;
//            }
            // We happen to store tilt in the view matrix.
            // Note: Can't detect tilt reliably
//            if (!viewState->viewMatrix.isIdentity())
//            {
//                canShortCircuitImportance = false;
//                return;
//            }
            // The tile source coordinate system must be the same as the display's system
	    //            if (!coordSys->isSameAs(coordAdapter->getCoordSystem()))
	    //            {
	    //                canShortCircuitImportance = false;
	    //                return;
	    //            }

            // We need to feel our way down to the appropriate level
            maxShortCircuitLevel = targetZoomLevel(viewState);
            if (singleLevelLoading)
            {
            	std::set<int> targetLevels;
            	targetLevels.insert(maxShortCircuitLevel);
            	control->setTargetLevels(targetLevels);
            }

//    		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Short circuiting to level %d",maxShortCircuitLevel);

        } else {
            // Note: Can't short circuit in this case.  Something wrong with the math
            canShortCircuitImportance = false;
        }
    }

    /** QuadLoader Calls **/
    virtual bool isReady()
    {
    	return (numFetches < simultaneousFetches);
    }

    virtual void startUpdates(ChangeSet &changes)
    {
    }

    virtual void endUpdates(ChangeSet &changes)
    {
    }

    virtual void adapterWakeUp()
    {
    }

    virtual void reset(ChangeSet &changes)
    {
    	// Note: Porting.  Fix this.
    }

    /// Called when the layer is shutting down.  Clean up any drawable data and clear out caches.
    virtual void shutdownLayer(ChangeSet &changes)
    {
    }

    // Call loadTile on the java side
    virtual void loadTile(const Quadtree::NodeInfo &tileInfo,int frame)
    {
    	// Note: Porting  Take frame into account
    	numFetches++;
    	env->CallVoidMethod(javaObj, tileLoadJava, tileInfo.ident.x, tileInfo.ident.y, tileInfo.ident.level);
    }

    // Call unloadTile on the java side
    virtual void unloadTile(const Quadtree::NodeInfo &tileInfo)
    {
    	env->CallVoidMethod(javaObj, tileUnloadJava, tileInfo.ident.x, tileInfo.ident.y, tileInfo.ident.level);
    }

    // Bro, do you even load frames?
    virtual bool canLoadFrames()
    {
    	return false;
    }

    // Number of frames we can load
    virtual int numFrames()
    {
    	// Note: Porting  Take frame into account
    	return 1;
    }

    // Current active frame
    virtual int currentFrame()
    {
    	// Note: Porting  Take frame into account
    	return 0;
    }

    virtual bool canLoadChildrenOfTile(const Quadtree::NodeInfo &tileInfo)
    {
    	// Note: Should be checking this for non-single loading mode

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

    virtual int numNetworkFetches() { return numFetches; }

    virtual int numLocalFetches() { return 0; }

    virtual void log()
    {
    }
};

typedef JavaClassInfo<QuadPagingLayerAdapter> QPLAdapterClassInfo;
template<> QPLAdapterClassInfo *QPLAdapterClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadPagingLayer_nativeInit
  (JNIEnv *env, jclass cls)
{
	QPLAdapterClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadPagingLayer_initialise
  (JNIEnv *env, jobject obj, jobject coordSysObj, jobject delegateObj, jobject changesObj)
{
	try
	{
		CoordSystem *coordSys = CoordSystemClassInfo::getClassInfo()->getObject(env,coordSysObj);
		ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
		if (!coordSys || !changes)
			return;

		QuadPagingLayerAdapter *adapter = new QuadPagingLayerAdapter(coordSys,delegateObj);
		QPLAdapterClassInfo::getClassInfo()->setHandle(env,obj,adapter);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::initialise()");
	}
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadPagingLayer_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		QPLAdapterClassInfo *classInfo = QPLAdapterClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            QuadPagingLayerAdapter *adapter = classInfo->getObject(env,obj);
            if (!adapter)
                return;
            delete adapter;

            classInfo->clearHandle(env,obj);
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadPagingLayer_setSimultaneousFetches
  (JNIEnv *env, jobject obj, jint numFetches)
{
	try
	{
		QPLAdapterClassInfo *classInfo = QPLAdapterClassInfo::getClassInfo();
		QuadPagingLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return;
		adapter->simultaneousFetches = numFetches;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::setSimultaneousFetches()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadPagingLayer_setUseTargetZoomLevel
  (JNIEnv *env, jobject obj, jboolean newVal)
{
	try
	{
		QPLAdapterClassInfo *classInfo = QPLAdapterClassInfo::getClassInfo();
		QuadPagingLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return;
		adapter->useTargetZoomLevel = newVal;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::setUseTargetZoomLevel()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadPagingLayer_nativeSetSingleLevelLoading
  (JNIEnv *env, jobject obj, jboolean newVal)
{
	try
	{
		QPLAdapterClassInfo *classInfo = QPLAdapterClassInfo::getClassInfo();
		QuadPagingLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return;
		adapter->singleLevelLoading = newVal;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::setSingleLevelLoading()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadPagingLayer_setImportance
  (JNIEnv *env, jobject obj, jdouble newImport)
{
	try
	{
		QPLAdapterClassInfo *classInfo = QPLAdapterClassInfo::getClassInfo();
		QuadPagingLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return;
		adapter->shortCircuitImportance = newImport;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::setImportance()");
	}
}


JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadPagingLayer_geoBoundsForTileNative
  (JNIEnv *env, jobject obj, jint x, jint y, jint level, jobject llObj, jobject urObj)
{
	try
	{
		QPLAdapterClassInfo *classInfo = QPLAdapterClassInfo::getClassInfo();
		QuadPagingLayerAdapter *adapter = classInfo->getObject(env,obj);
		Point2d *ll = Point2dClassInfo::getClassInfo()->getObject(env,llObj);
		Point2d *ur = Point2dClassInfo::getClassInfo()->getObject(env,urObj);
		if (!adapter || !ll || !ur)
			return;

	    Mbr mbr = adapter->getController()->getQuadtree()->generateMbrForNode(WhirlyKit::Quadtree::Identifier(x,y,level));

	    GeoMbr geoMbr;
	    CoordSystem *wkCoordSys = adapter->getController()->getCoordSys();
	    geoMbr.addGeoCoord(wkCoordSys->localToGeographic(Point3f(mbr.ll().x(),mbr.ll().y(),0.0)));
	    geoMbr.addGeoCoord(wkCoordSys->localToGeographic(Point3f(mbr.ur().x(),mbr.ll().y(),0.0)));
	    geoMbr.addGeoCoord(wkCoordSys->localToGeographic(Point3f(mbr.ur().x(),mbr.ur().y(),0.0)));
	    geoMbr.addGeoCoord(wkCoordSys->localToGeographic(Point3f(mbr.ll().x(),mbr.ur().y(),0.0)));

	    ll->x() = geoMbr.ll().x();
	    ll->y() = geoMbr.ll().y();
	    ur->x() = geoMbr.ur().x();
	    ur->y() = geoMbr.ur().y();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::geoBoundsForTileNative()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadPagingLayer_boundsForTileNative
(JNIEnv *env, jobject obj, jint x, jint y, jint level, jobject llObj, jobject urObj)
{
    try
    {
        QPLAdapterClassInfo *classInfo = QPLAdapterClassInfo::getClassInfo();
        QuadPagingLayerAdapter *adapter = classInfo->getObject(env,obj);
        Point2d *ll = Point2dClassInfo::getClassInfo()->getObject(env,llObj);
        Point2d *ur = Point2dClassInfo::getClassInfo()->getObject(env,urObj);
        if (!adapter || !ll || !ur)
            return;
        
        Mbr mbr = adapter->getController()->getQuadtree()->generateMbrForNode(WhirlyKit::Quadtree::Identifier(x,y,level));
        
        ll->x() = mbr.ll().x();
        ll->y() = mbr.ll().y();
        ur->x() = mbr.ur().x();
        ur->y() = mbr.ur().y();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::boundsForTileNative()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadPagingLayer_nativeStartLayer
  (JNIEnv *env, jobject obj, jobject sceneObj, jobject rendererObj, jobject llObj, jobject urObj, jint minZoom, jint maxZoom)
{
	try
	{
		QPLAdapterClassInfo *classInfo = QPLAdapterClassInfo::getClassInfo();
		QuadPagingLayerAdapter *adapter = classInfo->getObject(env,obj);
		Point2d *ll = Point2dClassInfo::getClassInfo()->getObject(env,llObj);
		Point2d *ur = Point2dClassInfo::getClassInfo()->getObject(env,urObj);
		Scene *scene = SceneClassInfo::getClassInfo()->getObject(env,sceneObj);
		SceneRendererES *renderer = (SceneRendererES *)MaplySceneRendererInfo::getClassInfo()->getObject(env,rendererObj);
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadPagingLayer_nativeShutdown
  (JNIEnv *env, jobject obj, jobject changesObj)
{
	try
	{
		QPLAdapterClassInfo *classInfo = QPLAdapterClassInfo::getClassInfo();
		QuadPagingLayerAdapter *adapter = classInfo->getObject(env,obj);
		ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
		if (!adapter || !changes)
			return;

		adapter->getController()->shutdown(*changes);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::nativeShutdown()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadPagingLayer_nativeViewUpdate
  (JNIEnv *env, jobject obj, jobject viewStateObj)
{
	try
	{
		QuadPagingLayerAdapter *adapter = QPLAdapterClassInfo::getClassInfo()->getObject(env,obj);
		ViewState *viewState = ViewStateClassInfo::getClassInfo()->getObject(env,viewStateObj);
		if (!adapter || !viewState)
			return;

		adapter->env = env;
		adapter->javaObj = obj;
		// Look for the wrapper object's methods
		jclass theClass = env->GetObjectClass(obj);
		adapter->tileLoadJava = env->GetMethodID(theClass,"loadTile","(III)V");
		adapter->tileUnloadJava = env->GetMethodID(theClass,"unloadTile","(III)V");
        env->DeleteLocalRef(theClass);

		adapter->getController()->viewUpdate(viewState);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::nativeViewUpdate()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_QuadPagingLayer_nativeEvalStep
  (JNIEnv *env, jobject obj, jobject changesObj)
{
	try
	{
		QuadPagingLayerAdapter *adapter = QPLAdapterClassInfo::getClassInfo()->getObject(env,obj);
		ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
		if (!adapter || !changes)
			return false;

		adapter->env = env;
		adapter->javaObj = obj;
		// Look for the wrapper object's methods
		jclass theClass = env->GetObjectClass(obj);
		adapter->tileLoadJava = env->GetMethodID(theClass,"loadTile","(III)V");
		adapter->tileUnloadJava = env->GetMethodID(theClass,"unloadTile","(III)V");
        env->DeleteLocalRef(theClass);

		// Note: Not passing in frame boundary info
		return adapter->getController()->evalStep(0.0,0.0,0.0,*changes);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::nativeEvalStep()");
	}
    
    return false;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_QuadPagingLayer_nativeRefresh
  (JNIEnv *env, jobject obj, jobject changesObj)
{
	try
	{
		QuadPagingLayerAdapter *adapter = QPLAdapterClassInfo::getClassInfo()->getObject(env,obj);
		ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
		if (!adapter || !changes)
			return false;

		if (adapter->getController()->getWaitForLocalLoads())
			return false;

		adapter->getController()->refresh(*changes);
		return true;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::nativeRefresh()");
	}
    
    return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadPagingLayer_nativeTileDidLoad
  (JNIEnv *env, jobject obj, jint x, jint y, jint level)
{
	try
	{
		QuadPagingLayerAdapter *adapter = QPLAdapterClassInfo::getClassInfo()->getObject(env,obj);
		if (!adapter)
			return;

		adapter->numFetches--;
	    adapter->getController()->tileDidLoad(Quadtree::Identifier(x,y,level),-1);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::nativeTileDidLoad()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadPagingLayer_nativeTileDidNotLoad
  (JNIEnv *env, jobject obj, jint x, jint y, jint level)
{
	try
	{
		QuadPagingLayerAdapter *adapter = QPLAdapterClassInfo::getClassInfo()->getObject(env,obj);
		if (!adapter)
			return;

		adapter->numFetches--;
	    adapter->getController()->tileDidNotLoad(Quadtree::Identifier(x,y,level),-1);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadPagingLayer::nativeTileDidNotLoad()");
	}
}

