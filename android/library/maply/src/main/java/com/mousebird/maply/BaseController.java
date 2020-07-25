package com.mousebird.maply;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.ConfigurationInfo;
import android.content.pm.PackageInfo;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.graphics.drawable.ColorDrawable;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import java.io.IOException;
import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.ThreadPoolExecutor;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLSurface;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.Dispatcher;
import okhttp3.MediaType;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.RequestBody;
import okhttp3.Response;

/**
 * The base controller is a base class for both Maply and WhirlyGlobe controllers.
 * <p>
 * Most of the functionality is shared between the 2D and 3D maps and so it is
 * implemented here.
 * 
 * @author sjg
 *
 */
public class BaseController implements RenderController.TaskManager, RenderControllerInterface
{
	// This may be a GLSurfaceView or a GLTextureView
	View baseView = null;
	public Activity activity = null;
    public OkHttpClient httpClient;


	/**
	 * Listener to receive the screenshot in an asynchronous way.
	*/
	public interface ScreenshotListener {
		public void onScreenshotResult(Bitmap screenshot);
	}

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
	private boolean disposeAfterRemoval = false;
	
	// Set when we're not in the process of shutting down
	protected boolean running = false;

	// Implements the GL renderer protocol
	protected RendererWrapper renderWrapper;

	// Coordinate system to display conversion
	protected CoordSystemDisplayAdapter coordAdapter;
	
	// Scene stores the objects
	public Scene scene = null;

    /**
     * Return the current scene.  Only for sure within the library.
     */
    public Scene getScene()
    {
        return scene;
    }

	/**
	 * Return the current coordinate system.
	 */
	public CoordSystem getCoordSystem() { return coordAdapter.coordSys; }

	public void takeScreenshot(ScreenshotListener listener)
	{
		if (baseView instanceof GLTextureView) {
			GLTextureView textureView = (GLTextureView) baseView;

			listener.onScreenshotResult(textureView.getBitmap());
		} else {
			GLSurfaceView surfaceView = null;

			if (baseView instanceof GLSurfaceView) {
				surfaceView = (GLSurfaceView) baseView;
			}

			this.renderWrapper.takeScreenshot(listener, surfaceView);
		}
	}

    /**
     * Return an HTTP Client for use in fetching data, probably tiles.
     */
    public synchronized OkHttpClient getHttpClient()
	{
		if (httpClient == null) {
			httpClient = new OkHttpClient();

			// This little dance lets the OKHttp client shutdown and then reject any random calls
			// we may send its way
			Dispatcher dispatch = httpClient.dispatcher();
			try {
				if (dispatch != null) {
					ExecutorService service = dispatch.executorService();
					if (service != null) {
						ThreadPoolExecutor exec = (ThreadPoolExecutor) service;
						exec.setRejectedExecutionHandler(new ThreadPoolExecutor.DiscardPolicy());
					}
				}
			}
			catch (Exception e)
			{
				Log.e("Maply","OkHttp discard policy change no longer working.");
			}
		}
		return httpClient;
	}
	
	// MapView defines how we're looking at the data
	protected com.mousebird.maply.View view = null;

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
	 * Utility routine to run a task on the main thread.
	 * Doesn't do anything clever, but it does end up looking very simple
	 * in Kotlin.
	 */
	public void addMainThreadTask(Runnable run) {
		Handler handler = new Handler(activity.getMainLooper());
		handler.post(run);
	}

	/**
	 * Utility routine to run a task on the main thread after
	 * a period of time.
	 * Doesn't do anything clever, but it does end up looking very simple
	 * in Kotlin.
	 */
	public void addMainThreadTaskAfter(double when,Runnable run) {
		Handler handler = new Handler(activity.getMainLooper());
		handler.postDelayed(run, (long)(when * 1000.0));
	}

	/**
	 * Activity for the whole app.
     */
	public Activity getActivity() { return activity; }

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

		if (numAvailable == 0)
			return null;

		if (numAvailable == 1)
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
		/**
		 * If set, we'll use a different library name rather than the default.
		 * Super special option.  You probably don't need that.
		 */
		public String loadLibraryName = null;
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
	 * The render controller handles marshalling objects and the actual run loop.
	 */
	public RenderController renderControl = null;

	/**
	 * The underlying render controller.  Only get this if you know what it does.
	 */
	public RenderController getRenderController() {
		return renderControl;
	}

	/**
	 * Load in the shared C++ library if needed.
	 */
	String loadLibraryName = "whirlyglobemaply";

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
	public BaseController(Activity mainActivity,Settings settings)
	{
		// Note: Can't pull this one in anymore in Android Studio.  Hopefully not still necessary
//		System.loadLibrary("gnustl_shared");
		if (settings != null && settings.loadLibraryName != null)
			loadLibraryName = settings.loadLibraryName;
		System.loadLibrary(loadLibraryName);
		libraryLoaded = true;

		activity = mainActivity;
		if (settings != null) {
			useTextureView = !settings.useSurfaceView;
			numWorkingThreads = settings.numWorkingThreads;
			width = settings.width;
			height = settings.height;
		}

		renderControl = new RenderController();
	}

	ColorDrawable tempBackground = null;
	
	protected void Init()
	{
		if (!libraryLoaded)
		{
//			System.loadLibrary("gnustl_shared");
			System.loadLibrary(loadLibraryName);
			libraryLoaded = true;
		}

		// Now for the object that kicks off the rendering
		renderWrapper = new RendererWrapper(this, renderControl);
		renderWrapper.scene = scene;
		renderWrapper.view = view;

		renderControl.Init(scene, coordAdapter,this);

		// Create the default layer thread
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
				if (Color.alpha(renderControl.clearColor) < 255) {
					glSurfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
					glSurfaceView.getHolder().setFormat(PixelFormat.TRANSLUCENT);
					glSurfaceView.setZOrderOnTop(true);
				} else {
					if (isProbablyEmulator())
						glSurfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
				}

				tempBackground = new ColorDrawable();
				// This eliminates the black flash, but only if the clearColor is set right
				tempBackground.setColor(renderControl.clearColor);
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
				if (Color.alpha(renderControl.clearColor) < 255) {
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
				tempBackground.setColor(renderControl.clearColor);
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

		startAnalytics();
	}

	// Kick off the analytics logic.
	private void startAnalytics()
	{
		SharedPreferences prefs = activity.getSharedPreferences("WGMaplyPrefs", Context.MODE_PRIVATE);

		// USER ID is a random string.  Only used for deconfliction.  No unique information here.
		String userID = prefs.getString("wgmaplyanalyticuser", null);
		if (userID == null) {
			UUID uuid = UUID.randomUUID();
			userID = uuid.toString();
			SharedPreferences.Editor editor = prefs.edit();
			editor.putString("wgmaplyanalyticuser", userID);
			editor.apply();
		}

		// Send it once a month at most
		long lastSent = prefs.getLong("wgmaplyanalytictime2", 0);
		final long now = new Date().getTime()/1000;
		final long howLong = now - lastSent;
		if (howLong > 30*24*60*60) {
			PackageInfo pInfo;
			try {
				pInfo = activity.getPackageManager().getPackageInfo(activity.getPackageName(), 0);
			}
			catch (Exception e) {
				return;
			}

			// We're not recording anything that can identify the user, just the app
			// create table register( userid VARCHAR(50), bundleid VARCHAR(100), bundlename VARCHAR(100), bundlebuild VARCHAR(100), bundleversion VARCHAR(100), osversion VARCHAR(20), model VARCHAR(100), wgmaplyversion VARCHAR(20));
			String bundleID = activity.getPackageName();
			String bundleName = pInfo.packageName;
			String bundleBuild = "unknown";
			String bundleVersion = pInfo.versionName;
			String osversion = "Android " + Build.VERSION.RELEASE.toString();
			String model = Build.MANUFACTURER + " " + Build.MODEL;
			String wgMaplyVersion = "3.0";
			String json = String.format(
					"{ \"userid\":\"%s\", \"bundleid\":\"%s\", \"bundlename\":\"%s\", \"bundlebuild\":\"%s\", \"bundleversion\":\"%s\", \"osversion\":\"%s\", \"model\":\"%s\", \"wgmaplyversion\":\"%s\" }",
					userID, bundleID, bundleName, bundleBuild, bundleVersion, osversion, model, wgMaplyVersion);

			RequestBody body = RequestBody.create(MediaType.parse("application/json"),json);
			if (body == null)
				return;
			Request request = new Request.Builder()
					.url("http://analytics.mousebirdconsulting.com:8081/register")
					.post(body)
					.build();
			if (request == null)
				return;

			OkHttpClient client = new OkHttpClient();
			client.newCall(request).enqueue(new Callback() {
				@Override
				public void onFailure(Call call, IOException e) {
				}

				@Override
				public void onResponse(Call call, Response response) throws IOException {
					// We got a response, so save that in prefs
					if (activity != null) {
						SharedPreferences prefs = activity.getSharedPreferences("WGMaplyPrefs", Context.MODE_PRIVATE);
						SharedPreferences.Editor editor = prefs.edit();
						editor.putLong("wgmaplyanalytictime2", now);
						editor.apply();
					}
				}
			});
		}
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
		newLayerThread.setRenderer(renderControl);

		if (handlesViewUpdates)
			newLayerThread.viewUpdated(view);

		return newLayerThread;
	}

	/**
	 * Remove a layer thread you had created earlier.
	 * This is serious stuff.  Only do this if you know why you're doing it.
	 */
	public void removeLayerThread(LayerThread layerThread)
	{
		// TODO: Check we're on the main thread here

		synchronized (layerThreads) {
			if (!layerThreads.contains(layerThread))
				return;
		}

		layerThread.shutdown();
		synchronized (layerThreads) {
			layerThreads.remove(layerThread);
		}
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

			if (renderWrapper != null)
			     renderWrapper.stopRendering();

			// Shut down the tile fetchers
			for (RemoteTileFetcher tileFetcher : tileFetchers)
				tileFetcher.shutdown();
			tileFetchers.clear();

			//		Choreographer.getInstance().removeFrameCallback(this);
			ArrayList<LayerThread> layerThreadsToRemove = null;
			if (layerThreads != null) {
				synchronized (layerThreads) {
					layerThreadsToRemove = new ArrayList<LayerThread>(layerThreads);
				}
				for (LayerThread layerThread : layerThreadsToRemove)
					layerThread.shutdown();
				synchronized (layerThreads) {
					layerThreads.clear();
				}
			}

			if (metroThread != null) {
				metroThread.shutdown();
				metroThread = null;
			}


			if (scene != null) {
				scene.teardownGL();
				scene.shutdown();
			}

			//		scene.dispose();
			//		view.dispose();

			if (coordAdapter != null)
				coordAdapter.shutdown();
			if (renderControl != null)
				renderControl.shutdown();

			// Shut down the contexts
			EGL10 egl = (EGL10) EGLContext.getEGL();
			if (glContexts != null) {
				for (ContextInfo context : glContexts) {
					egl.eglDestroySurface(renderControl.display, context.eglSurface);
					egl.eglDestroyContext(renderControl.display, context.eglContext);
				}
				glContexts = null;

			}

			// And the main one
			if (renderWrapper != null && renderWrapper.maplyRender != null && glContext != null) {
				egl.eglDestroySurface(renderControl.display, glContext.eglSurface);
				egl.eglDestroyContext(renderControl.display, glContext.eglContext);
				glContext = null;
			}

			// Clean up OpenGL ES resources
			setEGLContext(null);

			if (renderWrapper != null)
				renderWrapper.shutdown();

			if (httpClient != null)
			{
				if (httpClient.dispatcher() != null && httpClient.dispatcher().executorService() != null)
					httpClient.dispatcher().executorService().shutdown();
				// Note: This code can't be run on the main thread, but now is not the time
				//       to be spinning up an AsyncTask, so we just hope for the best
//				if (httpClient.connectionPool() != null) {
//					httpClient.connectionPool().evictAll();
//				}
				httpClient = null;
			}

			baseView = null;
			renderWrapper = null;
			coordAdapter = null;
			scene = null;
			view = null;
			// Using this as a sync object, so not a great idea
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
    static final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;

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
	public ContextInfo setupTempContext(RenderController.ThreadMode threadMode)
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
					retContext.eglContext = egl.eglCreateContext(renderControl.display,renderControl.config,renderControl.context, attrib_list);
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
					retContext.eglSurface = egl.eglCreatePbufferSurface(renderControl.display, renderControl.config, surface_attrs);

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

	public void clearTempContext(ContextInfo cInfo)
	{
		if (cInfo == null)
			return;

		synchronized (glContexts)
		{
			if (cInfo != glContext) {
				EGL10 egl = (EGL10) EGLContext.getEGL();
//				GLES20.glFlush();
//				GLES20.glFinish();
				if (!egl.eglMakeCurrent(renderControl.display, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT))
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
		boolean runNow = false;
		synchronized (this) {
			if (rendererAttached)
				runNow = true;
			else
				postSurfaceRunnables.add(run);
		}
		if (runNow) {
			Handler handler = new Handler(activity.getMainLooper());
			handler.post(run);
		}
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

	// Layout layer runs periodically to poke the layout manager
	LayoutLayer layoutLayer = null;

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
					layerThread.setRenderer(renderControl);
			}

			// Debugging output
			renderControl.setPerfInterval(perfInterval);

			// Kick off the layout layer
			layoutLayer = new LayoutLayer(this, renderControl.layoutManager);
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
			metroThread.setRenderer(renderControl);

			// Make our own context that we can use on the main thread
			EGL10 egl = (EGL10) EGLContext.getEGL();
			int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE};
			glContext = new ContextInfo();
			glContext.eglContext = egl.eglCreateContext(renderControl.display, renderControl.config, renderControl.context, attrib_list);
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
			glContext.eglSurface = egl.eglCreatePbufferSurface(renderControl.display, renderControl.config, surface_attrs);

			synchronized (layerThreads) {
				for (LayerThread layerThread : layerThreads)
					layerThread.viewUpdated(view);
			}

			setClearColor(renderControl.clearColor);

			// Register the shaders
			renderControl.setupShadersNative();

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
    public boolean setEGLContext(ContextInfo cInfo)
    {
		if (cInfo == null)
			cInfo = glContext;

		// This does seem to happen
		if (renderWrapper == null || renderWrapper.maplyRender == null || renderControl.display == null)
			return false;

		EGL10 egl = (EGL10) EGLContext.getEGL();
        if (cInfo != null)
        {
            if (!egl.eglMakeCurrent(renderControl.display, cInfo.eglSurface, cInfo.eglSurface, cInfo.eglContext)) {
                Log.d("Maply", "Failed to make current context.");
                return false;
            }

            return true;
        } else {
			if (renderWrapper != null && renderWrapper.maplyRender != null && renderControl.display != null)
				egl.eglMakeCurrent(renderControl.display, egl.EGL_NO_SURFACE, egl.EGL_NO_SURFACE, egl.EGL_NO_CONTEXT);
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

	/**
	 * Set the color for the OpenGL ES background.
     */
	public void setClearColor(int color)
	{
		renderControl.setClearColor(color);
	}

	/**
	 * Set the viewport the user is allowed to move within.  These are lat/lon coordinates
	 * in radians.
	 * @param ll Lower left corner.
	 * @param ur Upper right corner.
	 */
	public void setViewExtents(final Point2d ll,final Point2d ur)
	{
		if (!running || view == null || renderWrapper == null || renderWrapper.maplyRender == null || renderControl.frameSize == null) {
			addPostSurfaceRunnable(new Runnable() {
				@Override
				public void run() {
					setViewExtents(ll,ur);
				}
			});
			return;
		}

		CoordSystemDisplayAdapter coordAdapter = view.getCoordAdapter();
		if (coordAdapter == null)
			return;
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
		if (!running || view == null || renderWrapper == null || renderWrapper.maplyRender == null || renderControl.frameSize == null)
			return false;
		Point2d frameSize = renderControl.frameSize;

		return coordAdapter.screenPointFromGeoBatch(view,(int)frameSize.getX(),(int)frameSize.getY(),
				inX,inY,inZ,outX,outY);
	}

	/**
	 * Batch version of the geoPointFromScreen method.  This version is here for users to
	 * convert a whole group of coordinates all at once.  Doing it individually is just
	 * too slow in Java.
	 */
	public boolean geoPointFromScreenBatch(double[] inX,double[] inY,double[] outX,double[] outY)
	{
		if (!running || view == null || renderWrapper == null || renderWrapper.maplyRender == null || renderControl.frameSize == null)
			return false;
		Point2d frameSize = renderControl.frameSize;

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
			renderControl.setPerfInterval(perfInterval);
	}

	/** Calculate the height that corresponds to a given Mapnik-style map scale.
	 * <br>
	 * Figure out the viewer height that corresponds to a given scale denominator (ala Mapnik).
	 * This height will probably be use for visibility ranges on geometry.  This works as a mechanism for making geometry appear at certain map scales and disappear at others.
	 * @return Returns the height or 0.0 if the system isn't initialized yet.
	 */
	public double heightForMapScale(double scale)
	{
		if (!running || view == null || renderWrapper == null || renderWrapper.maplyRender == null || renderControl.frameSize == null)
			return 0.0;

		return renderControl.heightForMapScale(scale);
	}

	/** Return the current map zoom from the viewpoint.
	 * <br>
	 * Calculate the map zoom (TMS) based on the current screen size and the 3D viewport.
	 * @param geoCoord the location to calculate for. This is needed because zoom is dependant on latitude.
	 * @return Returns the map zoom or 0.0 if the system is not yet initialized.
	 */
	public double currentMapZoom(Point2d geoCoord)
	{
		if (!running || view == null || renderWrapper == null || renderWrapper.maplyRender == null || renderControl.frameSize == null)
			return 0.0;

		return renderControl.currentMapZoom(geoCoord);
	}

	/** Return the current scale denominator (Mapnik).
	 * <br>
	 * See https://github.com/mapnik/mapnik/wiki/ScaleAndPpi for a definition of the Mapnik scale denominator.
	 * @return Returns the current scale denominator or 0.0 if the system is not yet initialized.
	 */
	public double currentMapScale()
	{
		if (!running || view == null || renderWrapper == null || renderWrapper.maplyRender == null || renderControl.frameSize == null)
			return 0.0;

		return renderControl.currentMapScale();
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

	ArrayList<QuadSamplingLayer> samplingLayers = new ArrayList<QuadSamplingLayer>();

	/**
	 * Look for a sampling layer that matches the parameters given.
	 * Sampling layers can be shared for efficiency.  Don't be calling this yourself.
	 * The loaders do it for you.
	 */
	public QuadSamplingLayer findSamplingLayer(SamplingParams params,final QuadSamplingLayer.ClientInterface user)
	{
		QuadSamplingLayer theLayer = null;

		for (QuadSamplingLayer layer : samplingLayers) {
			if (layer.params.equals(params)) {
				theLayer = layer;
				break;
			}
		}

		if (theLayer == null) {
			// Set up the sampling layer
			theLayer = new QuadSamplingLayer(this,params);
			samplingLayers.add(theLayer);

			// On its own layer thread
			LayerThread layerThread = makeLayerThread(true);
			theLayer.layerThread = layerThread;
			layerThread.addLayer(theLayer);
		}

		// Do the addClient on the layer thread
		final QuadSamplingLayer delayLayer = theLayer;
		theLayer.layerThread.addTask(new Runnable() {
			@Override
			public void run() {
				delayLayer.addClient(user);
			}
		});

		return theLayer;
	}

	/**
	 * Release the given sampling layer
	 * @param samplingLayer
	 * @param user
	 */
	public void releaseSamplingLayer(final QuadSamplingLayer samplingLayer,final QuadSamplingLayer.ClientInterface user)
	{
		if (!samplingLayers.contains(samplingLayer))
			return;

		// Do the remove client on the layer thread
		samplingLayer.layerThread.addTask(new Runnable() {
			@Override
			public void run() {
				samplingLayer.removeClient(user);

				// Get rid of the sampling layer too
				if (samplingLayer.getNumClients() == 0) {
					// But that has to be done on the main thread
					if (samplingLayer.control.get() != null) {
						Handler handler = new Handler(samplingLayer.control.get().activity.getMainLooper());
						handler.post(new Runnable() {
							@Override
							public void run() {
								// Someone maybe started using it
								if (samplingLayer.getNumClients() == 0) {
									removeLayerThread(samplingLayer.layerThread);
									samplingLayers.remove(samplingLayer);
								}
							}
						});
					}
				}
			}
		});
	}

	ArrayList<RemoteTileFetcher> tileFetchers = new ArrayList<RemoteTileFetcher>();

	/**
	 * Either returns a RemoteTileFetcher with the given name or
	 * it creates one and then returns the same.
	 */
	public RemoteTileFetcher addTileFetcher(String name)
	{
		for (RemoteTileFetcher tileFetcher : tileFetchers) {
			if (tileFetcher.getFetcherName() == name)
				return tileFetcher;
		}

		RemoteTileFetcher tileFetcher = new RemoteTileFetcher(this,name);
		tileFetchers.add(tileFetcher);

		return tileFetcher;
	}
	
	/**
	 * Add a task according to the thread mode.  If it's ThreadAny, we'll put it on the layer thread.
	 * If it's ThreadCurrent, we'll do it immediately.
	 * 
	 * @param run Runnable to execute.
	 * @param mode Where to execute it.
	 */
	public void addTask(final Runnable run,final RenderController.ThreadMode mode)
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
		if (mode == RenderController.ThreadMode.ThreadCurrent) {

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
					if (!egl.eglMakeCurrent(renderControl.display,oldDrawSurface,oldReadSurface,oldContext))
					{
						Log.d("Maply","Failed to set context back to previous context.");
					}
			}
        } else
			baseLayerThread.addTask(run,true);
	}

    /**
     * Add a single VectorObject.  See addVectors() for details.
     */
    public ComponentObject addVector(final VectorObject vec,final VectorInfo vecInfo,RenderController.ThreadMode mode)
    {
        ArrayList<VectorObject> vecObjs = new ArrayList<VectorObject>();
        vecObjs.add(vec);
        return addVectors(vecObjs,vecInfo,mode);
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
	public ComponentObject addVectors(final List<VectorObject> vecs,final VectorInfo vecInfo,RenderController.ThreadMode mode)
	{
		if (!running)
			return null;

		return renderControl.addVectors(vecs,vecInfo,mode);
	}

	/**
	 * Instance an existing set of vectors and modify various parameters for reuse.
	 * This is useful if you want to overlay the same vectors twice with different widths,
	 * for example.
	 */
	public ComponentObject instanceVectors(final ComponentObject inCompObj,final VectorInfo vecInfo,RenderController.ThreadMode mode)
	{
		return renderControl.instanceVectors(inCompObj,vecInfo,mode);
	}

	/**
	 * Change the visual representation of the given vectors.
	 * @param vecObj The component object returned by the original addVectors() call.
	 * @param vecInfo Visual representation to use for the changes.
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
     */
	public void changeVector(final ComponentObject vecObj,final VectorInfo vecInfo,RenderController.ThreadMode mode)
	{
		if (!running)
			return;

		renderControl.changeVector(vecObj,vecInfo,mode);
	}

	/**
	 * Add wide vectors to the MaplyController to display.  Vectors are linear or areal
	 * features with line width, filled style, color and so forth defined by the
	 * WideVectorInfo class.
	 * <br>
	 * Wide vectors differ from regular lines in that they're implemented with a more
	 * complicated shader.  They can be arbitrarily large, have textures, and have a transparent
	 * falloff at the edges.  This makes them look anti-aliased.
	 *
	 * @param vecs A list of VectorObject's created by the user or read in from various sources.
	 * @param wideVecInfo A description of how the vectors should look.
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
	 * @return The ComponentObject representing the vectors.  This is necessary for modifying
	 * or deleting the vectors once created.
	 */
	public ComponentObject addWideVectors(final List<VectorObject> vecs,final WideVectorInfo wideVecInfo,RenderController.ThreadMode mode)
	{
		if (!running)
			return null;

		return renderControl.addWideVectors(vecs,wideVecInfo,mode);
	}

	/**
	 * Add wide vectors to the MaplyController to display.  Vectors are linear or areal
	 * features with line width, filled style, color and so forth defined by the
	 * WideVectorInfo class.
	 * <br>
	 * Wide vectors differ from regular lines in that they're implemented with a more
	 * complicated shader.  They can be arbitrarily large, have textures, and have a transparent
	 * falloff at the edges.  This makes them look anti-aliased.
	 *
	 * @param vec The vector object to turn into wide vectors.
	 * @param wideVecInfo A description of how the vectors should look.
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
	 * @return The ComponentObject representing the vectors.  This is necessary for modifying
	 * or deleting the vectors once created.
	 */
	public ComponentObject addWideVector(VectorObject vec,WideVectorInfo wideVecInfo,RenderController.ThreadMode mode)
	{
		ArrayList<VectorObject> vecObjs = new ArrayList<VectorObject>();
		vecObjs.add(vec);

		return addWideVectors(vecObjs,wideVecInfo,mode);
	}

	/**
	 * Instance an existing set of wide vectors but change their parameters.
	 * <br>
	 * Wide vectors can take up a lot of memory.  So if you want to display the same set with
	 * different parameters (e.g. width, color) this is the way to do it.
	 *
	 * @param inCompObj The Component Object returned by an addWideVectors call.
	 * @param wideVecInfo How we want the vectors to look.
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
     * @return The ComponentObject representing the instanced wide vectors.  This is necessary for modifying
	 * or deleting the instance once created.
     */
	public ComponentObject instanceWideVectors(final ComponentObject inCompObj,final WideVectorInfo wideVecInfo,RenderController.ThreadMode mode)
	{
		if (!running)
			return null;

		return renderControl.instanceWideVectors(inCompObj,wideVecInfo,mode);
	}

	/**
	 * Add a single Lofted Polygon.  See addLoftedPolys() for details.
	 */
	public ComponentObject addLoftedPoly(final VectorObject vec,final LoftedPolyInfo loftInfo,RenderController.ThreadMode mode)
	{
		ArrayList<VectorObject> vecObjs = new ArrayList<VectorObject>();
		vecObjs.add(vec);
		return addLoftedPolys(vecObjs,loftInfo,mode);
	}

	/**
	 * Add Lofted Polygons to the MaplyController to display.
	 * <br>
	 * Lofted polygons require areal features as outlines.  The result will be
	 * a tent-like visual with optional sides and a top.
	 *
	 * @param vecs A list of VectorObject's created by the user or read in from various sources.
	 * @param loftInfo A description of how the lofted polygons should look.
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
	 * @return The ComponentObject representing the vectors.  This is necessary for modifying
	 * or deleting the features once created.
	 */
	public ComponentObject addLoftedPolys(final List<VectorObject> vecs,final LoftedPolyInfo loftInfo,RenderController.ThreadMode mode)
	{
		if (!running)
			return null;

		return renderControl.addLoftedPolys(vecs,loftInfo,mode);
	}


	/**
	 * Add a single screen marker.  See addScreenMarkers() for details.
	 */
	public ComponentObject addScreenMarker(final ScreenMarker marker,final MarkerInfo markerInfo,RenderController.ThreadMode mode)
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
	public ComponentObject addScreenMarkers(final List<ScreenMarker> markers,final MarkerInfo markerInfo,RenderController.ThreadMode mode)
	{		
		if (!running)
			return null;

		return renderControl.addScreenMarkers(markers,markerInfo,mode);
	}

    /**
     * Add moving screen markers to the visual display.  These are the same as the regular
     * screen markers, but they have a start and end point and a duration.
     */
	public ComponentObject addScreenMovingMarkers(final List<ScreenMovingMarker> markers,final MarkerInfo markerInfo,RenderController.ThreadMode mode)
	{
		if (!running)
			return null;

        return renderControl.addScreenMovingMarkers(markers,markerInfo,mode);
	}

	/**
	 * Add a single screen marker.  See addMarkers() for details.
	 */
	public ComponentObject addMarker(final Marker marker,final MarkerInfo markerInfo,RenderController.ThreadMode mode)
	{
		if (!running)
			return null;

		ArrayList<Marker> markers = new ArrayList<Marker>();
		markers.add(marker);
		return addMarkers(markers,markerInfo,mode);
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
	public ComponentObject addMarkers(final List<Marker> markers,final MarkerInfo markerInfo,RenderController.ThreadMode mode)
	{
		if (!running)
			return null;

		return renderControl.addMarkers(markers,markerInfo,mode);
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
	public ComponentObject addStickers(final List<Sticker> stickers,final StickerInfo stickerInfo,RenderController.ThreadMode mode)
	{
		if (!running)
			return null;

		return renderControl.addStickers(stickers,stickerInfo,mode);
	}

	/**
	 * Change the visual representation for the given sticker.
	 *
	 * @param stickerObj The sticker to change.
	 * @param stickerInfo Parameters to change.
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
	 * @return This represents the stickers for later modification or deletion.
	 */
	public ComponentObject changeSticker(final ComponentObject stickerObj,final StickerInfo stickerInfo,RenderController.ThreadMode mode)
	{
		if (!running || stickerObj == null)
			return null;

		return renderControl.changeSticker(stickerObj,stickerInfo,mode);
	}

	/**
	 * Add the geometry points.  These are raw points that get fed to a shader.

	 * @param ptList The points to add.
	 * @param geomInfo Parameters to set things up with.
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
     * @return This represents the geometry points for later modifictation or deletion.
     */
	public ComponentObject addPoints(final List<Points> ptList,final GeometryInfo geomInfo,RenderController.ThreadMode mode)
	{
		if (!running)
			return null;

		return renderControl.addPoints(ptList,geomInfo,mode);
	}

	/**
	 * Add the geometry points.  These are raw points that get fed to a shader.

	 * @param pts The points to add.
	 * @param geomInfo Parameters to set things up with.
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
	 * @return This represents the geometry points for later modifictation or deletion.
	 */
	public ComponentObject addPoints(Points pts,final GeometryInfo geomInfo,RenderController.ThreadMode mode)
	{
		if (!running)
			return null;

		List<Points> ptList = new ArrayList<Points>();
		ptList.add(pts);

		return addPoints(ptList,geomInfo,mode);
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
		if (renderWrapper == null)
			return null;

		Point2d viewSize = getViewSize();
		Point2d frameSize = renderControl.frameSize;
		Point2d scale = new Point2d(frameSize.getX()/viewSize.getX(),frameSize.getY()/viewSize.getY());
		Point2d frameLoc = new Point2d(scale.getX()*screenLoc.getX(),scale.getY()*screenLoc.getY());

		// Ask the selection manager
		ViewState theViewState = view.makeViewState(renderControl);
		SelectedObject selManObjs[] = renderControl.selectionManager.pickObjects(renderControl.componentManager,theViewState, frameLoc);
		if (selManObjs != null)
			renderControl.componentManager.remapSelectableObjects(selManObjs);
		theViewState.dispose();

		Point2d geoPt = geoPointFromScreen(screenLoc);
		if (geoPt == null)
			return null;

		return selManObjs;
	}

	/**
	 * Returns an object (if any) at a given screen location
	 * @param screenLoc the screen location to be considered
	 * @return teh object at screenLoc or null if none was there
	 */
	protected Object getObjectAtScreenLoc(Point2d screenLoc)
	{
		if (renderWrapper == null)
			return null;

		Point2d viewSize = getViewSize();
		Point2d frameSize = renderControl.frameSize;
		Point2d scale = new Point2d(frameSize.getX()/viewSize.getX(),frameSize.getY()/viewSize.getY());
		Point2d frameLoc = new Point2d(scale.getX()*screenLoc.getX(),scale.getY()*screenLoc.getY());

		long selectID = renderControl.selectionManager.pickObject(view.makeViewState(renderControl), frameLoc);
		if (selectID != RenderController.EmptyIdentity)
		{
			return renderControl.componentManager.findObjectForSelectID(selectID);
		}

		return null;
	}

	/**
	 * Add a single screen label.  See addScreenLabels() for details.
	 */
	public ComponentObject addScreenLabel(ScreenLabel label,final LabelInfo labelInfo,RenderController.ThreadMode mode)
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
	public ComponentObject addScreenLabels(final List<ScreenLabel> labels,final LabelInfo labelInfo,RenderController.ThreadMode mode)
	{
		if (!running)
			return null;

		return renderControl.addScreenLabels(labels,labelInfo,mode);
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
	public ComponentObject addScreenMovingLabels(final List<ScreenMovingLabel> labels,final LabelInfo labelInfo,RenderController.ThreadMode mode)
	{
		if (!running)
			return null;

		return renderControl.addScreenMovingLabels(labels,labelInfo,mode);
	}

    /**
     * Add texture to the system with the given settings.
     * @param image Image to add.
     * @param settings Settings to use.
     * @param mode Add on the current thread or elsewhere.
     */
	public MaplyTexture addTexture(final Bitmap image,RenderController.TextureSettings settings,RenderController.ThreadMode mode)
    {
        if (renderControl == null)
            return null;
        if (settings == null)
        	settings = new RenderControllerInterface.TextureSettings();

        return renderControl.addTexture(image,settings,mode);
    }
    
    /**
	 * Create an empty texture of the given size.
	 * @param width Width of the resulting texture
	 * @param height Height of the resulting texture
	 * @param settings Other texture related settings
	 * @param mode Which thread to do the work on
     * @return The new texture (or a reference to it, anyway)
     */
	public MaplyTexture createTexture(final int width,final int height,RenderController.TextureSettings settings,RenderController.ThreadMode mode)
	{
        if (renderControl == null)
            return null;
		if (settings == null)
			settings = new RenderControllerInterface.TextureSettings();

		return renderControl.createTexture(width,height,settings,mode);
	}

	/**
	 * Add texture to the system with the given settings.
	 * @param rawTex Texture to add.
	 * @param settings Settings to use.
	 * @param mode Add on the current thread or elsewhere.
	 */
	public MaplyTexture addTexture(final Texture rawTex,final RenderController.TextureSettings settings,RenderController.ThreadMode mode)
	{
        if (renderControl == null)
            return null;

        return renderControl.addTexture(rawTex,settings,mode);
	}

	/**
	 * Remove a texture from the scene.
	 * @param tex Texture to remove.
	 * @param mode Remove immediately (current thread) or elsewhere.
     */
	public void removeTexture(final MaplyTexture tex,RenderController.ThreadMode mode)
	{
        ArrayList<MaplyTexture> texs = new ArrayList<MaplyTexture>();
        texs.add(tex);
        removeTextures(texs,mode);
	}

	/**
	 * Remove a whole group of textures from the scene.
	 * @param texs Textures to remove.
	 * @param mode Remove immediately (current thread) or elsewhere.
     */
	public void removeTextures(final List<MaplyTexture> texs,RenderController.ThreadMode mode)
	{
	    renderControl.removeTextures(texs,mode);
	}

	/**
	 * This version of removeTexture takes texture IDs.  Thus you don't
	 * have to keep the MaplyTexture around.
	 *
	 * @param texIDs Textures to remove
	 * @param mode Remove immediately (current thread) or elsewhere.
	 */
	public void removeTexturesByID(final List<Long> texIDs,RenderController.ThreadMode mode)
	{
	    renderControl.removeTexturesByID(texIDs,mode);
	}

	/** Add a render target to the system
	 * <br>
     * Sets up a render target and will start rendering to it on the next frame.
	 * Keep the render target around so you can remove it later.
	 */
	public void addRenderTarget(RenderTarget renderTarget)
	{
	    renderControl.addRenderTarget(renderTarget);
	}

	/**
	 * Point the render target at a different texture.
	 */
	public void changeRenderTarget(RenderTarget renderTarget, MaplyTexture tex)
	{
		if (renderTarget == null || tex == null)
			return;
		if (tex.texID == RenderController.EmptyIdentity) {
			return;
		}

		renderControl.changeRenderTarget(renderTarget,tex);
	}

	/** Remove the given render target from the system.
	 * <br>
	 * Ask the system to stop drawing to the given render target.  It will do this on the next frame.
	 */
	public void removeRenderTarget(RenderTarget renderTarget)
	{
	    renderControl.removeRenderTarget(renderTarget);
	}

    /**
     * Associate a shader with the given scene name.  These names let us override existing shaders, as well as adding our own.
     * @param shader The shader to add.
     */
    public void addShaderProgram(final Shader shader)
    {
		if (!rendererAttached) {
			addPostSurfaceRunnable(new Runnable() {
				@Override
				public void run() {
					addShaderProgram(shader);
				}
			});
			return;
		}

		renderControl.addShaderProgram(shader);
    }

	/**
	 * Find a shader by name
	 * @param name Name of the shader to return
	 * @return The shader with the name or null
	 */
	public Shader getShader(String name)
	{
	    return renderControl.getShader(name);
	}

	/**
	 * Take the given shader out of active use.
	 */
	public void removeShader(Shader shader)
	{
		renderControl.removeShader(shader);
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

        renderControl.addActiveObject(activeObject);
	}

	/**
	 * Add an active object to the beginning of the list.  Do this if you want to make sure
	 * yours is run first.
	 */
	public void addActiveObjectAtStart(final ActiveObject activeObject) {
		if (!rendererAttached) {
			addPostSurfaceRunnable(new Runnable() {
				@Override
				public void run() {
					addActiveObjectAtStart(activeObject);
				}
			});
			return;
		}

		renderControl.addActiveObjectAtStart(activeObject);
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

		renderControl.removeActiveObject(activeObject);
	}

	/**
	 * Disable the given objects. These were the objects returned by the various
	 * add calls.  Once called, the objects will be invisible, but can be made
	 * visible once again with enableObjects()
	 * 
	 * @param compObjs Objects to disable in the display.
	 * @param mode Where to execute the add.  Choose ThreadAny by default.
	 */
	public void disableObjects(final List<ComponentObject> compObjs,RenderController.ThreadMode mode)
	{
		if (!running)
			return;

		renderControl.disableObjects(compObjs,mode);
	}

	/**
	 * Disable the display for the given object.
	 *
	 * @param compObj Object to disable
	 * @param mode Where to execute the enable.  Choose ThreadAny by default.
	 */
	public void disableObject(ComponentObject compObj,RenderController.ThreadMode mode)
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
	public void enableObjects(final List<ComponentObject> compObjs,RenderController.ThreadMode mode)
	{
		if (!running)
			return;

		renderControl.enableObjects(compObjs,mode);
	}

	/**
	 * Enable the display for the given object.
	 *
	 * @param compObj Object to enable.
	 * @param mode Where to execute the enable.  Choose ThreadAny by default.
     */
	public void enableObject(ComponentObject compObj,RenderController.ThreadMode mode)
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
	public void removeObject(final ComponentObject compObj,RenderController.ThreadMode mode)
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
	public void removeObjects(final List<ComponentObject> compObjs,RenderController.ThreadMode mode)
	{
		if (!running)
			return;

		if (compObjs == null || compObjs.size() == 0)
			return;

		renderControl.removeObjects(compObjs,mode);
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
	public ComponentObject addParticleSystem(final ParticleSystem particleSystem, RenderController.ThreadMode mode) {
		if (!running)
			return null;

		final ComponentObject compObj = renderControl.componentManager.makeComponentObject();
		final ChangeSet changes = new ChangeSet();

		Runnable run = new Runnable() {
			@Override
			public void run() {
				long particleSystemID = renderControl.particleSystemManager.addParticleSystem(particleSystem, changes);
				if (particleSystemID != RenderController.EmptyIdentity) {
					compObj.addParticleSystemID(particleSystemID);
				}
				if (scene != null) {
					changes.process(renderControl, scene);
					changes.dispose();
				}
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
	public void addParticleBatch(final ParticleBatch particleBatch, RenderController.ThreadMode mode) {
		if (!running)
			return;

		if (particleBatch.isValid()) {
			Runnable run = new Runnable() {
				@Override
				public void run() {
					ChangeSet changes = new ChangeSet();
					renderControl.particleSystemManager.addParticleBatch(particleBatch.partSys.getID(), particleBatch,changes);
					if (scene != null) {
						changes.process(renderControl, scene);
						changes.dispose();
					}
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
	public ComponentObject addShapes(final List<Shape> shapes, final ShapeInfo shapeInfo, RenderController.ThreadMode mode) {
		if (!running)
			return null;

		return renderControl.addShapes(shapes,shapeInfo,mode);
	}

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

		renderControl.addLight(light);
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

		renderControl.removeLight(light);
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

		renderControl.clearLights();
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

		renderControl.resetLights();
	}

    /**
     * Billboards are rectangles pointed toward the viewer.  They can either be upright, tied to a
     * surface, or oriented completely toward the user.
     */
	public ComponentObject addBillboards(final List<Billboard> bills, final BillboardInfo info, final RenderController.ThreadMode threadMode) {
		if (!running)
			return null;

		return renderControl.addBillboards(bills,info,threadMode);
	}

	/**
	 * Returns the maximum line width on the current device.
	 */
	public float getMaxLineWidth()
	{
		setEGLContext(glContext);

		float widths[] = new float[2];
		GLES20.glGetFloatv(GLES20.GL_ALIASED_LINE_WIDTH_RANGE, widths, 0);

		setEGLContext(null);

		return widths[1];
	}

	/**
	 * Return the frame size we're rendering to.
	 */
	public Point2d getFrameSize()
	{
		if (renderWrapper == null || renderWrapper.maplyRender == null) {
			return null;
		}

		int[] frameSize = renderControl.getFrameBufferSize();
		return new Point2d(frameSize[0],frameSize[1]);
	}

	/**
	 * Return the current framebuffer size as ints.
	 */
	public int[] getFrameBufferSize()
	{
		return renderControl.getFrameBufferSize();
	}

	public void processChangeSet(ChangeSet changes)
	{
		changes.process(renderControl, scene);
		changes.dispose();
	}
}
