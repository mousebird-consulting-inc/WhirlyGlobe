package com.mousebird.maply;

/**
 * Raw Geometry object.  Collection of points and triangles.
 */
public class GeometryRaw
{
    GeometryRaw() { initialise(); }

    public enum GeometryType {None,Lines,Triangles};

    /**
     * Returns true if the raw geometry object is valid.
     * Checks that the number of points, normals, and texture coordinates matches.
     */
    public native boolean valid();

    public native void setTypeNative(int type);

    /**
     * Set the geometry type.  Can be lines or triangles.
     */
    public void setType(GeometryType type)
    {
        setTypeNative(type.ordinal());
    }

    /**
     * Apply a texture to the geometry.
     */
    public void setTexture(MaplyTexture tex)
    {
        setTextureNative(tex.texID);
    }
    public native void setTextureNative(long texID);

    /**
     * Add a group of points at once.
     * @param pts Points are organized as X,Y,Z in order.
     * @param numPts Number of points
     */
    public native void addPoints(double[] pts);

    /**
     * Add a group of normals at once.
     * @param norms Norms are organized as X,Y,Z in order.
     * @param numNorms Number of normals
     */
    public native void addNorms(double[] norms);

    /**
     * Add a group of texture coordinates.
     * @param texCoords Texture coordinates are organized as u,v.
     * @param numTexCoords Number of texture coordinates.
     */
    public native void addTexCoords(float[] texCoords);

    /**
     * Add a group of colors.
     * @param colors Individual colors as 32 bit values.
     * @param numColors Number of colors.
     */
    public native void addColors(int[] colors);

    /**
     * Add a group of triangles.  Indices refer to vertices in order.
     * @param tris Triangles are organized as three vertices per triangle.
     * @param numTris Number of triangles.
     */
    public native void addTriangles(int[] tris);

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
