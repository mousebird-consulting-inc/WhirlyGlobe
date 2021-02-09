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
	RenderController renderer = null;
	MapView view = null;
	Point3d startLoc = null;
	Point3d endLoc = null;
	Point2d viewBounds[] = null;
	Double startRot = null;
	Double dRot = null;
	double startTime,endTime;

	/**
	 * Construct a translation with input parameters.  You would set this up and then hand it
	 * over to a view for use.
	 *
	 * @param inView The view we're tied to.
	 * @param inRender Renderer we're using.
	 * @param newLoc New location we're translating to.
	 * @param newRot New rotation we want
	 * @param duration How long we want the animation to go.
	 * @param inBounds Bounding box we want to keep the animation within.
	 */
	MapAnimateTranslate(MapView inView,RenderController inRender,Point3d newLoc,Double newRot,float duration,Point2d inBounds[])
	{
		this(inView,inRender,newLoc,duration,inBounds);

		if (view != null && newRot != null) {
			startRot = view.getRot();

			// If the old and new rotations are within 180 degrees, just interpolate.
			// Otherwise, we need to go around the other way.
			// Note that we assume both angles are normalized.
			dRot = newRot - startRot;
			if (Math.abs(dRot) < 1.0e-6) {
				// Don't generate a bunch of rotations for minuscule offsets that won't make any difference
				dRot = 0.0;
			} else if (Math.abs(dRot) > Math.PI) {
				dRot += ((dRot > 0) ? -2 : 2) * Math.PI;
			}
		}
	}

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
	MapAnimateTranslate(MapView inView,RenderController inRender,Point3d newLoc,float duration,Point2d inBounds[])
	{
		view = inView;
		renderer = inRender;
		endLoc = newLoc;
		viewBounds = inBounds;
		startLoc = view.getLoc();
		startRot = view.getRot();

		startTime = System.currentTimeMillis()/1000.0;
		endTime = startTime+duration;
	}

	@Override
	public void updateView(MapView view) 
	{
		if (startTime == 0.0 || renderer == null || endTime == startTime)
			return;
		
		double curTime = Math.min(endTime, System.currentTimeMillis()/1000.0);

		// Calculate location
		double t = (curTime-startTime)/(endTime-startTime);
		Point3d newPos = endLoc.subtract(startLoc).multiplyBy(t).addTo(startLoc);
		if (endLoc.getZ() <= 0.0) {
			// Not doing height, leave it alone.
			newPos.setValue(newPos.getX(), newPos.getY(), startLoc.getZ());
		}
		if (MapGestureHandler.withinBounds(view, renderer.frameSize, newPos, viewBounds)) {
			view.setLoc(newPos);
		}

		if (startRot != null && dRot != null && dRot != 0.0) {
			view.setRot(startRot + t * dRot);
		}

		if (curTime >= endTime)
		{
			startTime = 0;
			view.cancelAnimation();
		}
	}
}
