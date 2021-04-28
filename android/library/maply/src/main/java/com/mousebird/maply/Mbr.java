/*  Mbr.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2021 mousebird consulting
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
 */
package com.mousebird.maply;

import org.jetbrains.annotations.NotNull;

import java.util.Arrays;
import java.util.List;
import java.util.Objects;

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
		// do not initialize ll/ur
	}

	/**
	 * Construct with the lower left and upper right coordinates.
	 * The points provided are copied.
	 * 
	 * @param inLL Lower left corner of the bounding box.
	 * @param inUR Upper right corner of the bounding box.
	 */
	public Mbr(Point2d inLL,Point2d inUR)
	{
		// Make copies
		if (inLL != null) {
			ll = new Point2d(inLL);
		}
		if (inUR != null) {
			ur = new Point2d(inUR);
		}
	}

	/**
	 * Construct with the lower left and upper right coordinates.
	 *
	 * @param llx X coordinate of the lower-left corner
	 * @param lly Y coordinate of the lower-left corner
	 * @param urx X coordinate of the upper-right corner
	 * @param ury Y coordinate of the upper-right corner
	 */
	public Mbr(double llx, double lly, double urx, double ury)
	{
		ll = new Point2d(llx, lly);
		ur = new Point2d(urx, ury);
	}

	/**
	 * Copy Construct
	 */
	public Mbr(Mbr other) {
		this((other != null) ? other.ll : null, (other != null) ? other.ur : null);
	}


	@NotNull
	public String toString() {
		return "ll: " + ll + " ur: " + ur;
	}

	/**
	 * Compare coordinate values.
     */
	@Override public boolean equals(Object thatObj)
	{
		if (this == thatObj) {
			return true;
		}
		if (thatObj instanceof Mbr) {
			Mbr that = (Mbr) thatObj;
			return Objects.equals(ll, that.ll) && Objects.equals(ur, that.ur);
		}
		return false;
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
			final double ll_x = Math.min(pt.getX(),ll.getX());
			final double ll_y = Math.min(pt.getY(),ll.getY());
			ll.setValue(ll_x, ll_y);
		}
		if (ur == null)
			ur = new Point2d(pt);
		else 
		{
			final double ur_x = Math.max(pt.getX(),ur.getX());
			final double ur_y = Math.max(pt.getY(),ur.getY());
			ur.setValue(ur_x, ur_y);
		}
	}
	
	/**
	 * The span is just the upper right minus the lower left.
	 */
	public Point2d span()
	{
		return isValid() ? new Point2d(ur.getX()-ll.getX(),ur.getY()-ll.getY()) : null;
	}
	
	/**
	 * The center of the bounding box.
	 */
	public Point2d middle()
	{
		return isValid() ? new Point2d((ur.getX()+ll.getX())/2.0,(ur.getY()+ll.getY())/2.0) : null;
	}
	
	public void expandByFraction(double bufferZone)
	{
		if (isValid()) {
			final Point2d spanViewMbr = span();
			ll.setValue(ll.getX() - spanViewMbr.getX() * bufferZone, ll.getY() - spanViewMbr.getY() * bufferZone);
			ur.setValue(ur.getX() + spanViewMbr.getX() * bufferZone, ur.getY() + spanViewMbr.getY() * bufferZone);
		}
	}

	public Mbr expandedByFraction(double bufferZone)
	{
		Mbr result = new Mbr(this);
		result.expandedByFraction(bufferZone);
		return result;
	}

	/**
	 * Check if the given bounding boxes overlap.
     */
	public boolean overlaps(Mbr that)
	{
		if (!isValid() || !that.isValid()) {
			return false;
		}
		// todo: this could really use a unit test
		return that.insideOrOnEdge(ll) ||
		       that.insideOrOnEdge(ur) ||
		       that.insideOrOnEdge(ll.getX(),ur.getY()) ||
		       that.insideOrOnEdge(ur.getX(),ll.getY()) ||
		       insideOrOnEdge(that.ll) ||
		       insideOrOnEdge(that.ur) ||
		       insideOrOnEdge(that.ll.getX(),that.ur.getY()) ||
		       insideOrOnEdge(that.ur.getX(),that.ll.getY()) ||
		       this.oneWayOverlap(that) ||
		       that.oneWayOverlap(this);
	}

	/**
	 * Check if the given point lies inside the bounding box or one of the edges.
	 */
	protected boolean insideOrOnEdge(Point2d pt) {
		return insideOrOnEdge(pt.getX(), pt.getY());
	}

	/**
	 * Check if the given point lies inside the bounding box or one of the edges.
	 */
	protected boolean insideOrOnEdge(double x, double y) {
		return ll.getX() <= x && ll.getY() <= y && x <= ur.getX() && y <= ur.getY();
	}

	protected boolean oneWayOverlap(Mbr that) {
		return (that.ll.getX() <= ll.getX() &&
		        ur.getX() <= that.ur.getX() &&
		        ll.getY() <= that.ll.getY() &&
		        that.ur.getY() <= ur.getY());
	}

	/**
	 * Return a list of points corresponding to the corners of the MBR.
	 */
	public List<Point2d> asPoints()
	{
		return isValid() ?
				Arrays.asList(ll, new Point2d(ur.getX(),ll.getY()),
		                      ur, new Point2d(ll.getX(),ur.getY())) :
				null;
	}
	
	/**
	 * True if the bounding box has been set.  False if it is uninitialized.
	 */
	public boolean isValid() {
		return ll != null && ur != null;
	}
}
