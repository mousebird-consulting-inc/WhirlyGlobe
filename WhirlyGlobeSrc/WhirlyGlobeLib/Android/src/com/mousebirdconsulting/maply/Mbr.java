package com.mousebirdconsulting.maply;

// Simple bounding box
public class Mbr 
{
	Point2d ll = null;
	Point2d ur = null;
	
	Mbr()
	{
	}
	Mbr(Point2d inLL,Point2d inUR)
	{		
		ll = new Point2d(inLL);
		ur = new Point2d(inUR);
	}
	
	public Point2d span()
	{
		return new Point2d(ur.getX()-ll.getX(),ur.getY()-ll.getY());
	}
	
	boolean isValid()
	{
		return ll != null && ur != null;
	}
}
