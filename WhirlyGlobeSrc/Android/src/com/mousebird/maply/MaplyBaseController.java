package com.mousebird.maply;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.graphics.drawable.ColorDrawable;
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
import java.util.Vector;

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
	// This may be a GLSurfaceView or a GLTextureView
	View baseView = null;
	Activity activity = null;
    private OkHttpClient httpClient;

	public static final String kToolkitDefaultTriangleNoLightingProgram = "Default Triangle;lighting=no";

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

	/**
	 * If set, we'll explicitly call dispose on any objects that were
	 * being kept around for selection.
	 */
	public boolean disposeAfterRemoval = false;
	
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
    OkHttpClient getHttpClient()
	{
		if (httpClient == null)
			httpClient = new OkHttpClient();
		return httpClient;
	}
	
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
	ShapeManager shapeManager = null;
	BillboardManager billboardManager = null;
	GeometryManager geomManager = null;

	// Manage bitmaps and their conversion to textures
	TextureManager texManager = new TextureManager();
	
	// Layer thread we use for data manipulation
	ArrayList<LayerThread> layerThreads = new ArrayList<LayerThread>();
	ArrayList<LayerThread> workerThreads = new ArrayList<LayerThread>();
		
	// Bounding box we're allowed to move within
	Point2d viewBounds[] = null;
	
	/**
	 * Returns the layer thread we used for processing requests.
	 */
	public LayerThread getLayerThread()
	{
		if (layerThreads == null)
			return null;
		synchronized (layerThreads) {
			if (layerThreads.size() == 0)
				return null;
			return layerThreads.get(0);
		}
	}

	private int lastLayerThreadReturned = 0;

	/**
	 * Returns a layer thread you can do whatever you like on.  You don't have
	 * to be particularly fast about it, it won't hold up the main layer thread.
	 * These layer threads are set up with the proper OpenGL contexts so they're
	 * fast to add new geometry using the ThreadCurrent option.
	 */
	public LayerThread getWorkingThread()
	{
		// The first one is for use by the toolkit
		int numAvailable = workerThreads.size();

		if (numAvailable < 0)
			return null;

		if (numAvailable == 0)
			return workerThreads.get(0);

		return workerThreads.get((lastLayerThreadReturned++) % numAvailable);
	}

	/**
	 * These are settings passed on construction.  We need these
	 * immediately at startup to create the right internal structures.
	 */
	public static class Settings
	{
		/**
		 * If set, we'll use a GLSurfaceView.  Otherwise a GLTexturesView.
		 * GLSurfaceView is the default.
		 */
		public boolean useSurfaceView = true;
		/**
		 * These are the number of working threads we'll create by default
		 * at startup.  These are fully capable of adding geometry to the
		 * system on their own (via ThreadCurrent).
		 */
		public int numWorkingThreads = 8;
		/**
		 * If set we'll override the width of the rendering surface.
		 *
		 * This is useful for scaling back the surface resolution
		 * for slower devices.
		 */
		public int width = 0;
		/**
		 * If set we'll override the height of the rendering surface.
		 *
		 * This is useful for scaling back the surface resolution
		 * for slower devices.
		 */
		public int height = 0;
	}

	// Set if we're using a TextureView rather than a SurfaceView
	boolean useTextureView = false;

	/**
	 * Returns true if we set up a TextureView rather than a SurfaceView.
     */
	public boolean usesTextureView()
	{
		return useTextureView;
	}

	boolean libraryLoaded = false;
	int numWorkingThreads = 8;
	int width = 0;
	int height = 0;

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
	public MaplyBaseController(Activity mainActivity,Settings settings)
	{
		System.loadLibrary("gnustl_shared");
		System.loadLibrary("Maply");
		libraryLoaded = true;
		activity = mainActivity;
		if (settings != null) {
			useTextureView = !settings.useSurfaceView;
			numWorkingThreads = settings.numWorkingThreads;
			width = settings.width;
			height = settings.height;
		}
	}

	ColorDrawable tempBackground = null;
	
	protected void Init()
	{
		if (!libraryLoaded)
		{
			System.loadLibrary("gnustl_shared");
			System.loadLibrary("Maply");
			libraryLoaded = true;
		}

		// Fire up the managers.  Can't do anything without these.
		vecManager = new VectorManager(scene);
		markerManager = new MarkerManager(scene);
        stickerManager = new StickerManager(scene);
		labelManager = new LabelManager(scene);
		layoutManager = new LayoutManager(scene);
		selectionManager = new SelectionManager(scene);
		particleSystemManager = new ParticleSystemManager(scene);
		shapeManager = new ShapeManager(scene);
		billboardManager = new BillboardManager(scene);
		geomManager = new GeometryManager(scene);

		// Now for the object that kicks off the rendering
		renderWrapper = new RendererWrapper(this);
		renderWrapper.scene = scene;
		renderWrapper.view = view;
		
		// Create the layer thread
        LayerThread layerThread = new LayerThread("Maply Layer Thread",view,scene,true);
		synchronized (layerThreads) {
			layerThreads.add(layerThread);
		}
		
        ActivityManager activityManager = (ActivityManager) activity.getSystemService(Context.ACTIVITY_SERVICE);
        ConfigurationInfo configurationInfo = activityManager.getDeviceConfigurationInfo();

        final boolean supportsEs2 = configurationInfo.reqGlEsVersion >= 0x20000 || isProbablyEmulator();
        if (supportsEs2)
        {
			if (!useTextureView) {
				GLSurfaceView glSurfaceView = new GLSurfaceView(activity);

				if (width > 0 && height > 0) {
					glSurfaceView.getHolder().setFixedSize(width, height);
				}

				// If the clear color has transparency, we need to set things up differently
				if (Color.alpha(clearColor) < 255) {
					glSurfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
					glSurfaceView.getHolder().setFormat(PixelFormat.TRANSLUCENT);
					glSurfaceView.setZOrderOnTop(true);
				} else {
					if (isProbablyEmulator())
						glSurfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
				}

				tempBackground = new ColorDrawable();
				// This eliminates the black flash, but only if the clearColor is set right
				tempBackground.setColor(clearColor);
				if (Build.VERSION.SDK_INT > 16)
					glSurfaceView.setBackground(tempBackground);
				glSurfaceView.setEGLContextClientVersion(2);
				glSurfaceView.setRenderer(renderWrapper);

				baseView = glSurfaceView;
			} else {
				GLTextureView glTextureView = new GLTextureView(activity);

				if (width > 0 && height > 0) {
					glTextureView.getSurfaceTexture().setDefaultBufferSize(width,height);
				}

				// If the clear color has transparency, we need to set things up differently
				if (Color.alpha(clearColor) < 255) {
					glTextureView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
					// Note: Do we need these in a TextureView
//					glTextureView.getHolder().setFormat(PixelFormat.TRANSLUCENT);
//					glTextureView.setZOrderOnTop(true);
				} else {
					if (isProbablyEmulator())
						glTextureView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
				}

				glTextureView.setOpaque(false);
				tempBackground = new ColorDrawable();
				// This eliminates the black flash, but only if the clearColor is set right
				tempBackground.setColor(clearColor);
				if (Build.VERSION.SDK_INT > 16 && Build.VERSION.SDK_INT < 24)
					glTextureView.setBackground(tempBackground);
				glTextureView.setEGLContextClientVersion(2);
				glTextureView.setRenderer(renderWrapper);

				baseView = glTextureView;
			}
        } else {
        	Toast.makeText(activity,  "This device does not support OpenGL ES 2.0.", Toast.LENGTH_LONG).show();
        	return;
        }   
        
		running = true;		
	}

	/**
	 * Makes a new layer thread for toolkit related tasks.
	 * @param handlesViewUpdates If set, the layer thread will deal with view updates.
	 *                           If not set, it's a simpler layer thread.
     */
	public LayerThread makeLayerThread(boolean handlesViewUpdates)
	{
		if (!running)
			return null;

		// Create the layer thread
		LayerThread newLayerThread = new LayerThread("External Maply Layer Thread",view,scene,handlesViewUpdates);

		synchronized (layerThreads) {
			layerThreads.add(newLayerThread);
		}

		// Kick off the layer thread for background operations
		newLayerThread.setRenderer(renderWrapper.maplyRender);

		if (handlesViewUpdates)
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
		return baseView;
	}

	/**
	 * Return the Android view size, rather than the frame size.
     */
	public Point2d getViewSize()
	{
		if (baseView == null)
			return new Point2d(0,0);
		return new Point2d(baseView.getWidth(),baseView.getHeight());
	}
	
	/**
	 * Call shutdown when you're done with the MaplyController.  It will shut down the layer
	 * thread(s) and all the associated logic.
	 */
	public void shutdown()
	{
		startupAborted = true;
		synchronized (this) {
			running = false;

			// This will make sure we have a valid context
			setEGLContext(glContext);

			renderWrapper.stopRendering();

			//		Choreographer.getInstance().removeFrameCallback(this);
			ArrayList<LayerThread> layerThreadsToRemove = null;
			synchronized (layerThreads) {
				layerThreadsToRemove = new ArrayList<LayerThread>(layerThreads);
			}
			for (LayerThread layerThread : layerThreadsToRemove)
				layerThread.shutdown();
			synchronized (layerThreads) {
				layerThreads.clear();
			}

			metroThread.shutdown();
			metroThread = null;


			scene.teardownGL();
			scene.shutdown();

			//		scene.dispose();
			//		view.dispose();

			coordAdapter.shutdown();
			vecManager.dispose();
			markerManager.dispose();
			stickerManager.dispose();
			selectionManager.dispose();
			labelManager.dispose();
			layoutManager.dispose();
			particleSystemManager.dispose();

			// Shut down the contexts
			EGL10 egl = (EGL10) EGLContext.getEGL();
			for (ContextInfo context : glContexts) {
				egl.eglDestroySurface(renderWrapper.maplyRender.display, context.eglSurface);
				egl.eglDestroyContext(renderWrapper.maplyRender.display, context.eglContext);
			}
			glContexts = null;

			// And the main one
			egl.eglDestroySurface(renderWrapper.maplyRender.display, glContext.eglSurface);
			egl.eglDestroyContext(renderWrapper.maplyRender.display, glContext.eglContext);
			glContext = null;

			// Clean up OpenGL ES resources
			setEGLContext(null);

			renderWrapper.shutdown();

			baseView = null;
			renderWrapper = null;
			coordAdapter = null;
			scene = null;
			view = null;
			vecManager = null;
			markerManager = null;
			stickerManager = null;
			labelManager = null;
			selectionManager = null;
			layoutManager = null;
			particleSystemManager = null;
			layoutLayer = null;
			shapeManager = null;
			billboardManager = null;

			texManager = null;
			// Using this as a synch object, so not a great idea
//			layerThreads = null;
			workerThreads = null;

			activity = null;
			tempBackground = null;
		}
	}
	
	ArrayList<Runnable> surfaceTasks = new ArrayList<Runnable>();
	
	// Metronome thread used to time the renderer
	protected MetroThread metroThread;

    // Note: Why isn't this in EGL10?
    private static final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;

	// Context and associated surface
	public class ContextInfo
	{
		EGLContext eglContext = null;
		EGLSurface eglSurface = null;
	};
	ArrayList<ContextInfo> glContexts = new ArrayList<ContextInfo>();
	ContextInfo glContext = null;

	// Are we on the GL rendering thread
	boolean isOnGLThread()
	{
		if (Thread.currentThread() == renderWrapper.renderThread)
			return true;

		return false;
	}

	// Are we are on one of our known layer threads?
	boolean isOnLayerThread()
	{
		synchronized (layerThreads) {
			for (LayerThread thread : layerThreads) {
				if (Looper.myLooper() == thread.getLooper())
					return true;
			}
		}

		return false;
	}

	// Make a temporary context for use within the base controller.
	// We expect these to be running on various threads
	ContextInfo setupTempContext(MaplyBaseController.ThreadMode threadMode)
	{
		// There's already a context, so just stick with that
		EGL10 egl = (EGL10) EGLContext.getEGL();
		ContextInfo retContext = null;

		// The main thread has its own context we use
		if (Looper.myLooper() == Looper.getMainLooper()) {
			setEGLContext(null);
			return glContext;
		} else if (isOnGLThread() || isOnLayerThread()) {
			// We're on a known layer thread, which has a well known context so do nothing
			return null;
		} else {
			synchronized (glContexts)
			{
				// See if we need to create a new context/surface
				if (glContexts.size() == 0)
				{
					int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE };
					retContext = new ContextInfo();
					retContext.eglContext = egl.eglCreateContext(renderWrapper.maplyRender.display,renderWrapper.maplyRender.config,renderWrapper.maplyRender.context, attrib_list);
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
					retContext.eglSurface = egl.eglCreatePbufferSurface(renderWrapper.maplyRender.display, renderWrapper.maplyRender.config, surface_attrs);

//					Log.d("Maply","Created context + " + retContext.eglContext.toString());
				} else {
					retContext = glContexts.get(0);
					glContexts.remove(0);
				}
			}

			setEGLContext(retContext);
		}

		return retContext;
	}

	void clearTempContext(ContextInfo cInfo)
	{
		if (cInfo == null)
			return;

		synchronized (glContexts)
		{
			if (cInfo != glContext) {
				EGL10 egl = (EGL10) EGLContext.getEGL();
//				GLES20.glFlush();
//				GLES20.glFinish();
				if (!egl.eglMakeCurrent(renderWrapper.maplyRender.display, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT))
				{
					Log.d("Maply","Failed to clear context");
				}
				glContexts.add(cInfo);
			}
		}
	}

	boolean rendererAttached = false;
    ArrayList<Runnable> postSurfaceRunnables = new ArrayList<Runnable>();

	/**
	 * Set if the renderer is set up and running.
	 */
	public boolean rendererIsAttached()
	{
		return rendererAttached;
	}

    /**
     * Add a runnable to be executed after the OpenGL surface is created.
     */
    public void addPostSurfaceRunnable(Runnable run)
    {
        if (rendererAttached)
            activity.runOnUiThread(run);
        else
            postSurfaceRunnables.add(run);
    }

	int displayRate = 2;

	/**
	 * Set the display rate for the GL render.
	 * 1 means 60fps.  2 means 30fps.  3 means 20fps.
     */
	public void setDisplayRate(int inRate)
	{
		displayRate = inRate;
		if (metroThread != null)
			metroThread.setFrameRate(inRate);
	}

	/**
	 * Force a render on the next frame.
	 * Mostly used internally.
	 */
	public void requestRender()
	{
		if (metroThread != null)
			metroThread.requestRender();
	}

	// Set if they shut things down before the surface was attached
	boolean startupAborted = false;

	// Called by the render wrapper when the surface is created.
	// Can't start doing anything until that happens
	void surfaceCreated(RendererWrapper wrap)
	{
		synchronized (this) {
			if (startupAborted)
				return;

			synchronized (layerThreads) {
				// Kick off the layer thread for background operations
				for (LayerThread layerThread : layerThreads)
					layerThread.setRenderer(renderWrapper.maplyRender);
			}

			// Debugging output
			renderWrapper.maplyRender.setPerfInterval(perfInterval);

			// Kick off the layout layer
			layoutLayer = new LayoutLayer(this, layoutManager);
			LayerThread baseLayerThread = null;
			synchronized (layerThreads) {
				baseLayerThread = layerThreads.get(0);
			}
			baseLayerThread.addLayer(layoutLayer);

			// Add a default cluster generator
			addClusterGenerator(new BasicClusterGenerator(new int[]{Color.argb(255, 255, 165, 0)}, 0, new Point2d(64, 64), this, activity));

			// Run any outstanding runnables
			if (surfaceTasks != null) {
				for (Runnable run : surfaceTasks) {
					Handler handler = new Handler(activity.getMainLooper());
					handler.post(run);
				}
				surfaceTasks = null;
			}

			if (baseView instanceof GLSurfaceView) {
				GLSurfaceView glSurfaceView = (GLSurfaceView) baseView;
				glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
			} else {
				GLTextureView glTextureView = (GLTextureView) baseView;
				glTextureView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
			}
			metroThread = new MetroThread("Metronome Thread", this, displayRate);
			metroThread.setRenderer(renderWrapper.maplyRender);

			// Make our own context that we can use on the main thread
			EGL10 egl = (EGL10) EGLContext.getEGL();
			int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE};
			glContext = new ContextInfo();
			glContext.eglContext = egl.eglCreateContext(renderWrapper.maplyRender.display, renderWrapper.maplyRender.config, renderWrapper.maplyRender.context, attrib_list);
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
			glContext.eglSurface = egl.eglCreatePbufferSurface(renderWrapper.maplyRender.display, renderWrapper.maplyRender.config, surface_attrs);

			synchronized (layerThreads) {
				for (LayerThread layerThread : layerThreads)
					layerThread.viewUpdated(view);
			}

			setClearColor(clearColor);

			// Create the working threads
			for (int ii = 0; ii < numWorkingThreads; ii++)
				workerThreads.add(makeLayerThread(false));

			rendererAttached = true;

			// Call the post surface setup callbacks
			for (final Runnable theRunnable : postSurfaceRunnables) {
				activity.runOnUiThread(new Runnable() {
					@Override
					public void run() {
						// If they shut things down right here, we have to check
						if (running)
							theRunnable.run();
					}
				});
			}
			postSurfaceRunnables.clear();
		}
	}

    /**
     * Set the EGL Context we created for the main thread, if we can.
     */
    boolean setEGLContext(ContextInfo cInfo)
    {
		if (cInfo == null)
			cInfo = glContext;

		// This does seem to happen
		if (renderWrapper == null || renderWrapper.maplyRender == null || renderWrapper.maplyRender.display == null)
			return false;

		EGL10 egl = (EGL10) EGLContext.getEGL();
        if (cInfo != null)
        {
            if (!egl.eglMakeCurrent(renderWrapper.maplyRender.display, cInfo.eglSurface, cInfo.eglSurface, cInfo.eglContext)) {
                Log.d("Maply", "Failed to make current context.");
                return false;
            }

            return true;
        } else {
			if (renderWrapper != null && renderWrapper.maplyRender != null && renderWrapper.maplyRender.display != null)
				egl.eglMakeCurrent(renderWrapper.maplyRender.display, egl.EGL_NO_SURFACE, egl.EGL_NO_SURFACE, egl.EGL_NO_CONTEXT);
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
			
	int clearColor = Color.BLACK;

	/**
	 * Set the color for the OpenGL ES background.
     */
	public void setClearColor(int color)
	{
		clearColor = color;

		if (renderWrapper == null)
			return;

//		if (tempBackground != null)
//			tempBackground.setColor(clearColor);

		if (renderWrapper.maplyRender != null)
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

	/**
	 * Batch version of the screenPointFromGeo method.  This version is here for users to
	 * convert a whole group of coordinates all at once.  Doing it individually is just
	 * too slow in Java.
	 * @param inX Longitude in radians.
	 * @param inY Latitude in radians.
	 * @param inZ Z value.  Set this to zero most of the time.
	 * @param outX X point on screen.  MAXFLOAT if this is behind the globe.
	 * @param outY Y point on screen.  MAXFLOAT if this is behind the globe.
	 */
	public boolean screenPointFromGeoBatch(double[] inX,double[] inY,double[] inZ,double[] outX,double[] outY)
	{
		if (coordAdapter == null)
			return false;
		Point2d frameSize = renderWrapper.maplyRender.frameSize;

		return coordAdapter.screenPointFromGeoBatch(view,(int)frameSize.getX(),(int)frameSize.getY(),
				inX,inY,inZ,outX,outY);
	}

	/**
	 * Batch version of the geoPointFromScreen method.  This version is here for users to
	 * convert a whole group of coordinates all at once.  Doing it individually is just
	 * too slow in Java.
	 * @param inX
	 * @param inY
	 * @param outX
	 * @param outY
	 */
	public boolean geoPointFromScreenBatch(double[] inX,double[] inY,double[] outX,double[] outY)
	{
		if (coordAdapter == null)
			return false;
		Point2d frameSize = renderWrapper.maplyRender.frameSize;

		return coordAdapter.geoPointFromScreenBatch(view,(int)frameSize.getX(),(int)frameSize.getY(),
				inX,inY,outX,outY);
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

	/** Calculate the height that corresponds to a given Mapnik-style map scale.
	 * <br>
	 * Figure out the viewer height that corresponds to a given scale denominator (ala Mapnik).
	 * This height will probably be use for visibility ranges on geometry.  This works as a mechanism for making geometry appear at certain map scales and disappear at others.
	 * @return Returns the height or 0.0 if the system isn't initialized yet.
	 */
	public double heightForMapScale(double scale)
	{
		if (view == null || renderWrapper == null)
			return 0.0;

		return view.heightForMapScale(scale,renderWrapper.maplyRender.frameSize.getX(),renderWrapper.maplyRender.frameSize.getY());
	}

	/** Return the current map zoom from the viewpoint.
	 * <br>
	 * Calculate the map zoom (TMS) based on the current screen size and the 3D viewport.
	 * @param geoCoord the location to calculate for. This is needed because zoom is dependant on latitude.
	 * @return Returns the map zoom or 0.0 if the system is not yet initialized.
	 */
	public double currentMapZoom(Point2d geoCoord)
	{
		if (view == null || renderWrapper == null)
			return 0.0;

		return view.currentMapZoom(renderWrapper.maplyRender.frameSize.getX(),renderWrapper.maplyRender.frameSize.getY(),geoCoord.getY());
	}

	/** Return the current scale denominator (Mapnik).
	 * <br>
	 * See https://github.com/mapnik/mapnik/wiki/ScaleAndPpi for a definition of the Mapnik scale denominator.
	 * @return Returns the current scale denominator or 0.0 if the system is not yet initialized.
	 */
	public double currentMapScale()
	{
		if (view == null || renderWrapper == null)
			return 0.0;

		return view.currentMapScale(renderWrapper.maplyRender.frameSize.getX(),renderWrapper.maplyRender.frameSize.getY());
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
	public void addLayer(final Layer layer)
	{
		if (!rendererAttached) {
			addPostSurfaceRunnable(new Runnable() {
				@Override
				public void run() {
					addLayer(layer);
				}
			});
			return;
		}

		synchronized (layerThreads) {
			if (layerThreads.size() > 0) {
				LayerThread baseLayerThread = layerThreads.get(0);
				baseLayerThread.addLayer(layer);
			}
		}
	}
	
	/**
	 * Remove a single layer.  The layer will stop receiving data and be shut down shortly
	 * after you call this.
	 */
	public void removeLayer(final Layer layer)
	{
		if (!rendererAttached) {
			addPostSurfaceRunnable(new Runnable() {
				@Override
				public void run() {
					removeLayer(layer);
				}
			});
			return;
		}

		synchronized (layerThreads) {
			if (layerThreads.size() > 0) {
				LayerThread baseLayerThread = layerThreads.get(0);
				baseLayerThread.removeLayer(layer);
			}
		}
	}
	
	/**
	 * Add a task according to the thread mode.  If it's ThreadAny, we'll put it on the layer thread.
	 * If it's ThreadCurrent, we'll do it immediately.
	 * 
	 * @param run Runnable to execute.
	 * @param mode Where to execute it.
	 */
	private void addTask(final Runnable run,final ThreadMode mode)
	{
		if (!running)
			return;

		if (!rendererAttached) {
			addPostSurfaceRunnable(new Runnable() {
				@Override
				public void run() {
					addTask(run,mode);
				}
			});
			return;
		}

		LayerThread baseLayerThread = null;
		synchronized (layerThreads) {
			baseLayerThread = layerThreads.get(0);
		}
		if (mode == ThreadMode.ThreadCurrent) {

			EGL10 egl = (EGL10) EGLContext.getEGL();
			EGLContext oldContext = egl.eglGetCurrentContext();
			EGLSurface  oldDrawSurface = egl.eglGetCurrentSurface(EGL10.EGL_DRAW);
			EGLSurface  oldReadSurface = egl.eglGetCurrentSurface(EGL10.EGL_READ);

			ContextInfo tempContext = setupTempContext(mode);

            run.run();

			clearTempContext(tempContext);

			if (oldContext != null)
			{
				if (renderWrapper != null)
					if (!egl.eglMakeCurrent(renderWrapper.maplyRender.display,oldDrawSurface,oldReadSurface,oldContext))
					{
						Log.d("Maply","Failed to set context back to previous context.");
					}
			}
        } else
			baseLayerThread.addTask(run,true);
	}

	protected ArrayList<ComponentObject> componentObjects = new ArrayList<ComponentObject>();

	// Add and track a new component object
	protected ComponentObject addComponentObj()
	{
		synchronized (componentObjects)
		{
			ComponentObject compObj = new ComponentObject();
			componentObjects.add(compObj);

			return compObj;
		}
	}

	// Remove an existing component object
	protected void removeComponentObj(ComponentObject compObj)
	{
		synchronized (componentObjects)
		{
			componentObjects.remove(compObj);
		}
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

		final ComponentObject compObj = addComponentObj();
		
		// Do the actual work on the layer thread
		Runnable run =
		new Runnable()
		{		
			@Override
			public void run()
			{
				// Vectors are simple enough to just add
				ChangeSet changes = new ChangeSet();
				long vecId = vecManager.addVectors(vecs, vecInfo, changes);
				if (scene != null)
					changes.process(scene);

				// Track the vector ID for later use
				if (vecId != EmptyIdentity)
					compObj.addVectorID(vecId);

				for (VectorObject vecObj : vecs)
				{
					// Keep track of this one for selection
					if (vecObj.selectable)
						compObj.addVector(vecObj);
				}

				if (vecInfo.disposeAfterUse || disposeAfterRemoval)
					for (VectorObject vecObj : vecs)
						if (!vecObj.selectable)
							vecObj.dispose();
			}
		};
		
		addTask(run, mode);
				
		return compObj;
	}

	public void changeVectors(final ComponentObject vecObj,final VectorInfo vecInfo,ThreadMode mode)
	{
		if (!running)
			return;

		if (vecObj == null)
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
							if (scene != null)
								changes.process(scene);
						}
					}
				};
		addTask(run, mode);
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

		final ComponentObject compObj = addComponentObj();

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
					if (marker.loc == null)
					{
						Log.d("Maply","Missing location for marker.  Skipping.");
						return;
					}

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
				if (scene != null)
					changes.process(scene);

				if (markerId != EmptyIdentity)
				{
					compObj.addMarkerID(markerId);
				}

				for (InternalMarker marker : intMarkers)
					marker.dispose();
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

		final ComponentObject compObj = addComponentObj();

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

						if (scene != null)
							changes.process(scene);

						if (stickerInfo.disposeAfterUse || disposeAfterRemoval)
							for (Sticker sticker : stickers)
								sticker.dispose();
					}
				};

		addTask(run, mode);

		return compObj;
	}

	/**
	 * Change the visual representation for the given sticker.
	 *
	 * @param stickerObj The sticker to change.
	 * @param stickerInfo Parameters to change.
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
	 * @return This represents the stickers for later modification or deletion.
	 */
	public ComponentObject changeSticker(final ComponentObject stickerObj,final StickerInfo stickerInfo,ThreadMode mode)
	{
		if (!running || stickerObj == null)
			return null;

		final ComponentObject compObj = addComponentObj();

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

						if (scene != null)
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
				for (long selectID : compObj.selectIDs) {
					Object selObj = selectionMap.get(selectID);
					if (selObj != null)
					{
						if (this.disposeAfterRemoval)
						{
							// Note: We should fix this for the other object types
							if (selObj.getClass() == VectorObject.class)
							{
								VectorObject vecObj = (VectorObject)selObj;
								vecObj.dispose();
							}
						}
						selectionMap.remove(selectID);
					}
				}
			}
		}
	}

	// Filled in by the subclass
	public Point2d geoPointFromScreen(Point2d screenPt)
	{
		return null;
	}

	// Filled in by the subclass
	public Point2d screenPointFromGeo(Point2d geoCoord)
	{
		return null;
	}

	// Returns all the objects near a point
	protected SelectedObject[] getObjectsAtScreenLoc(Point2d screenLoc)
	{
		Point2d viewSize = getViewSize();
		Point2d frameSize = renderWrapper.maplyRender.frameSize;
		Point2d scale = new Point2d(frameSize.getX()/viewSize.getX(),frameSize.getY()/viewSize.getY());
		Point2d frameLoc = new Point2d(scale.getX()*screenLoc.getX(),scale.getY()*screenLoc.getY());

		// Ask the selection manager
		SelectedObject selManObjs[] = selectionManager.pickObjects(view, frameLoc);
		if (selManObjs != null)
		{
			// Remap the objects
			synchronized(selectionMap) {
				for (SelectedObject selObj : selManObjs) {
					long selectID = selObj.getSelectID();
					selObj.selObj = selectionMap.get(selectID);
				}
			}
		}

		Point2d geoPt = geoPointFromScreen(screenLoc);
		if (geoPt == null)
			return null;

		// Also check any vectors that were selectable
		ComponentObject[] theCompObjs;
		ArrayList<SelectedObject> vecSelObjs = new ArrayList<SelectedObject>();
		synchronized (componentObjects)
		{
			theCompObjs = componentObjects.toArray(new ComponentObject[componentObjects.size()]);
		}
		for (ComponentObject compObj: theCompObjs)
		{
			if (compObj.vecObjs != null)
				for (VectorObject vecObj : compObj.vecObjs)
					if (vecObj.pointInside(geoPt))
					{
						// Note: Are the rest of the defaults useful?
						SelectedObject selObj = new SelectedObject();
						selObj.selObj = vecObj;
						vecSelObjs.add(selObj);
					}
		}

		if (vecSelObjs.size() == 0)
			return selManObjs;
		{
			if (selManObjs != null)
				for (SelectedObject selObj : selManObjs)
					vecSelObjs.add(selObj);
			return vecSelObjs.toArray(new SelectedObject[vecSelObjs.size()]);
		}
	}

	/**
	 * Returns an object (if any) at a given screen location
	 * @param screenLoc the screen location to be considered
	 * @return teh object at screenLoc or null if none was there
	 */
	protected Object getObjectAtScreenLoc(Point2d screenLoc)
	{
		Point2d viewSize = getViewSize();
		Point2d frameSize = renderWrapper.maplyRender.frameSize;
		Point2d scale = new Point2d(frameSize.getX()/viewSize.getX(),frameSize.getY()/viewSize.getY());
		Point2d frameLoc = new Point2d(scale.getX()*screenLoc.getX(),scale.getY()*screenLoc.getY());

		long selectID = selectionManager.pickObject(view, frameLoc);
		if (selectID != EmptyIdentity)
		{
			// Look for the object
			Object selObj = null;
			synchronized(selectionMap)
			{
				selObj = selectionMap.get(selectID);
			}

			return selObj;
		}

		return null;
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

		final ComponentObject compObj = addComponentObj();

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
					if (label.text != null && label.text.length() > 0) {
						InternalLabel intLabel = new InternalLabel(label, labelInfo);
						intLabels.add(intLabel);

						// Keep track of this one for selection
						if (label.selectable) {
							addSelectableObject(label.ident, label, compObj);
						}
					}
				}

				long labelId = EmptyIdentity;
				// Note: We can't run multiple of these at once.  The problem is that
				//  we need to pass the JNIEnv deep inside the toolkit and we're setting
				//  on JNIEnv at a time for the CharRenderer callback.
				synchronized (labelManager) {
					labelId = labelManager.addLabels(intLabels, labelInfo, changes);
				}
				if (labelId != EmptyIdentity)
					compObj.addLabelID(labelId);

				// Flush the text changes
				if (scene != null)
					changes.process(scene);

				for (InternalLabel label : intLabels)
					label.dispose();
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
		final Texture rawTex = new Texture();
		texture.texID = rawTex.getID();

        // Possibly do the work somewhere else
        Runnable run =
                new Runnable()
                {
                    @Override
                    public void run()
                    {
                        ChangeSet changes = new ChangeSet();

						rawTex.setBitmap(image);
                        changes.addTexture(rawTex, scene);

                        // Flush the texture changes
						if (scene != null)
							changes.process(scene);
                    }
                };

        addTask(run, mode);

        return texture;
    }

	/**
	 * Add texture to the system with the given settings.
	 * @param rawTex Texture to add.
	 * @param settings Settings to use.
	 * @param mode Add on the current thread or elsewhere.
	 */
	public MaplyTexture addTexture(final Texture rawTex,TextureSettings settings,ThreadMode mode)
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
						texture.texID = rawTex.getID();
						changes.addTexture(rawTex, scene);

						// Flush the texture changes
						if (scene != null)
							changes.process(scene);
					}
				};

		addTask(run, mode);

		return texture;
	}

	/**
	 * Remove a texture from the scene.
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
						if (scene != null)
							changes.process(scene);
					}
				};

		addTask(run, mode);
	}

	/**
	 * Remove a whole group of textures from the scene.
	 * @param texs Textures to remove.
	 * @param mode Remove immediately (current thread) or elsewhere.
     */
	public void removeTextures(final List<MaplyTexture> texs,ThreadMode mode)
	{
		// Do the actual work on the layer thread
		Runnable run =
				new Runnable()
				{
					@Override
					public void run()
					{
						ChangeSet changes = new ChangeSet();

						for (MaplyTexture tex : texs)
							changes.removeTexture(tex.texID);

						// Flush the texture changes
						if (scene != null)
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
    public void addShaderProgram(final Shader shader,final String sceneName)
    {
		if (!rendererAttached) {
			addPostSurfaceRunnable(new Runnable() {
				@Override
				public void run() {
					addShaderProgram(shader,sceneName);
				}
			});
			return;
		}

        scene.addShaderProgram(shader, sceneName);
    }

	/**
	 * Add an active object that will be called right before the render (on the render thread).
	 */
	public void addActiveObject(final ActiveObject activeObject)
	{
		if (!rendererAttached) {
			addPostSurfaceRunnable(new Runnable() {
				@Override
				public void run() {
					addActiveObject(activeObject);
				}
			});
			return;
		}

		renderWrapper.maplyRender.addActiveObject(activeObject);
	}

	/**
	 * Remove an active object added earlier.
	 */
	public void removeActiveObject(final ActiveObject activeObject)
	{
		if (!rendererAttached) {
			addPostSurfaceRunnable(new Runnable() {
				@Override
				public void run() {
					removeActiveObject(activeObject);
				}
			});
			return;
		}

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

		final ComponentObject[] localCompObjs = compObjs.toArray(new ComponentObject[compObjs.size()]);

		final MaplyBaseController control = this;
		Runnable run = new Runnable()
		{		
			@Override
			public void run()
			{
				ChangeSet changes = new ChangeSet();
				for (ComponentObject compObj : localCompObjs)
					if (compObj != null)
						compObj.enable(control, false, changes);
				if (scene != null)
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

		final ComponentObject[] localCompObjs = compObjs.toArray(new ComponentObject[compObjs.size()]);
		
		final MaplyBaseController control = this;
		Runnable run = 
		new Runnable()
		{		
			@Override
			public void run()
			{
				ChangeSet changes = new ChangeSet();
				for (ComponentObject compObj : localCompObjs)
					if (compObj != null)
						compObj.enable(control, true, changes);
				if (scene != null)
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
		Runnable run = new Runnable()
		{		
			@Override
			public void run()
			{
				ChangeSet changes = new ChangeSet();
				for (ComponentObject compObj : compObjs)
				{
					compObj.clear(control, changes);
					removeSelectableObjects(compObj);

					removeComponentObj(compObj);
				}
				if (scene != null)
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

	/**
	 * This adds a particle system to the scene, but does not kick off any particles.
	 * @param particleSystem The particle system to start.
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
     */
	public ComponentObject addParticleSystem(final ParticleSystem particleSystem, ThreadMode mode) {
		if (!running)
			return null;

		final ComponentObject compObj = addComponentObj();
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
				if (scene != null)
					changes.process(scene);
			}
		};

		addTask(run, mode);
		return compObj;
	}

	/**
	 * Particles are short term objects, typically very small.  We create them in large groups for efficience.
	 * You'll need to fill out the MaplyParticleSystem initially and then the MaplyParticleBatch to create them.
	 * @param particleBatch The batch of particles to add to an active particle system.
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
     */
	public void addParticleBatch(final ParticleBatch particleBatch, ThreadMode mode) {
		if (!running)
			return;

		if (particleBatch.isValid()) {
			Runnable run = new Runnable() {
				@Override
				public void run() {
					ChangeSet changes = new ChangeSet();
					particleSystemManager.addParticleBatch(particleBatch.getPartSys().getIdent(), particleBatch,changes);
					if (scene != null)
						changes.process(scene);
				}
			};
			addTask(run, mode);
		}
	}

	/**
	 * When the layout system clusters a bunch of markers or labels together, it needs new images to represent the cluster.
	 * You can provide a custom image for each group of markers by filling in one of these generates and passing it in.
     */
	public void addClusterGenerator(ClusterGenerator generator) {
		if (this.layoutLayer != null) {
			generator.baseController = this;
			layoutLayer.addClusterGenerator(generator);
		}
	}

	/**
	 * This method will add the given MaplyShape derived objects to the current scene.  It will use the parameters in the description dictionary and it will do it on the thread specified.
	 * @param shapes An array of Shape derived objects
	 * @param shapeInfo Info controlling how the shapes look
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
     */
	public ComponentObject addShapes(final List<Shape> shapes, final ShapeInfo shapeInfo, ThreadMode mode) {
		if (!running)
			return null;

		final ComponentObject compObj = addComponentObj();
		final ChangeSet changes = new ChangeSet();
		Runnable run = new Runnable() {
			@Override
			public void run() {
				long shapeId = shapeManager.addShapes(shapes, shapeInfo, changes);
				if (shapeId != EmptyIdentity)
					compObj.addShapeID(shapeId);
				if (scene != null)
					changes.process(scene);

				for (Shape shape : shapes)
					if (shape.isSelectable())
					{
						addSelectableObject(shape.getSelectID(), shape, compObj);
					}

				if (shapeInfo.disposeAfterUse || disposeAfterRemoval)
					for (Shape shape : shapes)
						shape.dispose();
			}
		};

		addTask(run, mode);
		return compObj;
	}

	/**
	 * Adds a group of points all at once.  We're assuming you want to draw a lot of points, so you have to group them together into a MaplyPoints.
	 * @param inPoints The points to add to the scene
	 * @param geomInfo Controls how the points look
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
     */
	public ComponentObject addPoints(final List<Points> inPoints,final GeometryInfo geomInfo, final ThreadMode mode)
	{
		if (!running)
			return null;

		final Points[] points = inPoints.toArray(new Points[inPoints.size()]);

		final ComponentObject compObj = addComponentObj();
		final ChangeSet changes = new ChangeSet();
		Runnable run = new Runnable() {
			@Override
			public void run() {
				for (Points pts : points) {
					long geomId = geomManager.addGeometryPoints(pts.rawPoints, pts.mat, geomInfo, changes);
					if (geomId != EmptyIdentity)
						compObj.addGeometryID(geomId);
				}

				if (geomInfo.disposeAfterUse || disposeAfterRemoval)
					for (Points pts : points)
						pts.rawPoints.dispose();

				if (scene != null)
					changes.process(scene);
			}
		};

		addTask(run, mode);
		return compObj;
	}

	private ArrayList<Light> lights = new ArrayList<>();

	/**
	 * Add the given light to the list of active lights.
	 * <br>
	 * This method will add the given light to our active lights.  Most shaders will recognize these lights and do the calculations.  If you have a custom shader in place, it may or may not use these.
	 * Triangle shaders use the lights, but line shaders do not.
	 * @param light Light to add.
     */
	public void addLight(final Light light) {
		if (!rendererAttached) {
			addPostSurfaceRunnable(new Runnable() {
				@Override
				public void run() {
					addLight(light);
				}
			});
			return;
		}

		if (this.lights == null)
			this.lights = new ArrayList<>();
		lights.add(light);
		this.updateLights();
	}

	/**
	 * Remove the given light (assuming it's active) from the list of lights.
	 * @param light Light to remove.
     */
	public void removeLight(final Light light) {
		if (!rendererAttached) {
			addPostSurfaceRunnable(new Runnable() {
				@Override
				public void run() {
					removeLight(light);
				}
			});
			return;
		}

		if (this.lights == null)
			return;
		this.lights.remove(light);
		this.updateLights();
	}

	private void updateLights() {
		List<DirectionalLight> theLights = new ArrayList<>();
		for (Light light : lights) {
			DirectionalLight theLight = new DirectionalLight();
			theLight.setPos(light.getPos());
			theLight.setAmbient(new Point4d(light.getAmbient()[0], light.getAmbient()[1], light.getAmbient()[2], light.getAmbient()[3]));
			theLight.setDiffuse(new Point4d(light.getDiffuse()[0], light.getDiffuse()[1], light.getDiffuse()[2], light.getDiffuse()[3]));
			theLight.setViewDependent(light.isViewDependent());
			theLights.add(theLight);
		}
		if (this.renderWrapper.getMaplyRender() != null) {
			this.renderWrapper.getMaplyRender().replaceLights(theLights);
			//this.renderWrapper.getMaplyRender().render(); // needed?
		}

		// Clean up lights
		for (DirectionalLight light : theLights)
			light.dispose();
	}

	/**
	 * Clear all the currently active lights.
	 * <br>
	 * There are a default set of lights, so you'll want to do this before adding your own.
	 */
	public void clearLights() {
		if (!rendererAttached) {
			addPostSurfaceRunnable(new Runnable() {
				@Override
				public void run() {
					clearLights();
				}
			});
			return;
		}

		this.lights = new ArrayList<>();
		this.updateLights();
	}

	/**
	 * Reset the lighting back to its default state at startup.
	 * <br>
	 * This clears out all the lights and adds in the default starting light source.
	 */
	public void resetLights() {
		if (!rendererAttached) {
			addPostSurfaceRunnable(new Runnable() {
				@Override
				public void run() {
					if (running)
						resetLights();
				}
			});
			return;
		}

		this.clearLights();

		Light light = new Light();
		light.setPos(new Point3d(0.75, 0.5, -1.0));
		light.setAmbient(0.6f, 0.6f, 0.6f, 1.0f);
		light.setDiffuse(0.5f, 0.5f, 0.5f, 1.0f);
		light.setViewDependent(false);
		this.addLight(light);
	}

	public ComponentObject addBillboards(final List<Billboard> bills, final BillboardInfo info, final ThreadMode threadMode) {
		if (!running)
			return null;

		final ComponentObject compObj = addComponentObj();

		// Do the actual work on the layer thread
		Runnable run =
				new Runnable()
				{
					@Override
					public void run()
					{
						ChangeSet changes = new ChangeSet();
						long shaderID = 0;
						String shaderName = info.getShaderName();
						if (shaderName == Billboard.MAPLY_BILLBOARD_ORIENTE_EYE)
							shaderID = scene.getProgramIDBySceneName("Default Billboard eye");
						else if (shaderName == Billboard.MAPLY_BILLBOARD_ORIENTE_GROUND)
							shaderID = scene.getProgramIDBySceneName("Default Billboard ground");
						else
							shaderID = scene.getProgramIDBySceneName(shaderName);

						for (Billboard bill : bills) {
							// Convert to display space
							Point3d center = bill.getCenter();
							Point3d localPt =coordAdapter.getCoordSystem().geographicToLocal(new Point3d(center.getX(),center.getY(),0.0));
							Point3d dispTmp =coordAdapter.localToDisplay(localPt);
							Point3d dispPt = dispTmp.multiplyBy(center.getZ()/6371000.0+1.0);
							bill.setCenter(dispPt);

							if (bill.getSelectable()) {
								bill.setSelectID(Identifiable.genID());
								addSelectableObject(bill.getSelectID(), bill, compObj);
							}

							// Turn any screen objects into billboard polygons
							bill.flatten();
						}

						long billId = billboardManager.addBillboards(bills, info,shaderID, changes);
						compObj.addBillboardID(billId);

						// Flush the text changes
						if (scene != null)
							changes.process(scene);

						if (info.disposeAfterUse || disposeAfterRemoval)
							for (Billboard bill : bills)
								if (!bill.getSelectable())
									bill.dispose();
					}
				};

		addTask(run, threadMode);

		return compObj;
	}
}
