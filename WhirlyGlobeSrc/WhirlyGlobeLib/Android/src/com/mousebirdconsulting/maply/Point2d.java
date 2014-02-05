package com.mousebirdconsulting.maply;

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
	Point2d()
	{
		initialise();
	}
	
	/**
	 * Construct from an existing Point2d
	 */
	Point2d(Point2d that)
	{
		initialise();
		setValue(that.getX(),that.getY());
	}
	
	/**
	 * Construct from two values.
	 */
	Point2d(double x,double y)
	{
		initialise();
		setValue(x,y);
	}
	
	/**
	 * Create a Point2D geo coordinate from degrees.
	 * @param lon Longitude first in degrees.
	 * @param lat Then latitude in degrees.
	 * @return A Point2d in radians.
	 */
	static Point2d FromDegrees(double lon,double lat)
	{
		return new Point2d(lon/180.0 * Math.PI,lat/180 * Math.PI);
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