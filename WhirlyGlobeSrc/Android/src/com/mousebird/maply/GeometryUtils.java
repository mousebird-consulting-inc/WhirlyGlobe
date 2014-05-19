package com.mousebird.maply;

/**
 * A collection or random geometry utilities that aren't associated with particular objects.
 * 
 * @author sjg
 *
 */
public class GeometryUtils 
{
	private GeometryUtils()
	{
	}
	
	/**
	 * Test if the given point is inside the given polygon in 2D.
	 * 
	 * @param pt Point we're testing
	 * @param poly Polygon we're testing
	 * @return true if the poin is inside, false otherwise.
	 */
	public static boolean PointInPolygon(Point2d pt,Point2d ring[])
	{
		int ii,jj;
		boolean c = false;
		for (ii = 0, jj = ring.length-1; ii < ring.length; jj = ii++) {
			if ( ((ring[ii].getY()>pt.getY()) != (ring[jj].getY()>pt.getY())) &&
				(pt.getX() < (ring[jj].getX()-ring[ii].getX()) * (pt.getY()-ring[ii].getY()) / (ring[jj].getY()-ring[ii].getY()) + ring[ii].getX()) )
				c = !c;
		}
		return c;
	}
}
