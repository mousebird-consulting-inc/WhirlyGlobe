package com.mousebirdconsulting.maply;

import android.graphics.Bitmap;

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
	
	public native void initialise();
	public native void dispose();
	private long nativeHandle;
}
