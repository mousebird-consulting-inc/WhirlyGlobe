package com.mousebird.maply;

import android.graphics.Bitmap;
import android.os.Handler;
import android.os.Looper;

import java.util.ArrayList;

/**
 * The Quad Image Offline layer works much like the quad image paging layer, but it works
 * outside of an OpenGL context (well sort of).  It makes textures corresponding to layers
 * of image data.
 */
public class QuadImageOfflineLayer extends Layer implements LayerThread.ViewWatcherInterface, QuadImageTileLayerInterface
{
    // Set when the layer is active.
    boolean valid = false;

    private QuadImageOfflineLayer()
    {
    }

    /**
     * Fill in this interface to get the rendered images back as they're ready.
     */
    public interface RenderedImageDelegate
    {
        public void renderedImage(QuadImageOfflineLayer layer,MaplyTexture tex,Point2d centerSize,int frame);
    }

    public MaplyBaseController maplyControl = null;
    public CoordSystem coordSys = null;
    QuadImageTileLayer.TileSource tileSource = null;
    boolean flipY = true;
    RenderedImageDelegate imageDelegate = null;

    public QuadImageOfflineLayer(MaplyBaseController inMaplyControl,CoordSystem inCoordSys,QuadImageTileLayer.TileSource inTileSource)
    {
        maplyControl = inMaplyControl;
        coordSys = inCoordSys;
        tileSource = inTileSource;
        ChangeSet changes = new ChangeSet();
        initialise(coordSys,changes);
        maplyControl.layerThread.addChanges(changes);
        setSimultaneousFetches(8);
    }

    public void setImageDelegate(RenderedImageDelegate inImageDelegate)
    {
        imageDelegate = inImageDelegate;
    }

    public void finalize() { dispose(); }

    @Override
    public float getMinTime()
    {
        // Update every 1/10s
        return 0.1f;
    }

    @Override
    public float getMaxLagTime()
    {
        // Want an update no less often than this
        // Note: What?
        return 4.0f;
    }

    /**
     * Called by the layer thread.  Don't call this directly.
     */
    public void startLayer(LayerThread layerThread)
    {
        super.startLayer(layerThread);
        layerThread.addWatcher(this);
        Point2d ll = new Point2d(coordSys.ll.getX(),coordSys.ll.getY());
        Point2d ur = new Point2d(coordSys.ur.getX(),coordSys.ur.getY());
        nativeStartLayer(layerThread.scene,layerThread.renderer,ll,ur,tileSource.minZoom(),tileSource.maxZoom(),tileSource.pixelsPerSide());

        scheduleEvalStep();

        valid = true;
    }

    /**
     * Called by the layer thread.  Don't call this directly.
     */
    public void shutdown()
    {
        valid = false;
        layerThread.removeWatcher(this);
        cancelEvalStep();
        ChangeSet changes = new ChangeSet();
        nativeShutdown(changes);
        layerThread.addChanges(changes);
        super.shutdown();
    }

    /**
     * The view updated.  Called by the layer thread.  Don't call this directly.
     */
    @Override
    public void viewUpdated(ViewState viewState)
    {
        if (!valid)
            return;

        nativeViewUpdate(viewState);

        scheduleEvalStep();
    }

    Handler evalStepHandle = null;
    Runnable evalStepRun = null;

    // Cancel the current evalStep
    void cancelEvalStep()
    {
        if (!valid)
            return;

        synchronized(this)
        {
            if (evalStepHandle != null)
            {
                evalStepHandle.removeCallbacks(evalStepRun);
                evalStepHandle = null;
                evalStepRun = null;
            }
        }
    }

    // Post an evalStep if there isn't one scheduled
    void scheduleEvalStep()
    {
        if (!valid)
            return;
//		cancelEvalStep();

        synchronized(this)
        {
            if (evalStepHandle == null)
            {
                evalStepRun = new Runnable()
                {
                    @Override
                    public void run()
                    {
                        evalStep();
                    }
                };
                evalStepHandle = layerThread.addTask(evalStepRun,true);
            }
        }
    }

    // Do something small and then return
    void evalStep()
    {
        if (!valid)
            return;

        synchronized(this)
        {
            evalStepHandle = null;
            evalStepRun = null;
        }

        // Note: Check that the renderer is set up and such.
        ChangeSet changes = new ChangeSet();
        boolean didSomething = nativeEvalStep(changes);
        layerThread.addChanges(changes);
        if (didSomething)
            scheduleEvalStep();
    }

    /**
     * If you call this, the layer will clear out all current geometry
     * and refetch everything.
     */
    public void refresh()
    {
        if (!valid)
            return;

        // Make sure this runs on the layer thread
        if (Looper.myLooper() != layerThread.getLooper())
        {
            Handler handle = new Handler();
            handle.post(
                    new Runnable()
                    {
                        @Override
                        public void run()
                        {
                            refresh();
                        }
                    });
            return;
        }

        ChangeSet changes = new ChangeSet();
        boolean doEvalStep = nativeRefresh(changes);
        layerThread.addChanges(changes);
        if (doEvalStep)
            scheduleEvalStep();
    }

    /* Called by the JNI side.  We need to start fetching
     * the given tile.
     */
    void startFetch(int level,int x,int y,int frame)
    {
        if (!valid)
            return;

        // If we're not going OSM style addressing, we need to flip the Y back to TMS
        if (!flipY)
            y = (1<<level)-y-1;

        MaplyTileID tileID = new MaplyTileID(x,y,level);
        tileSource.startFetchForTile(this, tileID, frame);
    }

	/*
	 * Called by the JNI side.  We're being woken up
	 */

    /**
     * When a tile source finishes loading a given image tile,
     * it calls this method to let the quad image tile layer know
     * about it.  You can call this on any thread.
     *
     * @param imageTile The image tile we've just loaded.  Pass in null on failure.
     */
    public void loadedTile(final MaplyTileID tileID,final int frame,final MaplyImageTile imageTile)
    {
        if (!valid)
            return;

        int y = tileID.y;
        if (!flipY)
            y =  (1<<tileID.level)-tileID.y-1;

        if (Looper.myLooper() != layerThread.getLooper())
        {
            layerThread.addTask(new Runnable()
            {
                @Override
                public void run()
                {
                    loadedTile(tileID,frame,imageTile);
//					Log.d("Maply","Responding to load for tile: " + tileID.level + ": (" + tileID.x + "," + tileID.y);
                }
            });
            return;
        }

        ChangeSet changes = new ChangeSet();
        if (imageTile != null)
            nativeTileDidLoad(tileID.x,y,tileID.level,frame,imageTile.bitmap,changes);
        else
            nativeTileDidNotLoad(tileID.x,y,tileID.level,frame,changes);
        layerThread.addChanges(changes);
    }

    public LayerThread getLayerThread()
    {
        return layerThread;
    }

    /** Enable/Disable the whole layer.
     *	By default this is on.  If you turn it off, it'll stop renering offline data.
     */
    public void setEnable(boolean enable)
    {
        ChangeSet changes = new ChangeSet();
        setEnable(enable,changes);
        layerThread.addChanges(changes);
    }

    native void setEnable(boolean enable,ChangeSet changes);

    public native boolean getEnable();

    /** The number of images we're expecting to get per tile.
     * This is the number of images the layer will ask for per tile.  The default is 1, which is the normal case.  If this is greater than one that typically means we're going to animate between them.
     * the MaplyTileSource delegate is always expected to provide this many imates.
     */
    public native void setImageDepth(int imageDepth);

    /**
     * The number of images expected per tile. These are often used as animation frames.
     */
    public native int getImageDepth();

    /** If set, we'll try to fetch frames individually.
     * When fetching from a data source that has multiple frames we'll fetch each frame individually and allow them to display as we go.
     * If this is false, we'll force all the frames to load for a given tile before we move on to the next tile.
     */
    public native void setAllowFrameLoading(boolean frameLoading);

    /**
     * Query the status for active frames.  This asks the quad image layer what the state of
     * frame loading is at this instant.  All arrays are imageDepth in size.
     * @param complete For each frame, whether or not it's completely loaded.
     * @param tilesLoaded For each frame, how many tiles are loaded.
     * @return The frame currently beng loaded.  Returns -1 if the call was invalid.
     */
    public QuadImageTileLayer.FrameStatus getFrameStatus()
    {
        if (getImageDepth() <= 1)
            return null;

        QuadImageTileLayer.FrameStatus status = new QuadImageTileLayer.FrameStatus(getImageDepth());
        status.currentFrame = getFrameStatusNative(status.complete,status.tilesLoaded);
        if (status.currentFrame == -1)
            return null;

        return status;
    }

    native int getFrameStatusNative(boolean complete[],int tilesLoaded[]);

    /** For the case where we're loading individual frames, this sets the order to load them in.
     * When doing animation and loading frames, we have the option of loading them one by one.  Normally we start from 0 and work our way up, but you can control that order here.
     */
    public void setFrameLoadingPriority(int[] priorites)
    {
        ChangeSet changes = new ChangeSet();
        setFrameLoadingPriority(priorites,changes);
        layerThread.addChanges(changes);
    }

    native void setFrameLoadingPriority(int[] priorites,ChangeSet changes);

    /** Status structures describing which frames are loaded.
     * Query this to find out which frames are completely loaded into memory and which are not.
     * This queries the underlying control logic and there is no delegate.  It's polling only.
     */
    public ArrayList<QuadImageTileLayer.FrameLoadStatus> getLoadedFrames()
    {
        int numFrames = getImageDepth();
        ArrayList<QuadImageTileLayer.FrameLoadStatus> frames = new ArrayList<QuadImageTileLayer.FrameLoadStatus>();
        if (numFrames > 0)
        {
            boolean[] complete = new boolean[numFrames];
            boolean[] currentFrame = new boolean[numFrames];
            int[] numTilesLoaded = new int[numFrames];
            getLoadedFrames(numFrames,complete,currentFrame,numTilesLoaded);

            for (int ii = 0; ii < numFrames; ii++)
            {
                QuadImageTileLayer.FrameLoadStatus status = new QuadImageTileLayer.FrameLoadStatus();
                status.complete = complete[ii];
                status.currentFrame = complete[ii];
                status.numTilesLoaded = numTilesLoaded[ii];
                frames.add(status);
            }
        }

        return frames;
    }

    private native void getLoadedFrames(int numFrames,boolean[] complete,boolean[] currentFrame,int[] numTilesLoaded);

    /** Maximum number of tiles to load in at once.
     * This is the maximum number of tiles the pager will have loaded into memory at once.  The default is 128 and that's generally good enough.  However, if your tile size is small, you may want to load in more.
     * Tile loading can get out of control when using elevation data.  The toolkit calculates potential screen coverage for each tile so elevation data makes all tiles more important.  As a result the system will happily page in way more data than you may want.  The limit becomes important in elevation mode, so leave it at 128 unless you need to change it.
     */
    public native void setMaxTiles(int maxTiles);

    /** Tinker with the importance for tiles.  This will cause more or fewer tiles to load
     * The system calculates an importance for each tile based on its size and location on the screen.  You can mess with those values here.
     * Any value less than 1.0 will make the tiles less important.  Any value greater than 1.0 will make tiles more important.
     */
    public native void setImportanceScale(float scale);

    /** Control how tiles are indexed, either from the lower left or the upper left.
     * If set, we'll use the OSM approach (also Google Maps) to y indexing.  That's that default and it's normally what you're run into.
     * Strictly speaking, TMS addressing (the standard) is flipped the other way.  So if you're tile source looks odd, try setting this to false.
     * Default value is true.
     */
    public void setFlipY(boolean inFlipY)
    {
        flipY = inFlipY;
    }

    /** Detail the levels you want loaded in target level mode.
     * The image display can work in one of two modes, quad tree where it loads everything starting from the min level or a target level mode where it just tries to load one or more target levels.  This is the array that controls which levels it will try to load.
     * We do this so that the user doesn't have to wait for the target level to load.  This can be distracting on large displays with small tiles.  If you use this mode, the layer will load lower levels first, filling in quicker and then load the target level.  This looks much better, but doesn't take as long as the full quad tree based loading.
     * The layer calculates the optimal target level (for 2D maps, if you're in that mode).  The entries in this array are relative to that level or absolute.  For example [0,-4,-2] means the layer will always try to load levels 0, targetLevel-4 and targetLevel-2, but only the latter two if they make sense.
     */
    public native void setMultiLevelLoads(int[] levels);

    /**
     * Calculate the current target zoom level and return it.
     */
    public native int getTargetZoomLevel();

    public native void reload(ChangeSet changes);

    /**
     * We can only have a certain number of fetches going at once.
     * We'll create this number of threads (in some cases) based
     * on this number.
     */
    public native void setSimultaneousFetches(int numFetches);

    /**
     * If set we'll calculate a single target zoom level for the whole
     * viewport, rather than evaluating tiles individually.  This works
     * for 2D maps, but not for 3D maps or globes.
     */
    public native void setUseTargetZoomLevel(boolean newVal);

    /**
     * If set we'll skip the lower levels of the pyramid and load only
     * the current target zoom level.
     */
    public native void setSingleLevelLoading(boolean newVal);

    float renderPeriod = 0.f;
    /**
     * How often the offline render will run.
     */
    public void setRenderPeriod(float period)
    {
        renderPeriod = period;

        if (period > 0.0)
            imageRenderPeriodic();
    }

    /**
     * Set (or change) the bounding box of the area we're rendering to.
     */
    public void setMbr(Mbr mbr)
    {
        setMbrNative(mbr.ll.getX(),mbr.ll.getY(),mbr.ur.getX(),mbr.ur.getY());

        imageRenderPeriodic();
    }
    native void setMbrNative(double sx,double sy,double ex,double ey);

    // Returns true if something changed (i.e. MBR or got a frame)
    native boolean getSomethingChanged();

    // Periodically render
    void imageRenderPeriodic()
    {
        layerThread.addDelayedTask(new Runnable()
        {
            @Override
            public void run()
            {
                if (getEnable() && getSomethingChanged())
                {
                    ChangeSet changes = new ChangeSet();
                    imageRenderToLevel(-1,changes);
                    changes.process(layerThread.scene);
                }

                // Kick off another render in a few seconds
                if (getEnable() && renderPeriod > 0.0)
                    imageRenderPeriodic();
            }
        }, (long)(renderPeriod*100));
    }

    native void imageRenderToLevel(int level,ChangeSet changes);

    // Called by the JNI side to hand us back rendered image data
    void imageRenderCallback(long texID,double centerSizeX,double centerSizeY,int frame)
    {
        if (imageDelegate != null) {
            MaplyTexture tex = new MaplyTexture();
            tex.controller = maplyControl;
            tex.texID = texID;
            imageDelegate.renderedImage(this, tex, new Point2d(centerSizeX, centerSizeY), frame);
        }
    }

    native void nativeShutdown(ChangeSet changes);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise(CoordSystem coordSys,ChangeSet changes);
    native void dispose();
    private long nativeHandle;

    native void nativeStartLayer(Scene scene,MaplyRenderer renderer,Point2d ll,Point2d ur,int minZoom,int maxZoom,int pixelsPerSide);
    native void nativeViewUpdate(ViewState viewState);
    native boolean nativeEvalStep(ChangeSet changes);
    native boolean nativeRefresh(ChangeSet changes);
    native void nativeTileDidLoad(int x, int y, int level, int frame, Bitmap bitmap, ChangeSet changes);
    native void nativeTileDidNotLoad(int x,int y,int level,int frame,ChangeSet changes);
}
