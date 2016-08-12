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
import android.graphics.Color;
import android.opengl.GLSurfaceView;
import android.view.Choreographer;
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
public class MapController extends MaplyBaseController implements View.OnTouchListener, Choreographer.FrameCallback
{

	/**
	 * Settings are parameters we need at the very start of the
	 * setup process.
	 */
	public static class Settings extends MaplyBaseController.Settings
	{
		/**
		 * Coordinate system to use for the map.
		 */
		public CoordSystem coordSys = null;
		/**
		 * Center of the coordinate system.
		 */
		public Point3d displayCenter = null;
		/**
		 * Clear color to use for the background.
		 */
		public int clearColor = Color.BLACK;
	}

	/**
	 * Construct with the activity and a coordinate system.  You use this one if you're
	 * using a custom coordinate system.
	 *
	 * @param mainActivity The activity this is part of.
     */
	public MapController(Activity mainActivity,Settings settings)
	{
		super(mainActivity,settings);

		if (settings.coordSys != null)
			InitCoordSys(mainActivity,settings.coordSys,settings.displayCenter,settings.clearColor);
		else
			Init(mainActivity,settings.clearColor);
	}

	protected void InitCoordSys(Activity mainActivity,CoordSystem coordSys,Point3d displayCenter,int clearColor)
	{
		Mbr mbr = coordSys.getBounds();
		double scaleFactor = 1.0;
		if (mbr != null) {
			// May need to scale this to the space we're expecting
			if (Math.abs(mbr.ur.getX() - mbr.ll.getX()) > 10.0 || Math.abs(mbr.ur.getY() - mbr.ll.getY()) > 10.0) {
				scaleFactor = 4.0 / Math.max (mbr.ur.getX() - mbr.ll.getX(), mbr.ur.getY() - mbr.ll.getY());
			}
		}
		Point3d center;
		if (displayCenter != null)
			center = displayCenter;
		else
			center = new Point3d(0,0,0);
		GeneralDisplayAdapter genCoordAdapter = new GeneralDisplayAdapter(coordSys,coordSys.ll,coordSys.ur,center,new Point3d(scaleFactor,scaleFactor,1.0));

		setupTheRest(genCoordAdapter);

		// Set up the bounds
		Point3d ll = new Point3d(),ur = new Point3d();
		coordAdapter.getBounds(ll,ur);
		setViewExtents(new Point2d(ll.getX(),ll.getY()),new Point2d(ur.getX(),ur.getY()));
	}

	/**
	 * Initialize a new map controller with the standard (spherical mercator)
	 * coordinate system.
	 *
     */
	public MapController(Activity mainActivity)
	{
		super(mainActivity,null);

		Init(mainActivity, Color.TRANSPARENT);
	}

	protected void Init(Activity mainActivity,int clearColor)
	{
		setupTheRest(new CoordSystemDisplayAdapter(new SphericalMercatorCoordSystem()));

		// Set up the bounds
		Point3d ll = new Point3d(),ur = new Point3d();
		coordAdapter.getBounds(ll,ur);
		// Allow E/W wraping
		ll.setValue(Float.MAX_VALUE, ll.getY(), ll.getZ());
		ur.setValue(-Float.MAX_VALUE, ur.getY(), ur.getZ());
		setViewExtents(new Point2d(ll.getX(),ll.getY()),new Point2d(ur.getX(),ur.getY()));
	}

	protected void setupTheRest(CoordSystemDisplayAdapter inCoordAdapter)
	{
		coordAdapter = inCoordAdapter;

		// Create the scene and map view
		mapScene = new MapScene(coordAdapter);
		scene = mapScene;
		mapView = new MapView(this,coordAdapter);
		view = mapView;

		super.Init();

		if (baseView != null)
		{
			if (baseView instanceof GLSurfaceView) {
				GLSurfaceView glSurfaceView = (GLSurfaceView)baseView;
				glSurfaceView.setOnTouchListener(this);
			} else {
				GLTextureView glTextureView = (GLTextureView)baseView;
				glTextureView.setOnTouchListener(this);
			}
			gestureHandler = new MapGestureHandler(this,baseView);
		}

		addPostSurfaceRunnable(new Runnable() {
			@Override
			public void run() {
				// No lights for the map by default
				clearLights();
			}
		});
	}
	
	@Override public void shutdown()
	{
		Choreographer c = Choreographer.getInstance();
		if (c != null)
			c.removeFrameCallback(this);
		mapView.cancelAnimation();
		super.shutdown();
		mapView = null;
		mapScene = null;
		if (gestureHandler != null)
		{
			gestureHandler.shutdown();
		}
		gestureDelegate = null;
		gestureHandler = null;
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
		if (!running || mapView == null)
			return null;

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
		if (!running || mapView == null)
			return null;

		CoordSystemDisplayAdapter coordAdapter = mapView.getCoordAdapter();
		CoordSystem coordSys = coordAdapter.getCoordSystem();
		
		Matrix4d modelMat = mapView.calcModelViewMatrix();
		Point3d dispPt = mapView.pointOnPlaneFromScreen(screenPt, modelMat, renderWrapper.maplyRender.frameSize, false);
		if (dispPt == null)
			return null;
		Point3d localPt = coordAdapter.displayToLocal(dispPt);
		if (localPt == null)
			return null;
		Point3d geoCoord = coordSys.localToGeographic(localPt);
		if (geoCoord == null)
			return null;
		
		return new Point2d(geoCoord.getX(),geoCoord.getY());
	}
	
	/**
	 * Returns what the user is currently looking at in geographic extents.
	 */
	public Mbr getCurrentViewGeo()
	{
		if (!running || mapView == null)
			return null;

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
		if (!running || mapView == null)
			return false;

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
	 * Set the zoom limits for the globe.
	 * @param inMin Closest the user is allowed to zoom in.
	 * @param inMax Farthest the user is allowed to zoom out.
	 */
	public void setZoomLimits(double inMin,double inMax)
	{
		if (gestureHandler != null)
			gestureHandler.setZoomLimits(inMin,inMax);
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
		if (!running || mapView == null)
			return 0.0;

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
	public void setPositionGeo(final double x,final double y,final double z)
	{
		if (!running)
			return;

		if (!rendererAttached) {
			addPostSurfaceRunnable(new Runnable() {
				@Override
				public void run() {
					setPositionGeo(x,y,z);
				}
			});
			return;
		}

		mapView.cancelAnimation();
		Point3d geoCoord = mapView.coordAdapter.coordSys.geographicToLocal(new Point3d(x,y,0.0));
		mapView.setLoc(new Point3d(geoCoord.getX(),geoCoord.getY(),z));
	}

	/**
	 * Return the position in lat/lon in radians.
	 * Height is height above the plane, which around M_PI in size.
     */
	public Point3d getPositionGeo()
	{
		if (!running || mapView == null)
			return null;

		Point3d loc = mapView.getLoc();
		if (loc == null)
			return null;
		Point3d geoLoc = mapView.coordAdapter.coordSys.localToGeographic(loc);
		return new Point3d(geoLoc.getX(),geoLoc.getY(),loc.getZ());
	}
	
	/**
	 * Animate to a new view position
	 * @param x Horizontal location of the center of the screen in geographic radians (not degrees).
	 * @param y Vertical location of the center of the screen in geographic radians (not degrees).
	 * @param z Height above the map in display units.
	 * @param howLong Time (in seconds) to animate.
	 */
	public void animatePositionGeo(final double x,final double y,final double z,final double howLong)
	{
		if (!running)
			return;

		if (!rendererAttached) {
			addPostSurfaceRunnable(new Runnable() {
				@Override
				public void run() {
					animatePositionGeo(x,y,z,howLong);
				}
			});
			return;
		}

		mapView.cancelAnimation();
		Point3d geoCoord = mapView.coordAdapter.coordSys.geographicToLocal(new Point3d(x,y,0.0));
		mapView.setAnimationDelegate(new MapAnimateTranslate(mapView, renderWrapper.maplyRender, new Point3d(geoCoord.getX(),geoCoord.getY(),z), (float) howLong, viewBounds));		
	}

	/**
	 * Set the heading for the current visual.  0 is due north.
     */
	public void setHeading(final double heading)
	{
		if (!running)
			return;

		if (!rendererAttached) {
			addPostSurfaceRunnable(new Runnable() {
				@Override
				public void run() {
					setHeading(heading);
				}
			});
			return;
		}

		mapView.cancelAnimation();
		mapView.setRot(heading);
	}

	/**
	 * Return the current heading.  0 is due north.
     */
	public double getHeading()
	{
		if (!running)
			return 0.0;

		return mapView.getRot();
	}

	/**
	 * If set we'll allow the user to rotate.
	 * If not, we'll keep north up at all times.
     */
	public void setAllowRotateGesture(boolean allowRotate)
	{
		if (!running)
			return;

		gestureHandler.allowRotate = allowRotate;
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
		 * @param selObjs The objects the user selected (e.g. MaplyScreenMarker).
		 * @param loc The location they tapped on.  This is in radians.
		 * @param screenLoc The location on the OpenGL surface.
		 */
		public void userDidSelect(MapController mapControl,SelectedObject[] selObjs,Point2d loc,Point2d screenLoc);
		
		/**
		 * The user tapped somewhere, but not on a selectable object.
		 * 
		 * @param mapControl The maply controller this is associated with.
		 * @param loc The location they tapped on.  This is in radians.
		 * @param screenLoc The location on the OpenGL surface.
		 */
		public void userDidTap(MapController mapControl,Point2d loc,Point2d screenLoc);

		/**
		 * The user long pressed somewhere, either on a selectable object or nor
		 * @param mapController The maply controller this is associated with.
		 * @param selObjs The objects (e.g. MaplyScreenMarker) that the user long pressed or null if there was none
		 * @param loc The location they tapped on.  This is in radians.
         * @param screenLoc The location on the OpenGL surface.
         */
		public void userDidLongPress(MapController mapController, SelectedObject[] selObjs, Point2d loc, Point2d screenLoc);

		/**
		 * Called when the map first starts moving.
		 *
		 * @param mapControl The map controller this is associated with.
		 * @param userMotion Set if the motion was caused by a gesture.
		 */
		public void mapDidStartMoving(MapController mapControl, boolean userMotion);

		/**
		 * Called when the map stops moving.
		 *
		 * @param mapControl The map controller this is associated with.
		 * @param corners Corners of the viewport.  If one of them is null, that means it doesn't land anywhere valid.
		 * @param userMotion Set if the motion was caused by a gesture.
		 */
		public void mapDidStopMoving(MapController mapControl, Point3d corners[], boolean userMotion);

		/**
		 * Called for every single visible frame of movement.  Be careful what you do in here.
		 *
		 * @param mapControl The map controller this is associated with.
		 * @param corners Corners of the viewport.  If one of them is null, that means it doesn't land anywhere valid.
		 * @param userMotion Set if the motion was caused by a gesture.
		 */
		public void mapDidMove(MapController mapControl,Point3d corners[], boolean userMotion);
	}

	/**
	 * Set the gesture delegate to get callbacks when the user taps somewhere.
	 */
	public GestureDelegate gestureDelegate = null;
	
	// Called by the gesture handler to let us know the user tapped
	// screenLoc is in view coordinates
	public void processTap(Point2d screenLoc)
	{
		if (gestureDelegate != null) {

			Matrix4d mapTransform = mapView.calcModelViewMatrix();
			Point3d loc = mapView.pointOnPlaneFromScreen(screenLoc, mapTransform, getViewSize(), false);

			Point3d localPt = mapView.getCoordAdapter().displayToLocal(loc);
			Point3d geoPt = null;
			if (localPt != null)
				geoPt = mapView.getCoordAdapter().getCoordSystem().localToGeographic(localPt);

//			Object selObj = this.getObjectAtScreenLoc(screenLoc);
			SelectedObject selObjs[] = this.getObjectsAtScreenLoc(screenLoc);

			if (selObjs != null) {
				if (geoPt != null)
					gestureDelegate.userDidSelect(this, selObjs, geoPt.toPoint2d(), screenLoc);
			} else {
				// Just a simple tap, then
				if (geoPt != null)
					gestureDelegate.userDidTap(this, geoPt.toPoint2d(), screenLoc);
			}
		}
	}



	/**
	 * Called by the gesture handler to let us know the user long pressed somewhere
	 * @param screenLoc
     */
    public void processLongPress(Point2d screenLoc) {

		Matrix4d mapTransform = mapView.calcModelViewMatrix();
		Point3d loc = mapView.pointOnPlaneFromScreen(screenLoc, mapTransform, renderWrapper.maplyRender.frameSize, false);

		if (gestureDelegate != null)
		{
			Point3d localPt = mapView.getCoordAdapter().displayToLocal(loc);
			Point3d geoPt = null;
			if (localPt != null)
				geoPt = mapView.getCoordAdapter().getCoordSystem().localToGeographic(localPt);

//			Object selObj = this.getObjectAtScreenLoc(screenLoc);
			SelectedObject selObjs[] = this.getObjectsAtScreenLoc(screenLoc);

			gestureDelegate.userDidLongPress(this, selObjs, geoPt.toPoint2d(), screenLoc);
		}

	}

	// Pass the touches on to the gesture handler
	@Override
	public boolean onTouch(View view, MotionEvent e) {
		if (running & gestureHandler != null)
			return gestureHandler.onTouch(view, e);

		return false;
	}

    boolean isPanning = false, isZooming = false, isRotating = false, isAnimating = false;
    
    public void panDidStart(boolean userMotion) { handleStartMoving(userMotion); isPanning = true; }
    public void panDidEnd(boolean userMotion) { isPanning = false; handleStopMoving(userMotion); }
    public void zoomDidStart(boolean userMotion) { handleStartMoving(userMotion); isZooming = true; }
    public void zoomDidEnd(boolean userMotion) { isZooming = false; handleStopMoving(userMotion); }
    public void rotateDidStart(boolean userMotion) { handleStartMoving(userMotion); isRotating = true; }
    public void rotateDidEnd(boolean userMotion) { isRotating = false; handleStopMoving(userMotion); }
    
    /**
     * Called by the gesture handler to filter out start motion events.
     *
     * @param userMotion Set if kicked off by user motion.
     */
    public void handleStartMoving(boolean userMotion)
    {
        if (renderWrapper == null || renderWrapper.maplyRender == null)
            return;
        
        if (!isPanning && !isRotating && !isZooming && !isAnimating)
            if (gestureDelegate != null) {
                gestureDelegate.mapDidStartMoving(this, userMotion);
                
                Choreographer c = Choreographer.getInstance();
                if (c != null)
                    c.postFrameCallback(this);
            }
    }
    
    /**
     * Called by the gesture handler to filter out end motion events.
     *
     * @param userMotion Set if kicked off by user motion.
     */
    public void handleStopMoving(boolean userMotion)
    {
        if (renderWrapper == null || renderWrapper.maplyRender == null)
            return;
        
        if (isPanning || isRotating || isZooming || isAnimating)
            return;
        
        if (gestureDelegate != null)
        {
            Point3d corners[] = getVisibleCorners();
            gestureDelegate.mapDidStopMoving(this,corners,userMotion);
        }
    }
    
    double lastViewUpdate = 0.0;
    /**
     * Frame callback for the Choreographer
     */
    public void doFrame(long frameTimeNanos)
    {
        if (mapView != null) {
            double newUpdateTime = mapView.getLastUpdatedTime();
            if (gestureDelegate != null && lastViewUpdate < newUpdateTime) {
                Point3d corners[] = getVisibleCorners();
                gestureDelegate.mapDidMove(this, corners, false);
                lastViewUpdate = newUpdateTime;
            }
        }
        
        Choreographer c = Choreographer.getInstance();
        if (c != null) {
            c.removeFrameCallback(this);
            c.postFrameCallback(this);
        }
    }
    
    
    /**
     * Calculate visible corners for what's currently being seen.
     * If the eye point is too high, expect null corners.
     * @return
     */
    public Point3d[] getVisibleCorners()
    {
		if (!running || mapView == null)
			return null;

        Point2d screenCorners[] = new Point2d[4];
        Point2d frameSize = renderWrapper.maplyRender.frameSize;
        screenCorners[0] = new Point2d(0.0, 0.0);
        screenCorners[1] = new Point2d(frameSize.getX(), 0.0);
        screenCorners[2] = new Point2d(frameSize.getX(), frameSize.getY());
        screenCorners[3] = new Point2d(0.0, frameSize.getY());
        
        Matrix4d modelMat = mapView.calcModelViewMatrix();
        
        Point3d retCorners[] = new Point3d[4];
        CoordSystemDisplayAdapter coordAdapter = mapView.getCoordAdapter();
        if (coordAdapter == null || renderWrapper == null || renderWrapper.maplyRender == null ||
            renderWrapper.maplyRender.frameSize == null)
            return retCorners;
        CoordSystem coordSys = coordAdapter.getCoordSystem();
        if (coordSys == null)
            return retCorners;
        for (int ii=0;ii<4;ii++)
        {
			Point3d planePt = mapView.pointOnPlaneFromScreen(screenCorners[ii],modelMat,frameSize,false);
            if (planePt != null)
                retCorners[ii] = coordSys.localToGeographic(coordAdapter.displayToLocal(planePt));
        }
        
        return retCorners;
    }

}
