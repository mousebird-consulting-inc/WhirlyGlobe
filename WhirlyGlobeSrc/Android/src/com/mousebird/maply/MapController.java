/*
 *  MaplyController.java
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

import android.app.Activity;
import android.view.MotionEvent;
import android.view.View;

import java.util.List;

/**
 * The MaplyController is the main object in the Maply library when using a 2D map.  
 * Toolkit users add and remove their geometry through here.
 * <p>
 * The controller starts by creating an OpenGL ES surface and handling
 * all the various setup between Maply and that surface.  It also kicks off
 * a LayerThread, which it uses to queue requests to the rest of Maply.
 * <p>
 * Once the controller is set up and running the toolkit user can make
 * calls to add and remove geometry.  Those calls are thread safe.
 * 
 * @author sjg
 *
 */
public class MapController extends MaplyBaseController implements View.OnTouchListener
{	
	public MapController(Activity mainActivity)
	{
		super(mainActivity);

		// Need a coordinate system to display conversion
		// For now this just sets up spherical mercator
		coordAdapter = new CoordSystemDisplayAdapter(new SphericalMercatorCoordSystem());

		// Create the scene and map view 
		mapScene = new MapScene(coordAdapter);
		scene = mapScene;
		mapView = new MapView(coordAdapter);		
		view = mapView;
		
		super.Init();
		
		if (glSurfaceView != null)
		{
			glSurfaceView.setOnTouchListener(this);
			gestureHandler = new MapGestureHandler(this,glSurfaceView);
		}
		
		// Set up the bounds
		Point3d ll = new Point3d(),ur = new Point3d();
		coordAdapter.getBounds(ll,ur);
		// Allow E/W wraping
		ll.setValue(Float.MAX_VALUE, ll.getY(), ll.getZ());
		ur.setValue(-Float.MAX_VALUE, ur.getY(), ur.getZ());
		setViewExtents(new Point2d(ll.getX(),ll.getY()),new Point2d(ur.getX(),ur.getY()));
	}
	
	@Override public void shutdown()
	{
		super.shutdown();
		mapView = null;
		mapScene = null;
	}
	
	@Override public void dispose()
	{
		// Note: Is this implied?
		super.dispose();
		
		mapScene.dispose();
		mapScene = null;
		mapView.dispose();
		mapView = null;
	}
	
	// Map version of view
	MapView mapView = null;
	
	// Map version of scene
	MapScene mapScene = null;
	
	/**
	 * Return the screen coordinate for a given geographic coordinate (in radians).
	 * 
	 * @param geoCoord Geographic coordinate to convert (in radians).
	 * @return Screen coordinate.
	 */
	public Point2d screenPointFromGeo(Point2d geoCoord)
	{
		return screenPointFromGeo(mapView,geoCoord);
	}
	
	/**
	 * Return the geographic point (radians) corresponding to the screen point.
	 * 
	 * @param screenPt Input point on the screen.
	 * @return The geographic coordinate (radians) corresponding to the screen point.
	 */
	public Point2d geoPointFromScreen(Point2d screenPt)
	{
		CoordSystemDisplayAdapter coordAdapter = mapView.getCoordAdapter();
		CoordSystem coordSys = coordAdapter.getCoordSystem();
		
		Matrix4d modelMat = mapView.calcModelViewMatrix();
		Point3d dispPt = mapView.pointOnPlaneFromScreen(screenPt, modelMat, renderWrapper.maplyRender.frameSize, false);
		Point3d localPt = coordAdapter.displayToLocal(dispPt);
		Point3d geoCoord = coordSys.localToGeographic(localPt);
		
		return new Point2d(geoCoord.getX(),geoCoord.getY());
	}
	
	/**
	 * Returns what the user is currently looking at in geographic extents.
	 */
	public Mbr getCurrentViewGeo()
	{
		Mbr geoMbr = new Mbr();
		
		Point2d frameSize = renderWrapper.maplyRender.frameSize;
		geoMbr.addPoint(geoPointFromScreen(new Point2d(0,0)));
		geoMbr.addPoint(geoPointFromScreen(new Point2d(frameSize.getX(),0)));
		geoMbr.addPoint(geoPointFromScreen(new Point2d(frameSize.getX(),frameSize.getY())));
		geoMbr.addPoint(geoPointFromScreen(new Point2d(0,frameSize.getY())));
		
		return geoMbr;
	}
	
	// Convert a geo coord to a screen point
	private Point2d screenPointFromGeo(MapView theMapView,Point2d geoCoord)
	{
		CoordSystemDisplayAdapter coordAdapter = theMapView.getCoordAdapter();
		CoordSystem coordSys = coordAdapter.getCoordSystem();
		Point3d localPt = coordSys.geographicToLocal(new Point3d(geoCoord.getX(),geoCoord.getY(),0.0));
		Point3d dispPt = coordAdapter.localToDisplay(localPt);
		
		Matrix4d modelMat = theMapView.calcModelViewMatrix();
		return theMapView.pointOnScreenFromPlane(dispPt, modelMat, renderWrapper.maplyRender.frameSize);
	}
	
	boolean checkCoverage(Mbr mbr,MapView theMapView,double height)
	{
		Point2d centerLoc = mbr.middle();
		Point3d localCoord = theMapView.coordAdapter.coordSys.geographicToLocal(new Point3d(centerLoc.getX(),centerLoc.getY(),0.0));
		theMapView.setLoc(new Point3d(localCoord.getX(),localCoord.getY(),height));
		
		List<Point2d> pts = mbr.asPoints();
		Point2d frameSize = renderWrapper.maplyRender.frameSize;
		for (Point2d pt : pts)
		{
			Point2d screenPt = screenPointFromGeo(theMapView,pt);
			if (screenPt.getX() < 0.0 || screenPt.getY() < 0.0 || screenPt.getX() > frameSize.getX() || screenPt.getY() > frameSize.getY())
				return false;
		}
		
		return true;
	}
	
	/**
	 * For a given position, how high do we have to be to see the given area.
	 * <p>
	 * Even for 2D maps we represent things in terms of height.
	 * 
	 * @param mbr Bounding box for the area we want to see in geographic (radians).
	 * @param pos Center of the viewing area in geographic (radians).
	 * @return Returns a height for the viewer.
	 */
	public double findHeightToViewBounds(Mbr mbr,Point2d pos)
	{
		// We'll experiment on a copy of the map view
		MapView newMapView = mapView.clone();
		newMapView.setLoc(new Point3d(pos.getX(),pos.getY(),2.0));
		
		double minHeight = newMapView.minHeightAboveSurface();
		double maxHeight = newMapView.maxHeightAboveSurface();
		
		boolean minOnScreen = checkCoverage(mbr,newMapView,minHeight);
		boolean maxOnScreen = checkCoverage(mbr,newMapView,maxHeight);
		
		// No idea, just give up
		if (!minOnScreen && !maxOnScreen)
			return mapView.getLoc().getZ();
		
		if (minOnScreen)
			return minHeight;
		
		// Do a binary search between the two heights
		double minRange = 1e-5;
		do
		{
			double midHeight = (minHeight + maxHeight)/2.0;
			boolean midOnScreen = checkCoverage(mbr,newMapView,midHeight);
			
			if (!minOnScreen && midOnScreen)
			{
				maxHeight = midHeight;
				maxOnScreen = midOnScreen;
			} else if (!midOnScreen && maxOnScreen)
			{
				checkCoverage(mbr,newMapView,midHeight);
				minHeight = midHeight;
				minOnScreen = midOnScreen;
			} else {
				// Shouldn't happen, but probably does
				break;
			}
			
			if (maxHeight-minHeight < minRange)
				break;
		} while (true);
		
		return maxHeight;
	}

	/**
	 * Set the current view position.
	 * @param x Horizontal location of the center of the screen in geographic radians (not degrees).
	 * @param y Vertical location of the center of the screen in geographic radians (not degrees).
	 * @param z Height above the map in display units.
	 */
	public void setPositionGeo(double x,double y,double z)
	{
		if (!running)
			return;

		mapView.cancelAnimation();
		Point3d geoCoord = mapView.coordAdapter.coordSys.geographicToLocal(new Point3d(x,y,0.0));
		mapView.setLoc(new Point3d(geoCoord.getX(),geoCoord.getY(),z));
	}

	/**
	 * Return the current position in local coordinates.
     */
	public Point3d getPosition()
	{
		return new Point3d(mapView.getLoc());
	}
	
	/**
	 * Animate to a new view position
	 * @param x Horizontal location of the center of the screen in geographic radians (not degrees).
	 * @param y Vertical location of the center of the screen in geographic radians (not degrees).
	 * @param z Height above the map in display units.
	 * @param howLong Time (in seconds) to animate.
	 */
	public void animatePositionGeo(double x,double y,double z,double howLong)
	{
		if (!running)
			return;

		mapView.cancelAnimation();
		Point3d geoCoord = mapView.coordAdapter.coordSys.geographicToLocal(new Point3d(x,y,0.0));
		mapView.setAnimationDelegate(new MapAnimateTranslate(mapView, renderWrapper.maplyRender, new Point3d(geoCoord.getX(),geoCoord.getY(),z), (float) howLong, viewBounds));		
	}
	
	// Gesture handler
	MapGestureHandler gestureHandler = null;
	
	/**
	 * Use this delegate when you want user interface feedback from the maply controller.
	 * 
	 * @author sjg
	 *
	 */
	public interface GestureDelegate
	{
		/**
		 * The user selected the given object.  Up to you to figure out what it is.
		 * 
		 * @param mapControl The maply controller this is associated with.
		 * @param selObj The object the user selected (e.g. MaplyScreenMarker).
		 * @param loc The location they tapped on.  This is in radians.
		 * @param screenLoc The location on the OpenGL surface.
		 */
		void userDidSelect(MapController mapControl,Object selObj,Point2d loc,Point2d screenLoc);
		
		/**
		 * The user tapped somewhere, but not on a selectable object.
		 * 
		 * @param mapControl The maply controller this is associated with.
		 * @param loc The location they tapped on.  This is in radians.
		 * @param screenLoc The location on the OpenGL surface.
		 */
		void userDidTap(MapController mapControl,Point2d loc,Point2d screenLoc);
	}

	/**
	 * Set the gesture delegate to get callbacks when the user taps somewhere.
	 */
	public GestureDelegate gestureDelegate = null;
	
	// Called by the gesture handler to let us know the user tapped
	public void processTap(Point2d screenLoc)
	{
		if (gestureDelegate != null)
		{
			Matrix4d mapTransform = mapView.calcModelViewMatrix();
			Point3d loc = mapView.pointOnPlaneFromScreen(screenLoc, mapTransform, renderWrapper.maplyRender.frameSize, false);
			
			// Look for a selection first
			long selectID = selectionManager.pickObject(mapView, screenLoc);
			if (selectID != EmptyIdentity)
			{
				// Look for the object
				Object selObj = null;
				synchronized(selectionMap)
				{
					selObj = selectionMap.get(selectID);
				}
				
				// Let the delegate know the user selected something
				gestureDelegate.userDidSelect(this, selObj, loc.toPoint2d(), screenLoc);
			} else 
				// Just a simple tap, then
				gestureDelegate.userDidTap(this, loc.toPoint2d(), screenLoc);
		}
	}
	
	// Pass the touches on to the gesture handler
	@Override
	public boolean onTouch(View view, MotionEvent e) {
		return gestureHandler.onTouch(view, e);
	}
}
