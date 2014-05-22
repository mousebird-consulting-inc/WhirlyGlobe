package com.mousebird.maply;

/**
 * The map scene represents an internal Maply Scene object and
 * is completely opaque to toolkit users.
 * 
 * @author sjg
 *
 */
class MapScene
{
	// Used to render individual characters using Android's Canvas/Paint/Typeface
	CharRenderer charRenderer = new CharRenderer();
	
	private MapScene()
	{
	}
	
	MapScene(CoordSystemDisplayAdapter coordAdapter)
	{
		initialise(coordAdapter,charRenderer);
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
	native void initialise(CoordSystemDisplayAdapter coordAdapter,CharRenderer charRenderer);
	native void dispose();
	private long nativeHandle;
}
