package com.mousebird.maply;

/**
 * Base class for Scene.  Use either a MapScene or a GlobeScene instead.
 * 
 * @author sjg
 *
 */
public class Scene 
{
	// Used to render individual characters using Android's Canvas/Paint/Typeface
	CharRenderer charRenderer = new CharRenderer();
	
	protected Scene()
	{
	}
	
	// Overridden by the subclass
	public void addChanges(ChangeSet changes)
	{
	}
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void addChangesNative(ChangeSet changes);
	protected long nativeHandle;
}
