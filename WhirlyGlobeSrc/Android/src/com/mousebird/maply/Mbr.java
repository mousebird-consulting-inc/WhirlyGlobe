package com.mousebird.maply;

/**
 * A simple Maply bounding box.  The coordinates will be in a particular
 * coordinate system, such as WGS84 geographic or spherical mercator,
 * though we do not call them out here.
 * 
 * @author sjg
 *
 */
public class Mbr 
{
	/**
	 * Lower left corner of the bounding box.
	 */
	Point2d ll = null;
	
	/**
	 * Upper right corner of the bounding box.
	 */
	Point2d ur = null;
	
	/**
	 * Construct the bounding box empty.
	 */
	Mbr()
	{
	}
	
	/**
	 * Construct with the lower left and upper right coordinates.
	 */
	Mbr(Point2d inLL,Point2d inUR)
	{		
		ll = new Point2d(inLL);
		ur = new Point2d(inUR);
	}
	
	/**
	 * The span is just the upper right minus the lower left.
	 */
	public Point2d span()
	{
		return new Point2d(ur.getX()-ll.getX(),ur.getY()-ll.getY());
	}
	
	/**
	 * True if the bounding box has been set.  False if it is unitialized.
	 */
	boolean isValid()
	{
		return ll != null && ur != null;
	}
}
