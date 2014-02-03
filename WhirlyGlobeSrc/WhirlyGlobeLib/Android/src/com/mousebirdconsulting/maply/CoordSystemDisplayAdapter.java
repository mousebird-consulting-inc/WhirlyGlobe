package com.mousebirdconsulting.maply;

/**
 * The coordinate system display adapter controls the conversion from source
 * data coordinates to display coordinates.  It's completely opaque outside
 * of the toolkit.
 * 
 * @author sjg
 *
 */
class CoordSystemDisplayAdapter 
{
	CoordSystemDisplayAdapter()
	{
		initialise();
	}
	
	public void finalize()
	{
		dispose();
	}

	native void initialise();
	native void dispose();
	private long nativeHandle;
}
