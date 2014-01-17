package com.mousebirdconsulting.maply;

import android.graphics.Bitmap;

/**
 * The named bitmap lets us track Bitmap objects via a filename or other
 * unique string.  Thus we can cache Textures in the rendering engine.
 * @author sjg
 *
 */
public class NamedBitmap 
{
	NamedBitmap(String inName,Bitmap inBitmap)
	{
		name = inName;
		bitmap = inBitmap;
	}
		
	String name;
	Bitmap bitmap;
}
