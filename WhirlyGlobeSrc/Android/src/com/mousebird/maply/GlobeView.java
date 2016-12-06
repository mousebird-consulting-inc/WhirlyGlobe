/*
 *  GlobeView.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/13/15.
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

import java.util.GregorianCalendar;

/**
 * The Globe View handles math related to user position and orientation.
 * It's largely opaque to toolkit users.  The MaplyController handles
 * passing data to and getting data from it.
 * 
 */
public class GlobeView extends View
{
	private GlobeView()
	{
	}

	GlobeController control = null;
	double lastUpdated = 0.0;

	GlobeView(GlobeController inControl,CoordSystemDisplayAdapter inCoordAdapter)
	{
		control = inControl;
		coordAdapter = inCoordAdapter;
		initialise(coordAdapter);
	}
	
	/**
	 * Make a copy of this MapView and return it.
	 * Handles the native side stuff
	 */
	protected GlobeView clone()
	{
		GlobeView that = new GlobeView(control,coordAdapter);
		nativeClone(that);
		return that;
	}

	public void finalize()
	{
		dispose();
	}
	
	// Return a view state for this Map View
	@Override public ViewState makeViewState(MaplyRenderer renderer)
	{
		return new GlobeViewState(this,renderer);
	}
		
	// These are viewpoint animations
	interface AnimationDelegate
	{
		// Called to update the view every frame
		public void updateView(GlobeView view);
	}
	
	AnimationDelegate animationDelegate = null;
	
	// Set the animation delegate.  Called every frame.
	void setAnimationDelegate(AnimationDelegate delegate)
	{
		animationDelegate = delegate;
	}
	
	// Clear the animation delegate
	@Override public void cancelAnimation() 
	{
		animationDelegate = null;

		control.activity.runOnUiThread(
				new Runnable() {
					@Override
					public void run() {
						control.handleStopMoving(false);
					}
				}
		);
	}
	
	// Called on the rendering thread right before we render
	@Override public void animate() 
	{
		if (animationDelegate != null)
		{
			animationDelegate.updateView(this);
			// Note: This is probably the wrong thread
			runViewUpdates();
		}
	}

	/**
	 * Check if the globe position is being animated.
     */
	@Override
	public boolean isAnimating()
	{
		if (animationDelegate != null)
			return true;

		return false;
	}
	
	/**
	 * Set if we want to keep north pointed upward as the user moves
	 */
	public boolean northUp = true;
	
	// Set the view location from a Point3d
	void setLoc(Point3d loc)
	{
		double z = loc.getZ();
		z = Math.min(maxHeightAboveSurface(), z);
		z = Math.max(minHeightAboveSurface(), z);
		
	    Quaternion newRot = makeRotationToGeoCoord(loc.getX(),loc.getY(),northUp);
	    setRotQuatNative(newRot);
	    setHeight(z);
		
		runViewUpdates();
	}
	
	// Set the view rotation from a Quaternion
	void setRotQuat(Quaternion quat)
	{
		setRotQuatNative(quat);
		
		runViewUpdates();
	}

	// Return the position in model coordinates for the eye
	Point3d getEyePosition()
	{
		Matrix4d mat = calcModelViewMatrix();
		Point4d pos = mat.inverse().multiply(new Point4d(0,0,0,1));
		return new Point3d(pos.getX(),pos.getY(),pos.getZ());
	}

	double getHeading()
	{
		double retHeading = 0.0;

		// Figure out where the north pole went
        Point3d northPole = getRotQuat().multiply(new Point3d(0,0,1)).normalized();
		if (northPole.getY() != 0.0)
			retHeading = Math.atan2(-northPole.getX(),northPole.getY());

		return retHeading;
	}

	void setHeading(double heading)
    {
        if (Double.isNaN(heading))
        {
            return;
        }

        // Undo the current heading
        Point3d localPt = currentUp();
        Point3d northPole = getRotQuat().multiply(new Point3d(0,0,1)).normalized();
        Quaternion posQuat = getRotQuat();
        if (northPole.getY() != 0.0)
        {
            // Then rotate it back on to the YZ axis
            // This will keep it upward
            double ang = Math.atan(northPole.getX()/northPole.getY());
            // However, the pole might be down now
            // If so, rotate it back up
            if (northPole.getY() < 0.0)
                ang += Math.PI;
            AngleAxis upRot = new AngleAxis(ang,localPt);
            posQuat = posQuat.multiply(upRot);
        }

        AngleAxis rot = new AngleAxis(heading,localPt);
        Quaternion newRotQuat = posQuat.multiply(rot);

        setRotQuat(newRotQuat);
    }

    Point3d currentUp()
    {
        Matrix4d modelMat = calcModelViewMatrix().inverse();

        Point4d newUp = modelMat.multiply(new Point4d(0,0,1,0));
        return new Point3d(newUp.getX(),newUp.getY(),newUp.getZ());
    }
		
	// Calculate the point on the view plane given the screen location
	native Point3d pointOnSphereFromScreen(Point2d screenPt,Matrix4d viewModelMatrix,Point2d frameSize,boolean clip);
	// Calculate the point on the screen from a point on the view plane
	native Point2d pointOnScreenFromSphere(Point3d pt,Matrix4d viewModelMatrix,Point2d frameSize);
	// Minimum possible height above the surface
	native double minHeightAboveSurface();
	// Maximum possible height above the surface
	native double maxHeightAboveSurface();
	// Current rotation Quaternion
	native Quaternion getRotQuat();
	// Set the current rotation Quaternion
	native void setRotQuatNative(Quaternion q);
	// Get the current view location
	public native Point3d getLoc();
	// Get just the height
	public native double getHeight();
	// Return the current tilt
	public native double getTilt();
	// Set the tilt directly
	public native void setTilt(double tilt);
    // Set just the height, rather than the whole location
	public native void setHeight(double z);
	// Calculate a rotation to the given (absolute) geographic location
	public native Quaternion makeRotationToGeoCoord(double x,double y,boolean northUp);
	// Run (0,0,1) through the given rotation to see where it winds up
	public native Point3d prospectiveUp(Quaternion rot);
	// Project a point from the screen to model space
	public native Point3d pointUnproject(Point2d touch,Point2d frameSize,boolean normalized);
	// If set, we can zoom closer to the globe
	public native void setContinuousZoom(boolean continuousZoom);
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(CoordSystemDisplayAdapter coordAdapter);
	// Make a copy of this globe view and return it
	protected native void nativeClone(GlobeView dest);
	native void dispose();
}
