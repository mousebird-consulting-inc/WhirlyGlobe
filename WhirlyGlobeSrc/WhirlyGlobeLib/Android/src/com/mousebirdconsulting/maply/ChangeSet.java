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
	
	// Add a texture to the list of changes to the scene
	public native void addTexture(Texture texture);
	
	// Remove a texture from the scene by ID
	public native void removeTexture(long texID);

	public native void initialise();
	public native void dispose();
	private long nativeHandle;
}
