package com.mousebirdconsulting.maply;

/**
 * The marker manager interfaces to the Maply C++/JNI side of things
 * and is invisible to toolkit users.
 * 
 * @author sjg
 *
 */
class MarkerManager 
{
	MarkerManager(MapScene scene)
	{
		initialise(scene);
	}
	
	public void finalize()
	{
		dispose();
	}
	
	// Add markers to the scene and return an ID to track them
	public native long addMarkers(InternalMarker markers[],MarkerInfo markerInfo,ChangeSet changes);
	
	// Remove markers by ID
	public native void removeMarkers(long ids[],ChangeSet changes);
	
	// Enable/disable markers by ID
	public native void enableMarkers(long ids[],boolean eanble,ChangeSet changes);
	
	native void initialise(MapScene scene);
	native void dispose();
	private long nativeHandle;
	private long nativeSceneHandle;
}
