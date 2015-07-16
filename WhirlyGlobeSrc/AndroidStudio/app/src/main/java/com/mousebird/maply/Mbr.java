/*
 *  Mbr.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2014 mousebird consulting
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

import java.util.ArrayList;
import java.util.List;

/**
 * A simple Maply bounding box.  The coordinates will be in a particular
 * coordinate system, such as WGS84 geographic or spherical mercator,
 * though we do not call that out here.
 *
 */
public class Mbr 
{
	/**
	 * Lower left corner of the bounding box.
	 */
	public Point2d ll = null;
	
	/**
	 * Upper right corner of the bounding box.
	 */
	public Point2d ur = null;
	
	/**
	 * Construct the bounding box empty.
	 */
	public Mbr()
	{
	}
	
	/**
	 * Construct with the lower left and upper right coordinates.
	 * 
	 * @param inLL Lower left corner of the bounding box.
	 * @param inUR Upper right corner of the bounding box.
	 */
	public Mbr(Point2d inLL,Point2d inUR)
	{		
		ll = new Point2d(inLL);
		ur = new Point2d(inUR);
	}
	
	/**
	 * Add a point to the bounding box, expanding the extents.
	 *
	 * @param pt Point to expand the bounding box with.
	 */
	public void addPoint(Point2d pt)
	{
		if (ll == null)
			ll = new Point2d(pt);
		else
		{
			double ll_x = Math.min(pt.getX(),ll.getX());
			double ll_y = Math.min(pt.getY(),ll.getY());
			ll.setValue(ll_x, ll_y);
		}
		if (ur == null)
			ur = new Point2d(pt);
		else 
		{
			double ur_x = Math.max(pt.getX(),ur.getX());
			double ur_y = Math.max(pt.getY(),ur.getY());
			ur.setValue(ur_x, ur_y);			
		}
	}
	
	/**
	 * The span is just the upper right minus the lower left.
	 */
	public Point2d span()
	{
		return new Point2d(ur.getX()-ll.getX(),ur.getY()-ll.getY());
	}
	
	/**
	 * The center of the bounding box.
	 */
	public Point2d middle()
	{
		return new Point2d((ur.getX()+ll.getX())/2.0,(ur.getY()+ll.getY())/2.0);
	}
	
	public void expandByFraction(double bufferZone)
	{
		Point2d spanViewMbr = span();
		ll.setValue(ll.getX()-spanViewMbr.getX()*bufferZone, ll.getY()-spanViewMbr.getY()*bufferZone);
		ur.setValue(ur.getX()+spanViewMbr.getX()*bufferZone, ur.getY()+spanViewMbr.getY()*bufferZone);		
	}
	
	/**
	 * Return a list of points corresponding to the corners of the MBR.
	 */
	public List<Point2d> asPoints()
	{
		ArrayList<Point2d> pts = new ArrayList<Point2d>();
		pts.add(ll);
		pts.add(new Point2d(ur.getX(),ll.getY()));
		pts.add(ur);
		pts.add(new Point2d(ll.getX(),ur.getY()));
		
		return pts;
	}
	
	/**
	 * True if the bounding box has been set.  False if it is uninitialized.
	 */
	public boolean isValid()
	{
		return ll != null && ur != null;
	}
}
