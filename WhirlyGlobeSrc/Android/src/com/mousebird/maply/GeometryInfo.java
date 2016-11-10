package com.mousebird.maply;

import android.graphics.Color;

/**
 * Parameters used for Geometry display.  Typically passed to an add call.
 */
public class GeometryInfo extends BaseInfo
{
    public GeometryInfo() { initialise(); }

    /**
     * Set the color used by the geometry.  Color values range from 0 to 1.0.
     * You must specify all four values.  Alpha controls transparency.
     * @param r Red component.
     * @param g Green component.
     * @param b Blue component.
     * @param a Alpha component.
     */
    public native void setColor(float r,float g,float b,float a);

    /**
     * Set the color used by the geometry.
     */
    public void setColor(int color)
    {
        setColor(Color.red(color)/255.f,Color.green(color)/255.f,Color.blue(color)/255.f,Color.alpha(color)/255.f);
    }

    /**
     * Set the point size when using points.
     */
    public native void setPointSize(float pointSize);

    public void finalize() {
        dispose();
    }

    static {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;
}
