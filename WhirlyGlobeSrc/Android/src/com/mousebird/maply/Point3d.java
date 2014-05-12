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
	
	public Point3d addTo(Point3d that)
	{
		return new Point3d(getX()+that.getX(),getY()+that.getY(),getZ()+that.getZ());
	}
	
	public Point3d multiplyBy(double t)
	{
		return new Point3d(getX()*t,getY()*t,getZ()*t);
	}

	public Point3d subtract(Point3d pt1) 
	{
		return new Point3d(getX()-pt1.getX(),getY()-pt1.getY(),getZ()-pt1.getZ());
	}
	
	public double length() 
	{
		double x = getX(), y = getY(), z = getZ();
		return Math.sqrt(x*x+y*y+z*z);
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
