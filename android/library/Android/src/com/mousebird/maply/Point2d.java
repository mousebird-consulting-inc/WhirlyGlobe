/*
 *  Point2d.java
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
 * The Point2d class is the simple, dumb, 2D coordinate
 * class.  The only things of note is this is wrapping the
 * internal Maply coordinate class and gets passed around
 * a lot.
 * 
 * @author sjg
 *
 */
public class Point2d 
{
	/**
	 * Construct with empty values.
	 */
	public Point2d()
	{
		initialise();
	}
	
	/**
	 * Construct from an existing Point2d
	 */
	public Point2d(Point2d that)
	{
		initialise();
		setValue(that.getX(),that.getY());
	}
	
	/**
	 * Construct from two values.
	 */
	public Point2d(double x,double y)
	{
		initialise();
		setValue(x,y);
	}

	@Override
	public boolean equals(Object obj)
	{
		if (!(obj instanceof Point2d))
			return false;
		Point2d that = (Point2d) obj;

		return getX() == that.getX() && getY() == that.getY();
	}

	public Point2d addTo(Point2d that)
	{
		return new Point2d(getX()+that.getX(),getY()+that.getY());
	}
	
	public Point2d multiplyBy(double t)
	{
		return new Point2d(getX()*t,getY()*t);
	}
	
	/**
	 * Create a Point2D geo coordinate from degrees.
	 * @param lon Longitude first in degrees.
	 * @param lat Then latitude in degrees.
	 * @return A Point2d in radians.
	 */
	public static Point2d FromDegrees(double lon,double lat)
	{
		return new Point2d(lon/180.0 * Math.PI,lat/180 * Math.PI);
	}

	/**
	 * Points are normally stored as radians, if they're positions on the globe or map.
	 * This converts and returns degrees.
     */
	public Point2d toDegrees()
	{
		return new Point2d(getX()/Math.PI * 180,getY()/Math.PI * 180);
	}
	
	public void finalize()
	{
		dispose();
	}

	public String toString()
	{
		return "(" + getX() + "," + getY() + ")";
	}

	/**
	 * Return the X value.
	 */
	public native double getX();
	/**
	 * Return the Y value.
	 */
	public native double getY();
	/**
	 * Set the value of the point.
	 */
	public native void setValue(double x,double y);
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
	private long nativeHandle;
}