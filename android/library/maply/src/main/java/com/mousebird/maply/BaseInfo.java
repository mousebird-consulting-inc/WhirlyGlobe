package com.mousebird.maply;

/**
 * This is the base class for all the various Info objects.
 *
 * @author sjg
 *
 */

public class BaseInfo
{
    protected BaseInfo()
    {
        setEnable(true);
        disposeAfterUse = false;
    }

    /**
     * If set, the toolkit will dispose of the objects that were
     * added after it's finished.
     * This saves memory by removing the C++ data for things like
     * VectorObjects.
     */
    public boolean disposeAfterUse = false;

    /**
     * Set the minimum/maximum cutoff for visibility of the feature.  This is the closest height the
     * feature will be visible from.  Defaults to 0.0 (always visible).
     */
    public native void setVisibleHeightRange(double minVis,double maxVis);

    /**
     * Set the minimum height visibility for a feature.
     */
    public void setMinVis(double minVis)
    {
        setVisibleHeightRange(minVis,getVisibleHeightMax());
    }

    /**
     * Set the maximum height visibility for a feature.
     */
    public void setMaxVis(double maxVis)
    {
        setVisibleHeightRange(getVisibleHeightMin(),maxVis);
    }

    /**
     * Returns the minimum cutoff for visibility of the feature.  This is the closest height the
     * feature will be visible from.  Defaults to 0.0 (always visible).
     */
    public native double getVisibleHeightMin();

    /**
     * Returns the minimum cutoff for visibility of the feature.  This is the largest height the
     * feature will be visible from.  Defaults to 0.0 (always visible).
     */
    public native double getVisibleHeightMax();

    /**
     * If set, the objects created will only be visible when in this range
     * of distance from the viewer.
     */
    public native void setViewDistRange(double minViewerDist,double maxViewerDist);

    /**
     * @return Minimum distance from the user that an object will be visible.
     */
    public native double getViewDistRangeMin();

    /**
     * @return Maximum distance form the user that an object will be visible.
     */
    public native double getViewDistRangeMax();


    /**
     * If the viewer center is defined, all the geometry is centered around it.
     * This is convenient with geometry that's relatively small in a larger area.
     */
    public native void setViewerCenter(Point3d center);

    /**
     * @return The current viewer center, if there is one.
     */
    public native Point3d getViewerCenter();

    /**
     * Set the drawOffset for geometry.  This is rarely used.
     */
    public native void setDrawOffset(float drawOffset);

    /**
     * @return The Z offset for geometry.  Rarely used anymore.
     */
    public native double getDrawOffset();

    /**
     * Set the drawPriority for the geometry.  Draw priority controls the order
     * in which features are drawn.
     */
    public native void setDrawPriority(int drawPriority);

    /**
     * Get the drawPriority for the geometry.  Draw priority controls the order
     * in which features are drawn.
     */
    public native int getDrawPriority();

    /**
     * Controls whether or not the geometry will be visible.  By
     * default this is true.
     * @param newEnable New value for the enable.
     */
    public native void setEnable(boolean newEnable);

    /**
     * @return Current enable value.
     */
    public native boolean getEnable();

    /**
     * The amount of time (in seconds) it takes for new geometry
     * to fade in and fade out.  By default, fade is off.
     */
    public native void setFade(double fade);

    /**
     * Geometry can be made to fade in when it appears and fade out when
     * it's removed.  These values can be set separately.
     * @param fadeIn Time to fade new geometry in.
     * @param fadeOut Time to fade geometry out when removed.
     */
    public native void setFadeInOut(double fadeIn,double fadeOut);

    /**
     * The amount of time (in seconds) it takes for new geometry
     * to fade in.  By default, fade is off.
     */
    public native float getFadeIn();

    /**
     * The amount of time (in seconds) it takes for geometry
     * to fade out when removed.  By default, fade is off.
     */
    public native float getFadeOut();

    /**
     * Geometry can be set to start fading out at a specific time.
     * @param fadeOutTime Time to start fading geometry out in seconds from the beginning epoch.
     */
    public native void setFadeOutTime(double fadeOutTime);

    /**
     * @return Time when geometry is set to fade out (if set)
     */
    public native double getFadeOutTime();

    /**
     * Geometry can be enabled over a specific time period.  If these are set,
     * the engine will turn the geometry off outside of this range.
     * @param startEnable When to start drawing the geometry.  Seconds from the beginning epoch.
     * @param endEnable When to stop drawing the geometry.  Seconds from the beginning epoch.
     */
    public native void setEnableTimes(double startEnable,double endEnable);

    /**
     * Set the shader to be used in rendering the given objects.
     */
    public void setShader(Shader shader)
    {
        setShaderID(shader.getID());
    }

    /**
     * Set the shader to be used in rendering, by ID
     */
    public native void setShaderID(long shaderID);

    /**
     * Return the unique ID for the shader or 0 if there is none.
     */
    public native long getShaderID();

    /**
     * If set, the renderer will compare this geometry to the z buffer when rendering.
     * Off by default.  Most things use priority rendering.
     */
    public native void setZBufferRead(boolean newVal);

    /**
     * If set, the geometry will write to the Z buffer when it renders.
     * On by default
     */
    public native void setZBufferWrite(boolean newVal);

    /**
     * By default, all geometry renders to the main screen.
     * If set, you can direct that geometry to a specific offscreen render target instead.
     */
    public void setRenderTarget(RenderTarget target)
    {
        setRenderTargetNative(target.renderTargetID);
    }

    private native void setRenderTargetNative(long targetID);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    protected long nativeHandle;
}
