package com.mousebirdconsulting.maply;

import android.graphics.Bitmap;

/**
 * The named bitmap lets us track Bitmap objects via a filename or other
 * unique string.  Thus we can cache Textures in the rendering engine.
 * 
 * @author sjg
 *
 */
public class NamedBitmap 
{
	/**
	 * Construct with the name, which should be unqiue, and the bitmap
	 * to use for the texture.
	 * 
	 * @param inName Name of the bitmap.  Probably the filename or resource name.
	 * @param inBitmap Bitmap to use for the texture.
	 */
	NamedBitmap(String inName,Bitmap inBitmap)
	{
		name = inName;
		bitmap = inBitmap;
	}
		
	String name;
	Bitmap bitmap;
}
