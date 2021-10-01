/*  Point3d.java
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

import androidx.annotation.Keep;
import androidx.annotation.Nullable;

import org.jetbrains.annotations.NotNull;

/**
 * The Point3d class is your standard x,y,z container.  The only
 * thing of note here is this is a wrapper for the interal Maply
 * 3D point class.
 * 
 * @author sjg
 */
public class Point3d 
{
	/**
	 * Initialize empty.
	 */
	public Point3d() {
		this(0,0,0);
	}
	
	/**
	 * Make a copy from the given Point3d
	 */
	@SuppressWarnings("CopyConstructorMissesField")	// represents a new native object
	public Point3d(@NotNull Point3d that) {
		this(that.getX(),that.getY(),that.getZ());
	}

	public Point3d(@NotNull Point2d start, double z) {
		this(start.getX(),start.getY(),z);
	}
	
	/**
	 * Initialize with 3 doubles.
	 */
	public Point3d(double x,double y,double z) {
		initialise(x, y, z);
	}
	
	public void finalize() {
		dispose();
	}

	@Override
	public boolean equals(Object obj) {
		if (!(obj instanceof Point3d)) {
			return false;
		}

		final Point3d that = (Point3d)obj;
		return getX() == that.getX() &&
		       getY() == that.getY() &&
		       getZ() == that.getZ();
	}

	public String toString() {
		return "(" + getX() + "," + getY() + "," + getZ() + ")";
	}
	
	/**
	 * Add this Point to the one given and return the result.
	 */
	@NotNull
	public Point3d addTo(@NotNull Point3d that) {
		return new Point3d(getX()+that.getX(),getY()+that.getY(),getZ()+that.getZ());
	}
	
	/**
	 * Scale this point as a vector by the given value.
	 */
	@NotNull
	public Point3d multiplyBy(double t) {
		return new Point3d(getX()*t,getY()*t,getZ()*t);
	}

	/**
	 * Return this point minus the one given.
	 */
	@NotNull
	public Point3d subtract(@NotNull Point3d pt1) {
		return new Point3d(getX()-pt1.getX(),getY()-pt1.getY(),getZ()-pt1.getZ());
	}

	/**
	 * Calculate the length of this as a vector.
	 */
	public double lengthSquared() {
		final double x = getX(), y = getY(), z = getZ();
		return x*x+y*y+z*z;
	}

	/**
	 * Calculate the length of this as a vector.
	 */
	public double length() {
		final double x = getX(), y = getY(), z = getZ();
		return Math.sqrt(x*x+y*y+z*z);
	}
	
	/**
	 * Return the normalized vector.
	 */
	@NotNull
	public Point3d normalized() {
		final double len = length();
		return (len == 0.0) ? new Point3d() : new Point3d(getX()/len,getY()/len,getZ()/len);
	}

	/**
	 * Normalize this as a vector.
	 */
	public void normalize() {
		final double len = length();
		if (len != 0.0) {
			setValue(getX() / len, getY() / len, getZ() / len);
		} else {
			setValue(0,0,0);
		}
	}

	/**
	 * Dot product
     */
	public double dot(@NotNull Point3d pt) {
		return getX() * pt.getX() + getY() * pt.getY() + getZ() * pt.getZ();
	}

	/**
	 * Truncate a Point3d and return just the 2D values
	 * 
	 * @return X and Y from the Point3d
	 */
	@NotNull
	public Point2d toPoint2d() {
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
	@Nullable
	public native Point3d cross(Point3d that);

	public native double norm();

	static {
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(double x, double y, double z);
	native void dispose();

	@Keep
	@SuppressWarnings("unused")	// Used by JNI
	private long nativeHandle;
}
