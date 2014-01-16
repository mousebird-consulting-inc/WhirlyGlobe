package com.mousebirdconsulting.maply;

public class Point2d 
{
	Point2d()
	{
		initialise();
	}
	Point2d(double x,double y)
	{
		initialise();
		setValue(x,y);
	}
	
	/**
	 * Create a Point2D geo coordinate from degrees.
	 * @param lon Longitude first in degrees.
	 * @param lat Then latitude in degrees.
	 * @return
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

	public native double getX();
	public native double getY();
	public native void setValue(double x,double y);
	
	public native void initialise();
	public native void dispose();
	private long nativeHandle;
}