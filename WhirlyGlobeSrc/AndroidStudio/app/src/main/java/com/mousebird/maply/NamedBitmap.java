/*
 *  NamedBitmap.java
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
 * The named bitmap lets us track Bitmap objects via a filename or other
 * unique string.  Thus we can cache Textures in the rendering engine.
 *
 */
public class NamedBitmap 
{
	/**
	 * Construct with the name, which should be unique, and the bitmap
	 * to use for the texture.
	 * 
	 * @param inName Name of the bitmap.  Probably the filename or resource name.
	 * @param inBitmap Bitmap to use for the texture.
	 */
	public NamedBitmap(String inName,Bitmap inBitmap)
	{
		name = inName;
		bitmap = inBitmap;
	}
		
	public String name;
	public Bitmap bitmap;
}
