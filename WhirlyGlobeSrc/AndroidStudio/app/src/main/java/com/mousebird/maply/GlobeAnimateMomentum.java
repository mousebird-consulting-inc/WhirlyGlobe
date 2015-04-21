/*
 *  GlobeAnimateMomentum.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/20/15.
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
public class GlobeAnimateMomentum implements GlobeView.AnimationDelegate
{
	GlobeView globeView = null;
	MaplyRenderer renderer = null;
	Point3d axis = null;
	boolean northUp;
	Quaternion startQuat = null;
	double velocity,acceleration;
	double startTime,maxTime;

	public GlobeAnimateMomentum(GlobeView inGlobeView,MaplyRenderer inRender,double inVelocity,double inAcceleration,Point3d inAxis,boolean inNorthUp)
	{
		globeView = inGlobeView;
		renderer = inRender;
		velocity = inVelocity;
		acceleration = inAcceleration;
		axis = inAxis;
		northUp = inNorthUp;

		startTime = System.currentTimeMillis()/1000.0;
		startQuat = globeView.getRotQuat();
		
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
	
	// Calculate a rotation for the given time
	Quaternion rotForTime(double sinceStart)
	{
		double totalAng = (velocity + 0.5 * acceleration * sinceStart) * sinceStart;
		AngleAxis angAxis = new AngleAxis(totalAng,axis);
		Quaternion newRotQuat = startQuat.multiply(angAxis);
		
		if (northUp)
		{
			Point3d northPole = newRotQuat.multiply(new Point3d(0,0,1)).normalized();
			if (northPole.getY() != 0.0)
			{
				Point3d newUp = globeView.prospectiveUp(newRotQuat);

                // Then rotate it back on to the YZ axis
                // This will keep it upward
                double ang = Math.atan(northPole.getX()/northPole.getY());
                // However, the pole might be down now
                // If so, rotate it back up
                if (northPole.getY() < 0.0)
                    ang += Math.PI;
                AngleAxis upRot = new AngleAxis(ang,newUp);
                newRotQuat = newRotQuat.multiply(upRot);
			}
		}
		
		return newRotQuat;
	}
	
	@Override
	public void updateView(GlobeView view) 
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

		Quaternion newQuat = rotForTime(sinceStart);
		globeView.setRotQuat(newQuat);
	}

}
