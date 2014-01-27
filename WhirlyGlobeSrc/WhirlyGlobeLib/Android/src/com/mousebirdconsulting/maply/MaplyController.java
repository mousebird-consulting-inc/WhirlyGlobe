package com.mousebirdconsulting.maply;

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
import android.view.*;
import android.widget.Toast;

public class MaplyController implements View.OnTouchListener
{	
	private GLSurfaceView glSurfaceView;
	
	// Represents an ID that doesn't have data associated with it
	static long EmptyIdentity = 0;
	
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
	
	public MaplyController(Activity mainActivity) 
	{
//		System.loadLibrary("Maply");
		
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
	public void surfaceCreated(RendererWrapper wrap)
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
	private class GestureListener implements GestureDetector.OnGestureListener
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
	 * Add zero or more vectors to the MaplyController.
	 * @param vecs The list of vectors to display.
	 */
	public ComponentObject addVectors(final List<VectorObject> vecs,final VectorInfo vecInfo)
	{
		final ComponentObject compObj = new ComponentObject();

		// Do the actual work on the layer thread
		layerThread.addTask(
			new Runnable()
			{		
				@Override
				public void run()
				{
					// Vectors are simple enough to just add
					ChangeSet changes = new ChangeSet();
					long vecId = vecManager.addVectors(vecs.toArray(new VectorObject [vecs.size()]),vecInfo,changes);
					mapScene.addChanges(changes);

					// Track the vector ID for later use
					if (vecId != EmptyIdentity)
						compObj.addVectorID(vecId);
				}
			}
		);
		
		
		return compObj;
	}
	
	/**
	 * Add zero or more markers to the MaplyController
	 * @param markers The list of markers to display.
	 * @param markerInfo
	 */
	public ComponentObject addScreenMarkers(final List<ScreenMarker> markers,final MarkerInfo markerInfo)
	{		
		final ComponentObject compObj = new ComponentObject();

		// Do the actual work on the layer thread
		layerThread.addTask(
			new Runnable()
			{		
				@Override
				public void run()
				{
					ChangeSet changes = new ChangeSet();
			
					// Convert to the internal representation of the engine
					InternalMarker intMarkers[] = new InternalMarker[markers.size()];
					int which = 0;
					for (ScreenMarker marker : markers)
					{
						InternalMarker intMarker = new InternalMarker(marker,markerInfo);
						// Map the bitmap to a texture ID
						long texID = EmptyIdentity;
						if (marker.image != null)
							texID = texManager.addTexture(marker.image, changes);
						if (texID != EmptyIdentity)
							intMarker.addTexID(texID);
						
						intMarkers[which] = intMarker;
						which++;
					}
							
					// Add the markers and flush the changes
					long markerId = markerManager.addMarkers(intMarkers, markerInfo, changes);
					mapScene.addChanges(changes);
					
					if (markerId != EmptyIdentity)
					{
						compObj.addMarkerID(markerId);
					}
				}
			}
		);

		return compObj;
	}
	
	/**
	 * Add zero or more screen labels to the MaplyController.
	 * @param labels
	 * @param labelInfo
	 * @return
	 */
	public ComponentObject addScreenLabels(final List<ScreenLabel> labels,final LabelInfo labelInfo)
	{
		final ComponentObject compObj = new ComponentObject();

		// Do the actual work on the layer thread
		layerThread.addTask(
			new Runnable()
			{		
				@Override
				public void run()
				{
					ChangeSet changes = new ChangeSet();
					
					// Note: Porting
					// We'll just turn these into markers for now
					InternalMarker intMarkers[] = new InternalMarker[labels.size()];		
					int which = 0;
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
							intMarkers[which] = intMarker;
						}
						which++;
					}
			
					MarkerInfo markerInfo = new MarkerInfo();
					markerInfo.setFade(labelInfo.fade);
					long markerId = markerManager.addMarkers(intMarkers, markerInfo, changes);
					if (markerId != EmptyIdentity)
						compObj.addMarkerID(markerId);
			
					// Flush the text changes
					mapScene.addChanges(changes);
				}
			}
		);
		
		return compObj;
	}

	/**
	 * Remove all data associated with the given component object.
	 * @param compObj Component object to remove.
	 */
	public void removeObject(ComponentObject compObj)
	{
		ChangeSet changes = new ChangeSet();
		compObj.clear(this, changes);
		mapScene.addChanges(changes);
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
