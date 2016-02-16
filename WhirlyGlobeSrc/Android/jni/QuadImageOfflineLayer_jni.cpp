/*
 *  QuadImageTileLayer_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2015 mousebird consulting
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
#import "com_mousebird_maply_QuadImageOfflineLayer.h"
#import "WhirlyGlobe.h"
#import "ImageWrapper.h"

using namespace WhirlyKit;

class QuadImageOfflineLayerAdapter : public QuadDataStructure, public QuadTileImageDataSource, public QuadDisplayControllerAdapter, public QuadTileOfflineDelegate
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
    float shortCircuitImportance;
    QuadTileOfflineLoader *tileLoader;
    QuadDisplayController *control;
    int imageDepth;
    bool enable;
    float fade;
    RGBAColor color;
    bool allowFrameLoading;
    std::vector<int> framePriorities;
    int maxTiles;
    float importanceScale;
    int tileSize;
    std::vector<int> levelLoads;
    ViewState *lastViewState;
    
    // Methods for Java quad image layer
    jmethodID startFetchJava,scheduleEvalStepJava,imageRenderCallbackJava;
    
    QuadImageOfflineLayerAdapter(CoordSystem *coordSys)
    : env(NULL), javaObj(NULL), renderer(NULL), coordSys(coordSys),
		  simultaneousFetches(1), tileLoader(NULL),imageDepth(1),enable(true), allowFrameLoading(true),
		  maxTiles(256), importanceScale(1.0), tileSize(256), lastViewState(NULL), scene(NULL), control(NULL), fade(1.0),
    color(RGBAColor(255,255,255,255))
    {
        useTargetZoomLevel = true;
        canShortCircuitImportance = false;
        singleLevelLoading = false;
        maxShortCircuitLevel = -1;
    }
    
    virtual ~QuadImageOfflineLayerAdapter()
    {
        if (coordSys)
            delete coordSys;
    }
    
    // Get Java methods for a particular instance
    void setJavaRefs(JNIEnv *env,jobject obj)
    {
        javaObj = (jobject)env->NewGlobalRef(obj);
        jclass theClass = env->GetObjectClass(javaObj);
        startFetchJava = env->GetMethodID(theClass,"startFetch","(IIII)V");
        scheduleEvalStepJava = env->GetMethodID(theClass,"scheduleEvalStep","()V");
        imageRenderCallbackJava = env->GetMethodID(theClass,"imageRenderCallback","(JDDIII)V");
        env->DeleteLocalRef(theClass);
    }
    
    void clearJavaRefs()
    {
        if (javaObj)
            env->DeleteGlobalRef(javaObj);
    }
    
    // Set up the tile loading
    QuadTileOfflineLoader *setupTileLoader()
    {
        // Set up the tile loader
        tileLoader = new QuadTileOfflineLoader("Image Layer",this);
        tileLoader->setNumImages(imageDepth);
        ChangeSet changes;
        tileLoader->setOn(enable);
        tileLoader->setOutputDelegate(this);
        
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
        control->init(scene,renderer);
        control->setMaxTiles(maxTiles);
        if (!framePriorities.empty())
            control->setFrameLoadingPriorities(framePriorities);
        
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
        // We fake anything below this
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
        
//        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Tile = %d: (%d,%d), import = %f",ident.level,ident.x,ident.y,import);
        
        return import;
    }
    
    // Calculate a target zoom level for display
    int targetZoomLevel(ViewState *viewState)
    {
        if (!viewState || !renderer || !scene)
            return minZoom;
        Point2f frameSize = renderer->getFramebufferSize();
        
        CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
        
        int zoomLevel = 0;
        Point3d local3d = coordAdapter->displayToLocal(viewState->eyePos);
        if (std::isnan(local3d.x()) || std::isnan(local3d.y()) || std::isnan(local3d.z()))
            return minZoom;
        Point3d center3d = CoordSystemConvert3d(coordAdapter->getCoordSystem(),coordSys,coordAdapter->displayToLocal(viewState->eyePos));
        Point2f centerLocal(center3d.x(),center3d.y());
        
        // Bounding box in local coordinate system
        Point3d ll,ur;
        ll = coordSys->geographicToLocal3d(GeoCoord(-M_PI,-M_PI/2.0));
        ur = coordSys->geographicToLocal3d(GeoCoord(M_PI,M_PI/2));
        
        // The coordinate adapter might have its own center
        Point3d adaptCenter = coordAdapter->getCenter();
        centerLocal += Point2f(adaptCenter.x(),adaptCenter.y());
        while (zoomLevel <= maxZoom)
        {
            WhirlyKit::Quadtree::Identifier ident;
            Point2d thisTileSize((ur.x()-ll.x())/(1<<zoomLevel),(ur.y()-ll.y())/(1<<zoomLevel));
            ident.x = (centerLocal.x()-ll.x())/thisTileSize.x();
            ident.y = (centerLocal.y()-ll.y())/thisTileSize.y();
            ident.level = zoomLevel;
            
            // Make an MBR right in the middle of where we're looking
            Mbr mbr = control->getQuadtree()->generateMbrForNode(ident);
            Point2f span = mbr.ur()-mbr.ll();
            mbr.ll() = centerLocal - span/2.0;
            mbr.ur() = centerLocal + span/2.0;
            Dictionary attrs;
            float import = ScreenImportance(viewState, frameSize, viewState->eyeVec, tileSize, coordSys, coordAdapter, mbr, ident, &attrs);
            import *= importanceScale;
            if (import <= control->getMinImportance())
            {
                zoomLevel--;
                break;
            }
            zoomLevel++;
        }
        
        return std::max(std::min(zoomLevel,maxZoom),0);
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

            // We happen to store tilt in the view matrix.
            // Note: Porting
            //            if (!viewState->viewMatrix.isIdentity())
            //            {
            //                canShortCircuitImportance = false;
            //                return;
            //            }
            // The tile source coordinate system must be the same as the display's system
            
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
            
    		__android_log_print(ANDROID_LOG_VERBOSE, "newViewState", "Short circuiting to level %d",maxShortCircuitLevel);
            
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
    
    /// Check if the given tile is a local or remote fetch.  This is a hint
    ///  to the pager.  It can display local tiles as a group faster.
    virtual bool tileIsLocal(int level,int col,int row,int frame)
    {
        return false;
    }
    
    // Callback letting us know a tile was removed
    void tileWasUnloaded(int level,int col,int row)
    {
      __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Tile did unload: %d: (%d,%d)",level,col,row);
        scheduleEvalStep();
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
    
    // Called by the offline renderer to update images
    virtual void offlineRender(QuadTileOfflineLoader *loader,QuadTileOfflineImage *image)
    {
        env->CallVoidMethod(javaObj, imageRenderCallbackJava, image->texture, image->centerSize.x(), image->centerSize.y(), (int)image->texSize.x(), (int)image->texSize.y(), image->frame);
    }
};

typedef JavaClassInfo<QuadImageOfflineLayerAdapter> QILAdapterClassInfo;
template<> QILAdapterClassInfo *QILAdapterClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_nativeInit
(JNIEnv *env, jclass cls)
{
    QILAdapterClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_initialise
(JNIEnv *env, jobject obj, jobject coordSysObj, jobject changesObj)
{
    try
    {
        CoordSystem *coordSys = CoordSystemClassInfo::getClassInfo()->getObject(env,coordSysObj);
        ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
        if (!coordSys || !changes)
            return;
        
        QuadImageOfflineLayerAdapter *adapter = new QuadImageOfflineLayerAdapter(coordSys);
        QILAdapterClassInfo::getClassInfo()->setHandle(env,obj,adapter);
        adapter->setJavaRefs(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageOfflineLayerAdapter *adapter = classInfo->getObject(env,obj);
        if (!adapter)
            return;
        adapter->clearJavaRefs();
        delete adapter;
        
        classInfo->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_setEnable
(JNIEnv *env, jobject obj, jboolean enable, jobject changeSetObj)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageOfflineLayerAdapter *adapter = classInfo->getObject(env,obj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!adapter || !changeSet || !adapter->tileLoader)
            return;
        ChangeSet changes;
        adapter->tileLoader->setOn(enable);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageTileLayer::setEnable()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_setImageDepth
(JNIEnv *env, jobject obj, jint imageDepth)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageOfflineLayerAdapter *adapter = classInfo->getObject(env,obj);
        if (!adapter)
            return;
        adapter->imageDepth = imageDepth;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::setImageDepth()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_getImageDepth
(JNIEnv *env, jobject obj)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageOfflineLayerAdapter *adapter = classInfo->getObject(env,obj);
        if (!adapter)
            return 1;
        return adapter->imageDepth;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::getImageDepth()");
    }
    
    return 1;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_setAllowFrameLoading
(JNIEnv *env, jobject obj, jboolean frameLoading)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageOfflineLayerAdapter *adapter = classInfo->getObject(env,obj);
        if (!adapter)
            return;
        adapter->allowFrameLoading = frameLoading;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::setAllowFrameLoading()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_getFrameStatusNative
(JNIEnv *env, jobject obj, jbooleanArray completeArray, jintArray tilesLoadedArray)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageOfflineLayerAdapter *adapter = classInfo->getObject(env,obj);
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
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::getFrameStatus()");
    }
    
    return -1;
}


JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_setFrameLoadingPriority
(JNIEnv *env, jobject obj, jintArray frameLoadingArr, jobject changeSetObj)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageOfflineLayerAdapter *adapter = classInfo->getObject(env,obj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!adapter || !changeSet)
            return;
        adapter->framePriorities.clear();
        
        ConvertIntArray(env,frameLoadingArr,adapter->framePriorities);
        if (adapter->control)
            adapter->control->setFrameLoadingPriorities(adapter->framePriorities);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::setFrameLoadingPriority()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_setMaxTiles
(JNIEnv *env, jobject obj, jint maxTiles)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageOfflineLayerAdapter *adapter = classInfo->getObject(env,obj);
        if (!adapter)
            return;
        adapter->maxTiles = maxTiles;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::setMaxTiles()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_setImportanceScale
(JNIEnv *env, jobject obj, jfloat scale)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageOfflineLayerAdapter *adapter = classInfo->getObject(env,obj);
        if (!adapter)
            return;
        adapter->importanceScale = scale;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::setImportanceScale()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_setMultiLevelLoads
(JNIEnv *env, jobject obj, jintArray levelLoadsArr)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageOfflineLayerAdapter *adapter = classInfo->getObject(env,obj);
        if (!adapter)
            return;
        
        ConvertIntArray(env,levelLoadsArr,adapter->levelLoads);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::setMultiLevelLoads()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_getTargetZoomLevel
(JNIEnv *env, jobject obj)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageOfflineLayerAdapter *adapter = classInfo->getObject(env,obj);
        if (!adapter || !adapter->lastViewState)
            return 0;
        
        return adapter->targetZoomLevel(adapter->lastViewState);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::getTargetZoomLevel()");
    }
    
    return 0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_OfflineTileLayer_reload
(JNIEnv *env, jobject obj, jobject changeSetObj)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageOfflineLayerAdapter *adapter = classInfo->getObject(env,obj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!adapter || !changeSet)
            return;
        
        // Note: Porting. This doesn't handle the case where we change parameters and then reload
        adapter->control->refresh(*changeSet);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::reload()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_getLoadedFrames
(JNIEnv *env, jobject obj, jint numFrames, jbooleanArray completeArr, jbooleanArray currentFrameArr, jintArray numTilesLoadedArr)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageOfflineLayerAdapter *adapter = classInfo->getObject(env,obj);
        if (!adapter)
            return;
        std::vector<FrameLoadStatus> frameLoadStats;
        adapter->control->getFrameLoadStatus(frameLoadStats);
        if (numFrames != frameLoadStats.size())
            return;
        
        // Pull the data out into the arrays
        for (unsigned int ii=0;ii<numFrames;ii++)
        {
            FrameLoadStatus &status = frameLoadStats[ii];
            jboolean completeJbool = status.complete;
            jboolean currentFrameJBool = status.currentFrame;
            env->SetBooleanArrayRegion(completeArr, (jsize)ii, (jsize)1, &completeJbool);
            env->SetBooleanArrayRegion(currentFrameArr, (jsize)ii, (jsize)1, &currentFrameJBool);
            env->SetIntArrayRegion(numTilesLoadedArr, (jsize)ii, (jsize)1, &status.numTilesLoaded);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::getLoadedFrames()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_setSimultaneousFetches
(JNIEnv *env, jobject obj, jint numFetches)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageOfflineLayerAdapter *adapter = classInfo->getObject(env,obj);
        if (!adapter)
            return;
        adapter->simultaneousFetches = numFetches;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::setSimultaneousFetches()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_setUseTargetZoomLevel
(JNIEnv *env, jobject obj, jboolean newVal)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageOfflineLayerAdapter *adapter = classInfo->getObject(env,obj);
        if (!adapter)
            return;
        adapter->useTargetZoomLevel = newVal;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::setUseTargetZoomLevel()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_setSingleLevelLoading
(JNIEnv *env, jobject obj, jboolean newVal)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageOfflineLayerAdapter *adapter = classInfo->getObject(env,obj);
        if (!adapter)
            return;
        adapter->singleLevelLoading = newVal;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::setSingleLevelLoading()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_nativeStartLayer
(JNIEnv *env, jobject obj, jobject sceneObj, jobject rendererObj, jobject llObj, jobject urObj, jint minZoom, jint maxZoom, jint pixelsPerSide)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageOfflineLayerAdapter *adapter = classInfo->getObject(env,obj);
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
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::nativeStartLayer()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_nativeShutdown
(JNIEnv *env, jobject obj, jobject changesObj)
{
    try
    {
        QILAdapterClassInfo *classInfo = QILAdapterClassInfo::getClassInfo();
        QuadImageOfflineLayerAdapter *adapter = classInfo->getObject(env,obj);
        ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
        if (!adapter || !changes)
            return;
        
        adapter->control->shutdown(*changes);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::nativeShutdown()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_nativeViewUpdate
(JNIEnv *env, jobject obj, jobject viewStateObj)
{
    try
    {
        QuadImageOfflineLayerAdapter *adapter = QILAdapterClassInfo::getClassInfo()->getObject(env,obj);
        ViewState *viewState = ViewStateClassInfo::getClassInfo()->getObject(env,viewStateObj);
        if (!adapter || !viewState)
            return;
        
        adapter->env = env;
        
        adapter->control->viewUpdate(viewState);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::nativeViewUpdate()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_nativeEvalStep
(JNIEnv *env, jobject obj, jobject changesObj)
{
    try
    {
        QuadImageOfflineLayerAdapter *adapter = QILAdapterClassInfo::getClassInfo()->getObject(env,obj);
        ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
        if (!adapter || !changes)
            return false;
        
        adapter->env = env;
        
        // Note: Not passing in frame boundary info
        return adapter->control->evalStep(0.0,0.0,0.0,*changes);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::nativeEvalStep()");
    }
    
    return false;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_nativeRefresh
(JNIEnv *env, jobject obj, jobject changesObj)
{
    try
    {
        QuadImageOfflineLayerAdapter *adapter = QILAdapterClassInfo::getClassInfo()->getObject(env,obj);
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
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::nativeRefresh()");
    }
    
    return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_nativeTileDidLoad
(JNIEnv *env, jobject obj, jint x, jint y, jint level, jint frame, jobject bitmapObj, jobject changesObj)
{
    try
    {
        QuadImageOfflineLayerAdapter *adapter = QILAdapterClassInfo::getClassInfo()->getObject(env,obj);
        ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
        if (!adapter || !changes)
        {
            return;
        }
        
        AndroidBitmapInfo info;
        if (AndroidBitmap_getInfo(env, bitmapObj, &info) < 0)
        {
            return;
        }
        if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
        {
            __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Only dealing with 8888 bitmaps in QuadImageOfflineLayer");
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
        
        adapter->tileLoaded(level,x,y,frame,rawDataRef,info.width,info.height,*changes);
        AndroidBitmap_unlockPixels(env, bitmapObj);
        //		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Tile did load: %d: (%d,%d) %d",level,x,y,frame);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::nativeTileDidLoad()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_nativeTileDidNotLoad
(JNIEnv *env, jobject obj, jint x, jint y, jint level, jint frame, jobject changesObj)
{
    try
    {
        QuadImageOfflineLayerAdapter *adapter = QILAdapterClassInfo::getClassInfo()->getObject(env,obj);
        ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
        if (!adapter || !changes)
            return;
        
        //		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Tile did not load: %d: (%d,%d) %d",level,x,y,frame);
        adapter->tileLoaded(level,x,y,frame,RawDataRef(),1,1,*changes);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::nativeTileDidLoad()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_getEnable
(JNIEnv *env, jobject obj)
{
    try
    {
        QuadImageOfflineLayerAdapter *adapter = QILAdapterClassInfo::getClassInfo()->getObject(env,obj);
        if (!adapter || !adapter->tileLoader)
            return false;
        
        return adapter->tileLoader->getOn();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::getEnable()");
    }
    
    return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_setMbrNative
(JNIEnv *env, jobject obj, jdouble sx, jdouble sy, jdouble ex, jdouble ey)
{
    try
    {
        QuadImageOfflineLayerAdapter *adapter = QILAdapterClassInfo::getClassInfo()->getObject(env,obj);
        if (!adapter || !adapter->tileLoader)
            return;
        
        Mbr mbr;
        mbr.addPoint(Point2f(sx,sy));
        mbr.addPoint(Point2f(ex,ey));
        adapter->tileLoader->setMbr(mbr);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::setMbrNative()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_getSomethingChanged
(JNIEnv *env, jobject obj)
{
    try
    {
        QuadImageOfflineLayerAdapter *adapter = QILAdapterClassInfo::getClassInfo()->getObject(env,obj);
        if (!adapter || !adapter->tileLoader)
            return false;
        
        return adapter->tileLoader->getSomethingChanged();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::getSomethingChanged()");
    }
    
    return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageOfflineLayer_imageRenderToLevel
(JNIEnv *env, jobject obj, jint level, jobject changesObj)
{
    try
    {
        QuadImageOfflineLayerAdapter *adapter = QILAdapterClassInfo::getClassInfo()->getObject(env,obj);
        ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
        if (!adapter || !changes || !adapter->tileLoader)
            return;
        adapter->tileLoader->imageRenderToLevel(level,*changes);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageOfflineLayer::imageRenderToLevel()");
    }
}
