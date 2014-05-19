package com.mousebird.maply;

/**
 * Implements a translation with momentum to a new point.
 * @author sjg
 *
 */
public class AnimateTranslateMomentum implements MapView.AnimationDelegate
{
	MapView mapView = null;
	MaplyRenderer renderer = null;
	double velocity,acceleration;
	Point3d dir = null;
	Mbr bounds = null;
	double startTime,maxTime;
	Point3d org = null;
	Point2d viewBounds[] = null;

	AnimateTranslateMomentum(MapView inView,MaplyRenderer inRender,double inVel,double inAcc,Point3d inDir,Point2d inBounds[])
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
		if (GestureHandler.withinBounds(view, renderer.frameSize, newPos, viewBounds))
			view.setLoc(newPos);
	}

}
