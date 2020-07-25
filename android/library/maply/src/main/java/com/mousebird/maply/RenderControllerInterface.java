package com.mousebird.maply;

import android.graphics.Bitmap;

import java.util.List;
import java.util.Vector;

import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLSurface;

/**
 * The Render Controller Interface defines the methods
 * a renderer has to implement.  These are all done more or less
 * by the same object behind the scenes, but this presents a coherent
 * interface for both onscreen and offscreen renderers.
 */
public interface RenderControllerInterface
{
    Scene getScene();
    CoordSystem getCoordSystem();

    // When adding features we can run on the current thread or delay the work till later
    public enum ThreadMode {ThreadCurrent,ThreadAny};

    public void clearLights();
    public void resetLights();
    public void addLight(Light light);
    public void removeLight(Light light);

    public ComponentObject addScreenMarkers(final List<ScreenMarker> markers,final MarkerInfo markerInfo,ThreadMode mode);
    public ComponentObject addScreenMovingMarkers(final List<ScreenMovingMarker> markers,final MarkerInfo markerInfo,ThreadMode mode);

    public ComponentObject addMarkers(final List<Marker> markers,final MarkerInfo markerInfo,ThreadMode mode);

    public ComponentObject addScreenLabels(final List<ScreenLabel> labels,final LabelInfo labelInfo,ThreadMode mode);
    public ComponentObject addScreenMovingLabels(final List<ScreenMovingLabel> labels,final LabelInfo labelInfo,ThreadMode mode);

    public ComponentObject addVectors(final List<VectorObject> vecs, final VectorInfo vecInfo, ThreadMode mode);
    public void changeVector(final ComponentObject vecObj, final VectorInfo vecInfo, ThreadMode mode);
    public ComponentObject instanceVectors(ComponentObject vecObj, final VectorInfo vecInfo, ThreadMode mode);

    public ComponentObject addWideVectors(final List<VectorObject> vecs,final WideVectorInfo wideVecInfo,ThreadMode mode);
    public ComponentObject instanceWideVectors(final ComponentObject inCompObj,final WideVectorInfo wideVecInfo,ThreadMode mode);

    // TODO: Fill this in
//    public ComponentObject addModelInstances();

    // TODO: Fill this in
//    public ComponentObject addGeometry();

    public ComponentObject addShapes(final List<Shape> shapes, final ShapeInfo shapeInfo, ThreadMode mode);

    public ComponentObject addStickers(final List<Sticker> stickers,final StickerInfo stickerInfo,ThreadMode mode);

    public ComponentObject changeSticker(final ComponentObject stickerObj,final StickerInfo stickerInfo,ThreadMode mode);

    public ComponentObject addBillboards(final List<Billboard> bills, final BillboardInfo info, final ThreadMode threadMode);

    public ComponentObject addLoftedPolys(final List<VectorObject> vecs, final LoftedPolyInfo info, final ThreadMode threadMode);

    public ComponentObject addPoints(final List<Points> inPoints,final GeometryInfo geomInfo, final ThreadMode mode);

    /**
     * Texture settings for adding textures to the system.
     */
    static public class TextureSettings
    {
        public TextureSettings()
        {
        }

        public enum FilterType {FilterNearest,FilterLinear};

        /**
         * Image format to use when creating textures.
         */
        public RenderController.ImageFormat imageFormat = RenderController.ImageFormat.MaplyImageIntRGBA;
        /**
         * Filter type for created textures.
         */
        public FilterType filterType = FilterType.FilterLinear;
        /**
         * Horizonal texture wrap.
         */
        public boolean wrapU = false;
        /**
         * Vertical texture wrap
         */
        public boolean wrapV = false;
    };

    public MaplyTexture addTexture(final Bitmap image,final TextureSettings settings,ThreadMode mode);
    public MaplyTexture addTexture(final Texture rawTex,final TextureSettings settings,ThreadMode mode);
    public MaplyTexture createTexture(final int width,final int height,final TextureSettings settings,ThreadMode mode);
    public void removeTextures(final List<MaplyTexture> texs,ThreadMode mode);
    public void removeTexture(final MaplyTexture tex,ThreadMode mode);
    public void removeTexturesByID(final List<Long> texIDs,ThreadMode mode);

    public void addRenderTarget(RenderTarget renderTarget);
    public void changeRenderTarget(RenderTarget renderTarget, MaplyTexture tex);
    public void removeRenderTarget(RenderTarget renderTarget);

    public void disableObjects(final List<ComponentObject> compObjs,ThreadMode mode);
    public void enableObjects(final List<ComponentObject> compObjs,ThreadMode mode);
    public void removeObjects(final List<ComponentObject> compObjs,ThreadMode mode);
    public void removeObject(final ComponentObject compObj,ThreadMode mode);

    public void addShaderProgram(final Shader shader);
    public Shader getShader(String name);
    public void removeShader(Shader shader);

    public void setClearColor(int color);

    public double heightForMapScale(double scale);

    public double currentMapZoom(Point2d geoCoord);

    public double currentMapScale();

    public int[] getFrameBufferSize();

    boolean setEGLContext(ContextInfo cInfo);

    ContextInfo setupTempContext(ThreadMode threadMode);

    void clearTempContext(ContextInfo cInfo);

    void processChangeSet(ChangeSet changes);

    public void requestRender();

    // Context and associated surface
    public class ContextInfo
    {
        EGLContext eglContext = null;
        EGLSurface eglSurface = null;
    };


}
