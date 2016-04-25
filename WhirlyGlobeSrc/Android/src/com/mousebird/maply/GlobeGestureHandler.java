/*
 *  GlobeGestureHandler.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/17/15.
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

import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;

/**
 * Implements the various gestures we need and handles conflict between them.
 * <p>
 * This is used by the GlobeController to deal with gestures on Android.  If
 * you want to mess with this, be sure to subclass the MaplyController and
 * create your own subclass of this. 
 *
 */
public class GlobeGestureHandler 
{
	GlobeController globeControl = null;
	GlobeView globeView = null;
	
	ScaleGestureDetector sgd = null;
	ScaleListener sl = null;
	GestureDetector gd = null;
	GestureListener gl = null;
	View view = null;
	double zoomLimitMin = 0.0;
	double zoomLimitMax = 1000.0;
	public GlobeGestureHandler(GlobeController inControl,View inView)
	{
		globeControl = inControl;
		globeView = globeControl.globeView;
		view = inView;
		sl = new ScaleListener(globeControl);
		sgd = new ScaleGestureDetector(view.getContext(),sl);
		gl = new GestureListener(globeControl);
		gd = new GestureDetector(view.getContext(),gl);
		sl.gl = gl;		
	}

	public void setZoomLimits(double inMin,double inMax)
	{
		zoomLimitMin = inMin;
		zoomLimitMax = inMax;
	}
	
	/**
	 * Check that a given position will be within the given bounds.
	 * This is used by the various gestures for bounds checking.
	 * 
	 * @param newPos Position we're to check.
	 * @return true if the new point is within the valid area.
	 */
	public static boolean withinBounds(GlobeView globeView,Point2d frameSize,Point3d newPos)
	{
		return true;
	}
	
	
	// Listening for a pinch scale event
	private class ScaleListener extends ScaleGestureDetector.SimpleOnScaleGestureListener
	{
		GlobeController maplyControl;
		double startZ;
		float startDist;
		GestureListener gl = null;
		public boolean isActive = false;
		Point3d centerGeoCoord = null;
		
		ScaleListener(GlobeController inMaplyControl)
		{
			maplyControl = inMaplyControl;
		}
		
		@Override
		public boolean onScaleBegin(ScaleGestureDetector detector)
		{
			startZ = maplyControl.globeView.getLoc().getZ();
			startDist = detector.getCurrentSpan();
//			Log.d("Maply","Starting zoom");

			// Find the center and zoom around that
			Point2d center = new Point2d(detector.getFocusX(),detector.getFocusY());
			Matrix4d modelTransform = maplyControl.globeView.calcModelViewMatrix();
			Point3d hit = maplyControl.globeView.pointOnSphereFromScreen(center, modelTransform, maplyControl.renderWrapper.maplyRender.frameSize, false);
			Point3d localPt = globeView.coordAdapter.displayToLocal(hit);
			centerGeoCoord = globeView.coordAdapter.getCoordSystem().localToGeographic(localPt);
			
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
				Point3d pos = maplyControl.globeView.getLoc();
				globeView.cancelAnimation();
				Point3d newPos = new Point3d(pos.getX(),pos.getY(),startZ*scale);
				if (withinBounds(globeView,maplyControl.renderWrapper.maplyRender.frameSize,newPos)) {
					double newZ = newPos.getZ();
					newZ = Math.min(newZ,zoomLimitMax);
					newZ = Math.max(newZ,zoomLimitMin);
					maplyControl.globeView.setLoc(new Point3d(newPos.getX(),newPos.getY(),newZ));
				}
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
		GlobeController globeControl;
		public boolean isActive = false;
		
		GestureListener(GlobeController inMaplyControl)
		{
			globeControl = inMaplyControl;
		}
		
		Point2d startScreenPos = null;
		Point3d startPos = null;
		Point3d startOnSphere = null;
		Quaternion startQuat = null;
		Matrix4d startTransform = null;
		@Override
		public boolean onDown(MotionEvent e) 
		{
//			Log.d("Maply","onDown");i

			if (globeControl.renderWrapper == null || globeControl.renderWrapper.maplyRender == null)
				return false;

			// Starting state for pan
			startScreenPos = new Point2d(e.getX(),e.getY());
			startTransform = globeControl.globeView.calcModelViewMatrix();
			startPos = globeControl.globeView.getLoc();
			startOnSphere = globeControl.globeView.pointOnSphereFromScreen(startScreenPos, startTransform, globeControl.renderWrapper.maplyRender.frameSize, false);
			startQuat = globeControl.globeView.getRotQuat();
			if (startOnSphere != null)
			{
				isActive = true;
			} else
				isActive = false;

			return true;
		}

        @Override
        public boolean onSingleTapUp(MotionEvent e)
        {
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
			Point3d hit = globeControl.globeView.pointOnSphereFromScreen(newScreenPos, startTransform, globeControl.renderWrapper.maplyRender.frameSize, false);
			if (hit != null)
			{
				globeView.cancelAnimation();

				// Figure out the rotation between the two
				Quaternion endRot = new Quaternion(startOnSphere,hit);
				Quaternion newRotQuat = startQuat.multiply(endRot);
				
				// Keep the north pole pointed up
				if (globeView.northUp)
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
				
				globeControl.globeView.setRotQuat(newRotQuat);
				
//				Log.d("Maply","New globe rotation");
			}

			return true;
		}
		
		// Calculate where the screen point is on a hypothetical plane
		Point3d pointOnPlaneFromScreen(Point2d touch, Matrix4d transform, Point2d frameSize)
		{
			Point3d screenPt = globeView.pointUnproject(touch,frameSize,false);
			screenPt.normalize();
			if (screenPt.getZ() == 0.0)
				return null;
			double t = -globeView.getHeight() / screenPt.getZ();
			
			return screenPt.multiplyBy(t);
		}
		
		// How long we'll animate the momentum 
		static final double AnimMomentumTime = 1.0;
		
		@Override
		public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
				float velocityY) 
		{
//			Log.d("Maply","Fling: (x,y) = " + velocityX + " " + velocityY);
			
			Point2d frameSize = globeControl.renderWrapper.maplyRender.frameSize;
			
			// Figure out two points in model space (current and after 1s)
			Point2d touch0 = new Point2d(e1.getX(),e1.getY());
			Point2d touch1 = touch0.addTo(new Point2d(AnimMomentumTime*velocityX,AnimMomentumTime*velocityY));
			
			Point3d p0 = globeView.pointUnproject(touch0,frameSize,false);
			Point3d p1 = globeView.pointUnproject(touch1,frameSize,false);
			Matrix4d modelMat = globeView.calcModelViewMatrix();
			Matrix4d invModelMat = modelMat.inverse();
			Point4d model_p0_4d = invModelMat.multiply(new Point4d(p0,1.0));
			Point4d model_p1_4d = invModelMat.multiply(new Point4d(p1,1.0));
			Point3d model_p0 = new Point3d(model_p0_4d.getX()/model_p0_4d.getW(),model_p0_4d.getY()/model_p0_4d.getW(),model_p0_4d.getZ());
			Point3d model_p1 = new Point3d(model_p1_4d.getX()/model_p1_4d.getW(),model_p1_4d.getY()/model_p1_4d.getW(),model_p1_4d.getZ());

			// Using a plane (rather than the sphere) calculate how far the user would move in 1s at constant velocity
			Point3d pt0 = pointOnPlaneFromScreen(touch0, startTransform, frameSize);
			Point3d pt1 = pointOnPlaneFromScreen(touch1, startTransform, frameSize);
			if (pt0 != null && pt1 != null)
			{	
				// That gives us a direction in map space
				Point3d dir = pt0.subtract(pt1);
				dir.multiplyBy(-1.0);
				double len = dir.length();
				dir = dir.multiplyBy(1.0/len);
				double modelVel = len / AnimMomentumTime;
				double angVel = modelVel;
				
				// Rotate around an axis derived from touch0 and touch1
				model_p0.normalize();
				model_p1.normalize();
				Point3d rotAxis = model_p0.cross(model_p1).normalized();
				
				// Acceleration based on how far we want this to go
				double accel = - angVel / (AnimMomentumTime * AnimMomentumTime);
	
				if (angVel > 0.0)
				{
					globeView.setAnimationDelegate(new GlobeAnimateMomentum(globeView,globeControl.renderWrapper.maplyRender,angVel,accel,rotAxis,globeView.northUp));
				}
			}
					
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
		public boolean onDoubleTapEvent(MotionEvent e) 
		{
//			Log.d("Maply","Double tap update");
			return false;
		}

		@Override
		public boolean onSingleTapConfirmed(MotionEvent e) 
		{
			globeControl.processTap(new Point2d(e.getX(),e.getY()));
			return true;
		}

		// Zoom in on double tap
		@Override
		public boolean onDoubleTap(MotionEvent e) 
		{
			CoordSystemDisplayAdapter coordAdapter = globeView.getCoordAdapter();
			Point2d frameSize = globeControl.renderWrapper.maplyRender.frameSize;

			// Figure out where they tapped
			Point2d touch = new Point2d(e.getX(),e.getY());
			Matrix4d mapTransform = globeView.calcModelViewMatrix();
			Point3d pt = globeView.pointOnSphereFromScreen(touch, mapTransform, frameSize, false);
			Point3d geoCoord = coordAdapter.getCoordSystem().localToGeographic(coordAdapter.displayToLocal(pt));

			if (pt != null)
			{
				// Zoom in where they tapped
				double height = globeView.getHeight();
				double newHeight = height/2.0;
				
				// Note: This isn't right.  Need the 
				Quaternion newQuat = globeView.makeRotationToGeoCoord(geoCoord.getX(), geoCoord.getY(), globeView.northUp);
				
				// Now kick off the animation
				globeView.setAnimationDelegate(new GlobeAnimateRotation(globeView, globeControl.renderWrapper.maplyRender, newQuat, newHeight, 0.5));
			}
			
			isActive = false;
			
			return true;
		}		
	}
	
	// Where we receive events from the gl view
	public boolean onTouch(View v, MotionEvent event) {
		boolean slWasActive = sl.isActive;
		boolean glWasActive = gl.isActive;

		// If they're using two fingers, cancel any outstanding pan
		if (event.getPointerCount() == 2) {
			gl.isActive = false;
		}

		// Try for a pinch or another gesture
		if (sl.isActive || event.getPointerCount() == 2) {
			sgd.onTouchEvent(event);
		}
		if (!sl.isActive || gl.isActive) {
			gd.onTouchEvent(event);
			if (event.getAction() == MotionEvent.ACTION_UP) {
				gl.isActive = false;
			}
		}

		if (!sl.isActive && !gl.isActive && !slWasActive)
		{
			if (event.getPointerCount() == 2 && (event.getActionMasked() == MotionEvent.ACTION_POINTER_UP))
			{
				double newHeight = globeView.getHeight()*2.0;

				// Now kick off the animation
				globeView.setAnimationDelegate(new GlobeAnimateRotation(globeView, globeControl.renderWrapper.maplyRender, globeView.getRotQuat(), newHeight, 0.5));

				sl.isActive = false;
				gl.isActive = false;

				return true;
			}
		}

		if (!glWasActive && gl.isActive)
			globeControl.panDidStart(true);
		if (glWasActive && !gl.isActive) {
			globeControl.panDidEnd(true);
		}

        if (!slWasActive && sl.isActive)
            globeControl.zoomDidStart(true);
        if (slWasActive && !sl.isActive)
            globeControl.zoomDidEnd(true);

		return true;
	}      
}
