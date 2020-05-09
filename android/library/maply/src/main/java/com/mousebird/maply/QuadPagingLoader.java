package com.mousebird.maply;

import android.os.Handler;

import java.lang.ref.WeakReference;

/**
 * General purpose quad paging loader.
 * <br>
 * This quadtree based paging loader is for fetching and load general geometry.
 * There are other loaders that handle images and image animations.  This one is
 * purely for geometry.
 *
 * You need to fill in at least a MaplyLoaderInterpreter, which is probably your own
 * implementation.
 *
 * This replaces the QuadPagingLayer from WhirlyGlobe-Maply 2.x.
 */
public class QuadPagingLoader extends QuadLoaderBase {
    boolean valid = false;

    /**
     * Initialize with the objects needed to run.
     *
     * @param params Sampling params describe how the space is broken down into tiles.
     * @param tileInfo If fetching data remotely, this is the remote URL (and other info).  Can be null.
     * @param inInterp The loader interpreter takes input data (if any) per tile and turns it into visual objects
     * @param control The globe or map controller we're adding objects to.
     */
    public QuadPagingLoader(final SamplingParams params,TileInfoNew tileInfo,LoaderInterpreter inInterp,BaseController control)
    {
        this(params,new TileInfoNew[]{tileInfo},inInterp,control);
    }

    /**
     * Initialize with the objects needed to run.
     *
     * @param params Sampling params describe how the space is broken down into tiles.
     * @param inInterp The loader interpreter takes input data (if any) per tile and turns it into visual objects
     * @param control The globe or map controller we're adding objects to.
     */
    public QuadPagingLoader(final SamplingParams params,LoaderInterpreter inInterp,BaseController control)
    {
        this(params,new TileInfoNew[] { },inInterp,control);
    }

    /**
     * Initialize with the objects needed to run.
     *
     * @param params Sampling params describe how the space is broken down into tiles.
     * @param inTileInfos If fetching data remotely, these are the remote URLs (and other info).  Can be null.
     * @param inInterp The loader interpreter takes input data (if any) per tile and turns it into visual objects
     * @param control The globe or map controller we're adding objects to.
     */
    public QuadPagingLoader(final SamplingParams params,TileInfoNew inTileInfos[],LoaderInterpreter inInterp,BaseController control)
    {
        super(control, params, 1, Mode.Object);

        tileInfos = inTileInfos;
        loadInterp = inInterp;
        valid = true;

        // Let them change settings before we kick things off
        Handler handler = new Handler(control.getActivity().getMainLooper());
        handler.post(new Runnable() {
            @Override
            public void run() {
                if (!valid)
                    return;

                delayedInit(params);
            }
        });
    }

    // Called a tick after creation to let users modify settings before we start
    public void delayedInit(final SamplingParams params) {
        if (tileFetcher == null) {
            tileFetcher = getController().addTileFetcher("Image Fetcher");
        }

        samplingLayer = new WeakReference<QuadSamplingLayer>(getController().findSamplingLayer(params, this));
        loadInterp.setLoader(this);
    }

    protected LoaderReturn makeLoaderReturn() {
        return new ObjectLoaderReturn(this);
    }

    @Override
    public void shutdown() {
        valid = false;

        super.shutdown();
    }
}
