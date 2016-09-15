/*
 *  TextureManager.java
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

import java.util.TreeSet;

/**
 * This class is used by the MaplyController to reference count textures.
 * It should be entirely invisible to toolkit users.
 *
 */
class TextureManager 
{
	class TextureWrapper implements Comparable<TextureWrapper>
	{
		TextureWrapper(Bitmap inBitmap)
		{
			bitmap = inBitmap;
		}
				
		// The bitmap
		Bitmap bitmap = null;
		// Texture ID in the rendering engine
		long texID = 0;
		// Number of things using it
		int refs = 0;

		@Override
		public int compareTo(TextureWrapper that) 
		{
			int hash1 = bitmap.hashCode();  int hash2 = that.bitmap.hashCode();
			if (hash1 == hash2)
				return 0;
			if (hash1 < hash2)
				return -1;
			return 1;
		}		
	};
	
	TreeSet<TextureWrapper> textures = new TreeSet<TextureWrapper>();
	
	/**
	 * Create a texture or find an existing one corresponding to the named
	 * bitmap.  Returns the texture ID or EmptyIdentity on failure.
	 * @param theBitmap
	 * @param changes
	 * @return
	 */
	long addTexture(Bitmap theBitmap, Scene scene, ChangeSet changes)
	{
		synchronized (this) {
			// Find an existing one
			TextureWrapper testWrapper = new TextureWrapper(theBitmap);
			if (textures.contains(testWrapper)) {
				TextureWrapper existingBitmap = textures.floor(testWrapper);
				existingBitmap.refs++;
				return existingBitmap.texID;
			}

			// Need to create it
			Texture texture = new Texture();
			if (!texture.setBitmap(theBitmap))
				return MaplyBaseController.EmptyIdentity;
			testWrapper.refs = 1;
			testWrapper.texID = texture.getID();

			// After we call addTexture it's no longer ours to play with
			changes.addTexture(texture, scene);
			textures.add(testWrapper);
		}
				
		return testWrapper.texID;
	}
	
	/**
	 * Look for the given texture ID, decrementing its reference count or removing it.
	 * @param texID
	 */
	void removeTexture(long texID, ChangeSet changes)
	{
		synchronized (this) {
			for (TextureWrapper texWrap : textures) {
				if (texWrap.texID == texID) {
					texWrap.refs--;
					// Remove the texture
					if (texWrap.refs <= 0) {
						changes.removeTexture(texWrap.texID);
						textures.remove(texWrap);
					}

					return;
				}
			}
		}
	}
}
