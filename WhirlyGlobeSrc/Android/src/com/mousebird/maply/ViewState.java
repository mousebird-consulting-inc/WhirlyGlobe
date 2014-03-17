package com.mousebird.maply;

/**
 * The view state encapulates what's in a view at a certain point in time.
 * It's here so we can pass that around without fear of making a mess
 * 
 * @author sjg
 *
 */
class ViewState 
{
	private ViewState()
	{
	}
	
	ViewState(MapView view,MaplyRenderer renderer)
	{
		initialise(view,renderer);
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
	native void initialise(MapView view,MaplyRenderer renderer);
	native void dispose();
	private long nativeHandle;
}
