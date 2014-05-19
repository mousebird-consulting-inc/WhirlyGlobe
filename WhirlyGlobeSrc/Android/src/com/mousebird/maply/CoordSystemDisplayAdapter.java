package com.mousebird.maply;

/**
 * The coordinate system display adapter controls the conversion from source
 * data coordinates to display coordinates.  It's completely opaque outside
 * of the toolkit.
 * 
 * @author sjg
 *
 */
class CoordSystemDisplayAdapter 
{
	CoordSystem coordSys = null;
	
	// Needed by the JNI side
	private CoordSystemDisplayAdapter()
	{		
	}
	
	CoordSystemDisplayAdapter(CoordSystem inCoordSys)
	{
		coordSys = inCoordSys;
		initialise(coordSys);
	}
	
	public void finalize()
	{
		dispose();
	}

	/**
	 * Convert the given coordinate in display space to the local coordinate system.
	 * 
	 * @param disp Point in display coordinates.
	 * @return Point in the local coordinate system.
	 */
	public native Point3d displayToLocal(Point3d disp);
	
	/**
	 * Convert from the local coordinate system to display space.
	 * 
	 * @param local Point in the local system.
	 * @return Point in the display system.
	 */
	public native Point3d localToDisplay(Point3d local);
	
	/**
	 * Return the bounding box of the given display adapter.  This will typically be
	 * what the coordinate system can support.
	 * 
	 * @param ll Lower left corner of the bounding box.
	 * @param ur Upper right corner of bounding box.
	 */
	public native void getBounds(Point3d ll,Point3d ur);
	
	/**
	 * The coordinate system we're using for local coordinates.  Display
	 * coordinates are a scaled version of that in 2D and spherical for the globe.
	 * 
	 * @return
	 */
	public CoordSystem getCoordSystem()
	{
		return coordSys;
	}

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(CoordSystem coordSys);
	native void dispose();
	private long nativeHandle;
}
