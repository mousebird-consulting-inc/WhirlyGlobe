

package com.mousebird.maply;

import android.os.Handler;

import androidx.annotation.Nullable;

import java.util.Objects;

/**
 * The Maply Quad Image Frame Loader is for paging individual frames of image pyramids.
 * <br>
 * This works much like the Quad Image Loader, but handles more than one frame.  You can animate
 * between the frames with the QuadImageFrameAnimator.
 */
@SuppressWarnings("unused")
public class QuadImageFrameLoader extends QuadImageLoaderBase {
    protected boolean valid = false;

    protected QuadImageFrameLoader() { }    // for JNI

    public QuadImageFrameLoader(BaseController control) {
        super(control);
        valid = true;
    }

    public QuadImageFrameLoader(final SamplingParams params, TileInfoNew[] inTileInfos, BaseController control) {
        this(params, inTileInfos, control, FramesLoadMode.All);
    }

    public QuadImageFrameLoader(final SamplingParams params, TileInfoNew[] inTileInfos,
                                BaseController control, FramesLoadMode framesLoadMode) {
        super(control, params, inTileInfos.length);
        tileInfos = inTileInfos;

        setLoadFramesMode(framesLoadMode);

        valid = true;
        final Handler handler = new Handler(Objects.requireNonNull(control.getActivity()).getMainLooper());
        handler.post(() -> {
            if (valid) {
                delayedInit(params);
            }
        });
    }

    public enum FrameLoadMode {Broad,Narrow}
    private final FrameLoadMode[] frameLoadModes = FrameLoadMode.values();

    // This is confusing because FrameLoadMode should be called have been called LoadMode
    public enum FramesLoadMode {All,Current}
    private final FramesLoadMode[] framesLoadModes = FramesLoadMode.values();

    /**
     * How frames are loaded (top down vs broad)
     */
    @Nullable
    public FrameLoadMode getLoadFrameMode() {
        final int mode = getLoadFrameModeNative();
        return (0 <= mode && mode < frameLoadModes.length) ? frameLoadModes[mode] : null;
    }

    /**
     * How frames are loaded (top-down vs broad).  Top-down is the default.
     */
    public void setLoadFrameMode(FrameLoadMode mode) {
        // If we changed the frame mode we may need to refresh the priorities
        if (setLoadFrameModeNative(mode.ordinal()) && samplingLayer != null) {
            final QuadSamplingLayer layer = getSamplingLayer();
            if (layer != null && layer.layerThread != null) {
                layer.layerThread.addTask(this::updatePriorities);
            }
        }
    }

    /**
     * Load all frames or only the one(s) current visible
     */
    @Nullable
    public FramesLoadMode getLoadFramesMode() {
        final int mode = getLoadFramesModeNative();
        return (0 <= mode && mode < framesLoadModes.length) ? framesLoadModes[mode] : null;
    }

    /**
     * Load all frames or only the one(s) current visible
     */
    public void setLoadFramesMode(FramesLoadMode mode) {
        // If we changed the frame mode we may need to refresh the priorities
        final ChangeSet changes = new ChangeSet();
        if (setLoadFramesModeNative(mode.ordinal(), changes) && samplingLayer != null) {
            final QuadSamplingLayer layer = getSamplingLayer();
            if (layer != null && layer.layerThread != null) {
                layer.layerThread.addChanges(changes);
            } else {
                changes.discard();
            }
        } else {
            changes.discard();
        }
    }

    /**
     * Get the current enable status
     */
    public native boolean getEnabled();
    /**
     * Enable or disable the loader
     */
    public void setEnabled(boolean b) {
        if (setEnabledNative(b)) {
            final QuadSamplingLayer layer = getSamplingLayer();
            if (layer != null && layer.layerThread != null) {
                final ChangeSet changes = new ChangeSet();
                changes.addFlush();
                layer.layerThread.addChanges(changes);
            }
        }
    }
    private native boolean setEnabledNative(boolean b);

    protected native int getLoadFrameModeNative();
    protected native boolean setLoadFrameModeNative(int mode);
    protected native int getLoadFramesModeNative();
    protected native boolean setLoadFramesModeNative(int mode, ChangeSet changes);
    protected native void updatePriorities();

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
     *   Return the number of foci.  Normally it's 1.
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
    public void setCurrentImage(double where) {
        setCurrentImage(0,where);
    }

    /**
     * Set the currentImage for the given focus.  See addFocus for what those are.
     * @param focusID Which focus to use.  Only use this method if you've got more than one.
     * @param where What to set the currentImage to.
     */
    public void setCurrentImage(int focusID,double where)
    {
        final double curFrame = Math.min(Math.max(where, 0.0), tileInfos.length - 1);
        if (setCurrentImageNative(focusID,where) && samplingLayer != null) {
            // setCurrentImage tells us if we changed the actual image
            final QuadSamplingLayer layer = getSamplingLayer();
            if (layer != null && layer.layerThread != null) {
                layer.layerThread.addTask(this::updatePriorities);
            }
        }
    }

    protected native boolean setCurrentImageNative(int focusID,double where);

    /**
     *   Return the interpolated location within the array of frames.
     */
    public double getCurrentImage() {
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
    public native boolean getRequireTopTiles();

    /**
     *  An optional render target for this loader.
     *
     *  The loader can draw to a render target rather than to the screen.
     *  You use this in a multi-pass rendering setup.
     *
     *  This version takes a specific focus.  See addFocus for what that means.
     */
    public void setRenderTarget(int focusID,RenderTarget renderTarget) {
        setRenderTargetIDNative(focusID,renderTarget.renderTargetID);
    }

    protected native void setRenderTargetIDNative(int focusID,long renderTargetID);

    /**
     *  In special cases we may have tiles that already have borders baked in.  In that case, call this
     *  method to set both the total textures size and the number of border pixels around the outside.
     *
     *  By default this functionality is off.
     */
    public native void setTextureSize(int tileSize,int borderSize);

    /**
     *  Shader to use for rendering the image frames for a particular focus.
     *
     *  Consult addFocus for what this means.
     */
    public void setShader(int focusID,Shader shader) {
        setShaderIDNative(focusID,(shader != null) ? shader.getID() : 0);
    }

    protected native void setShaderIDNative(int focusID,long renderTargetID);

    /**
     * Number of tile sources passed in as individual frames.
     */
    public int getNumFrames() {
        return tileInfos.length;
    }

    /**
     * Change the tile sources all at once.  This also forces a reload.
     */
    public void changeTileInfo(final TileInfoNew[] newTileInfo) {
        super.changeTileInfo(newTileInfo);
    }

    /**
     * Set a label to be displayed in debug output for this loader
     */
    public native void setLabel(@Nullable String label);
    @Nullable public native String getLabel();

    @Override
    public void shutdown() {
        valid = false;

        super.shutdown();
    }

    /**
     * The Maply Quad Image Frame Loader can generation per-frame stats.  These are them.
     */
    public static class FrameStats
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
    public static class Stats
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
        final int numFrames = getNumFrames();
        if (numFrames == 0)
            return null;

        final Stats stats = new Stats();
        stats.frameStats = new FrameStats[numFrames];
        final int[] totalTiles = new int[numFrames];
        final int[] tilesToLoad = new int[numFrames];

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
