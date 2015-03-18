/*
 *  QuadImageTileLayer_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2014 mousebird consulting
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
#import <android/bitmap.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_QuadImageTileLayer.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

// Interfaces between our image and what the toolkit is expecting
class ImageWrapper : public LoadedImage
{
public:
	ImageWrapper(RawDataRef rawData,int width,int height)
		: rawData(rawData), width(width), height(height)
	{
	}

	// Construct the texture
	// Note: Need to handle borderSize
    virtual Texture *buildTexture(int borderSize,int width,int height)
    {
    	// Test code.  Off by default, obviously
//    	for (unsigned int ii=0;ii<width*height;ii++)
//    		((unsigned int *)rawData->getRawData())[ii] = 0xff0000ff;

    	Texture *tex = new Texture("Tile Quad Loader",rawData,false);
    	tex->setWidth(width);
    	tex->setHeight(height);
    	return tex;
    }

    /// This means there's nothing to display, but the children are valid
    virtual bool isPlaceholder()
    {
        return false;
    }

    /// Return image width
    virtual int getWidth()
    {
    	return width;
    }

    /// Return image height
    virtual int getHeight()
    {
    	return height;
    }

    int width,height;
    RawDataRef rawData;
};

class QuadImageLayerAdapter : public QuadDataStructure, public QuadTileImageDataSource, public QuadDisplayControllerAdapter
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

	JNIEnv *env;
	jobject javaObj;
	Scene *scene;
	SceneRendererES *renderer;
	CoordSystem *coordSys;
	int minZoom,maxZoom;
	int simultaneousFetches;
	bool useTargetZoomLevel;
	bool singleLevelLoading;
	bool canShortCircuitImportance;
	int maxShortCircuitLevel;
	Point2d ll,ur;

	// Methods for Java quad image layer
	jmethodID startFetchJava,scheduleEvalStepJava;

	QuadImageLayerAdapter(CoordSystem *coordSys)
		: env(NULL), javaObj(NULL), renderer(NULL), coordSys(coordSys),
		  simultaneousFetches(1), tileLoader(NULL)
	{
		useTargetZoomLevel = true;
        canShortCircuitImportance = false;
        singleLevelLoading = false;
        maxShortCircuitLevel = -1;
	}

	virtual ~QuadImageLayerAdapter()
	{
		if (coordSys)
			delete coordSys;
	}

	float shortCircuitImportance;
	QuadTileLoader *tileLoader;
	QuadDisplayController *control;

	// Get Java methods for a particular instance
	void setJavaRefs(JNIEnv *env,jobject obj)
	{
		// Note: Should release this somewhere
		javaObj = (jobject)env->NewGlobalRef(obj);
		jclass theClass = env->GetObjectClass(javaObj);
		startFetchJava = env->GetMethodID(theClass,"startFetch","(III)V");
		scheduleEvalStepJava = env->GetMethodID(theClass,"scheduleEvalStep","()V");
	}

	void clearJavaRefs()
	{
		if (javaObj)
			env->DeleteGlobalRef(javaObj);
	}

	// Called to star the layer
	void start(Scene *inScene,SceneRendererES *inRenderer,const Point2d &inLL,const Point2d &inUR,int inMinZoom,int inMaxZoom)
	{
		scene = inScene;
		renderer = inRenderer;
		ll = inLL;  ur = inUR;  minZoom = inMinZoom;  maxZoom = inMaxZoom;

		// Set up the tile loader
		tileLoader = new QuadTileLoader("Image Layer",this,-1);
	    tileLoader->setIgnoreEdgeMatching(false);
	    tileLoader->setCoverPoles(false);
//	    tileLoader->setMinVis(_minVis);
//	    tileLoader->setMaxVis(_maxVis);
	    tileLoader->setDrawPriority(0);
	    tileLoader->setNumImages(1);
	    tileLoader->setIncludeElev(false);
	    tileLoader->setBorderTexel(0);
	    tileLoader->setBorderPixelFudge(0.5);
	    // Note: Should check this
	    tileLoader->setTextureAtlasSize(2048);
//	    ChangeSet changes;
//	    tileLoader->setEnable(_enable,changes);

		// Set up the display controller
		control = new QuadDisplayController(this,tileLoader,this);
		control->setMaxTiles(256);
		control->setMeteredMode(false);
		control->init(scene,renderer);

		// Note: Porting
		// Note: Explicitly setting the min importance for a 128*128 tile
//		getController()->setMinImportance(128*128);
		if (this->useTargetZoomLevel)
		{
			shortCircuitImportance = 256*256;
			control->setMinImportance(1.0);
		} else {
			shortCircuitImportance = 0.0;
			control->setMinImportance(256*256);
		}
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
        if (ident.level < 1)
            return MAXFLOAT;

        double import = 0;
        if (canShortCircuitImportance && maxShortCircuitLevel != -1)
        {
            if (TileIsOnScreen(viewState, frameSize, coordSys, scene->getCoordAdapter(), mbr, ident, attrs))
            {
                import = 1.0/(ident.level+10);
                if (ident.level <= maxShortCircuitLevel)
                	import += 1.0;
            }
        } else {
			// This is how much screen real estate we're covering for this tile
			import = ScreenImportance(viewState, frameSize, viewState->eyeVec, 1, coordSys, scene->getCoordAdapter(), mbr, ident, attrs) / 4;
        }
//		__android_log_print(ANDROID_LOG_VERBOSE, "importanceForTile", "tile %d: (%d,%d) import = %f",ident.level,ident.x,ident.y,import);

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

        while (zoomLevel < maxZoom)
        {
            WhirlyKit::Quadtree::Identifier ident;
            ident.x = 0;  ident.y = 0;  ident.level = zoomLevel;
            // Make an MBR right in the middle of where we're looking
            Mbr mbr = control->getQuadtree()->generateMbrForNode(ident);
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

        return zoomLevel;
    }

    /// Called when the view state changes.  If you're caching info, do it here.
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
            if (!coordAdapter->isFlat())
            {
                canShortCircuitImportance = false;
                return;
            }
            // We happen to store tilt in the view matrix.
            // Note: Porting
//            if (!viewState->viewMatrix.isIdentity())
//            {
//                canShortCircuitImportance = false;
//                return;
//            }
            // The tile source coordinate system must be the same as the display's system
            if (!coordSys->isSameAs(coordAdapter->getCoordSystem()))
            {
                canShortCircuitImportance = false;
                return;
            }

            // We need to feel our way down to the appropriate level
            maxShortCircuitLevel = targetZoomLevel(viewState);
            if (singleLevelLoading)
            {
            	std::set<int> targetLevels;
            	targetLevels.insert(maxShortCircuitLevel);
            	control->setTargetLevels(targetLevels);
            }

//    		__android_log_print(ANDROID_LOG_VERBOSE, "newViewState", "Short circuiting to level %d",maxShortCircuitLevel);

        } else {
            // Note: Can't short circuit in this case.  Something wrong with the math
            canShortCircuitImportance = false;
        }
    }

    /// QuadDataStructure shutdown
    virtual void shutdown()
    {
    }

    /// Called when the layer is shutting down.  Clean up any drawable data and clear out caches.
    virtual void shutdownLayer(ChangeSet &changes)
    {
    	if (control)
    	{
    		control->shutdown(changes);

    		delete control;
    		control = NULL;
    	}
    }

    virtual int maxSimultaneousFetches()
    {
    	return simultaneousFetches;
    }

    /// The quad loader is letting us know to start loading the image.
    /// We'll call the loader back with the image when it's ready.
    virtual void startFetch(QuadTileLoaderSupport *quadLoader,int level,int col,int row,int frame,Dictionary *attrs)
    {
    	env->CallVoidMethod(javaObj, startFetchJava, level, col, row);
    }

    /// The tile loaded correctly (or didn't if it's null)
    void tileLoaded(int level,int col,int row,RawDataRef imgData,int width,int height,ChangeSet &changes)
    {
    	if (imgData)
    	{
    		ImageWrapper tileWrapper(imgData,width,height);
    		tileLoader->loadedImage(this, &tileWrapper, level, col, row, -1, changes);
    	} else {
    		tileLoader->loadedImage(this, NULL, level, col, row, -1, changes);
    	}
    }

    /// Check if the given tile is a local or remote fetch.  This is a hint
    ///  to the pager.  It can display local tiles as a group faster.
    virtual bool tileIsLocal(int level,int col,int row,int frame)
    {
    	return false;
    }

    // Callback letting us know a tile was removed
    void tileWasUnloaded(int level,int col,int row)
    {

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

    // QuadDisplayControllerAdapter related methods

    // Call back to the java side to make it schedule itself (on the right thread).
    // We have no idea what thread we may be called on here.
    // There are callbacks that come from the scene side of things, so beware.
    void scheduleEvalStep()
    {
    	env->CallVoidMethod(javaObj, scheduleEvalStepJava);
    }

    // Called right after a tile loaded
    virtual void adapterTileDidLoad(const Quadtree::Identifier &tileIdent)
    {
    	scheduleEvalStep();
    }

    // Called right after a tile unloaded
    virtual void adapterTileDidNotLoad(const Quadtree::Identifier &tileIdent)
    {
    	scheduleEvalStep();
    }

    // Wake the thread up
    virtual void adapterWakeUp()
    {
    	if (maplyCurrentEnv)
    		maplyCurrentEnv->CallVoidMethod(javaObj, scheduleEvalStepJava);
    }
};

typedef JavaClassInfo<QuadImageLayerAdapter> QILAdapterClassInfo;
template<> QILAdapterClassInfo *QILAdapterClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_nativeInit
  (JNIEnv *env, jclass cls)
{
	QILAdapterClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_initialise
  (JNIEnv *env, jobject obj, jobject coordSysObj, jobject changesObj)
{
	try
	{
		CoordSystem *coordSys = CoordSystemClassInfo::getClassInfo()->getObject(env,coordSysObj);
		ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
		if (!coordSys || !changes)
			return;

		QuadImageLayerAdapter *adapter = new QuadImageLayerAdapter(coordSys);
		QILAdapterClassInfo::getClassInfo()->setHandle(env,obj,adapter);
		adapter->setJavaRefs(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return;
		adapter->clearJavaRefs();
		delete adapter;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setSimultaneousFetches
  (JNIEnv *env, jobject obj, jint numFetches)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return;
		adapter->simultaneousFetches = numFetches;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setSimultaneousFetches()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setUseTargetZoomLevel
  (JNIEnv *env, jobject obj, jboolean newVal)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return;
		adapter->useTargetZoomLevel = newVal;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setUseTargetZoomLevel()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setSingleLevelLoading
  (JNIEnv *env, jobject obj, jboolean newVal)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return;
		adapter->singleLevelLoading = newVal;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setSingleLevelLoading()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_nativeStartLayer
  (JNIEnv *env, jobject obj, jobject sceneObj, jobject rendererObj, jobject llObj, jobject urObj, jint minZoom, jint maxZoom)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		Point2d *ll = Point2dClassInfo::getClassInfo()->getObject(env,llObj);
		Point2d *ur = Point2dClassInfo::getClassInfo()->getObject(env,urObj);
		Scene *scene = SceneClassInfo::getClassInfo()->getObject(env,sceneObj);
		SceneRendererES *renderer = (SceneRendererES *)MaplySceneRendererInfo::getClassInfo()->getObject(env,rendererObj);
		if (!adapter || !ll || !ur || !scene || !renderer)
			return;

		adapter->env = env;

		adapter->start(scene,renderer,*ll,*ur,minZoom,maxZoom);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::nativeStartLayer()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_nativeShutdown
  (JNIEnv *env, jobject obj, jobject changesObj)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
		if (!adapter || !changes)
			return;

		adapter->control->shutdown(*changes);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::nativeShutdown()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_nativeViewUpdate
  (JNIEnv *env, jobject obj, jobject viewStateObj)
{
	try
	{
		QuadImageLayerAdapter *adapter = QILAdapterClassInfo::getClassInfo()->getObject(env,obj);
		ViewState *viewState = ViewStateClassInfo::getClassInfo()->getObject(env,viewStateObj);
		if (!adapter || !viewState)
			return;

		adapter->env = env;

		adapter->control->viewUpdate(viewState);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::nativeViewUpdate()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_QuadImageTileLayer_nativeEvalStep
  (JNIEnv *env, jobject obj, jobject changesObj)
{
	try
	{
		QuadImageLayerAdapter *adapter = QILAdapterClassInfo::getClassInfo()->getObject(env,obj);
		ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
		if (!adapter || !changes)
			return false;

		adapter->env = env;

		// Note: Not passing in frame boundary info
		return adapter->control->evalStep(0.0,0.0,0.0,*changes);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::nativeEvalStep()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_QuadImageTileLayer_nativeRefresh
  (JNIEnv *env, jobject obj, jobject changesObj)
{
	try
	{
		QuadImageLayerAdapter *adapter = QILAdapterClassInfo::getClassInfo()->getObject(env,obj);
		ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
		if (!adapter || !changes)
			return false;

		if (adapter->control->getWaitForLocalLoads())
			return false;

		adapter->control->refresh(*changes);
		return true;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::nativeRefresh()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_nativeTileDidLoad
  (JNIEnv *env, jobject obj, jint x, jint y, jint level, jobject bitmapObj, jobject changesObj)
{
	try
	{
		QuadImageLayerAdapter *adapter = QILAdapterClassInfo::getClassInfo()->getObject(env,obj);
		ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
		if (!adapter || !changes)
			return;

		AndroidBitmapInfo info;
		if (AndroidBitmap_getInfo(env, bitmapObj, &info) < 0)
		{
		    return;
	    }
		if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
		{
			__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Only dealing with 8888 bitmaps in QuadImageTileLayer");
		    return;
	    }
		// Copy the raw data over to the texture
		void* bitmapPixels;
		if (AndroidBitmap_lockPixels(env, bitmapObj, &bitmapPixels) < 0)
	    {
		    return;
	    }

		uint32_t* src = (uint32_t*) bitmapPixels;
		RawDataRef rawDataRef(new MutableRawData(bitmapPixels,info.height*info.width*4));

		adapter->tileLoaded(level,x,y,rawDataRef,info.width,info.height,*changes);
		AndroidBitmap_unlockPixels(env, bitmapObj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::nativeTileDidLoad()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_nativeTileDidNotLoad
  (JNIEnv *env, jobject obj, jint x, jint y, jint level, jobject changesObj)
{
	try
	{
		QuadImageLayerAdapter *adapter = QILAdapterClassInfo::getClassInfo()->getObject(env,obj);
		ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
		if (!adapter || !changes)
			return;

		adapter->tileLoaded(level,x,y,RawDataRef(),-1,1,*changes);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::nativeTileDidLoad()");
	}
}
