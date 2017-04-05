/*
 *  GeometryUtils.java
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

/**
 * A collection or random geometry utilities that aren't associated with particular objects.
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
	 * @return true if the point is inside, false otherwise.
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

	/**
	 * Find the point on a line segment closest to the given point.  Also returns the parametric value.
	 * @param p0 First point in the line segment.
	 * @param p1 Second point in the line segment.
	 * @param pt Point near the line segment.
	 * @return The closest point on the line segment to the point.
	 */
	public static Point2d ClosestPointOnLineSegment(Point2d p0,Point2d p1,Point2d pt)
	{
		double dx = p1.getX()-p0.getX(), dy = p1.getY()-p0.getY();
		double denom = dx*dx+dy*dy;

		if (denom == 0.0)
			return p0;

		double u = ((pt.getX()-p0.getX())*(p1.getX()-p0.getX())+(pt.getY()-p0.getY())*(p1.getY()-p0.getY()))/denom;

		if (u <= 0.0)
			return p0;

		if (u >= 1.0)
			return p1;

		return new Point2d(p0.getX()+dx*u,p0.getY()+dy*u);
	}

	/**
	 * Find the closest point on the polygon to the point passed in
	 *
	 * @param pts Polygon we're testing against.
	 * @param pt Point that's near the polygon.
	 * @param retClosePt The closest point on the polygon to the input pt.
	 */
	public static double ClosestPointToPolygon(Point2d pts[],Point2d pt,Point2d retClosePt)
	{
		double minDist2 = Double.MAX_VALUE;
		Point2d closePt = null;

		for (int ii=0;ii<pts.length;ii++)
		{
        	Point2d p0 = pts[ii];
        	Point2d p1 = pts[(ii+1)%pts.length];

			double t;
			Point2d thisClosePt = ClosestPointOnLineSegment(p0, p1, pt);
			double thisDist2 = (new Point2d(pt.getX()-thisClosePt.getX(),pt.getY()-thisClosePt.getY())).squaredNorm();
			if (thisDist2 < minDist2)
			{
				minDist2 = thisDist2;
				closePt = thisClosePt;
			}
		}

		retClosePt.setValue(closePt.getX(),closePt.getY());

		return Math.sqrt(minDist2);
	}
}
