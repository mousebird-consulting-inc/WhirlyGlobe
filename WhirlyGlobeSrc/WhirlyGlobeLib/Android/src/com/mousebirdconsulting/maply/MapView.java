package com.mousebirdconsulting.maply;

class MapView 
{
	MapView(CoordSystemDisplayAdapter coordAdapter)
	{
		initialise(coordAdapter);
	}
	
	public void finalize()
	{
		dispose();
	}
	
	// Set the view location (including height)
	public native void setLoc(double x,double y,double z);
	
	public native void initialise(CoordSystemDisplayAdapter coordAdapter);
	public native void dispose();
	private long nativeHandle;
}
