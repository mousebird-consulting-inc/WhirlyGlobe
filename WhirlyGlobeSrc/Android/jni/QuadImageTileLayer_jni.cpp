/*
 *  QuadImageTileLayer_jni.cpp
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

#import <math.h>
#import <jni.h>
#import <android/bitmap.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_QuadImageTileLayer.h"
#import "WhirlyGlobe.h"
#import "ImageWrapper.h"

using namespace WhirlyKit;

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
	double minVis, maxVis;
	bool handleEdges, coverPoles;
	Point2d ll,ur;
	float shortCircuitImportance;
	QuadTileLoader *tileLoader;
	QuadDisplayController *control;
	int drawPriority;
	int imageDepth;
	int borderTexel;
	int textureAtlasSize;
	bool enable;
	float fade;
	RGBAColor color;
	int imageFormat;
	float currentImage;
	bool animationWrap;
	int maxCurrentImage;
	bool allowFrameLoading;
	std::vector<int> framePriorities;
	float animationPeriod;
	int maxTiles;
	float importanceScale;
	int tileSize;
	std::vector<int> levelLoads;
	ViewState *lastViewState;
        std::string shaderName;
        SimpleIdentity shaderID;
    JavaVM* jvm;

	// Methods for Java quad image layer
    jmethodID startFetchJava,scheduleEvalStepJava;

	QuadImageLayerAdapter(CoordSystem *coordSys)
		: env(NULL), javaObj(NULL), renderer(NULL), coordSys(coordSys),
		  simultaneousFetches(1), tileLoader(NULL), minVis(0.0), maxVis(10.0),
		  handleEdges(true),coverPoles(false), drawPriority(0),imageDepth(1),
		  borderTexel(0),textureAtlasSize(2048),enable(true),fade(1.0),color(255,255,255,255),imageFormat(0),
		  currentImage(0.0), animationWrap(true), maxCurrentImage(-1), allowFrameLoading(true), animationPeriod(10.0),
		  maxTiles(256), importanceScale(1.0), tileSize(256), lastViewState(NULL), shaderID(EmptyIdentity), scene(NULL), control(NULL),scheduleEvalStepJava(0)
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

	// Get Java methods for a particular instance
	void setJavaRefs(JNIEnv *inEnv,jobject obj)
	{
        inEnv->GetJavaVM(&jvm);
        
        env = inEnv;
		javaObj = (jobject)env->NewGlobalRef(obj);
		jclass theClass = env->GetObjectClass(javaObj);
		startFetchJava = env->GetMethodID(theClass,"startFetch","(IIII)V");
		scheduleEvalStepJava = env->GetMethodID(theClass,"scheduleEvalStep","()V");
        env->DeleteLocalRef(theClass);
	}

	void clearJavaRefs()
	{
		if (javaObj)
			env->DeleteGlobalRef(javaObj);
	}

	// Set up the tile loading
	QuadTileLoader *setupTileLoader()
	{
		// Set up the tile loader
	  tileLoader = new QuadTileLoader("Image Layer",this,(imageDepth > 1 ? imageDepth : -1));
	    tileLoader->setIgnoreEdgeMatching(!handleEdges);
	    tileLoader->setCoverPoles(coverPoles);
	    tileLoader->setMinVis(minVis);
	    tileLoader->setMaxVis(maxVis);
	    tileLoader->setDrawPriority(drawPriority);
	    tileLoader->setNumImages(imageDepth);
	    tileLoader->setIncludeElev(false);
	    tileLoader->setBorderTexel(borderTexel);
	    tileLoader->setBorderPixelFudge(0.5);
	    tileLoader->setTextureAtlasSize(textureAtlasSize);
	    ChangeSet changes;
	    tileLoader->setEnable(enable,changes);
//	    tileLoader->setFade(fade,changes);
	    tileLoader->setUseTileCenters(false);
	    switch (imageFormat)
	    {
//        case MaplyImageIntRGBA:
//        case MaplyImage4Layer8Bit:
	    case 0:
	    case 16:
        default:
            tileLoader->setImageType(WKTileIntRGBA);
            break;
//        case MaplyImageUShort565:
        case 1:
            tileLoader->setImageType(WKTileUShort565);
            break;
//        case MaplyImageUShort4444:
        case 2:
            tileLoader->setImageType(WKTileUShort4444);
            break;
//        case MaplyImageUShort5551:
        case 3:
            tileLoader->setImageType(WKTileUShort5551);
            break;
//        case MaplyImageUByteRed:
        case 4:
            tileLoader->setImageType(WKTileUByteRed);
            break;
//        case MaplyImageUByteGreen:
        case 5:
            tileLoader->setImageType(WKTileUByteGreen);
            break;
//        case MaplyImageUByteBlue:
        case 6:
            tileLoader->setImageType(WKTileUByteBlue);
            break;
//        case MaplyImageUByteAlpha:
        case 7:
            tileLoader->setImageType(WKTileUByteAlpha);
            break;
//        case MaplyImageUByteRGB:
        case 8:
            tileLoader->setImageType(WKTileUByteRGB);
            break;
//        case MaplyImageETC2RGB8:
        case 9:
            tileLoader->setImageType(WKTileETC2_RGB8);
            break;
//        case MaplyImageETC2RGBA8:
        case 10:
            tileLoader->setImageType(WKTileETC2_RGBA8);
            break;
//        case MaplyImageETC2RGBPA8:
        case 11:
            tileLoader->setImageType(WKTileETC2_RGB8_PunchAlpha);
            break;
//        case MaplyImageEACR11:
        case 12:
            tileLoader->setImageType(WKTileEAC_R11);
            break;
//        case MaplyImageEACR11S:
        case 13:
            tileLoader->setImageType(WKTileEAC_R11_Signed);
            break;
//        case MaplyImageEACRG11:
        case 14:
            tileLoader->setImageType(WKTileEAC_RG11);
            break;
//        case MaplyImageEACRG11S:
        case 15:
            tileLoader->setImageType(WKTileEAC_RG11_Signed);
            break;
	    }
	    tileLoader->setColor(color);

	    // This will force the shader setup
	    if (!shaderName.empty())
	      setShaderName(shaderName);
	    setCurrentImage(currentImage,changes);

	    return tileLoader;
	}

	// Called to start the layer
	void start(Scene *inScene,SceneRendererES *inRenderer,const Point2d &inLL,const Point2d &inUR,int inMinZoom,int inMaxZoom,int inPixelsPerSide)
	{
		scene = inScene;
		renderer = inRenderer;
        ll = inLL;  ur = inUR;  minZoom = inMinZoom;  maxZoom = inMaxZoom; tileSize = inPixelsPerSide;

		tileLoader = setupTileLoader();

		// Set up the display controller
		control = new QuadDisplayController(this,tileLoader,this);
		// Note: Porting  Should turn this back on
		control->setMeteredMode(false);
		control->setFrameLoading(allowFrameLoading);
		control->init(scene,renderer);
		if (!framePriorities.empty())
			control->setFrameLoadingPriorities(framePriorities);
		control->setMaxTiles(maxTiles);

		// Note: Porting  Set up the shader

		// Note: Porting  Move this everywhere
		if (this->useTargetZoomLevel)
		{
			shortCircuitImportance = 256*256;
			control->setMinImportance(1.0);
		} else {
			shortCircuitImportance = 0.0;
			control->setMinImportance(1.0);
		}

	    if (imageDepth > 1)
	    {
	        if (animationWrap && maxCurrentImage == -1)
	            maxCurrentImage = imageDepth;

	        if (shaderID == EmptyIdentity)
	            shaderID = scene->getProgramIDByName(kToolkitDefaultTriangleMultiTex);

//	        if (animationPeriod > 0.0)
//	        	setAnimationPeriod(animationPeriod);
	    }
	}

	void setAnimationPeriod(float newAnimationPeriod)
	{
	    animationPeriod = newAnimationPeriod;
	}

        void setShaderName(const std::string &newName)
        {
	  shaderName = newName;
	  if (scene && tileLoader)
	  {
	      shaderID = scene->getProgramIDBySceneName(shaderName.c_str());
	      tileLoader->setProgramId(shaderID);
	  }
        }    

	// Change which image is displayed (or interpolation thereof)
	void setCurrentImage(float newCurrentImage,ChangeSet &changes)
	{
		currentImage = newCurrentImage;

	    unsigned int image0 = floorf(currentImage);
	    unsigned int image1 = ceilf(currentImage);
	    if (animationWrap)
	    {
	        if (image1 == imageDepth)
	            image1 = 0;
	    }
	    if (image0 >= imageDepth)
	        image0 = imageDepth-1;
	    if (image1 >= imageDepth)
	        image1 = -1;
	    float t = currentImage-image0;

	//    NSLog(@"currentImage = %d->%d -> %f",image0,image1,t);

	    // Change the images to give us start and finish
	    if (tileLoader)
	    	tileLoader->setCurrentImageStart(image0,image1,changes);
        
//        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "image0 = %d, image1 = %d, u_interp = %f",image0,image1,t);

        
        changes.push_back(new SetProgramValueReq(shaderID,"u_interp",t));
	}
    
    void setColor(const RGBAColor &newColor)
    {
        color = newColor;
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
        // We fake anything else
        return 0;
    }

    /// Return the maximum quad tree zoom level.  Must be at least minZoom
    virtual int getMaxZoom()
    {
    	return maxZoom;
    }

    /// Return an importance value for the given tile
    virtual double importanceForTile(const Quadtree::Identifier &ident,const Mbr &mbr,ViewState *viewState,const Point2f &frameSize,Dictionary *attrs)
    {
        if (ident.level == 0)
            return MAXFLOAT;

        Quadtree::Identifier tileID;
        tileID.level = ident.level;
        tileID.x = ident.x;
        tileID.y = ident.y;

        // Note: Porting.  Valid tile support.
//        if (canDoValidTiles && ident.level >= minZoom)
//        {
//            MaplyBoundingBox bbox;
//            bbox.ll.x = mbr.ll().x();  bbox.ll.y = mbr.ll().y();
//            bbox.ur.x = mbr.ur().x();  bbox.ur.y = mbr.ur().y();
//            if (![_tileSource validTile:tileID bbox:&bbox])
//                return 0.0;
//        }

        // Note: Porting.  Variable size tiles.
        int thisTileSize = tileSize;
//        if (variableSizeTiles)
//        {
//            thisTileSize = [_tileSource tileSizeForTile:tileID];
//            if (thisTileSize <= 0)
//                thisTileSize = [_tileSource tileSize];
//        }

        double import = 0.0;
        if (canShortCircuitImportance && maxShortCircuitLevel != -1)
        {
            if (TileIsOnScreen(viewState, frameSize, coordSys, scene->getCoordAdapter(), mbr, ident, attrs))
            {
                import = 1.0/(ident.level+10);
                if (ident.level <= maxShortCircuitLevel)
                    import += 1.0;
            }
            import *= importanceScale;
        } else {
        	// Note: Porting
//            if (elevDelegate)
//            {
//                import = ScreenImportance(viewState, frameSize, thisTileSize, coordSys, scene->getCoordAdapter(), mbr, _minElev, _maxElev, ident, attrs);
//            } else {
        	    import = ScreenImportance(viewState, frameSize, viewState->eyeVec, thisTileSize, coordSys, scene->getCoordAdapter(), mbr, ident, attrs);
//            }
            import *= importanceScale;
        }

//		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Tile = %d: (%d,%d), import = %f",ident.level,ident.x,ident.y,import);

        return import;
    }

    // Calculate a target zoom level for display
    int targetZoomLevel(ViewState *viewState)
    {
        if (!viewState)
            return minZoom;
        Point2f frameSize = renderer->getFramebufferSize();

        int zoomLevel = 0;
        WhirlyKit::Point3d center3d = scene->getCoordAdapter()->displayToLocal(Point3d(viewState->eyePos.x(),viewState->eyePos.y(),0.0));
        Point2f center(center3d.x(),center3d.y());
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
//    	__android_log_print(ANDROID_LOG_VERBOSE, "newViewState", "Got new view state");

    	lastViewState = viewState;

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
                for (int whichLevel : levelLoads)
                {
					if (whichLevel < 0)
						whichLevel = maxShortCircuitLevel+whichLevel;
					if (whichLevel >= 0 && whichLevel < maxShortCircuitLevel)
						targetLevels.insert(whichLevel);
                }
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
//   		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Asking for tile: %d : (%d,%d)",level,col,row);

    	env->CallVoidMethod(javaObj, startFetchJava, level, col, row, frame);
    }

    /// The tile loaded correctly (or didn't if it's null)
    void tileLoaded(int level,int col,int row,int frame,RawDataRef imgData,int width,int height,ChangeSet &changes)
    {
    	if (imgData)
    	{
    		ImageWrapper tileWrapper(imgData,width,height);
    		tileLoader->loadedImage(this, &tileWrapper, level, col, row, frame, changes);
    	} else {
            if (level < minZoom)
            {
                // This is meant to be a placeholder
                ImageWrapper placeholder;
                tileLoader->loadedImage(this, &placeholder, level, col, row, frame, changes);
            } else
                tileLoader->loadedImage(this, NULL, level, col, row, frame, changes);
    	}
    }

    /// The tile loaded correctly (or didn't if it's null)
    void tileLoaded(int level,int col,int row,int frame,std::vector<RawDataRef> &imgData,int width,int height,ChangeSet &changes)
    {
        std::vector<LoadedImage *> images(imgData.size());
        for (unsigned int ii=0;ii<imgData.size();ii++)
            images[ii] = new ImageWrapper(imgData[ii],width,height);
        tileLoader->loadedImages(this, images, level, col, row, frame, changes);
        for (auto wrap : images)
            delete wrap;
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
//      __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Tile did unload: %d: (%d,%d)",level,col,row);
        scheduleEvalStep();
    }

    // QuadDisplayControllerAdapter related methods

    // Call back to the java side to make it schedule itself (on the right thread).
    // We have no idea what thread we may be called on here.
    // There are callbacks that come from the scene side of things, so beware.
    void scheduleEvalStep()
    {
        if (scheduleEvalStepJava)
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
        // Let's be careful to get the right env for this thread
        JNIEnv *thisEnv = NULL;
        jvm->GetEnv((void **)&thisEnv, JNI_VERSION_1_6);
        
        if (thisEnv)
            thisEnv->CallVoidMethod(javaObj, scheduleEvalStepJava);
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setEnable
  (JNIEnv *env, jobject obj, jboolean enable, jobject changeSetObj)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!adapter || !changeSet || !adapter->tileLoader)
			return;
		ChangeSet changes;
//        adapter->env = env;
		adapter->tileLoader->setEnable(enable,*changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setEnable()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setDrawPriority
  (JNIEnv *env, jobject obj, jint drawPriority)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return;
		ChangeSet changes;
		adapter->drawPriority = drawPriority;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setDrawPriority()");
	}
}


JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setImageDepth
  (JNIEnv *env, jobject obj, jint imageDepth)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return;
		adapter->imageDepth = imageDepth;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setImageDepth()");
	}
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadImageTileLayer_getImageDepth
(JNIEnv *env, jobject obj)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
        if (!adapter)
            return 1;
        return adapter->imageDepth;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::getImageDepth()");
    }
    
    return 1;
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_QuadImageTileLayer_getCurrentImage
  (JNIEnv *env, jobject obj)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return 0.0;

		return adapter->currentImage;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::getCurrentImage()");
	}
    
    return 0.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setCurrentImage
  (JNIEnv *env, jobject obj, jfloat currentImage, jobject changeSetObj)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!adapter || !changeSet)
			return;

//        adapter->env = env;
		adapter->setCurrentImage(currentImage,*changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setCurrentImage()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setMaxCurrentImage
  (JNIEnv *env, jobject obj, jfloat maxCurrentImage)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return;
		adapter->maxCurrentImage = maxCurrentImage;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setMaxCurrentImage()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setAnimationPeriod
  (JNIEnv *env, jobject obj, jfloat period)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return;
//        adapter->env = env;
		adapter->setAnimationPeriod(period);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setAnimationPeriod()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setAnimationWrap
  (JNIEnv *env, jobject obj, jboolean animationWrap)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return;
		adapter->animationWrap = animationWrap;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setAnimationWrap()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setAllowFrameLoading
  (JNIEnv *env, jobject obj, jboolean frameLoading)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return;
		adapter->allowFrameLoading = frameLoading;
        if (adapter->control)
        {
//            adapter->env = env;
            adapter->control->setFrameLoading(frameLoading);
            adapter->scheduleEvalStep();
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setAllowFrameLoading()");
	}
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadImageTileLayer_getFrameStatusNative
(JNIEnv *env, jobject obj, jbooleanArray completeArray, jintArray tilesLoadedArray)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
        if (!adapter || !adapter->control)
            return -1;
        
        std::vector<WhirlyKit::FrameLoadStatus> frameLoadStatus;
        adapter->control->getFrameLoadStatus(frameLoadStatus);
        
        if (frameLoadStatus.size() != adapter->imageDepth)
            return -1;
        
        JavaBooleanArray complete(env,completeArray);
        JavaIntArray tilesLoaded(env,tilesLoadedArray);
        int whichFrame = -1;
        int ii = 0;
        for (const FrameLoadStatus &frameStatus : frameLoadStatus)
        {
            complete.rawBool[ii] = frameStatus.complete;
            tilesLoaded.rawInt[ii] = frameStatus.numTilesLoaded;
            if (frameStatus.currentFrame)
                whichFrame = ii;
            ii++;
        }
        
        return whichFrame;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::getFrameStatus()");
    }
    
    return -1;
}


JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setFrameLoadingPriority
  (JNIEnv *env, jobject obj, jintArray frameLoadingArr, jobject changeSetObj)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!adapter || !frameLoadingArr || !changeSet)
			return;
//        adapter->env = env;
		adapter->framePriorities.clear();

		ConvertIntArray(env,frameLoadingArr,adapter->framePriorities);
        if (adapter->control)
        {
            adapter->control->setFrameLoadingPriorities(adapter->framePriorities);
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setFrameLoadingPriority()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setColor
  (JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a, jobject changeSetObj)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!adapter || !changeSet)
			return;
//        adapter->env = env;
		adapter->setColor(RGBAColor(r*255.0,g*255.0,b*255.0,a*255.0));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setColor()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setMaxTiles
  (JNIEnv *env, jobject obj, jint maxTiles)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return;
		adapter->maxTiles = maxTiles;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setMaxTiles()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setImportanceScale
  (JNIEnv *env, jobject obj, jfloat scale)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return;
		adapter->importanceScale = scale;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setImportanceScale()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setTextureAtlasSize
  (JNIEnv *env, jobject obj, jint atlasSize)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return;
		adapter->textureAtlasSize = atlasSize;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setTextureAtlasSize()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setImageFormat
  (JNIEnv *env, jobject obj, jint imageFormat)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return;
		adapter->imageFormat = imageFormat;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setImageFormat()");
	}
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadImageTileLayer_getBorderTexel
  (JNIEnv *env, jobject obj)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return 0;
		return adapter->borderTexel;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::getBorderTexel()");
	}
    
    return 0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setBorderTexel
  (JNIEnv *env, jobject obj, jint borderTexel)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return;
		adapter->borderTexel = borderTexel;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setBorderTexel()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setMultiLevelLoads
  (JNIEnv *env, jobject obj, jintArray levelLoadsArr)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter)
			return;

//        adapter->env = env;
		ConvertIntArray(env,levelLoadsArr,adapter->levelLoads);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setMultiLevelLoads()");
	}
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadImageTileLayer_getTargetZoomLevel
  (JNIEnv *env, jobject obj)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		if (!adapter || !adapter->lastViewState)
			return 0;

//        adapter->env = env;
		return adapter->targetZoomLevel(adapter->lastViewState);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::getTargetZoomLevel()");
	}
    
    return 0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setShaderName
(JNIEnv *env, jobject obj, jstring shaderName)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
        if (!adapter)
            return;
//        adapter->env = env;
        
        const char *cName = env->GetStringUTFChars(shaderName,0);
        std::string name = cName;

        adapter->setShaderName(cName);

        env->ReleaseStringUTFChars(shaderName, cName);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setShaderName()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_reload
  (JNIEnv *env, jobject obj, jobject changeSetObj)
{
	try
	{
		QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
		QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
		ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!adapter || !changeSet)
			return;
//        adapter->env = env;

		// Note: Porting. This doesn't handle the case where we change parameters and then reload
		adapter->control->refresh(*changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::reload()");
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setHandleEdges
  (JNIEnv *env, jobject obj, jboolean newVal)
{
	QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
	QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
	if (!adapter)
		return;
	adapter->handleEdges = newVal;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setCoverPoles
  (JNIEnv *env, jobject obj, jboolean newVal)
{
	QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
	QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
	if (!adapter)
		return;
	adapter->coverPoles = newVal;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_setVisibility
  (JNIEnv *env, jobject obj, jdouble minVis, jdouble maxVis)
{
	QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
	QuadImageLayerAdapter *adapter = classInfo->getObject(env,obj);
	if (!adapter)
		return;
	adapter->minVis = minVis;
	adapter->maxVis = maxVis;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_nativeStartLayer
  (JNIEnv *env, jobject obj, jobject sceneObj, jobject rendererObj, jobject llObj, jobject urObj, jint minZoom, jint maxZoom, jint pixelsPerSide)
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

		adapter->start(scene,renderer,*ll,*ur,minZoom,maxZoom,pixelsPerSide);
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
//        adapter->env = env;
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

//		adapter->env = env;

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

//		adapter->env = env;

		// Note: Not passing in frame boundary info
		return adapter->control->evalStep(0.0,0.0,0.0,*changes);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::nativeEvalStep()");
	}
    
    return false;
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

//        adapter->env = env;
		adapter->control->refresh(*changes);
		return true;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::nativeRefresh()");
	}
    
    return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_nativeTileDidLoad__IIIILandroid_graphics_Bitmap_2Lcom_mousebird_maply_ChangeSet_2
  (JNIEnv *env, jobject obj, jint x, jint y, jint level, jint frame, jobject bitmapObj, jobject changesObj)
{
	try
	{
		QuadImageLayerAdapter *adapter = QILAdapterClassInfo::getClassInfo()->getObject(env,obj);
		ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
		if (!adapter || !changes)
		  {
			return;
		  }
//        adapter->env = env;

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
        
        if (info.height > 0 && info.width > 0)
        {
            uint32_t* src = (uint32_t*) bitmapPixels;
            RawDataRef rawDataRef(new MutableRawData(bitmapPixels,info.height*info.width*4));

            adapter->tileLoaded(level,x,y,frame,rawDataRef,info.width,info.height,*changes);
        }
        
		AndroidBitmap_unlockPixels(env, bitmapObj);
//		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Tile did load: %d: (%d,%d) %d",level,x,y,frame);
    }
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::nativeTileDidLoad()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_nativeTileDidLoad__IIII_3Landroid_graphics_Bitmap_2Lcom_mousebird_maply_ChangeSet_2
(JNIEnv *env, jobject obj, jint x, jint y, jint level, jint frame, jobjectArray bitmapsObj, jobject changesObj)
{
    try
    {
        QuadImageLayerAdapter *adapter = QILAdapterClassInfo::getClassInfo()->getObject(env,obj);
        ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
        if (!adapter || !changes || !bitmapsObj)
        {
            return;
        }
//        adapter->env = env;
        
        int numImages = env->GetArrayLength(bitmapsObj);
        int width,height;
        std::vector<RawDataRef> images;
        images.reserve(numImages);
        for (int ii=0;ii<numImages;ii++)
        {
            jobject bitmapObj = env->GetObjectArrayElement(bitmapsObj,ii);

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
            
            if (info.height > 0 && info.width > 0)
            {
                uint32_t* src = (uint32_t*) bitmapPixels;
                RawDataRef rawData(new MutableRawData(bitmapPixels,info.height*info.width*4));
                images.push_back(rawData);
                width = info.width;  height = info.height;
            } else
                __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Bad image in QuadImageTileLayer::nativeTileDidLoad2()");
            
            AndroidBitmap_unlockPixels(env, bitmapObj);
        }

        adapter->tileLoaded(level,x,y,frame,images,width,height,*changes);

        //		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Tile did load: %d: (%d,%d) %d",level,x,y,frame);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::nativeTileDidLoad2()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageTileLayer_nativeTileDidNotLoad
  (JNIEnv *env, jobject obj, jint x, jint y, jint level, jint frame, jobject changesObj)
{
	try
	{
		QuadImageLayerAdapter *adapter = QILAdapterClassInfo::getClassInfo()->getObject(env,obj);
		ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
		if (!adapter || !changes)
			return;
//        adapter->env = env;

//		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Tile did not load: %d: (%d,%d) %d",level,x,y,frame);
		adapter->tileLoaded(level,x,y,frame,RawDataRef(),1,1,*changes);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::nativeTileDidLoad()");
	}
}
