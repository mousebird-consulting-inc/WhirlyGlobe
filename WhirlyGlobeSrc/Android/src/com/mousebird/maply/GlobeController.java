/*
 *  GlobeController.java
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

import android.app.*;
import android.view.*;
import android.view.View;

import java.util.List;

/**
 * The GlobeController is the main object in the Maply library when using a 3D globe.  
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
public class GlobeController extends MaplyBaseController implements View.OnTouchListener
{	
	public GlobeController(Activity mainActivity)
	{
		super(mainActivity);
		
		// Need a coordinate system to display conversion
		// For now this just sets up spherical mercator
		coordAdapter = new FakeGeocentricDisplayAdapter();
		
		// Create the scene and map view 
		// Note: Expose the cull tree depth
		globeScene = new GlobeScene(coordAdapter,1);
		scene = globeScene;
		globeView = new GlobeView(coordAdapter);		
		view = globeView;
		
		super.Init();
		
		if (glSurfaceView != null)
		{
			glSurfaceView.setOnTouchListener(this);
			gestureHandler = new GlobeGestureHandler(this,glSurfaceView);
		}
	}
	
	@Override public void shutdown()
	{
		super.shutdown();
		globeView = null;
		globeScene = null;
	}
	
	@Override public void dispose()
	{
		// Note: Is this implied?
		super.dispose();
		
		globeScene.dispose();
		globeScene = null;
		globeView.dispose();
		globeView = null;
	}
	
	// Map version of view
	GlobeView globeView = null;
	
	// Map version of scene
	GlobeScene globeScene = null;
	
	/**
	 * Return the screen coordinate for a given geographic coordinate (in radians).
	 * 
	 * @param geoCoord Geographic coordinate to convert (in radians).
	 * @return Screen coordinate.
	 */
	public Point2d screenPointFromGeo(Point2d geoCoord)
	{
		return screenPointFromGeo(globeView,geoCoord);
	}
	
	/**
	 * Return the geographic point (radians) corresponding to the screen point.
	 * 
	 * @param screenPt Input point on the screen.
	 * @return The geographic coordinate (radians) corresponding to the screen point.
	 */
	public Point2d geoPointFromScreen(Point2d screenPt)
	{
		CoordSystemDisplayAdapter coordAdapter = globeView.getCoordAdapter();
		CoordSystem coordSys = coordAdapter.getCoordSystem();

		Matrix4d modelMat = globeView.calcModelViewMatrix();
		Point3d dispPt = globeView.pointOnSphereFromScreen(screenPt, modelMat, renderWrapper.maplyRender.frameSize, false);
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
	private Point2d screenPointFromGeo(GlobeView theGlobeView,Point2d geoCoord)
	{
		CoordSystemDisplayAdapter coordAdapter = theGlobeView.getCoordAdapter();
		CoordSystem coordSys = coordAdapter.getCoordSystem();
		Point3d localPt = coordSys.geographicToLocal(new Point3d(geoCoord.getX(),geoCoord.getY(),0.0));
		Point3d dispPt = coordAdapter.localToDisplay(localPt);

		Matrix4d modelMat = theGlobeView.calcModelViewMatrix();
		return theGlobeView.pointOnScreenFromSphere(dispPt, modelMat, renderWrapper.maplyRender.frameSize);
	}

	boolean checkCoverage(Mbr mbr,GlobeView theGlobeView,double height)
	{
		Point2d centerLoc = mbr.middle();
		Point3d localCoord = theGlobeView.coordAdapter.coordSys.geographicToLocal(new Point3d(centerLoc.getX(),centerLoc.getY(),0.0));
		theGlobeView.setLoc(new Point3d(localCoord.getX(),localCoord.getY(),height));

		List<Point2d> pts = mbr.asPoints();
		Point2d frameSize = renderWrapper.maplyRender.frameSize;
		for (Point2d pt : pts)
		{
			Point2d screenPt = screenPointFromGeo(theGlobeView,pt);
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
		GlobeView newGlobeView = globeView.clone();
		newGlobeView.setLoc(new Point3d(pos.getX(),pos.getY(),2.0));

		double minHeight = newGlobeView.minHeightAboveSurface();
		double maxHeight = newGlobeView.maxHeightAboveSurface();

		boolean minOnScreen = checkCoverage(mbr,newGlobeView,minHeight);
		boolean maxOnScreen = checkCoverage(mbr,newGlobeView,maxHeight);

		// No idea, just give up
		if (!minOnScreen && !maxOnScreen)
			return globeView.getHeight();

		if (minOnScreen)
			return minHeight;

		// Do a binary search between the two heights
		double minRange = 1e-5;
		do
		{
			double midHeight = (minHeight + maxHeight)/2.0;
			boolean midOnScreen = checkCoverage(mbr,newGlobeView,midHeight);

			if (!minOnScreen && midOnScreen)
			{
				maxHeight = midHeight;
				maxOnScreen = midOnScreen;
			} else if (!midOnScreen && maxOnScreen)
			{
				checkCoverage(mbr,newGlobeView,midHeight);
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

		globeView.cancelAnimation();
		Point3d geoCoord = globeView.coordAdapter.coordSys.geographicToLocal(new Point3d(x,y,0.0));
		globeView.setLoc(new Point3d(geoCoord.getX(),geoCoord.getY(),z));
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

		globeView.cancelAnimation();
		Point3d geoCoord = globeView.coordAdapter.coordSys.geographicToLocal(new Point3d(x,y,0.0));
        Quaternion newQuat = globeView.makeRotationToGeoCoord(x, y, globeView.northUp);
        globeView.setAnimationDelegate(new GlobeAnimateRotation(globeView, renderWrapper.maplyRender, newQuat, z, howLong));
	}
	
	// Gesture handler
	GlobeGestureHandler gestureHandler = null;
	
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
		 * @param globeControl The maply controller this is associated with.
		 * @param selObj The object the user selected (e.g. MaplyScreenMarker).
		 * @param loc The location they tapped on.  This is in radians.
		 * @param screenLoc The location on the OpenGL surface.
		 */
        public void userDidSelect(GlobeController globeControl,Object selObj,Point2d loc,Point2d screenLoc);
		
		/**
		 * The user tapped somewhere, but not on a selectable object.
		 * 
		 * @param globeControl The maply controller this is associated with.
		 * @param loc The location they tapped on.  This is in radians.  If null, then the user tapped outside the globe.
		 * @param screenLoc The location on the OpenGL surface.
		 */
        public void userDidTap(GlobeController globeControl,Point2d loc,Point2d screenLoc);

        /**
         * Called when the globe first starts moving.
         *
         * @param globeControl The globe controller this is associated with.
         * @param userMotion Set if the motion was caused by a gesture.
         */
        public void globeDidStartMoving(GlobeController globeControl, boolean userMotion);

        /**
         * Called when the globe stops moving.
         *
         * @param globeControl The globe controller this is associated with.
         * @param corners Corners of the viewport.  If one of them is null, that means it doesn't land on the globe.
         * @param userMotion SEt if the motion was caused by a gesture.
         */
        public void globeDidStopMoving(GlobeController globeControl, Point3d corners[], boolean userMotion);
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
			Matrix4d globeTransform = globeView.calcModelViewMatrix();
			Point3d loc = globeView.pointOnSphereFromScreen(screenLoc, globeTransform, renderWrapper.maplyRender.frameSize, false);

			// Look for a selection first
			long selectID = selectionManager.pickObject(globeView, screenLoc);
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
			{
				// Just a simple tap, then
				gestureDelegate.userDidTap(this, loc.toPoint2d(), screenLoc);
			}
		}
	}
	
	// Pass the touches on to the gesture handler
	@Override
	public boolean onTouch(View view, MotionEvent e) {
		return gestureHandler.onTouch(view, e);
	}

    boolean isTilting = false, isPanning = false, isZooming = false, isRotating = false, isAnimating = false;

    public void tiltDidStart(boolean userMotion) { handleStartMoving(userMotion); isTilting = true; }
    public void tiltDidEnd(boolean userMotion) { isTilting = false; handleStopMoving(userMotion); }
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
        if (!isPanning && !isRotating && !isZooming && !isAnimating && !isTilting)
            if (gestureDelegate != null)
                gestureDelegate.globeDidStartMoving(this,userMotion);
    }

    // Calculate visible corners
    Point3d[] calcCorners()
    {
        Point2d screenCorners[] = new Point2d[4];
        Point2d frameSize = renderWrapper.maplyRender.frameSize;
        screenCorners[0] = new Point2d(0.0, 0.0);
        screenCorners[1] = new Point2d(frameSize.getX(), 0.0);
        screenCorners[2] = new Point2d(frameSize.getX(), frameSize.getY());
        screenCorners[3] = new Point2d(0.0, frameSize.getY());

        Matrix4d modelMat = globeView.calcModelViewMatrix();

        Point3d retCorners[] = new Point3d[4];
        for (int ii=0;ii<4;ii++)
        {
            retCorners[ii] = globeView.pointOnSphereFromScreen(screenCorners[0],modelMat,frameSize,false);
        }

        return retCorners;
    }

    /**
     * Called by the gesture handler to filter out end motion events.
     *
     * @param userMotion Set if kicked off by user motion.
     */
    public void handleStopMoving(boolean userMotion)
    {
        if (isPanning || isRotating || isZooming || isAnimating || isTilting)
            return;

        if (gestureDelegate != null)
        {
            Point3d corners[] = calcCorners();
            gestureDelegate.globeDidStopMoving(this,corners,userMotion);
        }
    }
}
