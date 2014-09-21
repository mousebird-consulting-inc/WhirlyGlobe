/*
 *  ScreenMarker.java
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
 * The screen marker is a 2D rectangle that tracks a certain position in
 * geographic (lon/lat in radians).  Specific information set within
 * this object and more general information in the associated MarkerInfo.
 * 
 */
public class ScreenMarker 
{
	/**
	 * The location in geographic (WGS84) radians.  x is longitude, y is latitude.
	 * The marker will track this location.
	 */
	public Point2d loc = null;
	
	/**
	 * Size of the marker on screen.
	 */
	public Point2d size = new Point2d(16,16);
	
	/**
	 * If non-zero we'll rotate the label that number of radians around the attachment
	 * point.
	 */
	public double rotation = 0.0;

	/**
	 * If set, this is the image we'll use for this marker.  You'll need to
	 * set up the NamedBitmap, but the MaplyController will track the rest,
	 * including information about reuse.
	 */
	public NamedBitmap image = null;

	/**
	 * Background color for a marker can be overridden at this level.
	 */
	public int color = 0xFFFFFFFF;
	
	// Importance value for the layout engine
//	public float layoutImportance = Float.MAX_VALUE;
	
	/**
	 * If non-null an offset to tweak the label by.  We'll move the label
	 * this number of pixels from where it normally would be.
	 */
	public Point2d offset = null;
	
	// If set, this object is selectable
//	public boolean selectable = false;
	
	/**
	 * For selection, we include a user accessible object pointer.  You use
	 * this to identify a marker if you're doing user selection.
	 */
	public Object userObject = null;
}
