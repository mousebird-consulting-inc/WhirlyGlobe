package com.mousebirdconsulting.maply;

/**
 * The coord system is a very simple representation of the coordinate
 * systems supported by WhirlyGlobe-Maply. Very few moving parts are
 * accessible at this level.  In general, you'll want to instantiate
 * one of the subclasses and pass it around to Mapy objects as needed.
 * 
 * @author sjg
 *
 */
public class CoordSystem
{
	/**
	 * Only ever called by the subclass.  Don't use this directly please.
	 */
	CoordSystem()
	{
		initialise();
	}
	
	public void finalize()
	{
		dispose();
	}
		
	/**
	 * Lower left corner of the bounding box in local coordinates.
	 */
	public Point3d ll = null;
	/**
	 * Upper right corner of the bounding box in local coordinates.
	 */
	public Point3d ur = null;

	/**
	 * Convert from WGS84 longitude/latitude coordinates to the local coordinate system.
	 * 
	 * @param pt The input coordinate in longitude/latitude WGS84 radians.  Yes, radians.
	 * @return A point in the local coordinate system.
	 */
	public native Point3d geographicToLocal(Point3d pt);
	
	/**
	 * Convert from the local coordinate system to WGS84 longitude/latitude in radians.
	 * 
	 * @param pt A point in the local coordinate system.
	 * @return A coordinate in longitude/latitude WGS84 radians.  Yes, radians.
	 */
	public native Point3d localToGeographic(Point3d pt);
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
	private long nativeHandle;
}
