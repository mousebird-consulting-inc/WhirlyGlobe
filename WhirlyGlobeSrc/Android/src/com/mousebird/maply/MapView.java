package com.mousebird.maply;

import java.util.ArrayList;

/**
 * The Map View handles math related to user position and orientation.
 * It's largely opaque to toolkit users.  The MaplyController handles
 * passing data to and getting data from it.
 * 
 * @author sjg
 *
 */
class MapView 
{
	private MapView()
	{
	}
	
	MapView(CoordSystemDisplayAdapter coordAdapter)
	{
		initialise(coordAdapter);
	}

	public void finalize()
	{
		dispose();
	}
	
	// For objects that want to know when the view changes (every time it does)
	interface ViewWatcher
	{
		public void viewUpdated(MapView view);
	}
	
	// Set the view location from a Point3d
	void setLoc(Point3d loc)
	{
		setLoc(loc.getX(),loc.getY(),loc.getZ());
		
		runViewUpdates();
	}
	
	ArrayList<ViewWatcher> watchers = new ArrayList<ViewWatcher>();
	
	// Add a watcher for callbacks on each and every view related change
	void addViewWatcher(ViewWatcher watcher)
	{
		watchers.add(watcher);
	}
	// Remove an object that was watching view changes
	void removeViewWatcher(ViewWatcher watcher)
	{
		watchers.remove(watcher);
	}
	// Let everything know we changed the view
	void runViewUpdates()
	{
//		Point3d loc = getLoc();
//		Log.i("Maply","New pos: (" + loc.getX() + "," + loc.getY() + "," + loc.getZ() + ")");
		for (ViewWatcher watcher: watchers)
			watcher.viewUpdated(this);
	}
	
	// Minimum possible height above the surface
	native double minHeightAboveSurface();
	// Maximum possible height above the surface
	native double maxHeightAboveSurface();
	// Set the view location (including height)
	native void setLoc(double x,double y,double z);
	// Get the current view location
	public native Point3d getLoc();
	// Set the 2D rotation
	native void setRot(double rot);
	// Return the 2D rotation
	native double getRot();
	// Return the current model & view matrix combined (but not projection)
	native Matrix4d calcModelViewMatrix();	
	// Calculate the point on the view plan given the screen location
	native Point3d pointOnPlaneFromScreen(Point2d screenPt,Matrix4d viewModelMatrix,Point2d frameSize,boolean clip);
			
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(CoordSystemDisplayAdapter coordAdapter);
	native void dispose();
	private long nativeHandle;
}
