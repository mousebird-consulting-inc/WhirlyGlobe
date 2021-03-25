/*  ScreenMarker.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2021 mousebird consulting
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

import android.graphics.Bitmap;

import java.util.ArrayList;

/**
 * The screen marker is a 2D rectangle that tracks a certain position in
 * geographic (lon/lat in radians).  Specific information set within
 * this object and more general information in the associated MarkerInfo.
 * 
 */
public class ScreenMarker
{	
	/**
	 * Unique ID used by Maply for selection.
	 */
	long ident = Identifiable.genID();
	
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
	 * If set, this is the image we'll use for this marker.
	 */
	public Bitmap image = null;

	/**
	 * If set, we'll use this texture which has already been converted to Maply format.
	 */
	public MaplyTexture tex = null;

	/**
	 * If set we'll animate these images one after the other over the duration.
	 */
	public MaplyTexture images[] = null;

	/**
	 * If images are passed in, this is the time it will take to cycle through them all.
	 * By default this is 5s.
	 */
	public double period = 5.0;

	/**
	 * Background color for a marker can be overridden at this level.
	 */
	public int color = 0xFFFFFFFF;

	/**
	 * This is the importance value passed to the layout engine.  Layout
	 * will take place in importance priority.  Set it to MAX_VALUE to avoid
	 * layout entirely.
	 */
	public float layoutImportance = Float.MAX_VALUE;

	/**
	 * If set, this is the size of the object as seen by the layout engine.
	 * If not set, it uses the display size
	 */
	public Point2d layoutSize = null;
	
	/**
	 * If non-null an offset to tweak the label by.  We'll move the label
	 * this number of pixels from where it normally would be.
	 */
	public Point2d offset = new Point2d(0,0);

	/**
	 * Turn this on if you want the marker object to be selectable.
	 */
	public boolean selectable = false;
	
	/**
	 * For selection, we include a user accessible object pointer.  You use
	 * this to identify a marker if you're doing user selection.
	 */
	public Object userObject = null;

	/**
	 * If set, these are vertex attributes to be applied to be consolidated and passed
	 * to a custom shader.  Each set of attributes will be copied for each individual vertex
	 * that's created.
	 */
	public ArrayList<VertexAttribute> vertexAttributes = null;
}
