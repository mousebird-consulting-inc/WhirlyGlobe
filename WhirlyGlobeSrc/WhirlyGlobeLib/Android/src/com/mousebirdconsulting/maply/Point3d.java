package com.mousebirdconsulting.maply;

public class Point3d 
{
	Point3d()
	{
		initialise();
	}
	Point3d(Point3d that)
	{
		initialise();		
		setValue(that.getX(),that.getY(),that.getZ());
	}
	Point3d(double x,double y,double z)
	{
		initialise();
		setValue(x,y,z);
	}
	
	public void finalize()
	{
		dispose();
	}
	
	public String toString()
	{
		return "(" + getX() + "," + getY() + "," + getZ() + ")";
	}
	
	public native double getX();
	public native double getY();
	public native double getZ();
	public native void setValue(double x,double y,double z);
	
	public native void initialise();
	public native void dispose();
	private long nativeHandle;
}
