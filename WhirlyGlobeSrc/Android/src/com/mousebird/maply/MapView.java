/*
 *  MapView.java
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

import java.util.ArrayList;

/**
 * The Map View handles math related to user position and orientation.
 * It's largely opaque to toolkit users.  The MaplyController handles
 * passing data to and getting data from it.
 * 
 */
class MapView 
{	
	private MapView()
	{
	}
	
	CoordSystemDisplayAdapter coordAdapter = null;
	
	MapView(CoordSystemDisplayAdapter inCoordAdapter)
	{
		coordAdapter = inCoordAdapter;
		initialise(coordAdapter);
	}
	
	/**
	 * Make a copy of this MapView and return it.
	 * Handles the native side stuff
	 */
	protected MapView clone()
	{
		MapView that = new MapView(coordAdapter);
		nativeClone(that);
		return that;
	}

	public void finalize()
	{
		dispose();
	}

	/**
	 * Return the coordinate adapter used by this view.
	 * The coordinate adapter manages transformation from the local coordinate system
	 * to display coordinates and vice versa.
	 * @return
	 */
	CoordSystemDisplayAdapter getCoordAdapter()
	{
		return coordAdapter;
	}
	
	// For objects that want to know when the view changes (every time it does)
	interface ViewWatcher
	{
		public void viewUpdated(MapView view);
	}
	
	// These are viewpoint animations
	interface AnimationDelegate
	{
		// Called to update the view every frame
		public void updateView(MapView view);
	}
	
	AnimationDelegate animationDelegate = null;
	
	// Set the animation delegate.  Called every frame.
	void setAnimationDelegate(AnimationDelegate delegate)
	{
		animationDelegate = delegate;
	}
	
	// Clear the animation delegate
	public void cancelAnimation() 
	{
		animationDelegate = null;
	}
	
	// Called on the rendering thread right before we render
	public void animate() 
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
	
	ArrayList<ViewWatcher> watchers = new ArrayList<ViewWatcher>();
	
	// Add a watcher for callbacks on each and every view related change
	void addViewWatcher(ViewWatcher watcher)
	{
		watchers.add(watcher);
	}
	// Remove an object that was watching view changes
	void removeViewWatcher(ViewWatcher watcher)
	{
		watchers.remove(watcher);
	}
	// Let everything know we changed the view
	void runViewUpdates()
	{
//		Point3d loc = getLoc();
//		Log.i("Maply","New pos: (" + loc.getX() + "," + loc.getY() + "," + loc.getZ() + ")");
		for (ViewWatcher watcher: watchers)
			watcher.viewUpdated(this);
	}
	
	// Minimum possible height above the surface
	native double minHeightAboveSurface();
	// Maximum possible height above the surface
	native double maxHeightAboveSurface();
	// Set the view location (including height)
	private native void setLoc(double x,double y,double z);
	// Get the current view location
	public native Point3d getLoc();
	// Set the 2D rotation
	native void setRot(double rot);
	// Return the 2D rotation
	native double getRot();
	// Return the current model & view matrix combined (but not projection)
	native Matrix4d calcModelViewMatrix();	
	// Calculate the point on the view plane given the screen location
	native Point3d pointOnPlaneFromScreen(Point2d screenPt,Matrix4d viewModelMatrix,Point2d frameSize,boolean clip);
	// Calculate the point on the screen from a point on the view plane
	native Point2d pointOnScreenFromPlane(Point3d pt,Matrix4d viewModelMatrix,Point2d frameSize);
	// Make a copy of this map view and return it
	protected native void nativeClone(MapView dest);
			
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(CoordSystemDisplayAdapter coordAdapter);
	native void dispose();
	private long nativeHandle;
}
