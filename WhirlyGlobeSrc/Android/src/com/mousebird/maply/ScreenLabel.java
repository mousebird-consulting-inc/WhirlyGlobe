/*
 *  ScreenLabel.java
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
 * The screen label is a 2D label that tracks a location on the map.
 * The label has a geographic location as well as information about
 * the text it's displaying and other control attributes.
 * <p>
 * It works in combination with a LabelInfo class to describe more
 * broad attributes such as color and font size.
 *
 */
public class ScreenLabel 
{
	/**
	 * The location in geographic (WGS84) radians.  x is longitude, y is latitude.
	 * The label will track this location.
	 */
	public Point2d loc = null;

	/**
	 * If non-zero we'll rotate the label that number of radians around the attachment
	 * point.
	 */
	public double rotation = 0.0;

	/**
	 * The text to display for the label.
	 */
	public String text;

	/**
	 * If non-null an offset to tweak the label by.  We'll move the label
	 * this number of pixels from where it normally would be.
	 */
	public Point2d offset = null;
	
	// If set, this object is selectable
	public boolean selectable = false;

	// If set, this is the individual label's layout importance
	public float layoutImportance = 0.f;

	/**
	 * For selection, we include a user accessible object pointer.  You use
	 * this to identify a label if you're doing user selection.
	 */
	public Object userObject = null;
}
