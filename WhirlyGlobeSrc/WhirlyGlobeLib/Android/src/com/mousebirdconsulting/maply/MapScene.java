package com.mousebirdconsulting.maply;

class MapScene 
{
	MapScene()
	{
		initialise();
	}

	public native void initialise();
	public native void dispose();
	private long nativeHandle;
}
