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
	
	GlobeView(CoordSystemDisplayAdapter inCoordAdapter)
	{
		coordAdapter = inCoordAdapter;
		initialise(coordAdapter);
	}
	
	/**
	 * Make a copy of this MapView and return it.
	 * Handles the native side stuff
	 */
	protected GlobeView clone()
	{
		GlobeView that = new GlobeView(coordAdapter);
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
	
	// Set the view location from a Point3d
	void setLoc(Point3d loc)
	{
		double z = loc.getZ();
		z = Math.min(maxHeightAboveSurface(), z);
		z = Math.max(minHeightAboveSurface(), z);
		
		setLoc(loc.getX(),loc.getY(),z);
		
		runViewUpdates();
	}
		
	// Calculate the point on the view plane given the screen location
//	native Point3d pointOnSphereFromScreen(Point2d screenPt,Matrix4d viewModelMatrix,Point2d frameSize,boolean clip);
	// Calculate the point on the screen from a point on the view plane
//	native Point2d pointOnScreenFromSphere(Point3d pt,Matrix4d viewModelMatrix,Point2d frameSize);
	// Minimum possible height above the surface
	native double minHeightAboveSurface();
	// Maximum possible height above the surface
	native double maxHeightAboveSurface();
	// Set the view location (including height)
	private native void setLoc(double x,double y,double z);
	// Get the current view location
	public native Point3d getLoc();

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
