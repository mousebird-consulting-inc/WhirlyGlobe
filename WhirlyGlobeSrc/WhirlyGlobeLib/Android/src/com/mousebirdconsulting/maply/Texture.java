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
		bitmap = inBitmap;
	}
	
	public void finalize()
	{
		dispose();
	}

	// Textures need to be created with bitmaps
	public Bitmap bitmap = null;

	// Once created, this is how we identify it to the rendering engine
	long textureID = 0;
	
	public native void initialise();
	public native void dispose();
	private long nativeHandle;
}
