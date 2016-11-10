package com.mousebird.maply;

/**
 * Raw points passed to geometry manager.
 */
public class GeometryRawPoints
{
    GeometryRawPoints() { initialise(); }

    public enum Type {IntType,FloatType,Float2Type,Float3Type,Float4Type,Double2Type,Double3Type};

    /**
     * Returns true if the points object is valid.
     * Checks that the number of points, normals, and texture coordinates matches.
     */
    public native boolean valid();

    /**
     * Add a whole array of integer values for the given attribute.
     */
    public native void addIntValues(String name,int[] intVals);

    /**
     * Add a whole array of float values for the given attribute.
     */
    public native void addFloatValues(String name,float[] floatVals);

    /**
     * Add a whole array of 2D float values for the given attribute.
     */
    public native void addPoint2fValues(String name,float[] pt2fVals);

    /**
     * Add a whole array of 3D float values for the given attribute.
     */
    public native void addPoint3fValues(String name,float[] pt3fVals);

    /**
     * Add a whole array of 3D double values for the given attribute.
     */
    public native void addPoint3dValues(String name,double[] pt3dVals);

    /**
     * Add a whole array of 4D float values for the given attribute.
     */
    public native void addPoint4fValues(String name,float[] pt4fVals);

    public native int addAttributeNative(String name,int type);

    /**
     *
     * @param name
     * @param type
     * @return
     */
    public int addAttribute(String name,Type type)
    {
        return addAttributeNative(name,type.ordinal());
    }

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
