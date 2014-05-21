package com.mousebird.maply;

import java.util.List;

/**
 * The marker manager interfaces to the Maply C++/JNI side of things
 * and is invisible to toolkit users.
 * 
 * @author sjg
 *
 */
class MarkerManager 
{
	private MarkerManager()
	{		
	}
	
	MarkerManager(MapScene scene)
	{
		initialise(scene);
	}
	
	public void finalize()
	{
		dispose();
	}
	
	// Add markers to the scene and return an ID to track them
	public native long addMarkers(List<InternalMarker> markers,MarkerInfo markerInfo,ChangeSet changes);
	
	// Remove markers by ID
	public native void removeMarkers(long ids[],ChangeSet changes);
	
	// Enable/disable markers by ID
	public native void enableMarkers(long ids[],boolean enable,ChangeSet changes);
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(MapScene scene);
	native void dispose();
	private long nativeHandle;
	private long nativeSceneHandle;
}
