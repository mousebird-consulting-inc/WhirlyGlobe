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
	
	GlobeScene(CoordSystemDisplayAdapter coordAdapter)
	{
		initialise(coordAdapter,charRenderer);
	}
	
	public void finalize()
	{
		dispose();
	}
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(CoordSystemDisplayAdapter coordAdapter,CharRenderer charRenderer);
	native void dispose();
}
