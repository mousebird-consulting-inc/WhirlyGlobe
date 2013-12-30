package com.mousebirdconsulting.maply;

public class MapScene 
{
	MapScene()
	{
		initialise();
	}

	public native void initialise();
	public native void dispose();
	private long nativeHandle;
}
