

package com.mousebird.maply;

import android.os.Handler;

/**
 * The Maply Quad Image Frame Loader is for paging individual frames of image pyramids.
 * <br>
 * This works much like the Quad Image Loader, but handles more than one frame.  You can animate
 * between the frames with the QuadImageFrameAnimator.
 */
public class QuadImageFrameLoader extends QuadImageLoaderBase
{
    protected boolean valid = false;

    protected QuadImageFrameLoader() { }

    public QuadImageFrameLoader(BaseController control)
    {
        super(control);

        valid = true;
    }

    public QuadImageFrameLoader(final SamplingParams params,TileInfoNew inTileInfos[],BaseController control)
    {
        super(control, params, inTileInfos.length);
        tileInfos = inTileInfos;

        valid = true;
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

    public enum FrameLoadMode {Broad,Narrow};

    /**
     * How frames are loaded (top down vs broad).  Top down is the default.
     */
    public void setLoadFrameMode(FrameLoadMode mode)
    {
        setLoadFrameModeNative(mode.ordinal());
    }

    protected native void setLoadFrameModeNative(int mode);

    /**
     *   Add another rendering focus to the frame loader.
     *
     *   Normally you'd have one point of focus for a frame loader resulting in one image
     *   to be displayed.  But if you're using render targets, you may want to have two
     *   and combine them in some way yourself.  Or more.  No idea why you'd do that.
     *
     *   If you're going to do this, call addFocus right after you create the FrameLoader.
     */
    public native void addFocus();

    /**
     *   Return the number of focii.  Normally it's 1.
     *
     *   See addFocus for what these are.  You probably don't need to be using them.
     */
    public native int getNumFocus();

    /**
     * Set the interpolated location within the array of frames.
     * <br>
     * Each set of frames can be accessed from [0.0,numFrames].  Images will be interpolated between
     * those values and may be snapped if data has not yet loaded.
     * <br>
     * This value is used once per frame, so feel free to call this as much as you'd like.
     */
    public void setCurrentImage(double where)
    {
        setCurrentImage(0,where);
    }

    /**
     * Set the currentImage for the given focus.  See addFocus for what those are.
     * @param focusID Which focus to use.  Only use this method if you've got more than one.
     * @param where What to set the currentImage to.
     */
    public void setCurrentImage(int focusID,double where)
    {
//        double curFrame = std::min(std::max(where,0.0),(double)([loader->frameInfos count]-1));
        double curFrame = Math.min(Math.max(where,0.0),(double)(tileInfos.length-1));

        setCurrentImageNative(focusID,where);
    }

    protected native void setCurrentImageNative(int focusID,double where);

    /**
     *   Return the interpolated location within the array of frames.
     */
    public double getCurrentImage()
    {
        return getCurrentImage(0);
    }

    /**
     * Return the interpolated location for a given focus within the array of frames.
     */
    public native double getCurrentImage(int focusID);

    /**
     *  Set whether we require the top tiles to be loaded before a frame can be displayed.
     *
     *  Normally the system wants all the top level tiles to be loaded (just one at level 0)
     *  to be in memory before it will display a frame at all.  You can turn this off.
     */
    public native void setRequireTopTiles(boolean newVal);

    /**
     *  An optional render target for this loader.
     *
     *  The loader can draw to a render target rather than to the screen.
     *  You use this in a multi-pass rendering setup.
     *
     *  This version takes a specific focus.  See addFocus for what that means.
     */
    public void setRenderTarget(int focusID,RenderTarget renderTarget)
    {
        setRenderTargetIDNative(focusID,renderTarget.renderTargetID);
    }

    protected native void setRenderTargetIDNative(int focusID,long renderTargetID);

    /**
     *  Shader to use for rendering the image frames for a particular focus.
     *
     *  Consult addFocus for what this means.
     */
    public void setShader(int focusID,Shader shader)
    {
        setShaderIDNative(focusID,shader.getID());
    }

    protected native void setShaderIDNative(int focusID,long renderTargetID);

    /**
     * Number of tile sources passed in as individual frames.
     */
    public int getNumFrames() {
        return tileInfos.length;
    }

    @Override
    public void shutdown() {
        valid = false;

        super.shutdown();
    }

    /**
     * The Maply Quad Image Frame Loader can generation per-frame stats.  These are them.
     */
    public class FrameStats
    {
        /**
         * Number of tiles this frame is in (loading and loaded)
         */
        public int totalTiles;

        /**
         * Number of tiles this frame has yet to load
         */
        public int tilesToLoad;
    }

    /**
     * Stats generated by the Maply Quad Image Frame Loader.
     */
    public class Stats
    {
        /**
         * Total number of tiles managed by the loader
         */
        public int numTiles = 0;

        /**
         * Per frame stats for current loading state
         */
        public FrameStats[] frameStats = null;
    }

    /**
     * Pull out the per-frame instantaneous stats.
     */
    public Stats getStats()
    {
        int numFrames = getNumFrames();
        if (numFrames == 0)
            return null;

        Stats stats = new Stats();
        stats.frameStats = new FrameStats[numFrames];
        int totalTiles[] = new int[numFrames];
        int tilesToLoad[] = new int[numFrames];

        // Fetch the data like this because I'm lazy
        stats.numTiles = getStatsNative(totalTiles, tilesToLoad);
        for (int ii=0;ii<numFrames;ii++)
        {
            FrameStats frameStats = new FrameStats();
            frameStats.tilesToLoad = tilesToLoad[ii];
            frameStats.totalTiles = totalTiles[ii];
            stats.frameStats[ii] = frameStats;
        }

        return stats;
    }

    private native int getStatsNative(int[] totalTiles,int[] tilesToLoad);
}
