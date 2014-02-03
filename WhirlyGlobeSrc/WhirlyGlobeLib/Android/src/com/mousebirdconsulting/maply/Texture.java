package com.mousebirdconsulting.maply;

import android.graphics.Bitmap;

/**
 * Encapsulates a Maply Texture.  This is opaque to toolkit users.  They'll
 * use a NamedBitmap instead.
 * 
 * @author sjg
 *
 */
class Texture 
{
	Texture()
	{
		initialise();
	}
	Texture(Bitmap inBitmap)
	{
		setBitmap(inBitmap);
	}
	public void finalize()
	{
		dispose();
	}
	
	// This scoops out the bytes and creates an actual Maply Texture.
	// Returns false if it can't figure it out
	public native boolean setBitmap(Bitmap inBitmap);
	
	// Once created, this is how we identify it to the rendering engine
	public native long getID();
	
	native void initialise();
	native void dispose();
	private long nativeHandle;
}
