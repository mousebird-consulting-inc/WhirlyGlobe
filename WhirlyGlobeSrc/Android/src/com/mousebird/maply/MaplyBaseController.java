package com.mousebird.maply;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import com.squareup.okhttp.OkHttpClient;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLSurface;

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
    OkHttpClient httpClient = new OkHttpClient();

	// When adding features we can run on the current thread or delay the work till layter
	public enum ThreadMode {ThreadCurrent,ThreadAny};
	
	// Represents an ID that doesn't have data associated with it
	public static long EmptyIdentity = 0;
	
	// Draw priority defaults
	public static final int ImageLayerDrawPriorityDefault = 100;
	public static final int FeatureDrawPriorityBase = 20000;
	public static final int MarkerDrawPriorityDefault = 40000;
	public static final int LabelDrawPriorityDefault = 60000;
	public static final int ParticleDrawPriorityDefault = 1000;
	
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

    /**
     * Return the current scene.  Only for sure within the library.
     */
    public Scene getScene()
    {
        return scene;
    }

    /**
     * Return an HTTP Client for use in fetching data, probably tiles.
     */
    OkHttpClient getHttpClient() { return httpClient; }
	
	// MapView defines how we're looking at the data
	protected com.mousebird.maply.View view = null;

	// Managers are thread safe objects for handling adding and removing types of data
	VectorManager vecManager;
	MarkerManager markerManager;
    StickerManager stickerManager;
	LabelManager labelManager;
	SelectionManager selectionManager;
	LayoutManager layoutManager;
	ParticleSystemManager particleSystemManager;
	LayoutLayer layoutLayer = null;
	
	// Manage bitmaps and their conversion to textures
	TextureManager texManager = new TextureManager();
	
	// Layer thread we use for data manipulation
	ArrayList<LayerThread> layerThreads = new ArrayList<LayerThread>();
		
	// Bounding box we're allowed to move within
	Point2d viewBounds[] = null;
	
	/**
	 * Returns the layer thread we used for processing requests.
	 */
	public LayerThread getLayerThread()
	{
		if (layerThreads.size() == 0)
			return null;
		return layerThreads.get(0);
	}
	
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
	}
	
	protected void Init()
	{		
		// Fire up the managers.  Can't do anything without these.
		vecManager = new VectorManager(scene);
		markerManager = new MarkerManager(scene);
        stickerManager = new StickerManager(scene);
		labelManager = new LabelManager(scene);
		layoutManager = new LayoutManager(scene);
		selectionManager = new SelectionManager(scene);
		particleSystemManager = new ParticleSystemManager(scene);

		// Now for the object that kicks off the rendering
		renderWrapper = new RendererWrapper(this);
		renderWrapper.scene = scene;
		renderWrapper.view = view;
		
		// Create the layer thread
        LayerThread layerThread = new LayerThread("Maply Layer Thread",view,scene);
		layerThreads.add(layerThread);
		
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
        } else {
        	Toast.makeText(activity,  "This device does not support OpenGL ES 2.0.", Toast.LENGTH_LONG).show();
        	return;
        }   
        
		running = true;		
	}

	// Build and return a layer thread for use by the developer
	public LayerThread makeLayerThread()
	{
		if (!running)
			return null;

		// Create the layer thread
		LayerThread newLayerThread = new LayerThread("External Maply Layer Thread",view,scene);

		layerThreads.add(newLayerThread);

		// Kick off the layer thread for background operations
		newLayerThread.setRenderer(renderWrapper.maplyRender);
		newLayerThread.viewUpdated(view);

		return newLayerThread;
	}

	/** @brief Convert from a coordinate in the given system to display space.
	 @details This converts from a coordinate (3d) in the given coordinate system to the view controller's display space.  For the globe, display space is based on a radius of 1.0.
	 */

	public Point3d displayCoord (Point3d localCoord, CoordSystem fromSystem){

		Point3d loc3d = CoordSystem.CoordSystemConvert3d(fromSystem, coordAdapter.getCoordSystem(), localCoord);
		Point3d pt = coordAdapter.localToDisplay(loc3d);

		return pt;
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
		renderWrapper.stopRendering();

		running = false;
//		Choreographer.getInstance().removeFrameCallback(this);
		for (LayerThread layerThread : layerThreads)
			layerThread.shutdown();
		metroThread.shutdown();

		// Clean up OpenGL ES resources
		setEGLContext();
		scene.teardownGL();

		// Do a little dance to shut down rendering
		glSurfaceView.onPause();

		glSurfaceView = null;
		renderWrapper = null;
		coordAdapter = null;
		scene = null;
		view = null;
		vecManager = null;
		markerManager = null;
		texManager = null;
		layerThreads = null;
	}
	
	ArrayList<Runnable> surfaceTasks = new ArrayList<Runnable>();
	
	// Metronome thread used to time the renderer
	MetroThread metroThread;

    // Note: Why isn't this in EGL10?
    private static final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
    EGLContext eglContext = null;
    EGLSurface eglSurface = null;

    ArrayList<Runnable> postSurfaceRunnables = new ArrayList<Runnable>();

    /**
     * Add a runnable to be executed after the OpenGL surface is created.
     */
    public void addPostSurfaceRunnable(Runnable run)
    {
        if (layoutLayer != null)
            activity.runOnUiThread(run);
        else
            postSurfaceRunnables.add(run);
    }

    // Called by the render wrapper when the surface is created.
	// Can't start doing anything until that happens
	void surfaceCreated(RendererWrapper wrap)
	{
        // Kick off the layer thread for background operations
		for (LayerThread layerThread : layerThreads)
			layerThread.setRenderer(renderWrapper.maplyRender);

		// Note: Debugging output
		renderWrapper.maplyRender.setPerfInterval(perfInterval);
		
		// Kick off the layout layer
		layoutLayer = new LayoutLayer(this,layoutManager);
		LayerThread baseLayerThread = layerThreads.get(0);
		baseLayerThread.addLayer(layoutLayer);
		
		// Run any outstanding runnables
		if (surfaceTasks != null) {
			for (Runnable run : surfaceTasks) {
				Handler handler = new Handler(activity.getMainLooper());
				handler.post(run);
			}
			surfaceTasks = null;
		}

    	glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    	// Note: 3fps
    	metroThread = new MetroThread("Metronome Thread",this,2);

        // Make our own context that we can use on the main thread
        EGL10 egl = (EGL10) EGLContext.getEGL();
        int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE };
        eglContext = egl.eglCreateContext(renderWrapper.maplyRender.display,renderWrapper.maplyRender.config,renderWrapper.maplyRender.context, attrib_list);
        int[] surface_attrs =
                {
                        EGL10.EGL_WIDTH, 32,
                        EGL10.EGL_HEIGHT, 32,
//			    EGL10.EGL_COLORSPACE, GL10.GL_RGB,
//			    EGL10.EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGB,
//			    EGL10.EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
//			    EGL10.EGL_LARGEST_PBUFFER, GL10.GL_TRUE,
                        EGL10.EGL_NONE
                };
        eglSurface = egl.eglCreatePbufferSurface(renderWrapper.maplyRender.display, renderWrapper.maplyRender.config, surface_attrs);

		for (LayerThread layerThread : layerThreads)
	        layerThread.viewUpdated(view);

        // Call the post surface setup callbacks
        for (Runnable run : postSurfaceRunnables)
            activity.runOnUiThread(run);
        postSurfaceRunnables.clear();
	}

    /**
     * Set the EGL Context we created for the main thread, if we can.
     */
    boolean setEGLContext()
    {
        if (eglContext != null)
        {
            EGL10 egl = (EGL10) EGLContext.getEGL();
            if (!egl.eglMakeCurrent(renderWrapper.maplyRender.display, eglSurface, eglSurface, eglContext)) {
                Log.i("Maply", "Failed to make current context in main thread.");
                return false;
            }

            return true;
        }

        return false;
    }
	
	/**
	 * It takes a little time to set up the OpenGL ES drawable.  Add a runnable
	 * to be run after the surface is created.  If it's already been created we
	 * just run it here.
	 * <p>
	 * Only call this on the main thread.
	 */
	private void onSurfaceCreatedTask(Runnable run)
	{
		if (surfaceTasks != null)
			surfaceTasks.add(run);
		else
			run.run();
	}
			
	public void dispose()
	{
		running = false;
		vecManager.dispose();
		vecManager = null;
				
		renderWrapper = null;
	}

	/**
	 * Set the color for the OpenGL ES background.
     */
	public void setClearColor(int color)
	{
		if (renderWrapper == null)
			return;

		renderWrapper.maplyRender.setClearColor(Color.red(color)/255.f,Color.green(color)/255.f,Color.blue(color)/255.f,Color.alpha(color)/255.f);
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
	 * @param inPerfInterval
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
		LayerThread baseLayerThread = layerThreads.get(0);
		baseLayerThread.addLayer(layer);
	}
	
	/**
	 * Remove a single layer.  The layer will stop receiving data and be shut down shortly
	 * after you call this.
	 */
	public void removeLayer(Layer layer)
	{
		LayerThread baseLayerThread = layerThreads.get(0);
		baseLayerThread.removeLayer(layer);
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

		LayerThread baseLayerThread = layerThreads.get(0);
		if (Looper.myLooper() == baseLayerThread.getLooper() || (mode == ThreadMode.ThreadCurrent)) {

			// Only do this on the main thread
			if (Looper.myLooper() == Looper.getMainLooper())
	            setEGLContext();

            run.run();
        } else
			baseLayerThread.addTask(run,true);
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
				changes.process(scene);

				// Track the vector ID for later use
				if (vecId != EmptyIdentity)
					compObj.addVectorID(vecId);
			}
		};
		
		addTask(run, mode);
				
		return compObj;
	}

	public void changeVectors(final ComponentObject vecObj,final VectorInfo vecInfo,ThreadMode mode)
	{
		if (!running)
			return;

		// Do the actual work on the layer thread
		Runnable run =
				new Runnable()
				{
					@Override
					public void run()
					{
						// Vectors are simple enough to just add
						ChangeSet changes = new ChangeSet();
						long[] vecIDs = vecObj.getVectorIDs();
						if (vecIDs != null) {
							vecManager.changeVectors(vecIDs, vecInfo, changes);
							changes.process(scene);
						}
					}
				};
		addTask(run,mode);
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
						texID = texManager.addTexture(marker.image, scene, changes);
					if (texID != EmptyIdentity)
						intMarker.addTexID(texID);
					if (marker.vertexAttributes != null)
						intMarker.setVertexAttributes(marker.vertexAttributes.toArray());
					
					intMarkers.add(intMarker);
					
					// Keep track of this one for selection
					if (marker.selectable)
					{
						addSelectableObject(marker.ident,marker,compObj);
					}
				}

				// Add the markers and flush the changes
				long markerId = markerManager.addMarkers(intMarkers, markerInfo, changes);
				changes.process(scene);

				if (markerId != EmptyIdentity)
				{
					compObj.addMarkerID(markerId);
				}
			}
		};
		
		addTask(run, mode);

		return compObj;
	}

	/**
	 * Add stickers on top of the globe or map.  Stickers are 2D objects that drape over a defined
	 * area.
	 *
	 * @param stickers The list of stickers to apply.
	 * @param stickerInfo Parameters that cover all the stickers in question.
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
	 * @return This represents the stickers for later modification or deletion.
	 */
	public ComponentObject addStickers(final List<Sticker> stickers,final StickerInfo stickerInfo,ThreadMode mode)
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

                        // Stickers are added one at a time for some reason
                        for (Sticker sticker : stickers) {
                            long stickerID = stickerManager.addSticker(sticker, stickerInfo, changes);

                            if (stickerID != EmptyIdentity) {
                                compObj.addStickerID(stickerID);
                            }
                        }

						changes.process(scene);
					}
				};

		addTask(run, mode);

		return compObj;
	}

	/**
	 * Change the visual representation for the given sticker.
	 *
	 * @param sticker The sticker to change.
	 * @param stickerInfo Parameters to change.
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
	 * @return This represents the stickers for later modification or deletion.
	 */
	public ComponentObject changeSticker(final ComponentObject stickerObj,final StickerInfo stickerInfo,ThreadMode mode)
	{
		if (!running || stickerObj == null)
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

						long[] stickerIDs = stickerObj.getStickerIDs();
						if (stickerIDs != null && stickerIDs.length > 0) {
							long stickerID = stickerIDs[0];
							stickerManager.changeSticker(stickerID, stickerInfo, changes);
						}

						changes.process(scene);
					}
				};

		addTask(run, mode);

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
				changes.process(scene);
			}
		};
		
		addTask(run, mode);
		
		return compObj;
	}

    /**
     * Texture settings for adding textures to the system.
     */
    static public class TextureSettings
    {
        public TextureSettings()
        {
        }

        /**
         * Image format to use when creating textures.
         */
        QuadImageTileLayer.ImageFormat imageFormat = QuadImageTileLayer.ImageFormat.MaplyImageIntRGBA;
    }

    /**
     * Add texture to the system with the given settings.
     * @param image Image to add.
     * @param settings Settings to use.
     * @param mode Add on the current thread or elsewhere.
     */
	public MaplyTexture addTexture(final Bitmap image,TextureSettings settings,ThreadMode mode)
    {
        final MaplyTexture texture = new MaplyTexture();

        // Possibly do the work somewhere else
        Runnable run =
                new Runnable()
                {
                    @Override
                    public void run()
                    {
                        ChangeSet changes = new ChangeSet();

                        Texture rawTex = new Texture();
                        rawTex.setBitmap(image);
                        texture.texID = rawTex.getID();
                        changes.addTexture(rawTex,scene);

                        // Flush the texture changes
						changes.process(scene);
                    }
                };

        addTask(run, mode);

        return texture;
    }

	/**
	 * Remove a texture from the scene with the given settings.
	 * @param tex Texture to remove.
	 * @param mode Remove immediately (current thread) or elsewhere.
     */
	public void removeTexture(final MaplyTexture tex,ThreadMode mode)
	{
		// Do the actual work on the layer thread
		Runnable run =
				new Runnable()
				{
					@Override
					public void run()
					{
						ChangeSet changes = new ChangeSet();

						changes.removeTexture(tex.texID);

						// Flush the texture changes
						changes.process(scene);
					}
				};

		addTask(run, mode);
	}

    /**
     * Associate a shader with the given scene name.  These names let us override existing shaders, as well as adding our own.
     * @param shader The shader to add.
     * @param sceneName The scene name to associate it with.
     */
    public void addShaderProgram(Shader shader,String sceneName)
    {
        scene.addShaderProgram(shader, sceneName);
    }

	/**
	 * Add an active object that will be called right before the render (on the render thread).
	 */
	public void addActiveObject(ActiveObject activeObject)
	{
		if (renderWrapper == null || renderWrapper.maplyRender == null)
			return;
		renderWrapper.maplyRender.addActiveObject(activeObject);
	}

	/**
	 * Remove an active object added earlier.
	 */
	public void removeActiveObject(ActiveObject activeObject)
	{
		renderWrapper.maplyRender.removeActiveObject(activeObject);
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
					if (compObj != null)
						compObj.enable(control, false, changes);
				changes.process(scene);
			}
		};
		
		addTask(run, mode);
	}

	/**
	 * Disable the display for the given object.
	 *
	 * @param compObj Object to disable
	 * @param mode Where to execute the enable.  Choose ThreadAny by default.
	 */
	public void disableObject(ComponentObject compObj,ThreadMode mode)
	{
		if (!running)
			return;

		ArrayList<ComponentObject> compObjs = new ArrayList<ComponentObject>();
		compObjs.add(compObj);

		disableObjects(compObjs, mode);
	}

	/**
	 * Enable the display for the given objects.  These objects were returned
	 * by the various add calls.  To disable the display, call disableObjects().
	 *
	 * @param compObjs Objects to enable disable.
	 * @param mode Where to execute the enable.  Choose ThreadAny by default.
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
					if (compObj != null)
						compObj.enable(control, true, changes);
				changes.process(scene);
			}
		};
		
		addTask(run, mode);
	}

	/**
	 * Enable the display for the given object.
	 *
	 * @param compObj Object to enable.
	 * @param mode Where to execute the enable.  Choose ThreadAny by default.
     */
	public void enableObject(ComponentObject compObj,ThreadMode mode)
	{
		if (!running)
			return;

		ArrayList<ComponentObject> compObjs = new ArrayList<ComponentObject>();
		compObjs.add(compObj);

		enableObjects(compObjs, mode);
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
		removeObjects(compObjs, mode);
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
				changes.process(scene);
			}
		};
		
		addTask(run, mode);
	}
	
    private boolean isProbablyEmulator() {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1
                && (Build.FINGERPRINT.startsWith("generic")
                        || Build.FINGERPRINT.startsWith("unknown")
                        || Build.MODEL.contains("google_sdk")
                        || Build.MODEL.contains("Emulator")
                        || Build.MODEL.contains("Android SDK built for x86"));
    }


	public ComponentObject addParticleSystem(final ParticleSystem particleSystem, ThreadMode mode) {
		if (!running)
			return null;

		final ComponentObject compObj = new ComponentObject();
		final ChangeSet changes = new ChangeSet();

		for (Bitmap image : particleSystem.getTextures()) {
			MaplyTexture texture = this.addTexture(image, new TextureSettings(), mode);
			if (texture.texID != EmptyIdentity)
				particleSystem.addTexID(texture.texID);
		}

		Runnable run = new Runnable() {
			@Override
			public void run() {
				long particleSystemID = particleSystemManager.addParticleSystem(particleSystem, changes);
				if (particleSystemID != EmptyIdentity) {
					compObj.addParticleSystemID(particleSystemID);
				}
				changes.process(scene);
			}
		};

		addTask(run, mode);
		return compObj;
	}

	public void addParticleBatch(final ParticleBatch particleBatch, ThreadMode mode) {
		if (!running)
			return;

		if (particleBatch.isValid()) {
			Runnable run = new Runnable() {
				@Override
				public void run() {
					ChangeSet changes = new ChangeSet();
					particleSystemManager.addParticleBatch(particleBatch.getPartSys().getIdent(), particleBatch,changes);
					changes.process(scene);
				}
			};
			addTask(run, mode);
		}

	}
}
