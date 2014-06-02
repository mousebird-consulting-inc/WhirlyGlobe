/*
 *  MarkerInfo.java
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
public class MarkerInfo 
{
	public MarkerInfo()
	{
		initialise();
	}
	
	public void finalize()
	{
		dispose();
	}
	
	/**
	 * Create markers enabled or disabled.  The enable can be changed later.
	 */
	public native void setEnable(boolean enable);
	
	/**
	 * Set the drawOffset for a marker.  This is rarely used.
	 */
	public native void setDrawOffset(float drawOffset);
	
	/**
	 * Set the drawPriority for a marker.  Draw priority controls the order
	 * in which features are drawn.  For screen markers, drawPriority controls
	 * what order markers are drawn with respect to other screen objects, like labels,
	 * but not 3D objects like stickers or vectors.
	 */
	public native void setDrawPriority(int drawPriority);

	/**
	 * Set the minimum cutoff for visibility of the markers.  This is the closest height the
	 * markers will be visible from.  Defaults to 0.0 (always visible).
	 */
	public native void setMinVis(float minVis);
	
	/**
	 * Set the maximum cutoff for visibility of the markers.  This is the biggest height the
	 * markers will be visible from.  Defaults to off.
	 */
	public native void setMaxVis(float maxVis);
	
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
	public native void setColor(float r,float g,float b,float a);
	
	/**
	 * The amount of time (in seconds) it takes for new geometry
	 * to fade in and fade out.  By default, fade is off.
	 */
	public native void setFade(float fade);

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
	private long nativeHandle;
}
