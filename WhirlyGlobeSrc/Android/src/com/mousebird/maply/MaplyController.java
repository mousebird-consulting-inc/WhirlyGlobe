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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.app.*;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Looper;
import android.util.Log;
import android.view.*;
import android.widget.Toast;

/**
 * The MaplyController is the main object in the Maply library.  Toolkit
 * users add and remove their geometry through here.
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
public class MaplyController implements View.OnTouchListener
{	
	private GLSurfaceView glSurfaceView;
	Activity activity = null;

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
		void userDidSelect(MaplyController mapControl,Object selObj,Point2d loc,Point2d screenLoc);
		
		/**
		 * The user tapped somewhere, but not on a selectable object.
		 * 
		 * @param mapControl The maply controller this is associated with.
		 * @param loc The location they tapped on.  This is in radians.
		 * @param screenLoc The location on the OpenGL surface.
		 */
		void userDidTap(MaplyController mapControl,Point2d loc,Point2d screenLoc);
	}

	/**
	 * Set the gesture delegate to get callbacks when the user taps somewhere.
	 */
	public GestureDelegate gestureDelegate = null;
	
	// When adding features we can run on the current thread or delay the work till layter
	public enum ThreadMode {ThreadCurrent,ThreadAny};
	
	// Represents an ID that doesn't have data associated with it
	public static long EmptyIdentity = 0;
	
	/**
	 * This is how often we'll kick off a render when the frame sync comes in.
	 * We get a notification when the render for a given frame starts, this is
	 * usually 60 times a second.  This tells us how many to skip to achieve
	 * our desired frame rate.  2 means 30hz.  3 means 20hz and so forth.
	 */
	public int frameInterval = 2;
	
	// Set when we're not in the process of shutting down
	boolean running = false;
	
	// Implements the GL renderer protocol
	RendererWrapper renderWrapper;
	
	// Coordinate system to display conversion
	CoordSystemDisplayAdapter coordAdapter;
	
	// Scene stores the objects
	MapScene mapScene;
	
	// MapView defines how we're looking at the data
	MapView mapView;

	// Managers are thread safe objects for handling adding and removing types of data
	VectorManager vecManager;
	MarkerManager markerManager;
	LabelManager labelManager;
	SelectionManager selectionManager;
	LayoutManager layoutManager;
	LayoutLayer layoutLayer = null;
	
	// Manage bitmaps and their conversion to textures
	TextureManager texManager = new TextureManager();
	
	// Layer thread we use for data manipulation
	LayerThread layerThread = null;
	
	// Gesture handler
	GestureHandler gestureHandler = null;
	
	// Bounding box we're allowed to move within
	Point2d viewBounds[] = null;
	
	/**
	 * Returns the layer thread we used for processing requests.
	 */
	public LayerThread getLayerThread() { return layerThread; }
	
	/**
	 * Construct the maply controller with an Activity.  We need access to a few
	 * of the usual Android resources.
	 * <p>
	 * On construction the Controller will create the scene, the view, kick off
	 * the OpenGL ES surface, and construct a layer thread for handling data
	 * requests.
	 * <p>
	 * The controller also sets up some default gestures and handles those
	 * callbacks.
	 * <p>
	 * @param mainActivity Your main activity that we'll attach ourselves to.
	 */
	public MaplyController(Activity mainActivity) 
	{		
//		System.loadLibrary("Maply");
		activity = mainActivity;
		
		// Need a coordinate system to display conversion
		// For now this just sets up spherical mercator
		coordAdapter = new CoordSystemDisplayAdapter(new SphericalMercatorCoordSystem());

		// Create the scene and map view 
		mapScene = new MapScene(coordAdapter);
		mapView = new MapView(coordAdapter);		
		
		// Fire up the managers.  Can't do anything without these.
		vecManager = new VectorManager(mapScene);
		markerManager = new MarkerManager(mapScene);
		labelManager = new LabelManager(mapScene);
		layoutManager = new LayoutManager(mapScene);
		selectionManager = new SelectionManager(mapScene);

		// Now for the object that kicks off the rendering
		renderWrapper = new RendererWrapper(this);
		renderWrapper.mapScene = mapScene;
		renderWrapper.mapView = mapView;
		
		// Set up the bounds
		Point3d ll = new Point3d(),ur = new Point3d();
		coordAdapter.getBounds(ll,ur);
		// Allow E/W wraping
		ll.setValue(Float.MAX_VALUE, ll.getY(), ll.getZ());
		ur.setValue(-Float.MAX_VALUE, ur.getY(), ur.getZ());
		setViewExtents(new Point2d(ll.getX(),ll.getY()),new Point2d(ur.getX(),ur.getY()));

		// Create the layer thread
        layerThread = new LayerThread("Maply Layer Thread",mapView,mapScene);
		
        ActivityManager activityManager = (ActivityManager) mainActivity.getSystemService(Context.ACTIVITY_SERVICE);
        ConfigurationInfo configurationInfo = activityManager.getDeviceConfigurationInfo();
        
        final boolean supportsEs2 = configurationInfo.reqGlEsVersion >= 0x20000 || isProbablyEmulator();
        if (supportsEs2)
        {
        	glSurfaceView = new GLSurfaceView(mainActivity);
        	glSurfaceView.setOnTouchListener(this);
        	gestureHandler = new GestureHandler(this,glSurfaceView);
        	
        	if (isProbablyEmulator())
        	{
        		glSurfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        	}
        	
        	glSurfaceView.setEGLContextClientVersion(2);
        	glSurfaceView.setRenderer(renderWrapper);        	        

        	// Attach to the Choreographer on the main thread
        	// Note: I'd rather do this on the render thread, but there's no Looper
//        	Choreographer.getInstance().postFrameCallbackDelayed(this, 15);
        } else {
        	Toast.makeText(mainActivity,  "This device does not support OpenGL ES 2.0.", Toast.LENGTH_LONG).show();
        	return;
        }   
        
		running = true;
	}
	
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
	
	/**
	 * Return the main content view used to represent the Maply Control.
	 */
	public View getContentView()
	{
		return glSurfaceView;
	}
	
	/**
	 * Call shutdown when you're done with the MaplyController.  It will shut down the layer
	 * thread(s) and all the associated logic.
	 */
	public void shutdown()
	{
		running = false;
//		Choreographer.getInstance().removeFrameCallback(this);
		layerThread.shutdown();
		
		glSurfaceView = null;
		renderWrapper = null;
		coordAdapter = null;
		mapScene = null;
		mapView = null;
		vecManager = null;
		markerManager = null;
		texManager = null;
		layerThread = null;
	}
	
	// Called by the render wrapper when the surface is created.
	// Can't start doing anything until that happens
	void surfaceCreated(RendererWrapper wrap)
	{
        // Kick off the layer thread for background operations
		layerThread.setRenderer(renderWrapper.maplyRender);

		// Note: Debugging output
		renderWrapper.maplyRender.setPerfInterval(perfInterval);
		
		// Kick off the layout layer
		layoutLayer = new LayoutLayer(this,layoutManager);
		layerThread.addLayer(layoutLayer);

		// Set up a periodic update for the renderer
//    	glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
	}

//	long lastRender = 0;
//	// Called by the Choreographer to render a frame
//	@Override
//	public void doFrame(long frameTimeNanos) 
//	{		
//		// Need to do this every frame
//    	Choreographer.getInstance().postFrameCallbackDelayed(this, 15);
//
//    	// Compare how long since the last render
//    	long diff = frameTimeNanos - lastRender;
//		if (diff >= 16666700*(frameInterval-0.25))
//		{
//			glSurfaceView.queueEvent(new Runnable()
//			{
//				public void run()
//				{
//					glSurfaceView.requestRender();
//				}
//			});
//			glSurfaceView.requestRender();
//			lastRender = frameTimeNanos;
//		}
//	}
			
	public void dispose()
	{
		running = false;
		vecManager.dispose();
		vecManager = null;
		
		mapScene.dispose();
		mapScene = null;
		mapView.dispose();
		mapView = null;
		
		renderWrapper = null;
	}

	/**
	 * Set the viewport the user is allowed to move within.  These are lat/lon coordinates
	 * in radians.
	 * @param ll Lower left corner.
	 * @param ur Upper right corner.
	 */
	public void setViewExtents(Point2d ll,Point2d ur)
	{
		CoordSystemDisplayAdapter coordAdapter = mapView.getCoordAdapter();
		CoordSystem coordSys = coordAdapter.getCoordSystem();
		
		viewBounds = new Point2d[4];
		viewBounds[0] = coordAdapter.localToDisplay(coordSys.geographicToLocal(new Point3d(ll.getX(),ll.getY(),0.0))).toPoint2d();
		viewBounds[1] = coordAdapter.localToDisplay(coordSys.geographicToLocal(new Point3d(ur.getX(),ll.getY(),0.0))).toPoint2d();
		viewBounds[2] = coordAdapter.localToDisplay(coordSys.geographicToLocal(new Point3d(ur.getX(),ur.getY(),0.0))).toPoint2d();
		viewBounds[3] = coordAdapter.localToDisplay(coordSys.geographicToLocal(new Point3d(ll.getX(),ur.getY(),0.0))).toPoint2d();
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
	
	// Pass the touches on to the gesture handler
	@Override
	public boolean onTouch(View view, MotionEvent e) {
		return gestureHandler.onTouch(view, e);
	}
	
	int perfInterval = 0;
	/**
	 * Report performance stats in the console ever few frames.
	 * Setting this to zero turns it off.
	 * @param perfInterval
	 */
	public void setPerfInterval(int inPerfInterval)
	{
		perfInterval = inPerfInterval;
		if (renderWrapper != null && renderWrapper.maplyRender != null)
			renderWrapper.maplyRender.setPerfInterval(perfInterval);
	}

	/**
	 * Add a single VectorObject.  See addVectors() for details.
	 */
	public ComponentObject addVector(final VectorObject vec,final VectorInfo vecInfo,ThreadMode mode)
	{
		ArrayList<VectorObject> vecObjs = new ArrayList<VectorObject>();
		vecObjs.add(vec);
		return addVectors(vecObjs,vecInfo,mode);
	}
	
	/**
	 * Add a task according to the thread mode.  If it's ThreadAny, we'll put it on the layer thread.
	 * If it's ThreadCurrent, we'll do it immediately.
	 * 
	 * @param run Runnable to execute.
	 * @param mode Where to execute it.
	 */
	private void addTask(Runnable run,ThreadMode mode)
	{
		if (!running)
			return;
		
		if (Looper.myLooper() == layerThread.getLooper() || (mode == ThreadMode.ThreadCurrent))
			run.run();
		else
			layerThread.addTask(run,true);
	}

	/**
	 * Add vectors to the MaplyController to display.  Vectors are linear or areal
	 * features with line width, filled style, color and so forth defined by the
	 * VectorInfo class.
	 * 
	 * @param vecs A list of VectorObject's created by the user or read in from various sources.
	 * @param vecInfo A description of how the vectors should look.
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
	 * @return The ComponentObject representing the vectors.  This is necessary for modifying
	 * or deleting the vectors once created.
	 */
	public ComponentObject addVectors(final List<VectorObject> vecs,final VectorInfo vecInfo,ThreadMode mode)
	{
		if (!running)
			return null;

		final ComponentObject compObj = new ComponentObject();
		
		// Do the actual work on the layer thread
		Runnable run =
		new Runnable()
		{		
			@Override
			public void run()
			{
				// Vectors are simple enough to just add
				ChangeSet changes = new ChangeSet();
				long vecId = vecManager.addVectors(vecs,vecInfo,changes);
				mapScene.addChanges(changes);
	
				// Track the vector ID for later use
				if (vecId != EmptyIdentity)
					compObj.addVectorID(vecId);
			}
		};
		
		addTask(run,mode);
				
		return compObj;
	}

	/**
	 * Add a single screen marker.  See addScreenMarkers() for details.
	 */
	public ComponentObject addScreenMarker(final ScreenMarker marker,final MarkerInfo markerInfo,ThreadMode mode)
	{
		if (!running)
			return null;

		ArrayList<ScreenMarker> markers = new ArrayList<ScreenMarker>();
		markers.add(marker);
		return addScreenMarkers(markers,markerInfo,mode);
	}

	/**
	 * Add screen markers to the visual display.  Screen markers are 2D markers that sit
	 * on top of the screen display, rather than interacting with the geometry.  Their
	 * visual look is defined by the MarkerInfo class.
	 * 
	 * @param markers The markers to add to the display
	 * @param markerInfo How the markers should look.
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
	 * @return This represents the screen markers for later modification or deletion.
	 */
	public ComponentObject addScreenMarkers(final List<ScreenMarker> markers,final MarkerInfo markerInfo,ThreadMode mode)
	{		
		if (!running)
			return null;

		final ComponentObject compObj = new ComponentObject();

		// Do the actual work on the layer thread
		Runnable run =
		new Runnable()
		{		
			@Override
			public void run()
			{
				ChangeSet changes = new ChangeSet();
		
				// Convert to the internal representation of the engine
				ArrayList<InternalMarker> intMarkers = new ArrayList<InternalMarker>();
				for (ScreenMarker marker : markers)
				{
					InternalMarker intMarker = new InternalMarker(marker,markerInfo);
					// Map the bitmap to a texture ID
					long texID = EmptyIdentity;
					if (marker.image != null)
						texID = texManager.addTexture(marker.image, changes);
					if (texID != EmptyIdentity)
						intMarker.addTexID(texID);
					
					intMarkers.add(intMarker);
					
					// Keep track of this one for selection
					if (marker.selectable)
					{
						addSelectableObject(marker.ident,marker,compObj);
					}
				}

				// Add the markers and flush the changes
				long markerId = markerManager.addMarkers(intMarkers, markerInfo, changes);
				mapScene.addChanges(changes);
				
				if (markerId != EmptyIdentity)
				{
					compObj.addMarkerID(markerId);
				}
			}
		};
		
		addTask(run,mode);

		return compObj;
	}
	
	Map<Long, Object> selectionMap = new HashMap<Long, Object>();
	
	// Add selectable objects to the list
	private void addSelectableObject(long selectID,Object selObj,ComponentObject compObj)
	{
		synchronized(selectionMap)
		{
			compObj.addSelectID(selectID);
			selectionMap.put(selectID,selObj);
		}
	}
	
	// Remove selectable objects
	private void removeSelectableObjects(ComponentObject compObj)
	{
		if (compObj.selectIDs != null)
		{
			synchronized(selectionMap)
			{
				for (long selectID : compObj.selectIDs)
					selectionMap.remove(selectID);
			}
		}
	}

	/**
	 * Add a single screen label.  See addScreenLabels() for details.
	 */
	public ComponentObject addScreenLabel(ScreenLabel label,final LabelInfo labelInfo,ThreadMode mode)
	{
		if (!running)
			return null;
		
		ArrayList<ScreenLabel> labels = new ArrayList<ScreenLabel>();
		labels.add(label);
		return addScreenLabels(labels,labelInfo,mode);
	}

	/**
	 * Add screen labels to the display.  Screen labels are 2D labels that float above the 3D geometry
	 * and stay fixed in size no matter how the user zoom in or out.  Their visual appearance is controlled
	 * by the LabelInfo class.
	 * 
	 * @param labels Labels to add to the display.
	 * @param labelInfo The visual appearance of the labels.
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
	 * @return This represents the labels for modification or deletion.
	 */
	public ComponentObject addScreenLabels(final List<ScreenLabel> labels,final LabelInfo labelInfo,ThreadMode mode)
	{
		if (!running)
			return null;

		final ComponentObject compObj = new ComponentObject();

		// Do the actual work on the layer thread
		Runnable run =
		new Runnable()
		{		
			@Override
			public void run()
			{
				ChangeSet changes = new ChangeSet();
				
				// Convert to the internal representation for the engine
				ArrayList<InternalLabel> intLabels = new ArrayList<InternalLabel>();
				for (ScreenLabel label : labels)
				{
					InternalLabel intLabel = new InternalLabel(label,labelInfo);
					intLabels.add(intLabel);
				}

				long labelId = labelManager.addLabels(intLabels, labelInfo, changes);
				if (labelId != EmptyIdentity)
					compObj.addLabelID(labelId);
		
				// Flush the text changes
				mapScene.addChanges(changes);
			}
		};
		
		addTask(run,mode);
		
		return compObj;
	}

	/**
	 * Disable the given objects. These were the objects returned by the various
	 * add calls.  Once called, the objects will be invisible, but can be made
	 * visible once again with enableObjects()
	 * 
	 * @param compObjs Objects to disable in the display.
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
	 */
	public void disableObjects(final List<ComponentObject> compObjs,ThreadMode mode)
	{
		if (!running)
			return;

		if (compObjs == null || compObjs.size() == 0)
			return;
		
		final MaplyController control = this;
		Runnable run = new Runnable()
		{		
			@Override
			public void run()
			{
				ChangeSet changes = new ChangeSet();
				for (ComponentObject compObj : compObjs)
					compObj.enable(control, false, changes);
				mapScene.addChanges(changes);
			}
		};
		
		addTask(run,mode);
	}

	/**
	 * Enable the display for the given objects.  These objects were returned
	 * by the various add calls.  To disable the display, call disableObjects().
	 *
	 * @param compObjs Objects to disable.
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
	 */
	public void enableObjects(final List<ComponentObject> compObjs,ThreadMode mode)
	{
		if (!running)
			return;

		if (compObjs == null || compObjs.size() == 0)
			return;
		
		final MaplyController control = this;
		Runnable run = 
		new Runnable()
		{		
			@Override
			public void run()
			{
				ChangeSet changes = new ChangeSet();
				for (ComponentObject compObj : compObjs)
					compObj.enable(control, true, changes);
				mapScene.addChanges(changes);
			}
		};
		
		addTask(run,mode);
	}

	/**
	 * Remove a single objects from the display.  See removeObjects() for details.
	 */
	public void removeObject(final ComponentObject compObj,ThreadMode mode)
	{
		if (!running)
			return;

		if (compObj == null)
			return;

		ArrayList<ComponentObject> compObjs = new ArrayList<ComponentObject>();
		compObjs.add(compObj);
		removeObjects(compObjs,mode);
	}

	/**
	 * Remove the given component objects from the display.  This will permanently remove them
	 * from Maply.  The component objects were returned from the various add calls.
	 * 
	 * @param compObjs Component Objects to remove.
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
	 */
	public void removeObjects(final List<ComponentObject> compObjs,ThreadMode mode)
	{
		if (!running)
			return;

		if (compObjs == null || compObjs.size() == 0)
			return;
		
		final MaplyController control = this;
		Runnable run =
		new Runnable()
		{		
			@Override
			public void run()
			{
				ChangeSet changes = new ChangeSet();
				for (ComponentObject compObj : compObjs)
				{
					compObj.clear(control, changes);
					removeSelectableObjects(compObj);
				}
				mapScene.addChanges(changes);
			}
		};
		
		addTask(run,mode);
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
		mapView.setAnimationDelegate(new AnimateTranslate(mapView, renderWrapper.maplyRender, new Point3d(geoCoord.getX(),geoCoord.getY(),z), (float) howLong, viewBounds));		
	}
	
    private boolean isProbablyEmulator() {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1
                && (Build.FINGERPRINT.startsWith("generic")
                        || Build.FINGERPRINT.startsWith("unknown")
                        || Build.MODEL.contains("google_sdk")
                        || Build.MODEL.contains("Emulator")
                        || Build.MODEL.contains("Android SDK built for x86"));
    }
}
