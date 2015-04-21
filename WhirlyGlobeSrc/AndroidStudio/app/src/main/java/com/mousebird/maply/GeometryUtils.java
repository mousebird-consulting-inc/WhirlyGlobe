/*
 *  GeometryUtils.java
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
 * A collection or random geometry utilities that aren't associated with particular objects.
 * 
 */
public class GeometryUtils 
{
	private GeometryUtils()
	{
	}
	
	/**
	 * Test if the given point is inside the given polygon in 2D.
	 * 
	 * @param pt Point we're testing
	 * @param poly Polygon we're testing
	 * @return true if the point is inside, false otherwise.
	 */
	public static boolean PointInPolygon(Point2d pt,Point2d ring[])
	{
		int ii,jj;
		boolean c = false;
		for (ii = 0, jj = ring.length-1; ii < ring.length; jj = ii++) {
			if ( ((ring[ii].getY()>pt.getY()) != (ring[jj].getY()>pt.getY())) &&
				(pt.getX() < (ring[jj].getX()-ring[ii].getX()) * (pt.getY()-ring[ii].getY()) / (ring[jj].getY()-ring[ii].getY()) + ring[ii].getX()) )
				c = !c;
		}
		return c;
	}
}
