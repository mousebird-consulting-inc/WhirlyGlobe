package com.mousebird.maply;

import android.graphics.Color;

/**
 * Used to describe how we want to build lofted polygons, including side and top,
 * colors and such.
 */
public class LoftedPolyInfo extends BaseInfo {

    public static int LoftedPolyPriorityDefault = 70000;

    /**
     * Construct the lofted polyg info with default values.
     * At the very least, it will be white and visible.
     */
    public LoftedPolyInfo()
    {
        initialise();
        setDrawPriority(LoftedPolyPriorityDefault);
    }

    public void finalize() { dispose(); }

    /**
     * Height of the top of the lofted polygon in display units.
     * For the globe display units are based on a radius of 1.0.
     */
    public native void setHeight(double height);

    /**
     * If present, we'll start the lofted poly at this height.
     * The height is in globe units, based on a radius of 1.0.
     */
    public native void setBase(double base);

    /**
     * If on we'll create the geometry for the top.  On by default.
     */
    public native void setTop(boolean top);

    /**
     * If on we'll create geometry for the sides.  On by default.
     */
    public native void setSide(boolean side);

    /**
     * If set, this will draw an outline around the top of the lofted poly in lines.
     */
    public native void setOutline(boolean outline);

    /**
     * If set and we're drawing an outline, this will create lines up the sides.
     */
    public native void setOutlineSide(boolean outlineSide);

    /**
     * If set, this will draw an outline around the bottom of the lofted poly in lines.
     */
    public native void setOutlineBottom(boolean outlineBottom);

    /**
     * Draw priority of the lines created for the lofted poly outline.
     */
    public native void setOutlineDrawPriority(int drawPriority);

    /**
     * Color we'll use for the lofted polygons.  A bit of alpha looks good.
     */
    public void setColor(int color) {
        setColor(Color.red(color)/255.f,Color.green(color)/255.f,Color.blue(color)/255.f,Color.alpha(color)/255.f);
    }

    protected native void setColor(float r,float g,float b,float a);

    /**
     * If the outline is one this is the outline's color
     * @param color
     */
    public void setOutlineColor(int color) {
        setOutlineColor(Color.red(color)/255.f,Color.green(color)/255.f,Color.blue(color)/255.f,Color.alpha(color)/255.f);
    }

    /**
     * If the outline is on this is the outline's color.
     */
    protected native void setOutlineColor(float r,float g,float b,float a);

    /**
     * This is the outline's width if it's turned on.
     */
    public native void setOutlineWidth(double width);

    /**
     * All geometry is centered around this point already.
     * Useful for high precision operations.
     */
    public native void setCenter(Point2d center);

    /**
     * The system can calculate a center and use that.  Helps with precision.
     */
    public native void setUseCenter(boolean useCenter);

    /**
     * The size of the grid (in radians) we'll use to chop up the vector features to make them
     * follow the sphere (for a globe).
     */
    public native void setGridSize(double gridSize);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
}
