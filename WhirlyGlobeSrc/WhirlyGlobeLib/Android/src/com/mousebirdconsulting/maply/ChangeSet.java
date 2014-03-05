package com.mousebirdconsulting.maply;

/**
 * The change set is a largely opaque object the Maply uses to
 * track visual changes in the map or globe.  Most of the action
 * takes place behind the scenes and users of the Maply API should
 * not be manipulating these.
 * 
 * @author sjg
 *
 */
class ChangeSet
{
	ChangeSet()
	{
		initialise();
	}
	
	// Add a texture to the list of changes to the scene
	public native void addTexture(Texture texture);
	
	// Remove a texture from the scene by ID
	public native void removeTexture(long texID);
	
	// Merge a new set of changes in at the end
	// This clears the changes in the ChangeSet passed in
	public native void merge(ChangeSet changes);
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	public void finalize()
	{
		dispose();
	}
	native void initialise();
	native void dispose();	
	private long nativeHandle;	
}
