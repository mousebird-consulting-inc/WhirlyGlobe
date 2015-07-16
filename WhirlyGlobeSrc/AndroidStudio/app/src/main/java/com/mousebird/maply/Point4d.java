/*
 *  Point4d.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/20/15.
 *  Copyright 2011-2015 mousebird consulting
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
 * The Point4d class is your standard x,y,z,w container.  The only
 * thing of note here is this is a wrapper for the interal Maply
 * 4D point class.
 * 
 * @author sjg
 *
 */
public class Point4d 
{
	/**
	 * Initialize empty.
	 */
	public Point4d()
	{
		initialise();
	}
	
	/**
	 * Make a copy from the given Point4d
	 */
	public Point4d(Point4d that)
	{
		initialise();		
		setValue(that.getX(),that.getY(),that.getZ(),that.getW());
	}
	
	public Point4d(Point3d that,double w)
	{
		initialise();		
		setValue(that.getX(),that.getY(),that.getZ(),w);
	}
	
	/**
	 * Initialize with 3 doubles.
	 */
	public Point4d(double x,double y,double z,double w)
	{
		initialise();
		setValue(x,y,z,w);
	}
	
	public void finalize()
	{
		dispose();
	}
	
	public String toString()
	{
		return "(" + getX() + "," + getY() + "," + getZ() + getW() + ")";
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
	 * Return the W value.
	 */
	public native double getW();
	/**
	 * Set the value of the point.
	 */
	public native void setValue(double x,double y,double z,double w);
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
	private long nativeHandle;
}
