package com.mousebird.maply;

import java.util.List;

/**
 * The Wide Vector Manager is an interface to the Maply C++
 * wide vector manager and should be invisible to toolkit users.
 */
public class WideVectorManager {
    private WideVectorManager()
    {
    }

    WideVectorManager(Scene scene) { initialise(scene); }

    public void finalize()
    {
        dispose();
    }

    // Add vectors to the scene and return an ID to track them
    public native long addVectors(List<VectorObject> vecs, WideVectorInfo vecInfo, ChangeSet changes);

    // Remove vectors by ID
    public native void removeVectors(long ids[],ChangeSet changes);

    // Enable/disable vectors by ID
    public native void enableVectors(long ids[],boolean enable,ChangeSet changes);

    // Change the display of vectors
    public native void changeVectors(long ids[],VectorInfo vecInfo,ChangeSet changes);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise(Scene scene);
    native void dispose();
    private long nativeHandle;
}
