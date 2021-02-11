/*
 *  GlobeAnimateRotation.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/21/15.
 *  Copyright 2011-2015 mousebird consulting
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
 * Implements a rotation on the globe using a quaternion.
 * <p>
 * 
 * <p>
 * In general, the MaplyController creates these and you'd only be doing so
 * yourself if you've subclassed it.
 *
 */
public class GlobeAnimateRotation implements GlobeView.AnimationDelegate
{
	GlobeView globeView = null;
	RenderController renderer = null;
	Quaternion startQuat = null;
	Quaternion endQuat = null;
	double startHeight,endHeight;
	Double startRot = null;
	Double dRot = null;
	double startTime,animTime;

	public GlobeAnimateRotation(GlobeView inGlobeView,RenderController inRender,Quaternion newQuat,double newHeight,double animLen)
	{
		globeView = inGlobeView;
		renderer = inRender;

		startTime = System.currentTimeMillis()/1000.0;
		animTime = animLen;
		startQuat = globeView.getRotQuat();
		startHeight = globeView.getHeight();
		endHeight = newHeight;
		endQuat = newQuat;
	}

	public GlobeAnimateRotation(GlobeView inGlobeView,RenderController inRender,Quaternion newQuat,double newHeight,Double heading,double animLen)
	{
		this(inGlobeView,inRender,newQuat,newHeight,animLen);

		if (inGlobeView != null && heading != null && !inGlobeView.northUp) {
			startRot = inGlobeView.getHeading();

			// If the old and new rotations are within 180 degrees, just interpolate.
			// Otherwise, we need to go around the other way.
			// Note that we assume both angles are normalized.
			dRot = heading - startRot;
			if (Math.abs(dRot) < 1.0e-6) {
				// Don't generate a bunch of rotations for minuscule offsets that won't make any difference
				dRot = 0.0;
			} else if (Math.abs(dRot) > Math.PI) {
				dRot += ((dRot > 0) ? -2 : 2) * Math.PI;
			}
		}
	}

	@Override
	public void updateView(GlobeView view) 
	{
		if (startTime <= 0 || animTime <= 0 || globeView == null || renderer == null)
			return;

		double curTime = Math.min(startTime + animTime, System.currentTimeMillis()/1000.0);
		double t = (curTime-startTime)/animTime;

		Quaternion newQuat = startQuat.slerp(endQuat,t);
		globeView.setRotQuat(newQuat);

		double height = (endHeight-startHeight)*t + startHeight;
		globeView.setHeight(height);

		if (startRot != null && dRot != null && dRot != 0.0) {
			view.setHeading(startRot + t * dRot);
		}

		// Reached the end of the allotted time
		if (t >= 1.0)
		{
			startTime = 0;
			view.cancelAnimation();
		}
	}

}
