package com.mousebirdconsulting.maply;

public class ChangeSet 
{
	ChangeSet()
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
