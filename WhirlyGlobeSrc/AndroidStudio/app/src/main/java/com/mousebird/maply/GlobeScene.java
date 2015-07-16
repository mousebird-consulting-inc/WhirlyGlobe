package com.mousebird.maply;

/**
 * This is a subclass of the Scene specifically for Globes.  You never create these yourself, though.
 * 
 * @author sjg
 *
 */
public class GlobeScene extends Scene
{
	private GlobeScene()
	{
	}
	
	GlobeScene(CoordSystemDisplayAdapter coordAdapter,int cullTreeDepth)
	{
		initialise(coordAdapter,charRenderer,cullTreeDepth);
	}
	
	public void finalize()
	{
		dispose();
	}
	
	// Flush the given changes out to the Scene
	@Override public void addChanges(ChangeSet changes)
	{
		addChangesNative(changes);
	}

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(CoordSystemDisplayAdapter coordAdapter,CharRenderer charRenderer,int cullTreeDepth);
	native void dispose();
}
