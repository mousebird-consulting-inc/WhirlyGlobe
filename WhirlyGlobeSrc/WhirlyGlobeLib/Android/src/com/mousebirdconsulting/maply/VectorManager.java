package com.mousebirdconsulting.maply;

import java.util.List;

/**
 * The Vector Manager is an interface to the Maply C++ vector
 * manager and should be invisible to toolkit users.
 *
 * @author sjg
 *
 */
class VectorManager 
{
	private VectorManager()
	{
	}
	
	VectorManager(MapScene scene)
	{
		initialise(scene);
	}
	
	public void finalize()
	{
		dispose();
	}
	
	// Add vectors to the scene and return an ID to track them
	public native long addVectors(List<VectorObject> vecs,VectorInfo vecInfo,ChangeSet changes);
	
	// Remove vectors by ID
	public native void removeVectors(long ids[],ChangeSet changes);
	
	// Enable/disable vectors by ID
	public native void enableVectors(long ids[],boolean enable,ChangeSet changes);
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(MapScene scene);
	native void dispose();
	private long nativeHandle;
}
