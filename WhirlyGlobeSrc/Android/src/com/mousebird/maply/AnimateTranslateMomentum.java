package com.mousebird.maply;

public class AnimateTranslateMomentum implements MapView.AnimationDelegate
{
	double velocity,acceleration;
	Point3d dir = null;
	Mbr bounds = null;
	double startTime,maxTime;
	Point3d org = null;

	AnimateTranslateMomentum(MapView view,double inVel,double inAcc,Point3d inDir,Mbr inBounds)
	{
		velocity = inVel;
		acceleration = inAcc;
		dir = inDir;
		bounds = inBounds;
		
		startTime = System.currentTimeMillis()/1000.0;
		org = view.getLoc();
		
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
		Point3d newLoc = org.addTo(dir.multiplyBy(dist));
		
		// Note: Should be doing bounds checking
		view.setLoc(newLoc);
	}

}
