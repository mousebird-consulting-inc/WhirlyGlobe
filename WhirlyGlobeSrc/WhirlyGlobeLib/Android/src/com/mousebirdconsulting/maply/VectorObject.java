package com.mousebirdconsulting.maply;

public class VectorObject 
{
	public VectorObject()
	{
		initialise();
	}
	
	public void finalize()
	{
		dispose();
	}
	
	// Read objects from a geoJSON file
	public native boolean fromGeoJSON(String json);
	
	public native void initialise();
	public native void dispose();
	private long nativeHandle;
	
	static {
		System.loadLibrary("Maply");
	}
}
