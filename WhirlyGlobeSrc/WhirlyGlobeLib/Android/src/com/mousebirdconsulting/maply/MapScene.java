package com.mousebirdconsulting.maply;

/**
 * The map scene represents an internal Maply Scene object and
 * is completely opaque to toolkit users.
 * 
 * @author sjg
 *
 */
class MapScene
{
	private MapScene()
	{
	}
	
	MapScene(CoordSystemDisplayAdapter coordAdapter)
	{
		initialise(coordAdapter);
	}
	
	public void finalize()
	{
		dispose();
	}
	
	// Flush the given changes out to the Scene
	native void addChanges(ChangeSet changes);

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(CoordSystemDisplayAdapter coordAdapter);
	native void dispose();
	private long nativeHandle;
}
