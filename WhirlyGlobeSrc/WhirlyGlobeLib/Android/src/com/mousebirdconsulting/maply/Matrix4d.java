package com.mousebirdconsulting.maply;

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
		
	public native void initialise();
	public native void dispose();
	private long nativeHandle;
}
