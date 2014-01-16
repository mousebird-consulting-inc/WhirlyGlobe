package com.mousebirdconsulting.maply;

public class MarkerManager 
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
	
	public native void initialise(MapScene scene);
	public native void dispose();
	private long nativeHandle;
	private long nativeSceneHandle;
}
