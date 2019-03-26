package com.mousebird.maply;

import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.graphics.drawable.ColorDrawable;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;

/**
 * The Render Controller handles the object manipulation and rendering interface.
 * It can be a standalone object, doing offline rendering or it can be attached
 * to a BaseController and used to manage that controller's rendering.
 */
public class RenderController implements RenderControllerInterface
{
    // Draw priority defaults
    public static final int ImageLayerDrawPriorityDefault = 100;
    public static final int FeatureDrawPriorityBase = 20000;
    public static final int MarkerDrawPriorityDefault = 40000;
    public static final int LabelDrawPriorityDefault = 60000;
    public static final int ParticleDrawPriorityDefault = 1000;

    

    // Represents an ID that doesn't have data associated with it
    public static long EmptyIdentity = 0;

    /**
     * Enumerated values for image types.
     */
    public enum ImageFormat {MaplyImageIntRGBA,
        MaplyImageUShort565,
        MaplyImageUShort4444,
        MaplyImageUShort5551,
        MaplyImageUByteRed,MaplyImageUByteGreen,MaplyImageUByteBlue,MaplyImageUByteAlpha,
        MaplyImageUByteRGB,
        MaplyImageETC2RGB8,MaplyImageETC2RGBA8,MaplyImageETC2RGBPA8,
        MaplyImageEACR11,MaplyImageEACR11S,MaplyImageEACRG11,MaplyImageEACRG11S,
        MaplyImage4Layer8Bit};

    public Point2d frameSize = new Point2d();

    // Set if we're using a TextureView rather than a SurfaceView
    boolean useTextureView = false;

    /**
     * If set, we'll explicitly call dispose on any objects that were
     * being kept around for selection.
     */
    public boolean disposeAfterRemoval = false;

    // Set when we're not in the process of shutting down
    boolean running = false;

    // Implements the GL renderer protocol
    protected RendererWrapper renderWrapper;

    /**
     * Returns true if we set up a TextureView rather than a SurfaceView.
     */
    public boolean usesTextureView()
    {
        return useTextureView;
    }

    RenderController(Settings settings)
    {
        if (settings != null) {
            useTextureView = !settings.useSurfaceView;
            numWorkingThreads = settings.numWorkingThreads;
            width = settings.width;
            height = settings.height;
        }
    }

    public void Init()
    {

        // Fire up the managers.  Can't do anything without these.
        vecManager = new VectorManager(scene);
        wideVecManager = new WideVectorManager(scene);
        markerManager = new MarkerManager(scene);
        stickerManager = new StickerManager(scene);
        labelManager = new LabelManager(scene);
        layoutManager = new LayoutManager(scene);
        selectionManager = new SelectionManager(scene);
        componentManager = new ComponentManager(scene);
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
    }

    public boolean surfaceChanged(int width,int height)
    {
        frameSize.setValue(width, height);
        return resize(width,height);
    }

    View view = null;
    public void setView(View inView)
    {
        view = inView;
        setViewNative(inView);
    }

    ArrayList<ActiveObject> activeObjects = new ArrayList<ActiveObject>();

    /**
     * Add an active object that will be called right before the render (on the render thread).
     */
    void addActiveObject(ActiveObject activeObject)
    {
        synchronized (activeObjects) {
            activeObjects.add(activeObject);
        }
    }

    /**
     * Remove an active object added earlier.
     */
    void removeActiveObject(ActiveObject activeObject)
    {
        synchronized (activeObjects) {
            activeObjects.remove(activeObject);
        }
    }

    /**
     * Check if any of the active objects have changes for the next frame.
     */
    public boolean activeObjectsHaveChanges()
    {
        synchronized (activeObjects) {
            for (ActiveObject activeObject : activeObjects)
                if (activeObject.hasChanges())
                    return true;
        }

        return false;
    }

    public void doRender()
    {
        if (view != null)
            view.animate();

        // Run anyone who wants updates
        synchronized (activeObjects) {
            for (ActiveObject activeObject : activeObjects)
                activeObject.activeUpdate();
        }

        render();
    }

    public EGLDisplay display = null;
    public EGLConfig config = null;
    public EGLContext context = null;
    public void setConfig(EGLConfig inConfig)
    {
        config = inConfig;
        EGL10 egl = (EGL10) EGLContext.getEGL();
        display = egl.eglGetCurrentDisplay();
        context = egl.eglGetCurrentContext();
    }


    // MapView defines how we're looking at the data
    protected com.mousebird.maply.View view = null;

    // Managers are thread safe objects for handling adding and removing types of data
    protected VectorManager vecManager;
    protected WideVectorManager wideVecManager;
    protected MarkerManager markerManager;
    protected StickerManager stickerManager;
    protected LabelManager labelManager;
    protected SelectionManager selectionManager;
    protected ComponentManager componentManager;
    protected LayoutManager layoutManager;
    protected ParticleSystemManager particleSystemManager;
    protected LayoutLayer layoutLayer = null;
    protected ShapeManager shapeManager = null;
    protected BillboardManager billboardManager = null;
    protected GeometryManager geomManager = null;

    // Manage bitmaps and their conversion to textures
    TextureManager texManager = new TextureManager();

    // Layer thread we use for data manipulation
    ArrayList<LayerThread> layerThreads = new ArrayList<LayerThread>();
    ArrayList<LayerThread> workerThreads = new ArrayList<LayerThread>();

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
    }

    int numWorkingThreads = 8;
    int width = 0;
    int height = 0;



    public native void setScene(Scene scene);
    public native void setViewNative(View view);
    public native void setClearColor(float r,float g,float b,float a);
    protected native boolean teardown();
    protected native boolean resize(int width,int height);
    protected native void render();
    protected native boolean hasChanges();
    public native void setPerfInterval(int perfInterval);
    public native void addLight(DirectionalLight light);
    public native void replaceLights(List<DirectionalLight> lights);


    public void finalize()
    {
        dispose();
    }

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;
}
