package com.mousebird.maply;

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

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
	private long nativeHandle;
}
