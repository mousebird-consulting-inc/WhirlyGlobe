package com.mousebirdconsulting.maply;

/**
 * This encapsulates a Maply4d matrix.  In general, we don't manipulate
 * them, but we do pass them around.
 * 
 * @author sjg
 *
 */
public class Matrix4d 
{
	Matrix4d()
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
