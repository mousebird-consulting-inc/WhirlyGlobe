/*  QuadLoaderBase.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/22/19.
 *  Copyright 2011-2021 mousebird consulting
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
 */

package com.mousebird.maply;

import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.HashSet;

/**
 * Base class for the quad loaders.
 * <br>
 * The image, frame, and data paging loaders all share much of the same functionality.
 */
public class QuadLoaderBase implements QuadSamplingLayer.ClientInterface
{
    protected QuadLoaderBase() { }

    protected WeakReference<QuadSamplingLayer> samplingLayer;

    protected QuadLoaderBase(BaseController inControl)
    {
        control = new WeakReference<>(inControl);
    }

    protected QuadLoaderBase(BaseController inControl,SamplingParams params,int numFrames,Mode mode)
    {
        control = new WeakReference<>(inControl);
        initialise(params,numFrames,mode.ordinal());
    }

    /**
     *  Control how tiles are indexed, either from the lower left or the upper left.
     *
     *  If set, we'll use the OSM approach (also Google Maps) to y indexing.  That's that default and it's normally what you're run into.
     *
     *  Strictly speaking, TMS addressing (the standard) is flipped the other way.  So if your tile source looks odd, try setting this to false.
     *
     *  Default value is true.
     */
    public native void setFlipY(boolean newVal);

    /**
     *  Control how tiles are indexed, either from the lower left or the upper left.
     *
     *  If set, we'll use the OSM approach (also Google Maps) to y indexing.  That's that default and it's normally what you're run into.
     *
     *  Strictly speaking, TMS addressing (the standard) is flipped the other way.  So if your tile source looks odd, try setting this to false.
     *
     *  Default value is true.
     */
    public native boolean getFlipY();

    /**
     * Set for a lot of debugging output.
     */
    public native void setDebugMode(boolean debugMode);

    /**
     * Get the current debug mode flag
     */
    public native boolean getDebugMode();

    private WeakReference<BaseController> control;

    /**
     * Controller associated with this quad loader.
     * This is where you send geometry and such.
     */
    public BaseController getController() {
        WeakReference<BaseController> wr = control;
        return (wr != null) ? wr.get() : null;
    }

    public QuadSamplingLayer getSamplingLayer() {
        WeakReference<QuadSamplingLayer> wr = samplingLayer;
        return (wr != null) ? wr.get() : null;
    }

    /**
     *  Calculate the bounding box for a single tile in geographic.
     *  <br>
     *  This is a utility method for calculating the extents of a given tile in geographic (e.g. lon/lat).
     *
     *  @param tileID The ID for the tile we're interested in.
     *
     *  @return The lower left and upper right corner of the tile in geographic coordinates.
     *          Returns null in case of error.
     */
    public Mbr geoBoundsForTile(TileID tileID)
    {
        Mbr mbr = new Mbr(new Point2d(),new Point2d());
        geoBoundsForTileNative(tileID.x,tileID.y,tileID.level,mbr.ll,mbr.ur);
        return mbr;
    }

    protected native void geoBoundsForTileNative(int x,int y,int level,Point2d ll,Point2d ur);

    /**
     *  Calculate the bounding box for a single tile in the local coordinate system.
     *
     *  This utility method calculates the bounding box for a tile in the coordinate system used for the layer.
     *
     *  @param tileID The ID for the tile we're interested in.
     *
     *  @return The lower left and upper right corner of the tile in local coordinates.
     */
    @SuppressWarnings({"unused", "RedundantSuppression"})
    public Mbr boundsForTile(TileID tileID)
    {
        Mbr mbr = new Mbr(new Point2d(),new Point2d());
        boundsForTileNative(tileID.x,tileID.y,tileID.level,mbr.ll,mbr.ur);
        return mbr;
    }

    protected native void boundsForTileNative(int x,int y,int level,Point2d ll,Point2d ur);

    /**
     *  Return the center of the tile in display coordinates.
     *
     *  @param tileID The ID for the tile we're interested in.
     *
     *  @return Return the center in display space for the given tile.
     */
    @SuppressWarnings({"unused", "RedundantSuppression"})
    public Point3d displayCenterForTile(TileID tileID)
    {
        Point3d pt = new Point3d();
        displayCenterForTileNative(tileID.x,tileID.y,tileID.level,pt);
        return pt;
    }

    protected native void displayCenterForTileNative(int x,int y,int level,Point3d pt);

    protected TileFetcher tileFetcher = null;

    /**
     * Use a specific tile fetcher rather than the one shared by everyone else.
     */
    public void setTileFetcher(TileFetcher newFetcher) {
        tileFetcher = newFetcher;
    }

    protected LoaderInterpreter loadInterp = null;

    /**
     * Set the interpreter for the data coming back.  If you're just getting images, don't set this.
     * Only call this at startup.  If you need to change the interpreter, call changeLoaderInterpreter()
     */
    public void setLoaderInterpreter(LoaderInterpreter newInterp) {
        loadInterp = newInterp;
    }

    /**
     * Change the interpreter for the data coming back.  This will force a reload.
     * @param newInterpreter the new instance
     */
    @SuppressWarnings({"unused", "RedundantSuppression"})
    public void changeLoaderInterpreter(final LoaderInterpreter newInterpreter) {
        QuadSamplingLayer layer = getSamplingLayer();
        if (layer == null)
            return;

        final QuadLoaderBase theLoader = this;

        // Make this change on the layer thread
        layer.layerThread.addTask(() -> {
            loadInterp = newInterpreter;
            newInterpreter.setLoader(theLoader);
            ChangeSet changes = new ChangeSet();
            reloadNative(changes);
            QuadSamplingLayer layerInner = getSamplingLayer();
            if (layerInner != null) {
                layerInner.layerThread.addChanges(changes);
            }
        });
    }

    /**
     * Tile Info objects for individual frames.
     */
    TileInfoNew[] tileInfos = null;

    /**
     * The subclasses fill this in with the loaderReturn that they need.
     */
    protected LoaderReturn makeLoaderReturn()
    {
        return null;
    }

    /**
     * Attach a LoaderReturn to the frame assets.
     * Required for cancellation to be notated on the loader return.
     */
    public native void setLoadReturn(LoaderReturn loadReturn);

    /**
     * Detach a LoaderReturn from the frame assets
     */
    public native void clearLoadReturn(LoaderReturn loadReturn);

    protected boolean isShuttingDown = false;

    /**
     * Turn off the loader and shut things down.
     * <br>
     * This unregisters us with the sampling layer and shuts down the various objects we created.
     */
    public void shutdown()
    {
        loadInterp = null;
        QuadSamplingLayer layer = getSamplingLayer();
        if (layer == null || control == null || getController() == null) {
            return;
        }

        isShuttingDown = true;
        layer.removeClient(this);
        final QuadLoaderBase loaderBase = this;

        // Do all the shutdown on the layer thread
        layer.layerThread.addTask(() -> {
            // Clean things up
            ChangeSet changes = new ChangeSet();
            cleanupNative(changes);

            QuadSamplingLayer layerInner = getSamplingLayer();
            if (layerInner != null) {
                layerInner.layerThread.addChanges(changes);
            }

            // Back to the main thread for the sampling layer stuff
            Handler handler = new Handler(Looper.getMainLooper());
            handler.post(() -> {
                BaseController ctrl = getController();
                if (ctrl != null) {
                    QuadSamplingLayer layerInner2 = getSamplingLayer();
                    ctrl.releaseSamplingLayer(layerInner2, loaderBase);
                }

                clear(samplingLayer);
                clear(control);

                tileFetcher = null;
            });
        });
    }

    private static <T> void clear(WeakReference<T> weakRef) {
        if (weakRef != null) {
            weakRef.clear();
        }
    }

    protected native void cleanupNative(ChangeSet changes);

    protected native void mergeLoaderReturn(LoaderReturn loadReturn,ChangeSet changes);

    /* --- QuadSamplingLayer interface --- */

    public void samplingLayerConnect(QuadSamplingLayer layer,ChangeSet changes)
    {
        samplingLayerConnectNative(layer,changes);
    }

    private native void samplingLayerConnectNative(QuadSamplingLayer layer,ChangeSet changes);

    public void samplingLayerDisconnect(QuadSamplingLayer layer,ChangeSet changes)
    {
        samplingLayerDisconnectNative(layer,changes);
    }

    private native void samplingLayerDisconnectNative(QuadSamplingLayer layer,ChangeSet changes);

    // Used to initialize the loader for certain types of data.
    public enum Mode {SingleFrame,MultiFrame,Object}

    /* --- Callback from C++ side --- */

    // Process the cancels and starts we get from the C++ side
    @SuppressWarnings({"unused", "RedundantSuppression"})   // Called from C++
    public void processBatchOps(QIFBatchOps batchOps)
    {
        batchOps.process(tileFetcher);
    }

    // Frame assets are used C++ side, but we have to hold a reference to them
    //  or they disappear at inopportune times.  We don't look inside them here.
    HashSet<QIFFrameAsset> frameAssets = new HashSet<>();

    // Stop tracking a frame asset
    public void clearFrameAsset(QIFFrameAsset frameAsset)
    {
        frameAssets.remove(frameAsset);
    }

    // Start off fetches for all the frames within a given tile
    // Return an array of corresponding frame assets
    @SuppressWarnings({"unused", "RedundantSuppression"})   // Called from C++
    public void startTileFetch(QIFBatchOps batchOps, QIFFrameAsset[] inFrameAssets, final int tileX, final int tileY, final int tileLevel, int priority, double importance)
    {
        if (tileInfos.length == 0 || tileInfos.length != inFrameAssets.length)
            return;

        final TileID tileID = new TileID(tileX,tileY,tileLevel);

        int frame = 0;
        for (TileInfoNew tileInfo : tileInfos) {
            final int fFrame = frame;
            final long frameID = getFrameID(frame);
            //final int dispFrame = tileInfos.length > 1 ? frame : -1;

            // Put together a fetch request for, you know, fetching
            final TileFetchRequest fetchRequest = new TileFetchRequest();
            fetchRequest.priority = inFrameAssets[frame].getPriority();
            fetchRequest.importance = (float)importance;
            fetchRequest.callback = new TileFetchRequest.Callback() {
                @Override
                public void success(TileFetchRequest fetchRequest, byte[] data) {
                    fetchSuccess(fetchRequest, tileID, fFrame, frameID, data);
                }
                @Override
                public void failure(TileFetchRequest fetchRequest, String errorStr) {
                    fetchFailed(fetchRequest, errorStr);
                }
            };

            // Update the frame asset and track it
            QIFFrameAsset frameAsset = inFrameAssets[frame];
            frameAssets.add(frameAsset);

            if (tileInfo != null) {
                fetchRequest.fetchInfo = tileInfo.fetchInfoForTile(tileID, getFlipY());
                fetchRequest.tileSource = tileInfo.uniqueID;
                frameAsset.request = fetchRequest;

                // This will start the fetch request in a bit
                batchOps.addToStart(fetchRequest);
            } else {
                // There's no fetching to do, so we'll short circuit it
                new BackgroundFetch(fetchRequest)
                        .executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, (Void)null);

            }

            frame++;
        }
    }

    private void fetchSuccess(TileFetchRequest fetchRequest, TileID tileID, int frame, long frameID, byte[] data) {
        final LoaderInterpreter theLoadInterp = loadInterp;
        final QuadSamplingLayer layer = getSamplingLayer();

        if (theLoadInterp == null || layer == null || isShuttingDown || layer.isShuttingDown) {
            return;
        }

        if (!isFrameLoading(tileID,frameID)) {
            if (getDebugMode()) {
                Log.d("Maply", "Dropping fetched frame for " + tileID);
            }
            return;
        }

        ArrayList<byte[]> allData = null;
        final boolean merge = getModeNative() == Mode.SingleFrame.ordinal() && getNumFrames() > 1;
        if (merge) {
            allData = new ArrayList<>();
            if (!mergeLoadedFrame(tileID, frameID, data, allData)) {
                // Another fetch will handle it
                return;
            }
        }

        // Build a loader return object, fill in the data and then parse it
        final LoaderReturn loadReturn = makeLoaderReturn();
        loadReturn.setTileID(tileID);
        loadReturn.setFrame(frameID,frame);

        // Attach the loader return to the frame, enabling cancellation, etc.
        setLoadReturn(loadReturn);

        // In this mode we need to adjust the loader return to contain everything at once
        if (merge) {
            // Our data has been subsumed into allData
            loadReturn.addTileData(allData);
        } else if (data != null) {
            loadReturn.addTileData(data);
        }

        // We're on an AsyncTask in the background here, so do the loading
        if (loadInterp != null && layer != null && layer.layerThread.startOfWork()) {
            try {
                theLoadInterp.dataForTile(loadReturn, this);
            } finally {
                layer.layerThread.endOfWork();
            }
        }

        // Merge the data back in on the sampling layer's thread
        if (!isShuttingDown && !layer.isShuttingDown && !loadReturn.isCanceled()) {
            layer.layerThread.addTask(() -> {
                BaseController control = getController();
                if (control != null && layer.layerThread.startOfWork()) {
                    try {
                        if (loadInterp != null && !isShuttingDown && !loadReturn.isCanceled()) {
                            ChangeSet changes = new ChangeSet();
                            mergeLoaderReturn(loadReturn, changes);
                            layer.layerThread.addChanges(changes);
                        } else {
                            cleanupLoadedData(control, loadReturn);
                        }
                    } finally {
                        layer.layerThread.endOfWork();
                    }
                }
                loadReturn.dispose();
            });
        } else {
            if (layer.layerThread.startOfWork()) {
                try {
                    cleanupLoadedData(control, loadReturn);
                } finally {
                    layer.layerThread.endOfWork();
                }
            }
            loadReturn.dispose();
        }
    }

    private void fetchFailed(TileFetchRequest fetchRequest, String errorMessage) {
        final QuadSamplingLayer layer = getSamplingLayer();
        if (layer != null && !layer.isShuttingDown) {
            layer.layerThread.addTask(() -> {
                final QuadSamplingLayer layerInner = getSamplingLayer();
                if (layerInner != null && !layerInner.isShuttingDown) {
                    // Give the C++ code a chance to add changes in this case
                    ChangeSet changes = new ChangeSet();
                    mergeLoaderReturn(null, changes);
                    layerInner.layerThread.addChanges(changes);
                }
            });
        }
    }

    // This resolves "Warning: This AsyncTask class should be static or leaks might occur" on inline task
    private static class BackgroundFetch extends AsyncTask<Void, Void, Void> {
        public BackgroundFetch(TileFetchRequest request) {
            this.fetchRequest = request;
        }
        protected Void doInBackground(Void... unused) {
            fetchRequest.callback.success(fetchRequest, null);
            return null;
        }
        private final TileFetchRequest fetchRequest;
    }

    // Clean up data that's been processed but we shut down the loader before it got back
    private void cleanupLoadedData(WeakReference<BaseController> inControl,LoaderReturn loadReturn)
    {
        if (inControl != null) {
            cleanupLoadedData(inControl.get(), loadReturn);
        }
    }

    // Clean up data that's been processed but we shut down the loader before it got back
    private void cleanupLoadedData(BaseController inControl,LoaderReturn loadReturn)
    {
        if (inControl == null)
            return;

        ChangeSet changes = new ChangeSet();
        loadReturn.deleteComponentObjects(inControl.renderControl,inControl.renderControl.componentManager,changes);

        if (inControl.running)
            changes.process(inControl.renderControl,inControl.scene);
    }

    /**
     * Each frame has a 64 bit frame ID (other than just 0 through whatever)
     */
    public native long getFrameID(int frame);

    /**
     * When you refresh the loader, we get a new generation.
     * This is how we track data in transit.
     */
    public native int getGeneration();

    /**
     * Forces a reload of all currently loaded tiles.
     */
    public void reload()
    {
        reloadArea(null);
    }

    protected native void reloadNative(ChangeSet changes);

    public void reloadArea(Mbr[] areas) {
        QuadSamplingLayer layer = getSamplingLayer();
        if (layer != null) {
            layer.layerThread.addTask(() -> {
                QuadSamplingLayer layerInner = getSamplingLayer();
                if (layerInner != null) {
                    ChangeSet changes = new ChangeSet();
                    reloadAreaNative(changes,areas);
                    layerInner.layerThread.addChanges(changes);
                }
            });
        }
    }

    protected native void reloadAreaNative(ChangeSet changes,Mbr[] areas);

    protected native boolean isFrameLoading(TileID tileID, long frameID);

    protected native boolean mergeLoadedFrame(TileID tileID, long frameID,
                                              byte[] rawData, ArrayList<byte[]> allRawData);

    public native int getZoomSlot();

    public native int getNumFrames();

    public Mode getMode() {
        final int n = getModeNative();
        return (n >= 0 && n < modes.length) ? modes[n] : Mode.Object;
    }
    protected native int getModeNative();
    protected final Mode[] modes = new Mode[]{Mode.SingleFrame,Mode.MultiFrame,Mode.Object};

    public void finalize()
    {
        dispose();
    }
    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise(SamplingParams params,int numFrames,int mode);
    native void dispose();
    @SuppressWarnings({"unused", "RedundantSuppression"})   // Used from C++
    private long nativeHandle;
}
