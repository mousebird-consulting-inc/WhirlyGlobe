/*
 *  Mbr.java
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
 * A simple Maply bounding box.  The coordinates will be in a particular
 * coordinate system, such as WGS84 geographic or spherical mercator,
 * though we do not call that out here.
 *
 */
public class Mbr 
{
	/**
	 * Lower left corner of the bounding box.
	 */
	Point2d ll = null;
	
	/**
	 * Upper right corner of the bounding box.
	 */
	Point2d ur = null;
	
	/**
	 * Construct the bounding box empty.
	 */
	Mbr()
	{
	}
	
	/**
	 * Construct with the lower left and upper right coordinates.
	 * 
	 * @param inLL Lower left corner of the bounding box.
	 * @param inUR Upper right corner of the bounding box.
	 */
	Mbr(Point2d inLL,Point2d inUR)
	{		
		ll = new Point2d(inLL);
		ur = new Point2d(inUR);
	}
	
	/**
	 * The span is just the upper right minus the lower left.
	 */
	public Point2d span()
	{
		return new Point2d(ur.getX()-ll.getX(),ur.getY()-ll.getY());
	}
	
	/**
	 * True if the bounding box has been set.  False if it is unitialized.
	 */
	boolean isValid()
	{
		return ll != null && ur != null;
	}
}
