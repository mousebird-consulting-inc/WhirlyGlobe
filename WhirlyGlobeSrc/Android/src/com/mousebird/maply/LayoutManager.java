package com.mousebird.maply;

/**
 * The layout manager interfaces to the Maply C++/JNI side of things and
 * is invisible to toolkit users.
 * 
 * @author sjg
 *
 */
class LayoutManager 
{
	private LayoutManager()
	{		
	}
	
	LayoutManager(MapScene scene)
	{
		initialise(scene);
	}
	
	public void finalize()
	{
		dispose();
	}
	
	/**
	 * Set the total number of objects we'll display at once.
	 *
	 * @param numObjects Maximum number of objects to display.
	 */
	public native void setMaxDisplayObjects(int numObjects);
	
	/**
	 * Run the layout logic on the currently active objects.  Any
	 * changes will be reflected in the ChangeSet.
	 * 
	 * @param viewState View state to use for the display.
	 * @param changes Changes to propagate to the scene.
	 */
	public native void updateLayout(ViewState viewState,ChangeSet changes);
	
	/**
	 * True if there were any changes since layout was last run.
	 */
	public native boolean hasChanges();
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(MapScene scene);
	native void dispose();
	private long nativeHandle;
}
