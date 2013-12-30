package com.mousebirdconsulting.maply;

public class MapView 
{
	MapView()
	{
		initialise();
	}
	
	// Set the view location (including height)
	public native void setLoc(double x,double y,double z);
	public native void initialise();
	public native void dispose();
	private long nativeHandle;
}
