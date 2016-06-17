package com.mousebird.maply;

/**
 * The selection manager holeds the objects 
 */
class SelectionManager 
{
	private SelectionManager()
	{		
	}
	
	SelectionManager(Scene scene)
	{
		initialise(scene);
	}
	
	public void finalize()
	{
		dispose();
	}

	// Look for an object that the selection manager is handling
	public native long pickObject(View view,Point2d screenLoc);

	// Look for a list of objects the selection manager is handling
	public native SelectedObject[] pickObjects(View view,Point2d screenLoc);
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(Scene scene);
	native void dispose();
	private long nativeHandle;
	private long nativeSceneHandle;
}
