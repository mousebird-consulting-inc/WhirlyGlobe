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
 * Implements a rotation with momentum on the globe.
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
	MaplyRenderer renderer = null;
	Quaternion startQuat = null;
	Quaternion endQuat = null;
	double startHeight,endHeight;
	double startTime,animTime;

	public GlobeAnimateRotation(GlobeView inGlobeView,MaplyRenderer inRender,Quaternion newQuat,double newHeight,double animLen)
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
		
	@Override
	public void updateView(GlobeView view) 
	{
		if (startTime == 0.0)
			return;
		
		double sinceStart = System.currentTimeMillis()/1000.0-startTime;
		// Reached the end of the allotted time
		if (sinceStart > animTime)
		{
			sinceStart = animTime;
			startTime = 0;
			view.cancelAnimation();
		}

		double t = sinceStart/animTime;
		Quaternion newQuat = startQuat.slerp(endQuat,t);
		double height = (endHeight-startHeight)*t + startHeight;
		globeView.setRotQuat(newQuat);
		globeView.setHeight(height);
	}

}
