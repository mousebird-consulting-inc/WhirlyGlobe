package com.mousebird.maply;

import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.graphics.drawable.ColorDrawable;
import android.opengl.EGL14;
import android.opengl.EGLExt;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.util.Log;
import android.widget.Toast;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

/**
 * The Render Controller handles the object manipulation and rendering interface.
 * It can be a standalone object, doing offline rendering or it can be attached
 * to a BaseController and used to manage that controller's rendering.
 */
public class RenderController implements RenderControllerInterface
{
    public static final String kToolkitDefaultTriangleNoLightingProgram = "Default Triangle;lighting=no";

    // Draw priority defaults
    public static final int ImageLayerDrawPriorityDefault = 100;
    public static final int FeatureDrawPriorityBase = 20000;
    public static final int MarkerDrawPriorityDefault = 40000;
    public static final int LabelDrawPriorityDefault = 60000;
    public static final int ParticleDrawPriorityDefault = 1000;

    Point2d frameSize = new Point2d(0.0, 0.0);

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

    /**
     * If set, we'll explicitly call dispose on any objects that were
     * being kept around for selection.
     */
    public boolean disposeAfterRemoval = false;

    // Scene stores the objects
    public Scene scene = null;
    public CoordSystemDisplayAdapter coordAdapter = null;

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

    /**
     * This constructor assumes we'll be hooking up to surface provided later.
     */
    RenderController()
    {
        initialise();
    }

    // Set if this is a standalone renderer
    protected boolean offlineMode = false;

    // Construct a new render control based on an existing one
    public RenderController(RenderController baseControl,int width,int height)
    {
        frameSize = new Point2d(width,height);
        setConfig(baseControl, null);

        // Set up our own EGL context for offline work
        EGL10 egl = (EGL10) EGLContext.getEGL();
        int[] attrib_list = {BaseController.EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE};
        offlineGLContext = new ContextInfo();
        offlineGLContext.eglContext = egl.eglCreateContext(display, config, context, attrib_list);
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
        offlineGLContext.eglSurface = egl.eglCreatePbufferSurface(display, config, surface_attrs);

        setEGLContext(offlineGLContext);

        initialise(width,height);

        // Set up a passthrough coordinate system, view, and so on
        CoordSystem coordSys = new PassThroughCoordSystem();
        Point3d ll = new Point3d(0.0,0.0,0.0);
        Point3d ur = new Point3d(width,height,0.0);
        Point3d scale = new Point3d(1.0, 1.0, 1.0);
        Point3d center = new Point3d((ll.getX()+ur.getX())/2.0,(ll.getY()+ur.getY())/2.0,(ll.getZ()+ur.getZ())/2.0);
        coordAdapter = new GeneralDisplayAdapter(coordSys,ll,ur,center,scale);
        FlatView flatView = new FlatView(null,coordAdapter);
        Mbr extents = new Mbr(new Point2d(ll.getX(),ll.getY()),new Point2d(ur.getX(),ur.getY()));
        flatView.setExtents(extents);
        flatView.setWindow(new Point2d(width,height),new Point2d(0.0,0.0));
        view = flatView;

        scene = new Scene(coordAdapter,this);

        // Need a task manager that just runs things on the current thread
        //  after setting the proper context for rendering
        TaskManager taskMan = new TaskManager() {
            @Override
            public void addTask(Runnable run, ThreadMode mode) {
                EGL10 egl = (EGL10) EGLContext.getEGL();
                setEGLContext(offlineGLContext);

                run.run();
            }
        };

        // This will properly wire things up
        Init(scene,coordAdapter,taskMan);
        setScene(scene);
        setView(view);

        // Need all the default shaders
        setupShadersNative();

        clearContext();
    }

    /**
     * This constructor sets up its own render target.  Used for offline rendering.
     */
    public RenderController(int width,int height)
    {
        this(null,width,height);
    }

    /**
     * We don't want to deal with threads and such down here, so
     * the controller one level up gives us an addTask method
     * to hand over the runnables.
     */
    public interface TaskManager {
        public void addTask(Runnable run,ThreadMode mode);
    }

    TaskManager taskMan = null;

    public void Init(Scene inScene,CoordSystemDisplayAdapter inCoordAdapter,TaskManager inTaskMan)
    {
        scene = inScene;
        coordAdapter = inCoordAdapter;
        taskMan = inTaskMan;

        // Fire up the managers.  Can't do anything without these.
        vecManager = new VectorManager(scene);
        loftManager = new LoftedPolyManager(scene);
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
     * Add an active object to the beginning of the list.  Do this if you want to make sure
     * yours is run first.
     */
    void addActiveObjectAtStart(ActiveObject activeObject) {
        synchronized (activeObjects) {
            activeObjects.add(0,activeObject);
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
        boolean ret = false;

        synchronized (activeObjects) {
            // Can't short circuit this.  Some objects use the hasUpdate as a pre-render
            for (ActiveObject activeObject : activeObjects)
                if (activeObject.hasChanges())
                    ret = true;
        }

        return ret;
    }

    public boolean surfaceChanged(int width,int height)
    {
        frameSize.setValue(width, height);
        return resize(width,height);
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
    public void setConfig(RenderController otherControl,EGLConfig inConfig)
    {
        EGL10 egl = (EGL10) EGLContext.getEGL();

        if (otherControl == null) {
            display = egl.eglGetCurrentDisplay();
            context = egl.eglGetCurrentContext();
        } else {
            display = otherControl.display;
            context = otherControl.context;
            inConfig = otherControl.config;
        }

        // If we didn't pass in one, we're in offline mode and need to make one
        if (inConfig == null) {
            int[] attribList = {
                    EGL14.EGL_RED_SIZE, 8,
                    EGL14.EGL_GREEN_SIZE, 8,
                    EGL14.EGL_BLUE_SIZE, 8,
                    EGL14.EGL_ALPHA_SIZE, 8,
                    EGL14.EGL_DEPTH_SIZE, 16,
                    EGL14.EGL_RENDERABLE_TYPE, EGL14.EGL_OPENGL_ES2_BIT, EGLExt.EGL_OPENGL_ES3_BIT_KHR,
                    EGL14.EGL_NONE
            };
            EGLConfig[] configs = new EGLConfig[1];
            int[] numConfigs = new int[1];
            if (!egl.eglChooseConfig(display,attribList,configs, configs.length, numConfigs))
            {
                Log.e("Maply", "Unable set set up OpenGL ES for offline rendering.");
            } else {
                config = configs[0];
            }
        } else {
            config = inConfig;
        }
    }

    // Managers are thread safe objects for handling adding and removing types of data
    protected VectorManager vecManager;
    protected LoftedPolyManager loftManager;
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

    public void shutdown()
    {
        // Kill the shaders here because they don't do well being finalized
        for (Shader shader : shaders)
            shader.dispose();
        shaders.clear();

        if (vecManager != null)
            vecManager.dispose();
        if (loftManager != null)
            loftManager.dispose();
        if (wideVecManager != null)
            wideVecManager.dispose();
        if (stickerManager != null)
            stickerManager.dispose();
        if (selectionManager != null)
            selectionManager.dispose();
        if (componentManager != null)
            componentManager.dispose();
        if (labelManager != null)
            labelManager.dispose();
        if (layoutManager != null)
            layoutManager.dispose();
        if (particleSystemManager != null)
            particleSystemManager.dispose();

        vecManager = null;
        loftManager = null;
        wideVecManager = null;
        markerManager = null;
        stickerManager = null;
        labelManager = null;
        selectionManager = null;
        componentManager = null;
        layoutManager = null;
        particleSystemManager = null;
        layoutLayer = null;
        shapeManager = null;
        billboardManager = null;

        texManager = null;

        offlineGLContext = null;
    }

    /** RenderControllerInterface **/

    private ArrayList<Light> lights = new ArrayList<>();

    /**
     * Add the given light to the list of active lights.
     * <br>
     * This method will add the given light to our active lights.  Most shaders will recognize these lights and do the calculations.  If you have a custom shader in place, it may or may not use these.
     * Triangle shaders use the lights, but line shaders do not.
     * @param light Light to add.
     */
    public void addLight(final Light light) {
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
        if (this.lights == null)
            return;
        this.lights.remove(light);
        this.updateLights();
    }

    // Lights have to be rebuilt every time they change
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
        replaceLights(theLights.toArray(new DirectionalLight[0]));

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
        this.lights = new ArrayList<>();
        this.updateLights();
    }

    /**
     * Reset the lighting back to its default state at startup.
     * <br>
     * This clears out all the lights and adds in the default starting light source.
     */
    public void resetLights() {
        this.clearLights();

        Light light = new Light();
        light.setPos(new Point3d(0.75, 0.5, -1.0));
        light.setAmbient(0.6f, 0.6f, 0.6f, 1.0f);
        light.setDiffuse(0.5f, 0.5f, 0.5f, 1.0f);
        light.setViewDependent(false);
        this.addLight(light);
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
        final ComponentObject compObj = componentManager.makeComponentObject();
        final RenderController renderControl = this;

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

                            InternalMarker intMarker = new InternalMarker(marker);
                            long texID = EmptyIdentity;
                            if (marker.image != null) {
                                texID = texManager.addTexture(marker.image, scene, changes);
                                if (texID != EmptyIdentity)
                                    intMarker.addTexID(texID);
                            } else if (marker.tex != null) {
                                texID = marker.tex.texID;
                                intMarker.addTexID(texID);
                            } else if (marker.images != null)
                            {
                                for (MaplyTexture tex : marker.images) {
                                    intMarker.addTexID(tex.texID);
                                }
                            }
                            if (marker.vertexAttributes != null)
                                intMarker.setVertexAttributes(marker.vertexAttributes.toArray());

                            intMarkers.add(intMarker);

                            // Keep track of this one for selection
                            if (marker.selectable)
                            {
                                componentManager.addSelectableObject(marker.ident,marker,compObj);
                            }
                        }

                        // Add the markers and flush the changes
                        long markerId = markerManager.addScreenMarkers(intMarkers.toArray(new InternalMarker[0]), markerInfo, changes);
                        if (scene != null) {
                            changes.process(renderControl, scene);
                            changes.dispose();
                        }

                        if (markerId != EmptyIdentity)
                        {
                            compObj.addMarkerID(markerId);
                        }

                        for (InternalMarker marker : intMarkers)
                            marker.dispose();

                        componentManager.addComponentObject(compObj);
                    }
                };

        taskMan.addTask(run, mode);

        return compObj;
    }

    /**
     * Add moving screen markers to the visual display.  These are the same as the regular
     * screen markers, but they have a start and end point and a duration.
     */
    public ComponentObject addScreenMovingMarkers(final List<ScreenMovingMarker> markers,final MarkerInfo markerInfo,RenderController.ThreadMode mode)
    {
        final ComponentObject compObj = componentManager.makeComponentObject();
        final double now = System.currentTimeMillis() / 1000.0;
        final RenderController renderControl = this;

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
                        for (ScreenMovingMarker marker : markers)
                        {
                            if (marker.loc == null)
                            {
                                Log.d("Maply","Missing location for marker.  Skipping.");
                                return;
                            }

                            InternalMarker intMarker = new InternalMarker(marker,now);
                            long texID = EmptyIdentity;
                            if (marker.image != null) {
                                texID = texManager.addTexture(marker.image, scene, changes);
                                if (texID != EmptyIdentity)
                                    intMarker.addTexID(texID);
                            } else if (marker.tex != null) {
                                texID = marker.tex.texID;
                                intMarker.addTexID(texID);
                            } else if (marker.images != null)
                            {
                                for (MaplyTexture tex : marker.images) {
                                    intMarker.addTexID(tex.texID);
                                }
                            }
                            if (marker.vertexAttributes != null)
                                intMarker.setVertexAttributes(marker.vertexAttributes.toArray());

                            intMarkers.add(intMarker);

                            // Keep track of this one for selection
                            if (marker.selectable)
                            {
                                componentManager.addSelectableObject(marker.ident,marker,compObj);
                            }
                        }

                        // Add the markers and flush the changes
                        long markerId = markerManager.addScreenMarkers(intMarkers.toArray(new InternalMarker[0]), markerInfo, changes);
                        if (scene != null) {
                            changes.process(renderControl, scene);
                            changes.dispose();
                        }

                        if (markerId != EmptyIdentity)
                        {
                            compObj.addMarkerID(markerId);
                        }

                        for (InternalMarker marker : intMarkers)
                            marker.dispose();

                        componentManager.addComponentObject(compObj);
                    }
                };

        taskMan.addTask(run, mode);

        return compObj;
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
    public ComponentObject addMarkers(final List<Marker> markers,final MarkerInfo markerInfo,ThreadMode mode)
    {
        final ComponentObject compObj = componentManager.makeComponentObject();
        final RenderController renderControl = this;

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
                        for (Marker marker : markers)
                        {
                            if (marker.loc == null)
                            {
                                Log.d("Maply","Missing location for marker.  Skipping.");
                                return;
                            }

                            InternalMarker intMarker = new InternalMarker(marker);
                            // Map the bitmap to a texture ID
                            long texID = EmptyIdentity;
                            if (marker.image != null) {
                                texID = texManager.addTexture(marker.image, scene, changes);
                                if (texID != EmptyIdentity)
                                    intMarker.addTexID(texID);
                            } else if (marker.images != null)
                            {
                                for (MaplyTexture tex : marker.images) {
                                    intMarker.addTexID(tex.texID);
                                }
                            } else if (marker.tex != null)
                            {
                                intMarker.addTexID(marker.tex.texID);
                            }

                            intMarkers.add(intMarker);

                            // Keep track of this one for selection
                            if (marker.selectable)
                            {
                                componentManager.addSelectableObject(marker.ident,marker,compObj);
                            }
                        }

                        // Add the markers and flush the changes
                        long markerId = markerManager.addMarkers(intMarkers.toArray(new InternalMarker[0]), markerInfo, changes);
                        if (scene != null) {
                            changes.process(renderControl, scene);
                            changes.dispose();
                        }

                        if (markerId != EmptyIdentity)
                        {
                            compObj.addMarkerID(markerId);
                        }

                        for (InternalMarker marker : intMarkers)
                            marker.dispose();

                        componentManager.addComponentObject(compObj);
                    }
                };

        taskMan.addTask(run, mode);

        return compObj;
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
        final ComponentObject compObj = componentManager.makeComponentObject();
        final RenderController renderControl = this;

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
                                InternalLabel intLabel = new InternalLabel(label,labelInfo);
                                intLabels.add(intLabel);

                                // Keep track of this one for selection
                                if (label.selectable) {
                                    componentManager.addSelectableObject(label.ident, label, compObj);
                                }
                            }
                        }

                        long labelId = EmptyIdentity;
                        // Note: We can't run multiple of these at once.  The problem is that
                        //  we need to pass the JNIEnv deep inside the toolkit and we're setting
                        //  on JNIEnv at a time for the CharRenderer callback.
                        synchronized (labelManager) {
                            labelId = labelManager.addLabels(intLabels.toArray(new InternalLabel[0]), labelInfo, changes);
                        }
                        if (labelId != EmptyIdentity)
                            compObj.addLabelID(labelId);

                        // Flush the text changes
                        if (scene != null) {
                            changes.process(renderControl, scene);
                            changes.dispose();
                        }

                        for (InternalLabel label : intLabels)
                            label.dispose();

                        componentManager.addComponentObject(compObj);
                    }
                };

        taskMan.addTask(run, mode);

        return compObj;
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
    public ComponentObject addScreenMovingLabels(final List<ScreenMovingLabel> labels,final LabelInfo labelInfo,ThreadMode mode)
    {
        final ComponentObject compObj = componentManager.makeComponentObject();
        final double now = System.currentTimeMillis() / 1000.0;
        final RenderController renderControl = this;

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
                        for (ScreenMovingLabel label : labels)
                        {
                            if (label.text != null && label.text.length() > 0) {
                                InternalLabel intLabel = new InternalLabel(label,labelInfo,now);
                                intLabels.add(intLabel);

                                // Keep track of this one for selection
                                if (label.selectable) {
                                    componentManager.addSelectableObject(label.ident, label, compObj);
                                }
                            }
                        }

                        long labelId = EmptyIdentity;
                        // Note: We can't run multiple of these at once.  The problem is that
                        //  we need to pass the JNIEnv deep inside the toolkit and we're setting
                        //  on JNIEnv at a time for the CharRenderer callback.
                        synchronized (labelManager) {
                            labelId = labelManager.addLabels(intLabels.toArray(new InternalLabel[0]), labelInfo, changes);
                        }
                        if (labelId != EmptyIdentity)
                            compObj.addLabelID(labelId);

                        // Flush the text changes
                        if (scene != null) {
                            changes.process(renderControl, scene);
                            changes.dispose();
                        }

                        for (InternalLabel label : intLabels)
                            label.dispose();

                        componentManager.addComponentObject(compObj);
                    }
                };

        taskMan.addTask(run, mode);

        return compObj;
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
        final ComponentObject compObj = componentManager.makeComponentObject();
        final RenderController renderControl = this;

        // Do the actual work on the layer thread
        Runnable run =
                new Runnable()
                {
                    @Override
                    public void run()
                    {
                        // Vectors are simple enough to just add
                        ChangeSet changes = new ChangeSet();
                        long vecId = vecManager.addVectors(vecs.toArray(new VectorObject[0]), vecInfo, changes);
                        if (scene != null) {
                            changes.process(renderControl, scene);
                            changes.dispose();
                        }

                        // Track the vector ID for later use
                        if (vecId != EmptyIdentity)
                            compObj.addVectorID(vecId);

                        // Keep track of this one for selection
                        for (VectorObject vecObj : vecs)
                        {
                            if (vecObj.getSelectable()) {
                                compObj.addVector(vecObj);
                                componentManager.addSelectableObject(vecObj.ident, vecObj, compObj);
                            }
                        }

                        if (vecInfo.disposeAfterUse || disposeAfterRemoval)
                            for (VectorObject vecObj : vecs)
                                if (!vecObj.getSelectable())
                                    vecObj.dispose();

                        componentManager.addComponentObject(compObj);
                    }
                };

        taskMan.addTask(run, mode);

        return compObj;
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
    public ComponentObject addLoftedPolys(final List<VectorObject> vecs, final LoftedPolyInfo loftInfo, final ThreadMode mode)
    {
        final ComponentObject compObj = componentManager.makeComponentObject();
        final RenderController renderControl = this;

        // Do the actual work on the layer thread
        Runnable run =
                new Runnable()
                {
                    @Override
                    public void run()
                    {
                        // Vectors are simple enough to just add
                        ChangeSet changes = new ChangeSet();
                        long loftID = loftManager.addPolys(vecs.toArray(new VectorObject[0]), loftInfo, changes);
                        if (scene != null) {
                            changes.process(renderControl, scene);
                            changes.dispose();
                        }

                        // Track the vector ID for later use
                        if (loftID != EmptyIdentity)
                            compObj.addLoftID(loftID);

                        for (VectorObject vecObj : vecs)
                        {
                            // TODO: Porting
                            // Keep track of this one for selection
//					if (vecObj.getSelectable())
//						compObj.addSelectID(vecObj.getID());
                        }

                        if (loftInfo.disposeAfterUse || disposeAfterRemoval)
                            for (VectorObject vecObj : vecs)
                                if (!vecObj.getSelectable())
                                    vecObj.dispose();

                        componentManager.addComponentObject(compObj);
                    }
                };

        taskMan.addTask(run, mode);

        return compObj;
    }


    /**
     * Change the visual representation of the given vectors.
     * @param vecObj The component object returned by the original addVectors() call.
     * @param vecInfo Visual representation to use for the changes.
     * @param mode Where to execute the add.  Choose ThreadAny by default.
     */
    public void changeVector(final ComponentObject vecObj,final VectorInfo vecInfo,ThreadMode mode)
    {
        if (vecObj == null)
            return;
        final RenderController renderControl = this;

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
                            if (scene != null) {
                                changes.process(renderControl, scene);
                                changes.dispose();
                            }
                        }
                    }
                };
        taskMan.addTask(run, mode);
    }

    /**
     * Instance an existing set of vectors and modify various parameters for reuse.
     * This is useful if you want to overlay the same vectors twice with different widths,
     * for example.
     */
    public ComponentObject instanceVectors(final ComponentObject vecObj, final VectorInfo vecInfo, ThreadMode mode)
    {
        if (vecObj == null)
            return null;

        final ComponentObject compObj = componentManager.makeComponentObject();
        final RenderController renderControl = this;

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
                            for (long vecID : vecIDs) {
                                long newID = vecManager.instanceVectors(vecID, vecInfo, changes);
                                if (newID != EmptyIdentity)
                                    compObj.addVectorID(newID);
                            }
                            if (scene != null) {
                                changes.process(renderControl, scene);
                                changes.dispose();
                            }
                        }

                        componentManager.addComponentObject(compObj);
                    }
                };
        taskMan.addTask(run, mode);

        return compObj;
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
    public ComponentObject addWideVectors(final List<VectorObject> vecs,final WideVectorInfo wideVecInfo,ThreadMode mode)
    {
        final ComponentObject compObj = componentManager.makeComponentObject();
        final RenderController renderControl = this;

        // Do the actual work on the layer thread
        Runnable run =
                new Runnable()
                {
                    @Override
                    public void run()
                    {
                        // Vectors are simple enough to just add
                        ChangeSet changes = new ChangeSet();
                        long vecId = wideVecManager.addVectors(vecs.toArray(new VectorObject[0]), wideVecInfo, changes);
                        if (scene != null) {
                            changes.process(renderControl, scene);
                            changes.dispose();
                        }

                        // Track the vector ID for later use
                        if (vecId != EmptyIdentity)
                            compObj.addWideVectorID(vecId);

                        for (VectorObject vecObj : vecs)
                        {
                            // TODO: Porting
                            // Keep track of this one for selection
//							if (vecObj.getSelectable())
//								compObj.addVector(vecObj);
                        }

                        if (wideVecInfo.disposeAfterUse || disposeAfterRemoval)
                            for (VectorObject vecObj : vecs)
                                if (!vecObj.getSelectable())
                                    vecObj.dispose();

                        componentManager.addComponentObject(compObj);
                    }
                };

        taskMan.addTask(run, mode);

        return compObj;
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
    public ComponentObject instanceWideVectors(final ComponentObject inCompObj,final WideVectorInfo wideVecInfo,ThreadMode mode)
    {
        final ComponentObject compObj = componentManager.makeComponentObject();
        final RenderController renderControl = this;

        // Do the actual work on the layer thread
        Runnable run =
                new Runnable()
                {
                    @Override
                    public void run()
                    {
                        // Vectors are simple enough to just add
                        ChangeSet changes = new ChangeSet();

                        for (long vecID : inCompObj.getWideVectorIDs()) {
                            long instID = wideVecManager.instanceVectors(vecID,wideVecInfo,changes);

                            if (instID != EmptyIdentity)
                                compObj.addWideVectorID(instID);
                        }

                        if (scene != null) {
                            changes.process(renderControl, scene);
                            changes.dispose();
                        }

                        componentManager.addComponentObject(compObj);
                    }
                };

        taskMan.addTask(run, mode);

        return compObj;
    }
    // TODO: Fill this in
//    public ComponentObject addModelInstances();

    // TODO: Fill this in
//    public ComponentObject addGeometry();

    /**
     * This method will add the given MaplyShape derived objects to the current scene.  It will use the parameters in the description dictionary and it will do it on the thread specified.
     * @param shapes An array of Shape derived objects
     * @param shapeInfo Info controlling how the shapes look
     * @param mode Where to execute the add.  Choose ThreadAny by default.
     */
    public ComponentObject addShapes(final List<Shape> shapes, final ShapeInfo shapeInfo, ThreadMode mode) {
        final ComponentObject compObj = componentManager.makeComponentObject();
        final ChangeSet changes = new ChangeSet();
        final RenderController renderControl = this;

        Runnable run = new Runnable() {
            @Override
            public void run() {
                long shapeId = shapeManager.addShapes(shapes.toArray(new Shape[0]), shapeInfo, changes);
                if (shapeId != EmptyIdentity)
                    compObj.addShapeID(shapeId);
                if (scene != null) {
                    changes.process(renderControl, scene);
                    changes.dispose();
                }

                for (Shape shape : shapes)
                    if (shape.isSelectable())
                    {
                        componentManager.addSelectableObject(shape.getSelectID(), shape, compObj);
                    }

                if (shapeInfo.disposeAfterUse || disposeAfterRemoval)
                    for (Shape shape : shapes)
                        shape.dispose();

                componentManager.addComponentObject(compObj);
            }
        };

        taskMan.addTask(run, mode);
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
        final ComponentObject compObj = componentManager.makeComponentObject();
        final RenderController renderControl = this;

        // Do the actual work on the layer thread
        Runnable run =
                new Runnable()
                {
                    @Override
                    public void run()
                    {
                        ChangeSet changes = new ChangeSet();

                        // Stickers are added one at a time for some reason
                        long stickerID = stickerManager.addStickers(stickers.toArray(new Sticker[0]), stickerInfo, changes);

                        if (stickerID != EmptyIdentity) {
                            compObj.addStickerID(stickerID);
                        }
                        if (scene != null) {
                            changes.process(renderControl, scene);
                            changes.dispose();
                        }

                        if (stickerInfo.disposeAfterUse || disposeAfterRemoval)
                            for (Sticker sticker : stickers)
                                sticker.dispose();

                        componentManager.addComponentObject(compObj);
                    }
                };

        taskMan.addTask(run, mode);

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
        final ComponentObject compObj = componentManager.makeComponentObject();
        final RenderController renderControl = this;

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
                            for (long stickerID : stickerIDs)
                                stickerManager.changeSticker(stickerID, stickerInfo, changes);
                        }

                        if (scene != null) {
                            changes.process(renderControl, scene);
                            changes.dispose();
                        }

                        componentManager.addComponentObject(compObj);
                    }
                };

        taskMan.addTask(run, mode);

        return compObj;
    }

    /**
     * Billboards are rectangles pointed toward the viewer.  They can either be upright, tied to a
     * surface, or oriented completely toward the user.
     */
    public ComponentObject addBillboards(final List<Billboard> bills, final BillboardInfo info, final RenderController.ThreadMode threadMode) {
        final ComponentObject compObj = componentManager.makeComponentObject();
        final RenderController renderControl = this;

        // Do the actual work on the layer thread
        Runnable run =
                new Runnable()
                {
                    @Override
                    public void run()
                    {
                        ChangeSet changes = new ChangeSet();

                        // Have to set the shader ID if it's not already
                        long shaderID = info.getShaderID();
                        if (info.getShaderID() == 0) {
                            String shaderName = null;
                            // TODO: Share these constants with the c++ code
                            if (info.getOrient() == BillboardInfo.Orient.Eye)
                                shaderName = "billboardorienteye";
                            else
                                shaderName = "billboardorientground";
                            Shader shader = getShader(shaderName);

                            shaderID = shader.getID();
                            info.setShaderID(shaderID);
                        }

                        for (Billboard bill : bills) {
                            // Convert to display space
                            Point3d center = bill.getCenter();
                            Point3d localPt =coordAdapter.getCoordSystem().geographicToLocal(new Point3d(center.getX(),center.getY(),0.0));
                            Point3d dispTmp =coordAdapter.localToDisplay(localPt);
                            Point3d dispPt = dispTmp.multiplyBy(center.getZ()/6371000.0+1.0);
                            bill.setCenter(dispPt);

                            if (bill.getSelectable()) {
                                bill.setSelectID(Identifiable.genID());
                                componentManager.addSelectableObject(bill.getSelectID(), bill, compObj);
                            }

                            // Turn any screen objects into billboard polygons
                            bill.flatten();
                        }

                        long billId = billboardManager.addBillboards(bills.toArray(new Billboard[0]), info, changes);
                        compObj.addBillboardID(billId);

                        // Flush the text changes
                        if (scene != null) {
                            changes.process(renderControl, scene);
                            changes.dispose();
                        }

                        if (info.disposeAfterUse || disposeAfterRemoval)
                            for (Billboard bill : bills)
                                if (!bill.getSelectable())
                                    bill.dispose();

                        componentManager.addComponentObject(compObj);
                    }
                };

        taskMan.addTask(run, threadMode);

        return compObj;
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
        final ComponentObject compObj = componentManager.makeComponentObject();
        final RenderController renderControl = this;

        // Do the actual work on the layer thread
        Runnable run =
                new Runnable()
                {
                    @Override
                    public void run()
                    {
                        ChangeSet changes = new ChangeSet();

                        // Stickers are added one at a time for some reason
                        for (Points pts: ptList) {
                            Matrix4d mat = pts.mat != null ? pts.mat : new Matrix4d();
                            long geomID = geomManager.addGeometryPoints(pts.rawPoints,pts.mat,geomInfo,changes);

                            if (geomID != EmptyIdentity) {
                                compObj.addGeometryID(geomID);
                            }
                        }

                        if (scene != null) {
                            changes.process(renderControl, scene);
                            changes.dispose();
                        }

                        if (geomInfo.disposeAfterUse || disposeAfterRemoval)
                            for (Points pts: ptList)
                                pts.rawPoints.dispose();

                        componentManager.addComponentObject(compObj);
                    }
                };

        taskMan.addTask(run, mode);

        return compObj;
    }

    /**
     * Add texture to the system with the given settings.
     * @param image Image to add.
     * @param settings Settings to use.
     * @param mode Add on the current thread or elsewhere.
     */
    public MaplyTexture addTexture(final Bitmap image,final RenderController.TextureSettings settings,RenderController.ThreadMode mode)
    {
        final MaplyTexture texture = new MaplyTexture();
        final Texture rawTex = new Texture();
        texture.texID = rawTex.getID();
        final RenderController renderControl = this;

        // Possibly do the work somewhere else
        Runnable run =
                new Runnable()
                {
                    @Override
                    public void run()
                    {
                        ChangeSet changes = new ChangeSet();

                        rawTex.setBitmap(image,settings.imageFormat.ordinal());
                        rawTex.setSettings(settings.wrapU,settings.wrapV);
                        changes.addTexture(rawTex, scene, settings.filterType.ordinal());

                        // Flush the texture changes
                        if (scene != null) {
                            changes.process(renderControl, scene);
                            changes.dispose();
                        }
                    }
                };

        taskMan.addTask(run, mode);

        return texture;
    }

    /**
     * Add texture to the system with the given settings.
     * @param rawTex Texture to add.
     * @param settings Settings to use.
     * @param mode Add on the current thread or elsewhere.
     */
    public MaplyTexture addTexture(final Texture rawTex,final TextureSettings settings,ThreadMode mode)
    {
        final MaplyTexture texture = new MaplyTexture();
        final RenderController renderControl = this;

        // Possibly do the work somewhere else
        Runnable run =
                new Runnable()
                {
                    @Override
                    public void run()
                    {
                        ChangeSet changes = new ChangeSet();
                        texture.texID = rawTex.getID();
                        changes.addTexture(rawTex, scene, settings.filterType.ordinal());

                        // Flush the texture changes
                        if (scene != null) {
                            changes.process(renderControl, scene);
                            changes.dispose();
                        }
                    }
                };

        taskMan.addTask(run, mode);

        return texture;
    }

    /**
     * Create an empty texture of the given size.
     * @param width Width of the resulting texture
     * @param height Height of the resulting texture
     * @param settings Other texture related settings
     * @param mode Which thread to do the work on
     * @return The new texture (or a reference to it, anyway)
     */
    public MaplyTexture createTexture(final int width,final int height,final TextureSettings settings,ThreadMode mode)
    {
        final MaplyTexture texture = new MaplyTexture();
        final Texture rawTex = new Texture();
        texture.texID = rawTex.getID();
        texture.width = width;
        texture.height = height;
        final RenderController renderControl = this;

        // Possibly do the work somewhere else
        Runnable run =
                new Runnable()
                {
                    @Override
                    public void run()
                    {
                        ChangeSet changes = new ChangeSet();

                        rawTex.setSize(width,height);
                        rawTex.setIsEmpty(true);
                        changes.addTexture(rawTex, scene, settings.filterType.ordinal());

                        // Flush the texture changes
                        if (scene != null) {
                            changes.process(renderControl, scene);
                            changes.dispose();
                        }
                    }
                };

        taskMan.addTask(run, mode);

        return texture;
    }

    /**
     * Remove a texture from the scene.
     * @param texs Textures to remove.
     * @param mode Remove immediately (current thread) or elsewhere.
     */
    public void removeTextures(final List<MaplyTexture> texs,ThreadMode mode)
    {
        final RenderController renderControl = this;

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
                        if (scene != null) {
                            changes.process(renderControl, scene);
                            changes.dispose();
                        }
                    }
                };

        taskMan.addTask(run, mode);
    }

    public void removeTexture(final MaplyTexture tex,ThreadMode mode)
    {
        ArrayList<MaplyTexture> texs = new ArrayList<MaplyTexture>();
        texs.add(tex);

        removeTextures(texs,mode);
    }

    /**
     * This version of removeTexture takes texture IDs.  Thus you don't
     * have to keep the MaplyTexture around.
     *
     * @param texIDs Textures to remove
     * @param mode Remove immediately (current thread) or elsewhere.
     */
    public void removeTexturesByID(final List<Long> texIDs,ThreadMode mode)
    {
        final RenderController renderControl = this;

        // Do the actual work on the layer thread
        Runnable run =
                new Runnable()
                {
                    @Override
                    public void run()
                    {
                        ChangeSet changes = new ChangeSet();

                        for (Long texID : texIDs)
                            changes.removeTexture(texID);

                        // Flush the texture changes
                        if (scene != null) {
                            changes.process(renderControl, scene);
                            changes.dispose();
                        }
                    }
                };

        taskMan.addTask(run, mode);
    }

    /** Add a render target to the system
     * <br>
     * Sets up a render target and will start rendering to it on the next frame.
     * Keep the render target around so you can remove it later.
     */
    public void addRenderTarget(RenderTarget renderTarget)
    {
        scene.addRenderTargetNative(renderTarget.renderTargetID,
                renderTarget.texture.width,renderTarget.texture.height,
                renderTarget.texture.texID,
                renderTarget.clearEveryFrame,
                renderTarget.blend,
                Color.red(renderTarget.color)/255.f,Color.green(renderTarget.color)/255.f,Color.blue(renderTarget.color)/255.f,Color.alpha(renderTarget.color)/255.f);
    }

    /**
     * Point the render target at a different texture.
     */
    public void changeRenderTarget(RenderTarget renderTarget, MaplyTexture tex)
    {
       scene.changeRenderTarget(renderTarget.renderTargetID,tex.texID);
    }

    /** Remove the given render target from the system.
     * <br>
     * Ask the system to stop drawing to the given render target.  It will do this on the next frame.
     */
    public void removeRenderTarget(RenderTarget renderTarget)
    {
        scene.removeRenderTargetNative(renderTarget.renderTargetID);
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
        if (compObjs == null || compObjs.size() == 0)
            return;

        final ComponentObject[] localCompObjs = compObjs.toArray(new ComponentObject[compObjs.size()]);
        final RenderController renderControl = this;

        Runnable run = new Runnable()
        {
            @Override
            public void run()
            {
                ChangeSet changes = new ChangeSet();
                for (ComponentObject compObj : localCompObjs)
                    if (compObj != null)
                        componentManager.enableComponentObject(compObj,false,changes);
                if (scene != null) {
                    changes.process(renderControl, scene);
                    changes.dispose();
                }
            }
        };

        taskMan.addTask(run, mode);
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
        if (compObjs == null || compObjs == null)
            return;

        final ComponentObject[] localCompObjs = compObjs.toArray(new ComponentObject[compObjs.size()]);
        enableObjects(localCompObjs,mode);
    }

    /**
     * Enable the display for the given objects.  These objects were returned
     * by the various add calls.  To disable the display, call disableObjects().
     *
     * @param compObjs Objects to enable disable.
     * @param mode Where to execute the enable.  Choose ThreadAny by default.
     */
    public void enableObjects(final ComponentObject[] compObjs,ThreadMode mode) {
        if (compObjs == null || compObjs.length == 0)
            return;

        final RenderController renderControl = this;

        Runnable run =
                new Runnable()
                {
                    @Override
                    public void run()
                    {
                        ChangeSet changes = new ChangeSet();
                        for (ComponentObject compObj : compObjs)
                            if (compObj != null)
                                componentManager.enableComponentObject(compObj,true,changes);
                        if (scene != null) {
                            changes.process(renderControl, scene);
                            changes.dispose();
                        }
                    }
                };

        taskMan.addTask(run, mode);
    }

    /**
     * Remove the given component objects from the display.  This will permanently remove them
     * from Maply.  The component objects were returned from the various add calls.
     *
     * @param compObjs Component Objects to remove.
     * @param mode Where to execute the remove.  Choose ThreadAny by default.
     */
    public void removeObjects(final List<ComponentObject> compObjs,ThreadMode mode)
    {
        if (compObjs == null || compObjs == null)
            return;

        removeObjects(compObjs.toArray(new ComponentObject[0]),mode);
    }

    /**
     * Remove the given component objects from the display.  This will permanently remove them
     * from Maply.  The component objects were returned from the various add calls.
     *
     * @param compObjs Component Objects to remove.
     * @param mode Where to execute the remove.  Choose ThreadAny by default.
     */
    public void removeObjects(final ComponentObject[] compObjs,ThreadMode mode) {
        if (compObjs == null || compObjs.length == 0)
            return;

        final RenderController renderControl = this;

        Runnable run = new Runnable()
        {
            @Override
            public void run()
            {
                ChangeSet changes = new ChangeSet();

                componentManager.removeComponentObjects(compObjs,changes,disposeAfterRemoval);

                if (scene != null) {
                    changes.process(renderControl, scene);
                    changes.dispose();
                }
            }
        };

        taskMan.addTask(run, mode);
    }

    /**
     * Remove a simple object from the display.
     *
     * @param compObj Component Object to remove.
     * @param mode Where to execute the remove.  Choose ThreadAny by default.
     */
    public void removeObject(final ComponentObject compObj,ThreadMode mode)
    {
        List<ComponentObject> compObjs = new ArrayList<ComponentObject>();
        compObjs.add(compObj);

        removeObjects(compObjs,mode);
    }

    // All the shaders currently in use
    private ArrayList<Shader> shaders = new ArrayList<>();

    /**
     * Associate a shader with the given scene name.  These names let us override existing shaders, as well as adding our own.
     * @param shader The shader to add.
     */
    public void addShaderProgram(final Shader shader)
    {
        synchronized (shaders) {
            shaders.add(shader);
        }
        scene.addShaderProgram(shader);
    }

    /**
     * In the render controller setup, we stand up the full set of default
     * shaders used by the system.  To reflect things on this side, we'll
     * add them to this array as well.
     */
    protected void addPreBuiltShader(Shader shader)
    {
        synchronized (shaders) {
            shaders.add(shader);
        }
        shader.control = new WeakReference<RenderControllerInterface>(this);
    }

    /**
     * Find a shader by name
     * @param name Name of the shader to return
     * @return The shader with the name or null
     */
    public Shader getShader(String name)
    {
        synchronized (shaders) {
            for (Shader shader : shaders) {
                String shaderName = shader.getName();
                if (shaderName.equals(name))
                    return shader;
            }
        }

        return null;
    }

    /**
     * Remove the given shader from active use.
     */
    public void removeShader(Shader shader)
    {
        synchronized (shaders) {
            if (shaders.contains(shader)) {
                scene.removeShaderProgram(shader.getID());
                shaders.remove(shader);
            }
        }
    }

    int clearColor = Color.BLACK;

    /**
     * Set the color for the OpenGL ES background.
     */
    public void setClearColor(int color)
    {
        clearColor = color;

//		if (tempBackground != null)
//			tempBackground.setColor(clearColor);

        setClearColor(Color.red(color)/255.f,Color.green(color)/255.f,Color.blue(color)/255.f,Color.alpha(color)/255.f);
    }

    public double heightForMapScale(double scale)
    {
        return view.heightForMapScale(scale,frameSize.getX(),frameSize.getY());
    }

    public double currentMapZoom(Point2d geoCoord)
    {
        return view.currentMapZoom(frameSize.getX(),frameSize.getY(),geoCoord.getY());
    }

    public double currentMapScale()
    {
        return view.currentMapScale(frameSize.getX(),frameSize.getY());
    }

    /**
     * Returns the framebuffer size as ints.
     */
    public int[] getFrameBufferSize()
    {
        int[] sizes = new int[2];
        sizes[0] = (int)frameSize.getX();
        sizes[1] = (int)frameSize.getY();

        return sizes;
    }

    // A no-op for the standalone renderer
    public void requestRender()
    {
    }

    // Used in standalone mode
    ContextInfo offlineGLContext = null;

    // Used only in standalone mode
    public boolean setEGLContext(ContextInfo cInfo) {
        if (cInfo == null)
            cInfo = offlineGLContext;

        EGL10 egl = (EGL10) EGLContext.getEGL();
        if (cInfo != null)
        {
            if (!egl.eglMakeCurrent(display, cInfo.eglSurface, cInfo.eglSurface, cInfo.eglContext)) {
                Log.d("Maply", "Failed to make current context.");
                return false;
            }

            return true;
        } else {
                egl.eglMakeCurrent(display, egl.EGL_NO_SURFACE, egl.EGL_NO_SURFACE, egl.EGL_NO_CONTEXT);
        }

        return false;
    }

    // Return a description of the current context
    static public ContextInfo getEGLContext() {
        ContextInfo cInfo = new ContextInfo();
        EGL10 egl = (EGL10) EGLContext.getEGL();
        cInfo.eglContext = egl.eglGetCurrentContext();
        cInfo.eglSurface = egl.eglGetCurrentSurface(egl.EGL_DRAW);

        return cInfo;
    }

    public void processChangeSet(ChangeSet changes)
    {
        changes.process(this, scene);
        changes.dispose();
    }

    // Don't need this in standalone mode
    public ContextInfo setupTempContext(ThreadMode threadMode)
    {
        return null;
    }

    // Don't need this in standalone mode
    public void clearTempContext(ContextInfo cInfo)
    {
    }

    /**
     * In offline render mode, clear the context
     * Only do this if you're working in offline mode
     */
    public void clearContext()
    {
        EGL10 egl = (EGL10) EGLContext.getEGL();
        egl.eglMakeCurrent(display, egl.EGL_NO_SURFACE, egl.EGL_NO_SURFACE, egl.EGL_NO_CONTEXT);
    }

    /**
     * Render to and return a Bitmap
     * You should have already set the context at this point
     */
    public Bitmap renderToBitmap() {
        Bitmap bitmap = Bitmap.createBitmap((int)frameSize.getX(), (int)frameSize.getY(), Bitmap.Config.ARGB_8888);
        renderToBitmapNative(bitmap);

        return bitmap;
    }

    public native void setScene(Scene scene);
    public native void setupShadersNative();
    public native void setViewNative(View view);
    public native void setClearColor(float r,float g,float b,float a);
    protected native boolean teardown();
    protected native boolean resize(int width,int height);
    protected native void render();
    protected native boolean hasChanges();
    public native void setPerfInterval(int perfInterval);
    public native void addLight(DirectionalLight light);
    public native void replaceLights(DirectionalLight[] lights);
    protected native void renderToBitmapNative(Bitmap outBitmap);

    public void finalize()
    {
        dispose();
    }

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise(int width,int height);
    native void initialise();
    native void dispose();
    private long nativeHandle;
}
