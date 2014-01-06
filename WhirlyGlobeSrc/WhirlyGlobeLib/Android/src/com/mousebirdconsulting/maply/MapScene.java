package com.mousebirdconsulting.maply;

class MapScene 
{
	MapScene(CoordSystemDisplayAdapter coordAdapter)
	{
		initialise(coordAdapter);
	}
	
	public void finalize()
	{
		dispose();
	}
	
	// Flush the given changes out to the Scene
	public native void addChanges(ChangeSet changes);

	public native void initialise(CoordSystemDisplayAdapter coordAdapter);
	public native void dispose();
	private long nativeHandle;
}
