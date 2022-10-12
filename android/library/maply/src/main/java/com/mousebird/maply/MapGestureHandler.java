/*  MapGestureHandler.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2022 mousebird consulting
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
 */

package com.mousebird.maply;

import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.lang.ref.WeakReference;

/**
 * Implements the various gestures we need and handles conflict between them.
 * <p>
 * This is used by the MaplyController to deal with gestures on Android.  If
 * you want to mess with this, be sure to subclass the MaplyController and
 * create your own subclass of this. 
 */
public class MapGestureHandler
{
	@NotNull
	private final WeakReference<MapController> mapControl;
	@NotNull
	private final MapView mapView;

	public boolean allowRotate = false;
	public boolean allowZoom = true;
	public boolean allowPan = true;

	@Nullable
	private ScaleGestureDetector sgd;
	@Nullable
	private ScaleListener sl;
	@Nullable
	private GestureDetector gd;
	@Nullable
	private GestureListener gl;

	public double zoomLimitMin = 0.0;
	public double zoomLimitMax = 1000.0;
	private double startRot = Double.MAX_VALUE;
	private double startViewRot = Double.MAX_VALUE;

	private long lastZoomEnd = 0;

	public MapGestureHandler(MapController inControl,View inView)
	{
		mapControl = new WeakReference<>(inControl);
		mapView = inControl.mapView;
		gl = new GestureListener();
		gd = new GestureDetector(inView.getContext(),gl);
		sl = new ScaleListener(gl);
		sgd = new ScaleGestureDetector(inView.getContext(),sl);
	}

	public void shutdown() {
		gd = null;
		sgd = null;
		if(sl != null) {
			sl.shutdown();
			sl = null;
		}
		if(gl != null) {
			gl.shutdown();
			gl = null;
		}
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
	 * @param newLocalPos Position we're to check.
	 * @param bounds Bounding box, probably from the maplyControl.
	 * @return true if the new point is within the valid area.
	 */
	public static boolean withinBounds(MapView mapView,Point2d frameSize,Point3d newLocalPos,Point2d[] bounds)
	{
		if (bounds == null)
			return true;

		// We make a copy of the map view so we can mess with it
		final MapView thisMapView = mapView.clone();

		// The corners of the view should be within the bounds
		final Point2d[] corners = new Point2d[] {
			new Point2d(0, 0),
			new Point2d(frameSize.getX(), 0.0),
			new Point2d(frameSize.getX(), frameSize.getY()),
			new Point2d(0.0, frameSize.getY()),
		};

		Point2d locOffset = new Point2d(0,0);
		for (int tests=0;tests<4;tests++) {
			final Point3d testLoc = new Point3d(newLocalPos.getX()+locOffset.getX(),
			                                    newLocalPos.getY()+locOffset.getY(),
			                                     newLocalPos.getZ());
			thisMapView.setLoc(testLoc);

			final Matrix4d fullMatrix = thisMapView.calcModelViewMatrix();

			boolean checkOkay = true;
			for (int ii = 0; ii < 4; ii++) {
				final Point3d hit = thisMapView.pointOnPlaneFromScreen(corners[ii], fullMatrix, frameSize, false);
				final Point2d hit2d = new Point2d(hit.getX(), hit.getY());
				if (!GeometryUtils.PointInPolygon(hit2d, bounds)) {
					final Point2d closePt = new Point2d(0, 0);
					final double dist = GeometryUtils.ClosestPointToPolygon(bounds,hit2d,closePt);
					if (dist != Double.MAX_VALUE) {
						final Point2d thisOffset = new Point2d(1.01 * (closePt.getX() - hit2d.getX()),
						                                       1.01 * (closePt.getY() - hit2d.getY()));

						locOffset = locOffset.addTo(thisOffset);
						checkOkay = false;

						break;
					}
				}
			}

			if (checkOkay) {
				newLocalPos.setValue(testLoc.getX(),testLoc.getY(),testLoc.getZ());
				return true;
			}
		}

		return false;
	}

	// Listening for a pinch scale event
	private class ScaleListener extends ScaleGestureDetector.SimpleOnScaleGestureListener
	{
		@Nullable
		private GestureListener gl;
		public boolean isActive = false;
		private double startZ;
		private float startDist;
		private Point2d startScreenCoord;
		private Point3d startDisplayCoord;

		ScaleListener(@NotNull GestureListener inGl) {
			gl = inGl;
		}

		public void shutdown() {
			gl = null;
		}

		@Override
		public boolean onScaleBegin(ScaleGestureDetector detector) {
			if (!allowZoom)
				return false;

			final MapController control = mapControl.get();
			final RendererWrapper wrapper = (control != null) ? control.renderWrapper : null;
			final RenderController render = (wrapper != null) ? wrapper.maplyRender.get() : null;
			if (render == null) {
				return false;
			}

			final Point2d frameSize = render.frameSize;
			final MapView view = control.mapView;
			final Point3d curLoc = view.getLoc();

			startZ = curLoc.getZ();
			startDist = detector.getCurrentSpan();
//			Log.d("Maply","Starting zoom");

			// Find the center and zoom around that
			startScreenCoord = new Point2d(detector.getFocusX(),detector.getFocusY());
			final Matrix4d modelTransform = view.calcModelViewMatrix();
			startDisplayCoord = view.pointOnPlaneFromScreen(startScreenCoord, modelTransform, frameSize, false);

			// Cancel the panning
			if (gl != null) {
				gl.isActive = false;
			}
			isActive = true;
			return true;
		}

		@Override
		public boolean onScale(ScaleGestureDetector detector)
		{
			if (!allowZoom)
				return false;

			final float curDist = detector.getCurrentSpan();
			if (curDist <= 0.0 || startDist <= 0.0) {
				isActive = false;
				return false;
			}

			final MapController control = mapControl.get();
			final RendererWrapper wrapper = (control != null) ? control.renderWrapper : null;
			final RenderController render = (wrapper != null) ? wrapper.maplyRender.get() : null;
			if (render == null) {
				return false;
			}

			final Point2d viewSize = control.getViewSize();
			final Point2d frameSize = render.frameSize;
			final MapView view = control.mapView;
			final Point3d curLoc = view.getLoc();
			final float scale = startDist / curDist;
			final double newZ = Math.max(Math.min(startZ * scale,zoomLimitMax),zoomLimitMin);

			final MapView testView = view.clone();
			testView.setLoc(new Point3d(curLoc.getX(), curLoc.getY(), newZ), false);

			final Matrix4d modelTransform = testView.calcModelViewMatrix();
			final Point2d currentScalePointScreenLoc =
					testView.pointOnScreenFromPlane(startDisplayCoord, modelTransform, frameSize);
			final Point2d screenOffset = startScreenCoord.subtract(currentScalePointScreenLoc);
			final Point2d newMapCenterPoint = frameSize.divideBy(2).subtract(screenOffset);
			final Point3d newDisplayPoint = testView.pointOnPlaneFromScreen(newMapCenterPoint, modelTransform, frameSize, true);
			final Point3d newLoc = view.coordAdapter.displayToLocal(newDisplayPoint);
			newLoc.setValue(newLoc.getX(), newLoc.getY(), newZ);
			testView.setLoc(newLoc, false);

			if (scale < 1.0 || withinBounds(testView,viewSize,newLoc,control.viewBounds)) {
				view.cancelAnimation();
				view.setLoc(newLoc);
//				Log.d("Maply","Zoom: " + maplyControl.mapView.getLoc().getZ() + " Scale: " + scale);
			}
			return true;
		}

		@Override
		public void onScaleEnd(ScaleGestureDetector detector)
		{
			if (!allowZoom)
				return;

//			Log.d("Maply","Ending scale");
			lastZoomEnd = System.currentTimeMillis();
			isActive = false;
		}
	}

	// Listening for the rest of the interesting events
	private class GestureListener implements GestureDetector.OnGestureListener,
			GestureDetector.OnDoubleTapListener
	{
		public boolean isActive = false;
		private Point3d startLoc = null;
		private Point3d startOnPlane = null;
		private Matrix4d startTransform = null;

		GestureListener() {
		}

		public void shutdown() {

		}

		@Override
		public boolean onDown(MotionEvent e)
		{
//			Log.d("Maply","onDown");

			final MapController control = mapControl.get();
			if (control == null) {
				return false;
			}

			// Starting state for pan
			final Point2d startScreenPos = new Point2d(e.getX(), e.getY());
			startTransform = mapView.calcModelViewMatrix();
			final Point2d viewSize = control.getViewSize();
			final Point2d viewCenter = viewSize.multiplyBy(0.5);
			startLoc = mapView.pointOnPlaneFromScreen(viewCenter, startTransform, viewSize, false);
			startOnPlane = mapView.pointOnPlaneFromScreen(startScreenPos, startTransform, viewSize, false);
			isActive = true;
			return true;
		}

		@Override
		public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX,
								float distanceY)
		{
			if (!isActive || !allowPan) {
				return false;
			}

			final MapController control = mapControl.get();
			if (control == null) {
				isActive = false;
				return false;
			}

			final Point2d newScreenPos = new Point2d(e2.getX(),e2.getY());

			// New state for pan
			Point3d hit = mapView.pointOnPlaneFromScreen(newScreenPos, startTransform, control.getViewSize(), false);
			if (hit != null)
			{
				Point3d newPos = new Point3d(startOnPlane.getX()-hit.getX()+startLoc.getX(),
						startOnPlane.getY()-hit.getY()+startLoc.getY(),
						control.mapView.getLoc().getZ());
				mapView.cancelAnimation();

				// If the point is within bounds, set it
				Point3d locPos = mapView.coordAdapter.displayToLocal(newPos);
				if (withinBounds(mapView, control.getViewSize(), locPos, control.viewBounds)) {
					mapView.setLoc(locPos);
				}
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

			if (!allowPan)
				return false;

			if (System.currentTimeMillis() - lastZoomEnd < 175)
			{
				isActive = false;
				return true;
			}

			final MapController control = mapControl.get();
			final RendererWrapper wrapper = (control != null) ? control.renderWrapper : null;
			final RenderController render = (wrapper != null) ? wrapper.maplyRender.get() : null;
			if (render == null) {
				isActive = false;
				return false;
			}

			// Project the points into map space
			final Matrix4d mapTransform = control.mapView.calcModelViewMatrix();
			final Point2d touch0 = new Point2d(e1.getX(),e1.getY());
			final Point2d touch1 = touch0.addTo(new Point2d(velocityX,velocityY));
			final Point2d viewSize = control.getViewSize();
			final Point3d dpt0 = mapView.pointOnPlaneFromScreen(touch0, mapTransform, viewSize, false);
			final Point3d dpt1 = mapView.pointOnPlaneFromScreen(touch1, mapTransform, viewSize, false);
			final Point3d pt0 = mapView.coordAdapter.displayToLocal(dpt0);
			final Point3d pt1 = mapView.coordAdapter.displayToLocal(dpt1);

			// That gives us a direction in map space
			Point3d dir = pt0.subtract(pt1);
			final double len = dir.length();
			dir = dir.multiplyBy((len == 0.0) ? 0.0 : 1.0/len);
			final double modelVel = len / AnimMomentumTime;

			// Acceleration based on how far we want this to go
			@SuppressWarnings("PointlessArithmeticExpression")
			final double accel = -modelVel / (AnimMomentumTime * AnimMomentumTime);

			// Now kick off the animation
			mapView.setAnimationDelegate(
					new MapAnimateTranslateMomentum(mapView, render, modelVel, accel, dir, control.viewBounds));

			isActive = false;

			return true;
		}

		@Override
		public void onLongPress(MotionEvent e)
		{
//			Log.d("Maply","Long Press");
			if (sl == null || gl == null)
				return;

			if (!sl.isActive && gl.isActive && e.getPointerCount() == 1) {
				final MapController control = mapControl.get();
				if (control != null) {
					control.processLongPress(new Point2d(e.getX(), e.getY()));
				}
			}
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
			if (System.currentTimeMillis() - lastZoomEnd < 175) {
				return true;
			}

			final MapController theControl = mapControl.get();
			if (theControl != null) {
				theControl.processTap(new Point2d(e.getX(), e.getY()));
			}

			return true;
		}

		// Zoom in on double tap
		@Override
		public boolean onDoubleTap(MotionEvent e)
		{
			if (!allowZoom)
				return false;

			final MapController control = mapControl.get();
			final RendererWrapper wrapper = (control != null) ? control.renderWrapper : null;
			final RenderController render = (wrapper != null) ? wrapper.maplyRender.get() : null;
			if (render == null) {
				isActive = false;
				return false;
			}

			// Figure out where they tapped
			Point2d touch = new Point2d(e.getX(),e.getY());
			Matrix4d mapTransform = control.mapView.calcModelViewMatrix();
			Point3d pt = mapView.pointOnPlaneFromScreen(touch, mapTransform, control.getViewSize(), false);
			if (pt == null)
				return false;
			Point3d locPt = mapView.getCoordAdapter().displayToLocal(pt);
			if (locPt == null)
				return false;

			// Zoom in where they tapped
			Point3d loc = mapView.getLoc();
			double newZ = loc.getZ()/2.0;
			newZ = Math.min(newZ,zoomLimitMax);
			newZ = Math.max(newZ,zoomLimitMin);
			loc.setValue(locPt.getX(), locPt.getY(), newZ);

			// Now kick off the animation
			// Start after the stop/end events for the current tap
			control.addPostSurfaceRunnable(() ->
					mapView.setAnimationDelegate(
						new MapAnimateTranslate(mapView, render, loc, 0.5f,
						                        control.viewBounds, control.zoomAnimationEasing)));

			isActive = false;

			return true;
		}
	}

	// Update rotation when there are two fingers working
	void handleRotation(MotionEvent event)
	{
		if (allowRotate && event.getPointerCount() > 1)
		{
			final MotionEvent.PointerCoords p0 = new MotionEvent.PointerCoords();
			final MotionEvent.PointerCoords p1 = new MotionEvent.PointerCoords();
			event.getPointerCoords(0,p0);
			event.getPointerCoords(1,p1);
			final double cX = (p0.x+p1.x)/2.0;
			final double cY = (p0.y+p1.y)/2.0;
			final double dx = p0.x-cX;
			final double dy = p0.y-cY;

			// Calculate a starting rotation
			if (startRot == Double.MAX_VALUE) {
				startRot = Math.atan2(dy, dx);
				startViewRot = mapView.getRot();
			} else {
				// Update an existing rotation
				final double curRot = Math.atan2(dy, dx);
				final double diffRot = curRot-startRot;
				mapView.setRot(startViewRot+diffRot);
			}
		}
	}

	// Cancel an outstanding rotation
	void cancelRotation()
	{
		startRot = Double.MAX_VALUE;
	}

	// Where we receive events from the gl view
	public boolean onTouch(@SuppressWarnings("unused") View v, MotionEvent event)
	{
		final MapController control = mapControl.get();
		if (control == null || sl == null || gl == null || gd == null || sgd == null) {
			return false;
		}

		final boolean slWasActive = sl.isActive;
		final boolean glWasActive = gl.isActive;
		final boolean rotWasActive = startRot != Double.MAX_VALUE;

		// If they're using two fingers, cancel any outstanding pan
		if (event.getPointerCount() == 2)
			gl.isActive = false;

		// Try for a pinch or another gesture
		if (sl.isActive || event.getPointerCount() == 2) {
			sgd.onTouchEvent(event);
			handleRotation(event);
		}
		if (!sl.isActive && event.getPointerCount() == 1) {
			gd.onTouchEvent(event);
			if (event.getAction() == MotionEvent.ACTION_UP) {
				gl.isActive = false;
			}
			cancelRotation();
		}

		if (!sl.isActive && !gl.isActive && !slWasActive && allowZoom) {
			if (System.currentTimeMillis() - lastZoomEnd >= 175) {
				if (event.getPointerCount() == 2 && (event.getActionMasked() == MotionEvent.ACTION_POINTER_UP)) {
					Point3d loc = mapView.getLoc();
					double newZ = loc.getZ() * 2.0;
					newZ = Math.min(newZ, zoomLimitMax);
					newZ = Math.max(newZ, zoomLimitMin);
					loc.setValue(loc.getX(), loc.getY(), newZ);

					// Now kick off the animation
					final RendererWrapper renderWrap = control.renderWrapper;
					if (renderWrap !=  null) {
						final RenderController render = renderWrap.maplyRender.get();
						if (render != null) {
							mapView.setAnimationDelegate(
									new MapAnimateTranslate(mapView, render, loc, 0.5f,
									                        control.viewBounds, control.zoomAnimationEasing));
						}
					}

					sl.isActive = false;
					gl.isActive = false;
				}
			}
		}

		if(!glWasActive && this.gl.isActive) {
			control.panDidStart(true);
		}

		if(glWasActive && !this.gl.isActive) {
			control.panDidEnd(true);
		}

		if(!slWasActive && this.sl.isActive) {
			control.zoomDidStart(true);
		}

		if(slWasActive && !this.sl.isActive) {
			control.zoomDidEnd(true);
		}

		if (!rotWasActive && startRot != Double.MAX_VALUE) {
			control.rotateDidStart(true);
		}

		if (rotWasActive && startRot == Double.MAX_VALUE) {
			control.rotateDidEnd(true);
		}


		return true;
	}
}
