package com.mousebirdconsulting.maply;

public class VectorManager 
{
	VectorManager(MapScene scene)
	{
		initialise(scene);
	}

	// Add vectors to the scene and return an ID to track them
	public native long addVector(VectorObject vecObj);
	public native void initialise(MapScene scene);
	public native void dispose();
	private long nativeHandle;
}
