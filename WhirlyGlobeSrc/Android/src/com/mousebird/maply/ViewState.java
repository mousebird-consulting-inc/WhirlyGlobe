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
	
	/**
	 * Compare two view states and return true if they're equal.
	 * 
	 * @param viewState View state to compare.
	 * @return Return true if the view states are the same.
	 */
	public native boolean isEqual(ViewState viewState);

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(MapView view,MaplyRenderer renderer);
	native void dispose();
	private long nativeHandle;
}
