package com.mousebirdconsulting.maply;

// Attribute dictionary used on vectors
public class AttrDictionary 
{
	public AttrDictionary()
	{
		initialise();
	}
	
	public void finalize()
	{
		dispose();
	}
	
	// Return a string for the given attribute
	public native String getString(String attrName);
	// Return an int for the given attribute or null if it's missing
	public native Integer getInt(String attrName);
	// Return a double for the given attribute or null if it's missing
	public native Double getDouble(String attrName);
	
	public native void initialise();
	public native void dispose();
	private long nativeHandle;	
}
