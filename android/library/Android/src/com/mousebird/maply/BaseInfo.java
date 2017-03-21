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
        setDrawOffset(0.f);
        setDrawPriority(0);
        setMinVis(Float.MAX_VALUE);
        setMaxVis(Float.MAX_VALUE);
        setFade(0.f);
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
     * Controls whether or not the geometry will be visible.  By
     * default this is true.
     * @param newEnable New value for the enable.
     */
    public native void setEnable(boolean newEnable);

    /**
     * Set the drawOffset for geometry.  This is rarely used.
     */
    public native void setDrawOffset(float drawOffset);

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
     * Set the minimum cutoff for visibility of the feature.  This is the closest height the
     * feature will be visible from.  Defaults to 0.0 (always visible).
     */
    public native void setMinVis(float minVis);

    /**
     * Returns the minimum cutoff for visibility of the feature.  This is the closest height the
     * feature will be visible from.  Defaults to 0.0 (always visible).
     */
    public native float getMinVis();

    /**
     * Set the maximum cutoff for visibility of the features.  This is the biggest height the
     * features will be visible from.  Defaults to off.
     */
    public native void setMaxVis(float maxVis);

    /**
     * Returns the maximum cutoff for visibility of the features.  This is the biggest height the
     * features will be visible from.  Defaults to off.
     */
    public native float getMaxVis();

    /**
     * The amount of time (in seconds) it takes for new geometry
     * to fade in and fade out.  By default, fade is off.
     */
    public native void setFade(float fade);

    /**
     * The amount of time (in seconds) it takes for new geometry
     * to fade in and fade out.  By default, fade is off.
     */
    public native float getFade();

    /**
     * Set the shader to be used in rendering the given objects.
     */
    public native void setShader(Shader shader);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    protected long nativeHandle;
}
