package com.mousebird.maply;

/**
 * The Wide Vector Manager is an interface to the Maply C++
 * wide vector manager and should be invisible to toolkit users.
 */
public class WideVectorManager {
    @SuppressWarnings("unused")     // Used by JNI
    private WideVectorManager() {
    }

    WideVectorManager(Scene scene) { initialise(scene); }

    // Add vectors to the scene and return an ID to track them
    public native long addVectors(VectorObject[] objects, WideVectorInfo vecInfo, ChangeSet changes);

    // Remove vectors by ID
    public native void removeVectors(long[] ids, ChangeSet changes);

    // Enable/disable vectors by ID
    public native void enableVectors(long[] ids, boolean enable, ChangeSet changes);

    // Instance the given wide vectors with the given changes
    public native long instanceVectors(long vecId,WideVectorInfo vecInfo,ChangeSet changes);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise(Scene scene);
    native void dispose();
    public void finalize() {
        dispose();
    }

    @SuppressWarnings("unused")     // Used by JNI
    private long nativeHandle;
}
