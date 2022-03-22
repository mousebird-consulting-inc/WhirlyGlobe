/*  MarkerInfo.java
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
 */

package com.mousebird.maply;

import android.graphics.Color;
import android.os.Build;

import androidx.annotation.ColorInt;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;

import org.jetbrains.annotations.NotNull;

/**
 * This class holds the visual information for a set of 2D or 3D markers.
 * Rather than have each of those represent their own visual information,
 * we share it here.
 * <p>
 * Toolkit users fill this class out and pass it into the addScreenMarkers()
 * or addMarkers() call on the MaplyController.
 * 
 * @author sjg
 *
 */
public class MarkerInfo extends BaseInfo
{
	/**
	 * Default priority for markers.  Screen markers offset this by a large amount.
	 */
	public static int MarkerPriorityDefault = 40000;
	public static int ScreenMarkerPriorityDefault = 140000;

	public MarkerInfo()
	{
		initialise();
		setColor(1.f,1.f,1.f,1.f);
		setDrawPriority(ScreenMarkerPriorityDefault);
		setClusterGroup(-1);
	}
	
	public void finalize() {
		dispose();
	}

	/**
	 * Set the color from a standard Android Color value.
	 * @param argb Color value, including alpha.
	 */
	public native void setColorARGB(@ColorInt int argb);
	public native @ColorInt int getColorARGB();

	public void setColor(@ColorInt int color) {
		setColorARGB(color);
	}

	@RequiresApi(api = Build.VERSION_CODES.O)
	public Color getColor() {
		return Color.valueOf(getColorARGB());
	}

	@RequiresApi(api = Build.VERSION_CODES.O)
	public void setColor(@NotNull Color color) {
		setColorARGB(color.toArgb());
	}

	public void setColor(float r,float g,float b,float a) {
		setComponents((int)(r*255),(int)(g*255),(int)(b*255),(int)(a*255));
	}

	/**
	 * Set the background color for the markers.  If the marker has a texture, this will
	 * be multiplied with the texture.
	 * Color components range from 0.0 to 1.0.
	 * 
	 * @param r red
	 * @param g green
	 * @param b blue
	 * @param a alpha
	 */
	private native void setComponents(int r,int g,int b,int a);

	/**
	 * Set the color expression, overriding color
	 */
	public native void setColorExp(@Nullable ColorExpressionInfo expr);
	public native ColorExpressionInfo getColorExp();

	/**
	 * Set the opacity expression, overriding the alpha component of color/colorExpr
	 */
	public native void setOpacityExp(@Nullable FloatExpressionInfo expr);
	public native FloatExpressionInfo getOpacityExp();

	/**
	 * Set the scale expression
	 */
	public native void setScaleExp(@Nullable FloatExpressionInfo expr);
	public native FloatExpressionInfo getScaleExp();

	/**
	 * Set the default layout importance for any markers added with this info.
	 *
	 * The layout engine controls how text and screen objects are displayed.
	 * It tries to avoid overlaps, placing objects with larger importance values first.
	 * <p>
	 * Defaults to MAXFLOAT, which means the layout engine does not control the markers.
	 * <p>
	 * Note that any value other than MAXFLOAT on an individual marker overrides the value set
	 * here, but a value other than MAXFLOAT here cannot be overridden by MAXFLOAT on a marker.
	 * So if some markers in a group added with this info object are to use layout and others not,
	 * it must be set to MAXFLOAT here.
	 */
	public native void setLayoutImportance(float newImport);
	public native float getLayoutImportance();

	/**
	 * If greater than -1 we'll sort these markers into cluster groups when zooming out.
	 * The number passed in determines which group it's sorted into.
	 */
	public native void setClusterGroup(int clusterGroup);
	public native int getClusterGroup();

	/**
	 * Set the zoom slot to use for expression-based properties.
	 *
	 * Must be set for expression properties to work correctly
	 */
	public native void setZoomSlot(int slot);
	public native int getZoomSlot();

	/**
	 * Set the shader program to use
	 */
	public void setShader(@Nullable Shader shader) {
		if (shader != null) {
			setShaderProgramId(shader.getID());
		} else {
			setShaderProgramId(0);

		}
	}

	/**
	 * Set the shader program to use
	 */
	public native void setShaderProgramId(long id);
	public native long getShaderProgramId();

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
}
