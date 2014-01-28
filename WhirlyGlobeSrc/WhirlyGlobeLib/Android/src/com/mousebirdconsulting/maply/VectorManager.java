package com.mousebirdconsulting.maply;

public class VectorManager 
{
	VectorManager(MapScene scene)
	{
		initialise(scene);
	}
	
	public void finalize()
	{
		dispose();
	}
	
	// Add vectors to the scene and return an ID to track them
	public native long addVectors(VectorObject vecObjs[],VectorInfo vecInfo,ChangeSet changes);
	
	// Remove vectors by ID
	public native void removeVectors(long ids[],ChangeSet changes);
	
	// Enable/disable vectors by ID
	public native void enableVectors(long ids[],boolean enable,ChangeSet changes);
	
	public native void initialise(MapScene scene);
	public native void dispose();
	private long nativeHandle;
	private long nativeSceneHandle;
}
