package com.mousebird.maply;

import java.util.ArrayList;
import java.util.List;

import android.app.*;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Looper;
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
	
	// Represents an ID that doesn't have data associated with it
	public static long EmptyIdentity = 0;
	
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
	
	// Manage bitmaps and their conversion to textures
	TextureManager texManager = new TextureManager();
	
	// Layer thread we use for data manipulation
	LayerThread layerThread = null;
	
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
		coordAdapter = new CoordSystemDisplayAdapter();

		// Create the scene and map view 
		mapScene = new MapScene(coordAdapter);
		mapView = new MapView(coordAdapter);		
		
		// Fire up the managers.  Can't do anything without these.
		vecManager = new VectorManager(mapScene);
		markerManager = new MarkerManager(mapScene);

		// Now for the object that kicks off the rendering
		renderWrapper = new RendererWrapper(this);
		renderWrapper.mapScene = mapScene;
		renderWrapper.mapView = mapView;

		// Create the layer thread
        layerThread = new LayerThread("Maply Layer Thread",mapView,mapScene);
		
        ActivityManager activityManager = (ActivityManager) mainActivity.getSystemService(Context.ACTIVITY_SERVICE);
        ConfigurationInfo configurationInfo = activityManager.getDeviceConfigurationInfo();
        
        final boolean supportsEs2 = configurationInfo.reqGlEsVersion > 0x20000 || isProbablyEmulator();
        if (supportsEs2)
        {
        	glSurfaceView = new GLSurfaceView(mainActivity);
        	glSurfaceView.setOnTouchListener(this);
        	setupGestures(glSurfaceView);
        	
        	if (isProbablyEmulator())
        	{
        		glSurfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        	}
        	
        	glSurfaceView.setEGLContextClientVersion(2);
        	glSurfaceView.setRenderer(renderWrapper);
        	mainActivity.setContentView(glSurfaceView);        
        	        
        } else {
        	Toast.makeText(mainActivity,  "This device does not support OpenGL ES 2.0.", Toast.LENGTH_LONG).show();
        	return;
        }        
	}
	
	// Called by the render wrapper when the surface is created.
	// Can't start doing anything until that happens
	void surfaceCreated(RendererWrapper wrap)
	{
        // Kick off the layer thread for background operations
		layerThread.setRenderer(renderWrapper.maplyRender);
	}
	
	// Tie in the gestures we want
	ScaleGestureDetector sgd;
	GestureDetector gd;
	private void setupGestures(View view)
	{
		ScaleListener sl = new ScaleListener(this);
		sgd = new ScaleGestureDetector(view.getContext(),sl);
		GestureListener gl = new GestureListener(this);
		gd = new GestureDetector(view.getContext(),gl);
		sl.gl = gl;
	}
	
	// Listening for a pinch scale event
	private class ScaleListener extends ScaleGestureDetector.SimpleOnScaleGestureListener
	{
		MaplyController maplyControl;
		double startZ;
		float startDist;
		GestureListener gl = null;
		
		ScaleListener(MaplyController inMaplyControl)
		{
			maplyControl = inMaplyControl;
		}
		
		@Override
		public boolean onScaleBegin(ScaleGestureDetector detector)
		{
			startZ = maplyControl.mapView.getLoc().getZ();
			startDist = detector.getCurrentSpan();
			// Cancel the panning
			if (gl != null)
				gl.valid = false;
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
				maplyControl.mapView.setLoc(pos.getX(),pos.getY(),startZ*scale);
//				Log.d("Maply","Zoom: " + maplyControl.mapView.getLoc().getZ() + " Scale: " + scale);
				return true;
			}
			
			return false;
		}
		
		@Override
		public void onScaleEnd(ScaleGestureDetector detector)
		{
		}
	}
	
	// Listening for the rest of the interesting events
	private class GestureListener implements GestureDetector.OnGestureListener,
				GestureDetector.OnDoubleTapListener
	{
		MaplyController maplyControl;
		public boolean valid = false;
		
		GestureListener(MaplyController inMaplyControl)
		{
			maplyControl = inMaplyControl;
		}
		
		Point2d startScreenPos;
		Point3d startPos;
		Point3d startOnPlane;
		Matrix4d startTransform;
		@Override
		public boolean onDown(MotionEvent e) 
		{
			// Starting state for pan
			startScreenPos = new Point2d(e.getX(),e.getY());
			startTransform = maplyControl.mapView.calcModelViewMatrix();
			startPos = maplyControl.mapView.getLoc();
			startOnPlane = maplyControl.mapView.pointOnPlaneFromScreen(startScreenPos, startTransform, maplyControl.renderWrapper.maplyRender.frameSize, false);
			valid = true;

			return false;
		}

		@Override
		public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX,
				float distanceY) 
		{
			if (!valid)
				return false;
			
			Point2d newScreenPos = new Point2d(e2.getX(),e2.getY());
			
			// New state for pan
			Point3d hit = maplyControl.mapView.pointOnPlaneFromScreen(newScreenPos, startTransform, maplyControl.renderWrapper.maplyRender.frameSize, false);
			if (hit != null)
			{
				Point3d newPos = new Point3d(startOnPlane.getX()-hit.getX()+startPos.getX(),
						startOnPlane.getY()-hit.getY()+startPos.getY(),
						maplyControl.mapView.getLoc().getZ());
				maplyControl.mapView.setLoc(newPos);
//				Log.d("Maply","New Pos = (" + newPos.getX() + "," + newPos.getY() + "," + newPos.getZ() + ")");
			}
			
			return false;
		}
		
		@Override
		public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
				float velocityY) 
		{
//			Log.d("Maply","Fling");
			return false;
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
			return true;
		}

		@Override
		public boolean onSingleTapConfirmed(MotionEvent e) {
			// TODO Auto-generated method stub
			return false;
		}

		// Zoom in on double tap
		@Override
		public boolean onDoubleTap(MotionEvent e) 
		{
			Point3d loc = mapView.getLoc();
			loc.setValue(loc.getX(), loc.getY(), loc.getZ()/2.0);
			mapView.setLoc(loc);
			return true;
		}		
	}
	
	// Where we receive events from the gl view
	@Override
	public boolean onTouch(View v, MotionEvent event) 
	{
		// Try for a pinch or another gesture
		return gd.onTouchEvent(event) ||
				sgd.onTouchEvent(event);
	}      
			
	public void dispose()
	{
		vecManager.dispose();
		vecManager = null;
		
		mapScene.dispose();
		mapScene = null;
		mapView.dispose();
		mapView = null;
		
		renderWrapper = null;
	}

	/**
	 * Add a single VectorObject.  See addVectors() for details.
	 */
	public ComponentObject addVector(final VectorObject vec,final VectorInfo vecInfo)
	{
		ArrayList<VectorObject> vecObjs = new ArrayList<VectorObject>();
		vecObjs.add(vec);
		return addVectors(vecObjs,vecInfo);
	}

	/**
	 * Add vectors to the MaplyController to display.  Vectors are linear or areal
	 * features with line width, filled style, color and so forth defined by the
	 * VectorInfo class.
	 * 
	 * @param vecs A list of VectorObject's created by the user or read in from various sources.
	 * @param vecInfo A description of how the vectors should look.
	 * @return The ComponentObject representing the vectors.  This is necessary for modifying
	 * or deleting the vectors once created.
	 */
	public ComponentObject addVectors(final List<VectorObject> vecs,final VectorInfo vecInfo)
	{
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
		if (Looper.myLooper() == layerThread.getLooper())
			run.run();
		else
			layerThread.addTask(run);
		
		return compObj;
	}

	/**
	 * Add a single screen marker.  See addScreenMarkers() for details.
	 */
	public ComponentObject addScreenMarker(final ScreenMarker marker,final MarkerInfo markerInfo)
	{
		ArrayList<ScreenMarker> markers = new ArrayList<ScreenMarker>();
		markers.add(marker);
		return addScreenMarkers(markers,markerInfo);
	}

	/**
	 * Add screen markers to the visual display.  Screen markers are 2D markers that sit
	 * on top of the screen display, rather than interacting with the geometry.  Their
	 * visual look is defined by the MarkerInfo class.
	 * 
	 * @param markers The markers to add to the display
	 * @param markerInfo How the markers should look.
	 * @return This represents the screen markers for later modification or deletion.
	 */
	public ComponentObject addScreenMarkers(final List<ScreenMarker> markers,final MarkerInfo markerInfo)
	{		
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
		if (Looper.myLooper() == layerThread.getLooper())
			run.run();
		else
			layerThread.addTask(run);

		return compObj;
	}

	/**
	 * Add a single screen label.  See addScreenLabels() for details.
	 */
	public ComponentObject addScreenLabel(ScreenLabel label,final LabelInfo labelInfo)
	{
		ArrayList<ScreenLabel> labels = new ArrayList<ScreenLabel>();
		labels.add(label);
		return addScreenLabels(labels,labelInfo);
	}

	/**
	 * Add screen labels to the display.  Screen labels are 2D labels that float above the 3D geometry
	 * and stay fixed in size no matter how the user zoom in or out.  Their visual appearance is controlled
	 * by the LabelInfo class.
	 * 
	 * @param labels Labels to add to the display.
	 * @param labelInfo The visual appearance of the labels.
	 * @return This represents the labels for modification or deletion.
	 */
	public ComponentObject addScreenLabels(final List<ScreenLabel> labels,final LabelInfo labelInfo)
	{
		final ComponentObject compObj = new ComponentObject();

		// Do the actual work on the layer thread
		Runnable run =
		new Runnable()
		{		
			@Override
			public void run()
			{
				ChangeSet changes = new ChangeSet();
				
				// Note: Porting
				// We'll just turn these into markers for now
				ArrayList<InternalMarker> intMarkers = new ArrayList<InternalMarker>();		
				for (ScreenLabel label: labels)
				{
					// Render the text into a bitmap
					if (label.text != null && label.text.length() > 0)
					{
						Paint p = new Paint();
						p.setTextSize(labelInfo.fontSize);
						p.setColor(Color.WHITE);
						Rect bounds = new Rect();
						p.getTextBounds(label.text, 0, label.text.length(), bounds);
						int textLen = bounds.right;
						int textHeight = -bounds.top;
		
						// Draw into a bitmap
						Bitmap bitmap = Bitmap.createBitmap(textLen, textHeight, Bitmap.Config.ARGB_8888);
						Canvas c = new Canvas(bitmap);
						c.drawColor(0x00000000);
						c.drawText(label.text, bounds.left, -bounds.top, p);
						
						InternalMarker intMarker = new InternalMarker();
						intMarker.setLoc(label.loc);
						intMarker.setHeight(textHeight);
						intMarker.setWidth(textLen);
						
						NamedBitmap nameBitmap = new NamedBitmap(label.text,bitmap);
						long texID = texManager.addTexture(nameBitmap, changes);
						if (texID != EmptyIdentity)
						{
							intMarker.addTexID(texID);
							compObj.addTexID(texID);
						}
						intMarkers.add(intMarker);
					}
				}
		
				MarkerInfo markerInfo = new MarkerInfo();
				markerInfo.setEnable(labelInfo.enable);
				markerInfo.setFade(labelInfo.fade);
				long markerId = markerManager.addMarkers(intMarkers, markerInfo, changes);
				if (markerId != EmptyIdentity)
					compObj.addMarkerID(markerId);
		
				// Flush the text changes
				mapScene.addChanges(changes);
			}
		};
		if (Looper.myLooper() == layerThread.getLooper())
			run.run();
		else
			layerThread.addTask(run);
		
		return compObj;
	}

	/**
	 * Disable the given objects. These were the objects returned by the various
	 * add calls.  Once called, the objects will be invisible, but can be made
	 * visible once again with enableObjects()
	 * 
	 * @param compObjs Objects to disable in the display.
	 */
	public void disableObjects(final List<ComponentObject> compObjs)
	{
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
		if (Looper.myLooper() == layerThread.getLooper())
			run.run();
		else
			layerThread.addTask(run);
	}

	/**
	 * Enable the display for the given objects.  These objects were returned
	 * by the various add calls.  To disable the display, call disableObjects().
	 *
	 * @param compObjs Objects to disable.
	 */
	public void enableObjects(final List<ComponentObject> compObjs)
	{
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
		if (Looper.myLooper() == layerThread.getLooper())
			run.run();
		else
			layerThread.addTask(run);
	}

	/**
	 * Remove a single objects from the display.  See removeObjects() for details.
	 */
	public void removeObject(final ComponentObject compObj)
	{
		if (compObj == null)
			return;

		ArrayList<ComponentObject> compObjs = new ArrayList<ComponentObject>();
		compObjs.add(compObj);
		removeObjects(compObjs);
	}

	/**
	 * Remove the given component objects from the display.  This will permanently remove them
	 * from Maply.  The component objects were returned from the various add calls.
	 * 
	 * @param compObjs Component Objects to remove.
	 */
	public void removeObjects(final List<ComponentObject> compObjs)
	{
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
					compObj.clear(control, changes);
				mapScene.addChanges(changes);
			}
		};
		if (Looper.myLooper() == layerThread.getLooper())
			run.run();
		else
			layerThread.addTask(run);
	}
	
	/**
	 * Set the current view position.
	 * @param x Horizontal location of the center of the screen in radians (not degrees).
	 * @param y Vertical location of the center of the screen in radians (not degrees).
	 * @param z Height above the map in display units.
	 */
	public void setPosition(double x,double y,double z)
	{
		mapView.setLoc(x, y, z);
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
