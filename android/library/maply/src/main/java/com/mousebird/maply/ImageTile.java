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
 * The Maply Image Tile represents the image(s) passed back from the network.
 * Odds are something else is creating them, so you don't need to do so
 * yourself.  They wrap a low level bytes sort of implementation.
 *
 */
public class ImageTile
{
	ImageTile()
	{
		initialise();
	}

	ImageTile(Bitmap bitmap)
	{
		initialise();
		setBitmap(bitmap);
	}

	private native void setBitmap(Bitmap bitmap);

	/**
	 * If the image has a border built in, set that here.
	 */
	public native void setBorderSize(int borderSize);

	/**
	 * Turn the data into a raw texture.  This can be down later, but if you're on
	 * your own thread, you may just want to do it here.
	 */
	public native void preprocessTexture();

	public void finalize() { dispose(); }

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
	private long nativeHandle;
}
