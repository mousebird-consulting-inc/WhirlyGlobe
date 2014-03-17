package com.mousebird.maply;

import android.graphics.Bitmap;


/**
 * The Maply Image Tile represents the image(s) passed back to a QuadImagePagingLayer.
 * 
 * @author sjg
 *
 */
public class MaplyImageTile 
{
	Bitmap bitmap;
	
	/**
	 * Construct with a bitmap.
	 * @param bitmap The bitmap we're handing back
	 */
	public MaplyImageTile(Bitmap inBitmap)
	{
		bitmap = inBitmap;
	}
}
