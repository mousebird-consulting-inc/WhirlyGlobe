/*
 *  Texture.java
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
 * Encapsulates a Maply Texture.  This is opaque to toolkit users.  They'll
 * use a MaplyTexture instead.
 *
 */
class Texture 
{
	/**
	 * Create an empty texture.
	 */
	public Texture()
	{
		initialise();
	}
	
	/**
	 * Initialize a texture with a bitmap.  The contents of the bitmap will fill in the texture.
	 */
	public Texture(Bitmap inBitmap,QuadImageTileLayer.ImageFormat imageFormat)
	{
		setBitmap(inBitmap,imageFormat.ordinal());
	}

	/**
	 * Set wrapping parameters.
     */
	public native void setSettings(boolean wrapU,boolean wrapV);

	public void finalize()
	{
		dispose();
	}
	
	/**
	 * This scoops out the bytes and creates an actual Maply Texture.
	 * 
	 * @param inBitmap Bitmap to use for the Texture.
	 * @return Returns false if it can't figure it out
	 */
	public native boolean setBitmap(Bitmap inBitmap,int imageFormat);
	
	// Once created, this is how we identify it to the rendering engine
	public native long getID();
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
	private long nativeHandle;
}
