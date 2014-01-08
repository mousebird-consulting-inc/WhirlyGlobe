package com.mousebirdconsulting.maply;

class MapView 
{
	MapView(CoordSystemDisplayAdapter coordAdapter)
	{
		initialise(coordAdapter);
	}
	
	public void finalize()
	{
		dispose();
	}
	
	// SEt the view location from a Point3d
	public void setLoc(Point3d loc)
	{
		setLoc(loc.getX(),loc.getY(),loc.getZ());
	}
	
	// Minimum possible height above the surface
	public native double minHeightAboveSurface();
	// Maximum possible height above the surface
	public native double maxHeightAboveSurface();
	// Set the view location (including height)
	public native void setLoc(double x,double y,double z);
	// Get the current view location
	public native Point3d getLoc();
	// Set the 2D rotation
	public native void setRot(double rot);
	// Return the 2D rotation
	public native double getRot();
	// Call after making adjustments to the view
	public native void runViewUpdates();
	// Return the current model & view matrix combined (but not projection)
	public native Matrix4d calcModelViewMatrix();	
	// Calculate the point on the view plan given the screen location
	public native Point3d pointOnPlaneFromScreen(Point2d screenPt,Matrix4d viewModelMatrix,Point2d frameSize,boolean clip);
	
	public native void initialise(CoordSystemDisplayAdapter coordAdapter);
	public native void dispose();
	private long nativeHandle;
}
