package com.mousebirdconsulting.maply;

/**
 * The Vector Manager is an interface to the Maply C++ vector
 * manager and should be invisible to toolkit users.
 *
 * @author sjg
 *
 */
class VectorManager 
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
	
	native void initialise(MapScene scene);
	native void dispose();
	private long nativeHandle;
	private long nativeSceneHandle;
}
