package com.mousebird.maply;

/**
 * A wrapper around the Proj4 general scheme for coordinate systems.
 * You set this up with a string that Proj4 recognizes and it will pass that directly through and
 * use it.
 *
 */
public class Proj4CoordSystem extends CoordSystem {

    /**
     * Construct with the proj4 string that defines your projection.
     */
    public Proj4CoordSystem(String str)
    {
        initialise(str);
    }

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise(String str);
}
