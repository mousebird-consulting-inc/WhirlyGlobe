/*
 *  GestureHandler.java
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

import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;

/**
 * Implements the various gestures we need and handles conflict between them.
 * <p>
 * This is used by the MaplyController to deal with gestures on Android.  If
 * you want to mess with this, be sure to subclass the MaplyController and
 * create your own subclass of this. 
 *
 */
public class GestureHandler 
{
	MaplyController mapControl = null;
	MapView mapView = null;
	
	ScaleGestureDetector sgd = null;
	ScaleListener sl = null;
	GestureDetector gd = null;
	GestureListener gl = null;
	View view = null;
	public GestureHandler(MaplyController inControl,View inView)
	{
		mapControl = inControl;
		mapView = mapControl.mapView;
		view = inView;
		sl = new ScaleListener(mapControl);
		sgd = new ScaleGestureDetector(view.getContext(),sl);
		gl = new GestureListener(mapControl);
		gd = new GestureDetector(view.getContext(),gl);
		sl.gl = gl;		
	}
	
	/**
	 * Check that a given position will be within the given bounds.
	 * This is used by the various gestures for bounds checking.
	 * 
	 * @param newPos Position we're to check.
	 * @param bounds Bounding box, probably from the maplyControl.
	 * @return true if the new point is within the valid area.
	 */
	public static boolean withinBounds(MapView mapView,Point2d frameSize,Point3d newPos,Point2d[] bounds)
	{
		if (bounds == null)
			return true;
		
		// We make a copy of the map view so we can mess with it
		// Note: This is horribly inefficient
		MapView thisMapView = mapView.clone();
		thisMapView.setLoc(newPos);
		
	    Matrix4d fullMatrix = thisMapView.calcModelViewMatrix();

	    // The corners of the view should be within the bounds
	    Point2d corners[] = new Point2d[4];
	    corners[0] = new Point2d(0,0);
	    corners[1] = new Point2d(frameSize.getX(), 0.0);
	    corners[2] = new Point2d(frameSize.getX(), frameSize.getY());
	    corners[3] = new Point2d(0.0, frameSize.getY());
	    for (int ii=0;ii<4;ii++)
	    {
	    	Point3d hit = thisMapView.pointOnPlaneFromScreen(corners[ii], fullMatrix, frameSize, false);
	    	if (!GeometryUtils.PointInPolygon(new Point2d(hit.getX(),hit.getY()), bounds))
	    		return false;
	    }
	    
	    return true;
	}
	
	// Listening for a pinch scale event
	private class ScaleListener extends ScaleGestureDetector.SimpleOnScaleGestureListener
	{
		MaplyController maplyControl;
		double startZ;
		float startDist;
		GestureListener gl = null;
		public boolean isActive = false;
//		Point3d centerGeoCoord = null;
		
		ScaleListener(MaplyController inMaplyControl)
		{
			maplyControl = inMaplyControl;
		}
		
		@Override
		public boolean onScaleBegin(ScaleGestureDetector detector)
		{
			startZ = maplyControl.mapView.getLoc().getZ();
			startDist = detector.getCurrentSpan();
//			Log.d("Maply","Starting zoom");

			// Find the center and zoom around that
//			Point2d center = new Point2d(detector.getFocusX(),detector.getFocusY());
//			Matrix4d modelTransform = maplyControl.mapView.calcModelViewMatrix();
//			Point3d hit = maplyControl.mapView.pointOnPlaneFromScreen(center, modelTransform, maplyControl.renderWrapper.maplyRender.frameSize, false);
//			Point3d localPt = mapView.coordAdapter.displayToLocal(hit);
//			centerGeoCoord = mapView.coordAdapter.getCoordSystem().localToGeographic(localPt);
			
			// Cancel the panning
			if (gl != null)
				gl.isActive = false;
			isActive = true;
			return true;
		}
		
		@Override
		public boolean onScale(ScaleGestureDetector detector)
		{
			float curDist = detector.getCurrentSpan();
			if (curDist > 0.0 && startDist > 0.0)
			{				
				float scale = startDist/curDist;
				Point3d pos = maplyControl.mapView.getLoc();
				mapView.cancelAnimation();
				Point3d newPos = new Point3d(pos.getX(),pos.getY(),startZ*scale);
				if (withinBounds(mapView,maplyControl.renderWrapper.maplyRender.frameSize,newPos,maplyControl.viewBounds))
					maplyControl.mapView.setLoc(newPos);
//				Log.d("Maply","Zoom: " + maplyControl.mapView.getLoc().getZ() + " Scale: " + scale);
				return true;
			}
			
			isActive = false;
			return false;
		}
		
		@Override
		public void onScaleEnd(ScaleGestureDetector detector)
		{
//			Log.d("Maply","Ending scale");
			isActive = false;
		}
	}
	
	// Listening for the rest of the interesting events
	private class GestureListener implements GestureDetector.OnGestureListener,
				GestureDetector.OnDoubleTapListener
	{
		MaplyController maplyControl;
		public boolean isActive = false;
		
		GestureListener(MaplyController inMaplyControl)
		{
			maplyControl = inMaplyControl;
		}
		
		Point2d startScreenPos = null;
		Point3d startPos = null;
		Point3d startOnPlane = null;
		Matrix4d startTransform = null;
		@Override
		public boolean onDown(MotionEvent e) 
		{
//			Log.d("Maply","onDown");

			// Starting state for pan
			startScreenPos = new Point2d(e.getX(),e.getY());
			startTransform = maplyControl.mapView.calcModelViewMatrix();
			startPos = maplyControl.mapView.getLoc();
			startOnPlane = maplyControl.mapView.pointOnPlaneFromScreen(startScreenPos, startTransform, maplyControl.renderWrapper.maplyRender.frameSize, false);
			isActive = true;
			return true;
		}

		@Override
		public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX,
				float distanceY) 
		{
			if (!isActive)
				return false;
			
			Point2d newScreenPos = new Point2d(e2.getX(),e2.getY());
			
			// New state for pan
			Point3d hit = maplyControl.mapView.pointOnPlaneFromScreen(newScreenPos, startTransform, maplyControl.renderWrapper.maplyRender.frameSize, false);
			if (hit != null)
			{
				Point3d newPos = new Point3d(startOnPlane.getX()-hit.getX()+startPos.getX(),
						startOnPlane.getY()-hit.getY()+startPos.getY(),
						maplyControl.mapView.getLoc().getZ());
				mapView.cancelAnimation();
								
				// If the point is within bounds, set it
				if (withinBounds(mapView,maplyControl.renderWrapper.maplyRender.frameSize,newPos,maplyControl.viewBounds))
					maplyControl.mapView.setLoc(newPos);
				
//				Log.d("Maply","New Pos = (" + newPos.getX() + "," + newPos.getY() + "," + newPos.getZ() + ")");
			}
			
			return true;
		}
		
		// How long we'll animate the momentum 
		static final double AnimMomentumTime = 1.0;
		
		@Override
		public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
				float velocityY) 
		{
//			Log.d("Maply","Fling: (x,y) = " + velocityX + " " + velocityY);
			
			// Project the points into map space
			Matrix4d mapTransform = maplyControl.mapView.calcModelViewMatrix();
			Point2d touch0 = new Point2d(e1.getX(),e1.getY());
			Point2d touch1 = touch0.addTo(new Point2d(velocityX,velocityY));
			Point3d pt0 = mapView.pointOnPlaneFromScreen(touch0, mapTransform, maplyControl.renderWrapper.maplyRender.frameSize, false);
			Point3d pt1 = mapView.pointOnPlaneFromScreen(touch1, mapTransform, maplyControl.renderWrapper.maplyRender.frameSize, false);
			
			// That gives us a direction in map space
			Point3d dir = pt0.subtract(pt1);
			dir.multiplyBy(-1.0);
			double len = dir.length();
			dir = dir.multiplyBy(1.0/len);
			double modelVel = len / AnimMomentumTime;
			
			// Acceleration based on how far we want this to go
			double accel = - modelVel / (AnimMomentumTime * AnimMomentumTime);
			
			// Now kick off the animation
			mapView.setAnimationDelegate(new AnimateTranslateMomentum(mapView, mapControl.renderWrapper.maplyRender, modelVel, accel, dir, maplyControl.viewBounds));
		
			isActive = false;
			
			return true;
		}
		
		@Override
		public void onLongPress(MotionEvent e) 
		{
//			Log.d("Maply","Long Press");
		}

		@Override
		public void onShowPress(MotionEvent e) 
		{
//			Log.d("Maply","ShowPress");
		}

		@Override
		public boolean onSingleTapUp(MotionEvent e) 
		{
//			Log.d("Maply","Single Tap Up");
			return false;
		}

		@Override
		public boolean onDoubleTapEvent(MotionEvent e) 
		{
//			Log.d("Maply","Double tap update");
			return false;
		}

		@Override
		public boolean onSingleTapConfirmed(MotionEvent e) 
		{
			mapControl.processTap(new Point2d(e.getX(),e.getY()));
			return true;
		}

		// Zoom in on double tap
		@Override
		public boolean onDoubleTap(MotionEvent e) 
		{
			// Figure out where they tapped
			Point2d touch = new Point2d(e.getX(),e.getY());
			Matrix4d mapTransform = maplyControl.mapView.calcModelViewMatrix();
			Point3d pt = mapView.pointOnPlaneFromScreen(touch, mapTransform, maplyControl.renderWrapper.maplyRender.frameSize, false);

			// Zoom in where they tapped
			Point3d loc = mapView.getLoc();
			loc.setValue(pt.getX(), pt.getY(), loc.getZ()/2.0);
			
			// Now kick off the animation
			mapView.setAnimationDelegate(new AnimateTranslate(mapView, mapControl.renderWrapper.maplyRender, loc, (float) 0.1, maplyControl.viewBounds));
			isActive = false;
			
			return true;
		}		
	}
	
	// Where we receive events from the gl view
	public boolean onTouch(View v, MotionEvent event) 
	{		
		// If they're using two fingers, cancel any outstanding pan
		if (event.getPointerCount() == 2)
			gl.isActive = false;
		
		// Try for a pinch or another gesture
		boolean slWasActive = sl.isActive;
		if (sl.isActive || event.getPointerCount() == 2)
		{
			sgd.onTouchEvent(event);
		}
		if (!sl.isActive && event.getPointerCount() == 1)
			gd.onTouchEvent(event);
		
		if (!sl.isActive && !gl.isActive && !slWasActive)
		{
			if (event.getPointerCount() == 2 && (event.getActionMasked() == MotionEvent.ACTION_POINTER_UP))
			{
				Point3d loc = mapView.getLoc();
				loc.setValue(loc.getX(), loc.getY(), loc.getZ()*2.0);
				
				// Now kick off the animation
				mapView.setAnimationDelegate(new AnimateTranslate(mapView, mapControl.renderWrapper.maplyRender, loc, (float) 0.1, mapControl.viewBounds));

				sl.isActive = false;
				gl.isActive = false;
				return true;			
			}
		}

		return true;
	}      
}
