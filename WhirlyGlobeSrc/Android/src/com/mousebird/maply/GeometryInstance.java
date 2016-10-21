package com.mousebird.maply;

/**
 * Represents a single Geometry Instance
 */
public class GeometryInstance
{
    public GeometryInstance() {
        initialise();
    }

    /**
     * Set the instance center.
     */
    public native void setCenter(double x,double y,double z);

    /**
     * If using motion, set the end position.
     */
    public native void setEndCenter(double x,double y,double z);

    /**
     * If using motion, how long the instance takes to move from one center to another.
     */
    public native void setDuration(double duration);

    /**
     * Matrix to apply for rotation and so forth.
     */
    public native void setMatrix(Matrix4d mat);

    /**
     * If called, this will override the color on the geometry.
     */
    public native void setColor(int color);

    /**
     * If set, the model will be selectable.
     */
    public native void setSelectable(boolean selectable);

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
