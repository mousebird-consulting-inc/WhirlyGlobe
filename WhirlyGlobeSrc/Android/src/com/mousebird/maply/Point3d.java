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
	
	public String toString()
	{
		return "(" + getX() + "," + getY() + "," + getZ() + ")";
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
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
	private long nativeHandle;
}
