package com.mousebirdconsulting.maply;

/**
 * The view state encapulates what's in a view at a certain point in time.
 * It's here so we can pass that around without fear of making a mess
 * 
 * @author sjg
 *
 */
class ViewState 
{
	ViewState(MapView view,MaplyRenderer renderer)
	{
		initialise(view,renderer);
	}

	public void finalize()
	{
		dispose();
	}

	native void initialise(MapView view,MaplyRenderer renderer);
	native void dispose();
	private long nativeHandle;
}
