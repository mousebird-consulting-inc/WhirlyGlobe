package com.mousebirdconsulting.maply;

public class CoordSystem 
{
	CoordSystem()
	{
		initialise();
	}
	
	public void finalize()
	{
		dispose();
	}
		
	// Bounding box
	// Note: Yes, this is a terrible place for this
	public Point3d ll = null;
	public Point3d ur = null;

	// WGS84 radians to local coordinates
	public native Point3d geographicToLocal(Point3d pt);
	// Local coordiantes to WGS84 radians
	public native Point3d localToGeographic(Point3d pt);
	
	public native void initialise();
	public native void dispose();
	private long nativeHandle;
}
