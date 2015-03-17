package com.mousebird.maply;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.widget.Toast;

/**
 * The base controller is a base class for both Maply and WhirlyGlobe controllers.
 * <p>
 * Most of the functionality is shared between the 2D and 3D maps and so it is
 * implemented here.
 * 
 * @author sjg
 *
 */
public class MaplyBaseController 
{
	public GLSurfaceView glSurfaceView;
	Activity activity = null;
	
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
	protected RendererWrapper renderWrapper;
	
	// Coordinate system to display conversion
	protected CoordSystemDisplayAdapter coordAdapter;
	
	// Scene stores the objects
	protected Scene scene = null;
	
	// MapView defines how we're looking at the data
	protected com.mousebird.maply.View view = null;

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
	public MaplyBaseController(Activity mainActivity) 
	{		
		System.loadLibrary("Maply");
		activity = mainActivity;

		// Need a coordinate system to display conversion
		// For now this just sets up spherical mercator
		coordAdapter = new CoordSystemDisplayAdapter(new SphericalMercatorCoordSystem());
	}
	
	protected void Init()
	{		
		// Fire up the managers.  Can't do anything without these.
		vecManager = new VectorManager(scene);
		markerManager = new MarkerManager(scene);
		labelManager = new LabelManager(scene);
		layoutManager = new LayoutManager(scene);
		selectionManager = new SelectionManager(scene);

		// Now for the object that kicks off the rendering
		renderWrapper = new RendererWrapper(this);
		renderWrapper.scene = scene;
		renderWrapper.view = view;
		
		// Set up the bounds
		Point3d ll = new Point3d(),ur = new Point3d();
		coordAdapter.getBounds(ll,ur);
		// Allow E/W wraping
		ll.setValue(Float.MAX_VALUE, ll.getY(), ll.getZ());
		ur.setValue(-Float.MAX_VALUE, ur.getY(), ur.getZ());
		setViewExtents(new Point2d(ll.getX(),ll.getY()),new Point2d(ur.getX(),ur.getY()));

		// Create the layer thread
        layerThread = new LayerThread("Maply Layer Thread",view,scene);
		
        ActivityManager activityManager = (ActivityManager) activity.getSystemService(Context.ACTIVITY_SERVICE);
        ConfigurationInfo configurationInfo = activityManager.getDeviceConfigurationInfo();
        
        final boolean supportsEs2 = configurationInfo.reqGlEsVersion >= 0x20000 || isProbablyEmulator();
        if (supportsEs2)
        {
        	glSurfaceView = new GLSurfaceView(activity);
        	
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
        	Toast.makeText(activity,  "This device does not support OpenGL ES 2.0.", Toast.LENGTH_LONG).show();
        	return;
        }   
        
		running = true;		
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
		scene = null;
		view = null;
		vecManager = null;
		markerManager = null;
		texManager = null;
		layerThread = null;
	}
	
	ArrayList<Runnable> surfaceTasks = new ArrayList<Runnable>();
	
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
		
		// Run any outstanding runnables
		for (Runnable run: surfaceTasks)
		{
			Handler handler = new Handler(activity.getMainLooper());
			handler.post(run);
		}
		surfaceTasks = null;

		// Set up a periodic update for the renderer
//    	glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
	}
	
	/**
	 * It takes a little time to set up the OpenGL ES drawable.  Add a runnable
	 * to be run after the surface is created.  If it's already been created we
	 * just run it here.
	 * <p>
	 * Only call this on the main thread.
	 */
	public void onSurfaceCreatedTask(Runnable run)
	{
		if (surfaceTasks != null)
			surfaceTasks.add(run);
		else
			run.run();
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
		CoordSystemDisplayAdapter coordAdapter = view.getCoordAdapter();
		CoordSystem coordSys = coordAdapter.getCoordSystem();
		
		viewBounds = new Point2d[4];
		viewBounds[0] = coordAdapter.localToDisplay(coordSys.geographicToLocal(new Point3d(ll.getX(),ll.getY(),0.0))).toPoint2d();
		viewBounds[1] = coordAdapter.localToDisplay(coordSys.geographicToLocal(new Point3d(ur.getX(),ll.getY(),0.0))).toPoint2d();
		viewBounds[2] = coordAdapter.localToDisplay(coordSys.geographicToLocal(new Point3d(ur.getX(),ur.getY(),0.0))).toPoint2d();
		viewBounds[3] = coordAdapter.localToDisplay(coordSys.geographicToLocal(new Point3d(ll.getX(),ur.getY(),0.0))).toPoint2d();
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
	 * Add a single layer.  This will start processing its data on the layer thread at some
	 * point in the near future.
	 */
	public void addLayer(Layer layer)
	{
		layerThread.addLayer(layer);
	}
	
	/**
	 * Remove a single layer.  The layer will stop receiving data and be shut down shortly
	 * after you call this.
	 */
	public void removeLayer(Layer layer)
	{
		layerThread.removeLayer(layer);
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
				scene.addChanges(changes);
	
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
				scene.addChanges(changes);
				
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
				scene.addChanges(changes);
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
		
		final MaplyBaseController control = this;
		Runnable run = new Runnable()
		{		
			@Override
			public void run()
			{
				ChangeSet changes = new ChangeSet();
				for (ComponentObject compObj : compObjs)
					compObj.enable(control, false, changes);
				scene.addChanges(changes);
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
		
		final MaplyBaseController control = this;
		Runnable run = 
		new Runnable()
		{		
			@Override
			public void run()
			{
				ChangeSet changes = new ChangeSet();
				for (ComponentObject compObj : compObjs)
					compObj.enable(control, true, changes);
				scene.addChanges(changes);
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
		
		final MaplyBaseController control = this;
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
				scene.addChanges(changes);
			}
		};
		
		addTask(run,mode);
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
