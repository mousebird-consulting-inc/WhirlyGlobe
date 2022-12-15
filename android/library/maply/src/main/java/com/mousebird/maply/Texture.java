/*
 *  Texture.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2022 mousebird consulting
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

import androidx.annotation.Keep;
import androidx.annotation.Nullable;

import org.jetbrains.annotations.NotNull;

/**
 * Encapsulates a Maply Texture.  This is opaque to toolkit users.  They'll
 * use a MaplyTexture instead.
 *
 */
@SuppressWarnings("unused")
class Texture
{
	/**
	 * Create an empty texture.
	 */
	public Texture() {
		initialise();
	}
	
	/**
	 * Initialize a texture with a bitmap.  The contents of the bitmap will fill in the texture.
	 */
	public Texture(Bitmap inBitmap, RenderController.ImageFormat imageFormat) {
		setBitmap(inBitmap, imageFormat);
	}

	/**
	 * Initialize a texture with a bitmap.  The contents of the bitmap will fill in the texture.
	 */
	public Texture(Bitmap inBitmap, RenderController.ImageFormat imageFormat,
				   RenderControllerInterface.TextureSettings.FilterType filterType) {
		this(inBitmap, imageFormat);
		setFilterType(filterType);
	}

	/**
	 * Set wrapping parameters.
     */
	public native void setSettings(boolean wrapU,boolean wrapV);

	/**
	 * This scoops out the bytes and creates an actual Maply Texture.
	 * 
	 * @param bitmap Bitmap to use for the Texture.
	 * @return Returns false if it can't figure it out
	 */
	public boolean setBitmap(Bitmap bitmap, RenderController.ImageFormat format) {
		return setBitmapNative(bitmap, format.ordinal());
	}
	public boolean setBitmap(Bitmap bitmap, RenderController.ImageFormat format, RenderControllerInterface.TextureSettings.FilterType filter) {
		setFilterType(filter);
		return setBitmapNative(bitmap, format.ordinal());
	}
	private native boolean setBitmapNative(Bitmap inBitmap,int imageFormat);
	
	/**
	 * Create an empty texture of the given size.
     */
	public native void setSize(int sizeX,int sizeY);

	/**
	 * Set if we're creating this texture empty
     */
	public native void setIsEmpty(boolean isEmpty);

	/**
	 * Once created, this is how we identify it to the rendering engine
	 */
	public native long getID();

	/**
	 * Set the pixel format
	 */
	@SuppressWarnings("ConstantConditions")
	public void setFormat(@NotNull RenderController.ImageFormat fmt) {
		if (fmt != null) {
			setFormatNative(fmt.ordinal());
		}
	}
	@Nullable
	public RenderController.ImageFormat getFormat() {
		final int fmt = getFormatNative();
		return (0 <= fmt && fmt < formatValues.length) ? formatValues[fmt] : null;
	}

	private final RenderController.ImageFormat[] formatValues = RenderController.ImageFormat.values();
	private native void setFormatNative(int fmt);
	private native int getFormatNative();

	/**
	 * Set the filtering type
	 */
	@SuppressWarnings("ConstantConditions")
	public void setFilterType(@NotNull RenderControllerInterface.TextureSettings.FilterType type) {
		if (type != null) {
			setFilterTypeNative(type.ordinal());
		}
	}
	@Nullable
	public RenderControllerInterface.TextureSettings.FilterType getFilterType() {
		final int type = getFilterTypeNative();
		return (0 <= type && type < filterTypeValues.length) ? filterTypeValues[type] : null;
	}

	private final RenderControllerInterface.TextureSettings.FilterType[] filterTypeValues =
			RenderControllerInterface.TextureSettings.FilterType.values();
	private native void setFilterTypeNative(int type);
	private native int getFilterTypeNative();

	protected void finalize() {
		dispose();
	}

	static {
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
	@Keep
	private long nativeHandle;
}
