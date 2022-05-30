package com.mousebird.maply;

import android.graphics.Bitmap;

import java.io.Closeable;
import java.util.Collection;
import java.util.List;

import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

/**
 * The Render Controller Interface defines the methods that a renderer has to implement.  These are
 * all done more or less by the same object behind the scenes, but this presents a coherent interface
 * for both on- and off-screen renderers.
 */
public interface RenderControllerInterface
{
    Scene getScene();
    CoordSystem getCoordSystem();

    // When adding features we can run on the current thread or delay the work till later
    enum ThreadMode {ThreadCurrent,ThreadAny}

    void clearLights();
    void resetLights();
    void addLight(Light light);
    void removeLight(Light light);

    ComponentObject addScreenMarkers(final Collection<ScreenMarker> markers, final MarkerInfo markerInfo, ThreadMode mode);
    ComponentObject addScreenMovingMarkers(final Collection<ScreenMovingMarker> markers,final MarkerInfo markerInfo,ThreadMode mode);

    ComponentObject addMarkers(final Collection<Marker> markers,final MarkerInfo markerInfo,ThreadMode mode);

    ComponentObject addScreenLabels(final Collection<ScreenLabel> labels,final LabelInfo labelInfo,ThreadMode mode);
    ComponentObject addScreenMovingLabels(final Collection<ScreenMovingLabel> labels,final LabelInfo labelInfo,ThreadMode mode);

    ComponentObject addVectors(final Collection<VectorObject> vecs, final VectorInfo vecInfo, ThreadMode mode);
    void changeVector(final ComponentObject vecObj, final VectorInfo vecInfo, ThreadMode mode);
    ComponentObject instanceVectors(ComponentObject vecObj, final VectorInfo vecInfo, ThreadMode mode);

    ComponentObject addWideVectors(final Collection<VectorObject> vecs,final WideVectorInfo wideVecInfo,ThreadMode mode);
    ComponentObject instanceWideVectors(final ComponentObject inCompObj,final WideVectorInfo wideVecInfo,ThreadMode mode);

    // TODO: Fill this in
//    public ComponentObject addModelInstances();

    // TODO: Fill this in
//    public ComponentObject addGeometry();

    ComponentObject addShapes(final Collection<Shape> shapes, final ShapeInfo shapeInfo, ThreadMode mode);

    ComponentObject addStickers(final Collection<Sticker> stickers,final StickerInfo stickerInfo,ThreadMode mode);

    ComponentObject changeSticker(final ComponentObject stickerObj,final StickerInfo stickerInfo,ThreadMode mode);

    ComponentObject addBillboards(final Collection<Billboard> bills, final BillboardInfo info, final ThreadMode threadMode);

    ComponentObject addLoftedPolys(final Collection<VectorObject> vecs, final LoftedPolyInfo info, final ThreadMode threadMode);

    ComponentObject addPoints(final Collection<Points> inPoints,final GeometryInfo geomInfo, final ThreadMode mode);

    /**
     * Texture settings for adding textures to the system.
     */
    class TextureSettings
    {
        public TextureSettings()
        {
        }

        public enum FilterType {FilterNearest,FilterLinear}

        /**
         * Image format to use when creating textures.
         */
        public RenderController.ImageFormat imageFormat = RenderController.ImageFormat.MaplyImageIntRGBA;
        /**
         * Filter type for created textures.
         */
        public FilterType filterType = FilterType.FilterLinear;
        /**
         * Horizontal texture wrap.
         */
        public boolean wrapU = false;
        /**
         * Vertical texture wrap
         */
        public boolean wrapV = false;
    }

    MaplyTexture addTexture(final Bitmap image,final TextureSettings settings,ThreadMode mode);
    MaplyTexture addTexture(final Texture rawTex,final TextureSettings settings,ThreadMode mode);
    MaplyTexture createTexture(final int width,final int height,final TextureSettings settings,ThreadMode mode);
    void removeTextures(final Collection<MaplyTexture> texs,ThreadMode mode);
    void removeTexture(final MaplyTexture tex,ThreadMode mode);
    void removeTexturesByID(final Collection<Long> texIDs,ThreadMode mode);

    void addRenderTarget(RenderTarget renderTarget);
    void changeRenderTarget(RenderTarget renderTarget, MaplyTexture tex);
    void removeRenderTarget(RenderTarget renderTarget);

    void disableObjects(final Collection<ComponentObject> compObjs,ThreadMode mode);
    void enableObjects(final Collection<ComponentObject> compObjs,ThreadMode mode);
    void removeObjects(final Collection<ComponentObject> compObjs,ThreadMode mode);
    void removeObject(final ComponentObject compObj,ThreadMode mode);

    void addShaderProgram(final Shader shader);
    Shader getShader(String name);
    void removeShader(Shader shader);

    void setClearColor(int color);

    double heightForMapScale(double scale);

    double currentMapZoom(Point2d geoCoord);

    double currentMapScale();

    int[] getFrameBufferSize();

    boolean getOfflineMode();

    ContextInfo setEGLContext(ContextInfo cInfo);

    ContextWrapper wrapTempContext(RenderController.ThreadMode threadMode);

    ContextInfo setupTempContext(ThreadMode threadMode);

    void clearTempContext(ContextInfo cInfo);

    void processChangeSet(ChangeSet changes);

    void requestRender();

    // Context and associated surface
    class ContextInfo {
        EGLDisplay eglDisplay;
        EGLContext eglContext;
        EGLSurface eglDrawSurface;
        EGLSurface eglReadSurface;
        public ContextInfo(EGLDisplay display, EGLContext context,
                           EGLSurface drawSurface, EGLSurface readSurface) {
            eglDisplay = display;
            eglContext = context;
            eglReadSurface = readSurface;
            eglDrawSurface = drawSurface;
        }
    }

    class ContextWrapper implements Closeable {
        final public ContextInfo context;
        public ContextWrapper(RenderControllerInterface control, ContextInfo context) {
            this.control = control;
            this.context = context;
        }
        public void close() {
            if (control != null && context != null) {
                control.clearTempContext(context);
            }
        }
        private final RenderControllerInterface control;
    }

    // Used to track down problems with GL Context allocation
    void dumpFailureInfo(String failureLocation);
}
