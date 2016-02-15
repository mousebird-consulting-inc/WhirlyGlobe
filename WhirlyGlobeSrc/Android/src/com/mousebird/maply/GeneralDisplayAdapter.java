package com.mousebird.maply;

/**
 * Created by sjg on 2/13/16.
 */
public class GeneralDisplayAdapter extends CoordSystemDisplayAdapter
{
    public GeneralDisplayAdapter(CoordSystem coordSys,Point3d ll,Point3d ur,Point3d center,Point3d scale)
    {
        initialise(coordSys,ll,ur,center,scale);
    }

    public void finalize()
    {
        dispose();
    }

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise(CoordSystem coordSys,Point3d ll,Point3d ur,Point3d center,Point3d scale);
    native void dispose();
}
