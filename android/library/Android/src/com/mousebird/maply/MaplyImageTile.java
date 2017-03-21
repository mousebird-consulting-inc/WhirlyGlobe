/*
 *  MaplyImageTile.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2014 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

package com.mousebird.maply;

import android.graphics.Bitmap;


/**
 * The Maply Image Tile represents the image(s) passed back to a QuadImagePagingLayer.
 * Normally, you shouldn't be creating these.  However, if you have an object that
 * implements the QuadImageTileLayer.TileSource, you'll need to fill these in and
 * return them on demand.
 *
 */
public class MaplyImageTile 
{
	Bitmap[] bitmaps = null;
	Bitmap bitmap = null;
	
	/**
	 * Construct with a bitmap.
	 * @param bitmap The bitmap we're handing back
	 */
	public MaplyImageTile(Bitmap inBitmap)
	{
		bitmap = inBitmap;
	}

	public MaplyImageTile(Bitmap[] inBitmaps)
	{
		bitmaps = inBitmaps;
	}
}
