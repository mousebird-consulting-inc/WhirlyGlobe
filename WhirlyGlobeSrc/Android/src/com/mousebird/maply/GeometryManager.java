package com.mousebird.maply;

/**
 * The geometry manager interfaces to the Maply C++/JNI side of things
 * and is invisible to toolkit users.
 */
public class GeometryManager
{
    private GeometryManager()
    {
    }

    GeometryManager(Scene scene) { initialise(scene); }

    public void finalize() { dispose(); }

    // Add a group of geometry and instances
//    public native long addGeometry(GeometryRaw[] geom,GeometryInstance[] inst,GeometryInfo info,ChangeSet changes);

    // Add a group of geometry as a model for instancing later
//    public native long addBaseGeometry(GeometryRaw[] geom,ChangeSet changes);

    // Add one or more instances of previously defined geometry
//    public native long addGeometryInstances(long baseGeomID,GeometryInstance[] inst,GeometryInfo info,ChangeSet changes);

    // Add a group of geometry points.  Points are special
    public native long addGeometryPoints(GeometryRawPoints points,Matrix4d mat,GeometryInfo info,ChangeSet changes);

    // Enable/disable geometry by ID
    public native void enableGeometry(long ids[],boolean enable,ChangeSet changes);

    // Remove geometry by ID
    public native void removeGeometry(long ids[],ChangeSet changes);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise(Scene scene);
    native void dispose();
    private long nativeHandle;
    private long nativeSceneHandle;

}
