

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
    boolean valid = false;

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
//        double curFrame = std::min(std::max(where,0.0),(double)([loader->frameInfos count]-1));
        double curFrame = Math.min(Math.max(where,0.0),(double)(tileInfos.length-1));

        setCurrentImageNative(where);
    }

    protected native void setCurrentImageNative(double where);

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
}
