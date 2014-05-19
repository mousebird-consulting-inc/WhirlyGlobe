package com.mousebird.maply;

/**
 * Animates a translation (and/or zoom) from the current point to a new on.
 * 
 * @author sjg
 *
 */
public class AnimateTranslate implements MapView.AnimationDelegate 
{
	MaplyRenderer renderer = null;
	MapView view = null;
	Point3d startLoc = null;
	Point3d endLoc = null;
	Point2d viewBounds[] = null;
	double startTime,endTime;
	
	AnimateTranslate(MapView inView,MaplyRenderer inRender,Point3d newLoc,float duration,Point2d inBounds[])
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
		if (startTime == 0.0)
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
		if (GestureHandler.withinBounds(view, renderer.frameSize, newPos, viewBounds))
			view.setLoc(newPos);
	}
}
