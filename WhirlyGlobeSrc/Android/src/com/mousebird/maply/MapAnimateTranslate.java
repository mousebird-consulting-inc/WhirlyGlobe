/*
 *  AnimateTranslate.java
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
 * Animates a translation (and/or zoom) from the current point to a new one.
 * <p>
 * The MaplyController uses these to animate translation from one point to another.  You
 * don't use this directly unless you've subclasses MaplyController and are doing your own thing.
 *
 */
public class MapAnimateTranslate implements MapView.AnimationDelegate 
{
	MaplyRenderer renderer = null;
	MapView view = null;
	Point3d startLoc = null;
	Point3d endLoc = null;
	Point2d viewBounds[] = null;
	double startTime,endTime;
	
	/**
	 * Construct a translation with input parameters.  You would set this up and then hand it
	 * over to a view for use.
	 * 
	 * @param inView The view we're tied to.
	 * @param inRender Renderer we're using.
	 * @param newLoc New location we're translating to.
	 * @param duration How long we want the animation to go.
	 * @param inBounds Bounding box we want to keep the animation within.
	 */
	MapAnimateTranslate(MapView inView,MaplyRenderer inRender,Point3d newLoc,float duration,Point2d inBounds[])
	{
		view = inView;
		renderer = inRender;
		endLoc = newLoc;
		viewBounds = inBounds;
		startLoc = view.getLoc();
		
		startTime = System.currentTimeMillis()/1000.0;
		endTime = startTime+duration;
	}

	@Override
	public void updateView(MapView view) 
	{
		if (startTime == 0.0 || renderer == null)
			return;
		
		double curTime = System.currentTimeMillis()/1000.0;
		if (curTime > endTime)
		{
			curTime = endTime;
			startTime = 0;
			view.cancelAnimation();
		}
		
		// Calculate location
		double t = (curTime-startTime)/(endTime-startTime);
		Point3d newPos = endLoc.subtract(startLoc).multiplyBy(t).addTo(startLoc);
		if (MapGestureHandler.withinBounds(view, renderer.frameSize, newPos, viewBounds))
			view.setLoc(newPos);
	}
}
