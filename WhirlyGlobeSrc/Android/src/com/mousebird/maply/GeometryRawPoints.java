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

    public native void addIntValues(String name,int[] intVals,int numVals);
    public native void addFloatValues(String name,float[] floatVals,int numVals);
    public native void addPoint2fValues(String name,float[] pt2fVals,int numPts);
    public native void addPoint3fValues(String name,float[] pt3fVals,int numPts);
    public native void addPoint3dValues(String name,double[] pt3dVals,int numPts);
    public native void addPoint4fValues(String name,float[] pt4fVals,int numPts);

    public native int addAttribute(String name,Type type);

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
