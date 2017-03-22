/*
 *  AnimateTranslateMomentum.java
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
 * Implements a translation with momentum to a new point.
 * <p>
 * Translation with momentum is typically used at the end of a pan or fling
 * gesture when the user has let go.  The acceleration is negative, thus slowing
 * the map down.
 * <p>
 * In general, the MaplyController creates these and you'd only be doing so
 * yourself if you've subclassed it.
 *
 */
public class MapAnimateTranslateMomentum implements MapView.AnimationDelegate
{
	MapView mapView = null;
	MaplyRenderer renderer = null;
	double velocity,acceleration;
	Point3d dir = null;
	Mbr bounds = null;
	double startTime,maxTime;
	Point3d org = null;
	Point2d viewBounds[] = null;

	/**
	 * Constructs an animation that translates from one point in a specific direction with a given acceleration
	 * (usually deceleration).
	 * 
	 * @param inView The map view this is tied to.
	 * @param inRender Renderer we're using.
	 * @param inVel Starting velocity.
	 * @param inAcc Acceleration to apply, probably negative.
	 * @param inDir Direction to go, rather than a destination point.
	 * @param inBounds Bounding box to stay within.
	 */
	MapAnimateTranslateMomentum(MapView inView,MaplyRenderer inRender,double inVel,double inAcc,Point3d inDir,Point2d inBounds[])
	{
		mapView = inView;
		renderer = inRender;
		velocity = inVel;
		acceleration = inAcc;
		dir = inDir;
		viewBounds = inBounds;
		
		startTime = System.currentTimeMillis()/1000.0;
		org = mapView.getLoc();
		
		// Calculate the max time
		if (acceleration != 0.0)
		{
			maxTime = -velocity / acceleration;
			if (maxTime < 0.0)
				maxTime = 0.0;
			
			if (maxTime == 0.0)
				startTime = 0.0;
		} else
			maxTime = Double.MAX_VALUE;
	}
	
	@Override
	public void updateView(MapView view) 
	{
		if (startTime == 0.0)
			return;
		
		double sinceStart = System.currentTimeMillis()/1000.0-startTime;
		// Reached the end of the allotted time
		if (sinceStart > maxTime)
		{
			sinceStart = maxTime;
			startTime = 0;
			view.cancelAnimation();
		}
		
		// Calculate distance
//		Point3d oldLoc = view.getLoc();
		double dist = (velocity + 0.5 * acceleration * sinceStart) * sinceStart;
		Point3d newPos = org.addTo(dir.multiplyBy(dist));
		
		// Bounds check and set
		if (MapGestureHandler.withinBounds(view, renderer.frameSize, newPos, viewBounds))
			view.setLoc(newPos);
	}

}
