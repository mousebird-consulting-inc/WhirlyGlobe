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

	native void initialise(CoordSystemDisplayAdapter coordAdapter);
	native void dispose();
	private long nativeHandle;
}
