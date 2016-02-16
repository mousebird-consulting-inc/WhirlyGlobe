/*
 *  Point3d.java
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
 * The Point3d class is your standard x,y,z container.  The only
 * thing of note here is this is a wrapper for the interal Maply
 * 3D point class.
 * 
 * @author sjg
 *
 */
public class Point3d 
{
	/**
	 * Initialize empty.
	 */
	public Point3d()
	{
		initialise();
	}
	
	/**
	 * Make a copy from the given Point3d
	 */
	public Point3d(Point3d that)
	{
		initialise();		
		setValue(that.getX(),that.getY(),that.getZ());
	}
	
	/**
	 * Initialize with 3 doubles.
	 */
	public Point3d(double x,double y,double z)
	{
		initialise();
		setValue(x,y,z);
	}
	
	public void finalize()
	{
		dispose();
	}

	@Override
	public boolean equals(Object obj)
	{
		if (!(obj instanceof Point3d))
			return false;

		Point3d that = (Point3d) obj;
		return getX() == that.getX() && getY() == that.getY() && getZ() == that.getZ();
	}

	public String toString()
	{
		return "(" + getX() + "," + getY() + "," + getZ() + ")";
	}
	
	/**
	 * Add this Point to the one given and return the result.
	 */
	public Point3d addTo(Point3d that)
	{
		return new Point3d(getX()+that.getX(),getY()+that.getY(),getZ()+that.getZ());
	}
	
	/**
	 * Scale this point as a vector by the given value.
	 */
	public Point3d multiplyBy(double t)
	{
		return new Point3d(getX()*t,getY()*t,getZ()*t);
	}

	/**
	 * Return this point minus the one given.
	 */
	public Point3d subtract(Point3d pt1) 
	{
		return new Point3d(getX()-pt1.getX(),getY()-pt1.getY(),getZ()-pt1.getZ());
	}
	
	/**
	 * Calculate the length of this as a vector.
	 */
	public double length() 
	{
		double x = getX(), y = getY(), z = getZ();
		return Math.sqrt(x*x+y*y+z*z);
	}
	
	/**
	 * Return the normalized vector.
	 */
	public Point3d normalized()
	{
		double len = length();
		return new Point3d(getX()/len,getY()/len,getZ()/len);
	}

	/**
	 * Normalize this as a vector.
	 */
	public void normalize()
	{
		double len = length();
		setValue(getX()/len,getY()/len,getZ()/len);
	}

	/**
	 * Dot product
     */
	public double dot(Point3d pt)
	{
		return getX() * pt.getX() + getY() * pt.getY() + getZ() * pt.getZ();
	}

	/**
	 * Truncate a Point3d and return just the 2D values
	 * 
	 * @return X and Y from the Point3d
	 */
	public Point2d toPoint2d() 
	{
		return new Point2d(getX(),getY());
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
	 * Return the Z value.
	 */
	public native double getZ();
	/**
	 * Set the value of the point.
	 */
	public native void setValue(double x,double y,double z);
	
	/**
	 * Return the cross product of this vector and the one given.
	 */
	public native Point3d cross(Point3d that);
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
	private long nativeHandle;
}
