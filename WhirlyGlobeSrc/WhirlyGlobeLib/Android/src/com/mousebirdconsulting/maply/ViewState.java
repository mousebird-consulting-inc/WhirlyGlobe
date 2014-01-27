package com.mousebirdconsulting.maply;

// The view state encapulates what's in a view at a certain point in time.
// It's here so we can pass that around without fear of making a mess
public class ViewState 
{
	ViewState(MapView view,MaplyRenderer renderer)
	{
		initialise(view,renderer);
	}

	public void finalize()
	{
		dispose();
	}

	public native void initialise(MapView view,MaplyRenderer renderer);
	public native void dispose();
	private long nativeHandle;
}
