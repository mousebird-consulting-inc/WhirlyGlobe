/*  MapboxVectorInterpreter.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/16/19.
 *  Copyright 2011-2021 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

package com.mousebird.maply;

import android.graphics.Bitmap;
import android.graphics.Color;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Collections;
import java.util.zip.GZIPInputStream;
import java.util.zip.ZipEntry;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLContext;

/**
 * The Mapbox Vector (Tile) Interpreter parses raw vector tile data
 * and turns it into visual objects.
 */
public class MapboxVectorInterpreter implements LoaderInterpreter
{
    VectorStyleInterface imageStyleGen;
    VectorStyleInterface styleGen;
    WeakReference<BaseController> vc;
    RenderController tileRender;
    MapboxVectorTileParser parser;
    MapboxVectorTileParser imageParser;

    /**
     * This version of the init builds visual features for vector tiles.
     * <br>
     * This interpreter can be used as overlay data or a full map, depending
     * on how your style is configured.
     */
    public MapboxVectorInterpreter(VectorStyleInterface inStyleInter,BaseController inVC) {
        styleGen = inStyleInter;
        vc = new WeakReference<>(inVC);
        parser = new MapboxVectorTileParser(inStyleInter,inVC);
    }

    /**
     * This version of the init takes an image style set, a vector style set,
     * and an offline renderer to build the image tiles.
     * <br>
     * Image tiles will be used as a background and vectors put on top of them.
     * This is very nice for the globe, but requires specialized style sheets.
     *
     * @param inImageStyle Style used when rendering to image tiles
     * @param inTileRender Renderer used to draw the image tiles
     * @param inVectorStyle Style used in the main controller (e.g. the overlay)
     * @param inVC Controller where everything eventually goes
     */
    public MapboxVectorInterpreter(VectorStyleInterface inImageStyle,RenderController inTileRender,
                                   VectorStyleInterface inVectorStyle,BaseController inVC)
    {
        imageStyleGen = inImageStyle;
        styleGen = inVectorStyle;
        tileRender = inTileRender;
        vc = new WeakReference<>(inVC);
        tileRender.clearLights();

        parser = new MapboxVectorTileParser(styleGen,inVC);
        if (inTileRender != null) {
            imageParser = new MapboxVectorTileParser(imageStyleGen, inTileRender);
            imageParser.setLocalCoords(true);
        }
    }

    WeakReference<QuadPagingLoader> objectLoader;
    WeakReference<QuadImageLoaderBase> imageLoader;

    public void setLoader(QuadLoaderBase inLoader) {
        if (inLoader instanceof QuadPagingLoader) {
            objectLoader = new WeakReference<>((QuadPagingLoader)inLoader);
        } else if (inLoader instanceof QuadImageLoaderBase) {
            imageLoader = new WeakReference<>((QuadImageLoaderBase)inLoader);
        }
    }

    static final double MAX_EXTENT = 20037508.342789244;
    static final double WGS84_a    = 6378137.0;  // meters
    static final double WGS84_a_2  = WGS84_a / 2.0;

    // Convert to spherical mercator directly
    static Point2d toMerc(Point2d pt)
    {
        return new Point2d(
                Math.toDegrees(pt.getX()) * MAX_EXTENT / 180.0,
                WGS84_a_2 * Math.log((1.0 + Math.sin(pt.getY())) / (1.0 - Math.sin(pt.getY()))));
    }

    public void dataForTile(LoaderReturn loadReturn,QuadLoaderBase loader)
    {
        if (styleGen != null) styleGen.setZoomSlot(loader.getZoomSlot());
        if (imageStyleGen != null) imageStyleGen.setZoomSlot(loader.getZoomSlot());

        byte[] data = loadReturn.getFirstData();
        if (data == null)
            return;

        GZIPInputStream in = null;
        ByteArrayOutputStream bout = null;
        ByteArrayInputStream bin = null;
        try {
            // Unzip if it's compressed
            bin = new ByteArrayInputStream(data);
            in = new GZIPInputStream(bin, data.length);
            // Bail as soon as possible if there's a problem
            if (in.available() != 0) {
                bout = new ByteArrayOutputStream(data.length * 2);

                byte[] buffer = new byte[1024];
                for (int count; (count = in.read(buffer)) != -1; ) {
                    bout.write(buffer, 0, count);
                }

                data = bout.toByteArray();
            }
        } catch (Exception ex) {
            // We'll try the raw data if we can't decompress it
        }  finally  {
            // Clean everything up in the opposite order they were created.
            if (bout != null) try {
                bout.close ();
            } catch (IOException ignore){}

            if (in != null) try {
                in.close ();
            } catch (IOException ignore){}

            if (bin != null) try {
                bin.close ();
            } catch (IOException ignore){}
        }

        // Don't let the sampling layer shut down while we're working
        QuadSamplingLayer samplingLayer = loader.samplingLayer.get();
        if (samplingLayer == null || !samplingLayer.layerThread.startOfWork())
            return;

        try {
            // Parse the data into vectors
            // This will skip layers we don't care about
            TileID tileID = loadReturn.getTileID();
            Mbr locBounds = loader.geoBoundsForTile(tileID);
            locBounds.ll = toMerc(locBounds.ll);
            locBounds.ur = toMerc(locBounds.ur);
            VectorTileData tileData = new VectorTileData(tileID, locBounds, loader.geoBoundsForTile(tileID));
            parser.parseData(data, tileData);
            BaseController theVC = vc.get();
            ArrayList<ComponentObject> ovlObjs = new ArrayList<>();
            if (theVC != null) {
                ComponentObject[] thisOvjObjs = tileData.getComponentObjects("overlay");
                if (thisOvjObjs != null)
                    Collections.addAll(ovlObjs, thisOvjObjs);
                loadReturn.mergeChanges(tileData.getChangeSet());
            }

            // If we have a tile renderer, draw the data into that
            Bitmap tileBitmap = null;
            if (tileRender != null) {
                synchronized (tileRender) {
                    tileRender.setClearColor(imageStyleGen.backgroundColorForZoom(tileID.level));
                    Mbr imageBounds = new Mbr(new Point2d(0.0, 0.0), tileRender.frameSize);
                    VectorTileData imageTileData = new VectorTileData(tileID, imageBounds, locBounds);

                    // Need to activate the renderer, add the data, enable the objects and then clean it all up
                    // We need to use a specific context that comes with the tile renderer
                    RenderControllerInterface.ContextInfo cInfo = RenderController.getEGLContext();
                    tileRender.setEGLContext(null);
                    imageParser.parseData(data, imageTileData);
                    ChangeSet changes = imageTileData.getChangeSet();
                    changes.process(tileRender, tileRender.getScene());
                    changes.dispose();
                    tileRender.enableObjects(imageTileData.getComponentObjects(), RenderControllerInterface.ThreadMode.ThreadCurrent);
                    tileBitmap = tileRender.renderToBitmap();
                    tileRender.removeObjects(imageTileData.getComponentObjects(), RenderControllerInterface.ThreadMode.ThreadCurrent);
                    tileRender.clearContext();
                    imageTileData.dispose();

                    // Reset the OpenGL context back to what it was before
                    // It would have been set up by our own renderer for us on a specific thread
                    theVC.renderControl.setEGLContext(cInfo);
                }
            }

            // Sort out overlays if they're there
            ComponentObject[] regObjs = tileData.getComponentObjects();
            if (!ovlObjs.isEmpty()) {
                // Filter the overlays out of regular objects
                ArrayList<ComponentObject> minusOvls = new ArrayList<>();
                for (ComponentObject compObj : regObjs) {
                    // Look for it in the overlays
                    boolean found = false;
                    for (ComponentObject ovlObj : ovlObjs) {
                        if (ovlObj.getID() == compObj.getID()) {
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                        minusOvls.add(compObj);
                }

                regObjs = minusOvls.toArray(new ComponentObject[1]);
            }

            // Merge the results into the loadReturn
            if (objectLoader != null) {
                ObjectLoaderReturn objLoadReturn = (ObjectLoaderReturn) loadReturn;
                if (!ovlObjs.isEmpty())
                    objLoadReturn.addOverlayComponentObjects(ovlObjs.toArray(new ComponentObject[1]));
                objLoadReturn.addComponentObjects(regObjs);
            } else if (imageLoader != null) {
                ImageLoaderReturn imgLoadReturn = (ImageLoaderReturn) loadReturn;
                if (!ovlObjs.isEmpty())
                    imgLoadReturn.addOverlayComponentObjects(ovlObjs.toArray(new ComponentObject[1]));
                imgLoadReturn.addComponentObjects(regObjs);
                if (tileBitmap != null) {
                    imgLoadReturn.addBitmap(tileBitmap);
                } else if (imgLoadReturn.getImages().length == 0) {
                    // Make a single color background image
                    // We have to do this each time because it can change per level
                    final int bgColor = styleGen.backgroundColorForZoom(tileID.level);

                    // The color will be the same for all the tiles in a level, try to reuse it
                    Bitmap img;
                    synchronized (backgroundLock) {
                        if (lastBackground != null && lastBackgroundColor == bgColor) {
                            img = lastBackground;
                        } else {
                            img = Bitmap.createBitmap(BackImageSize, BackImageSize, Bitmap.Config.ARGB_8888);
                            img.eraseColor(bgColor);
                            lastBackground = img;
                            lastBackgroundColor = bgColor;
                        }
                    }

                    ImageTile tile = new ImageTile(img);
                    // We're on a background thread, so set up the texture now
                    tile.preprocessTexture();

                    imgLoadReturn.addImageTile(tile);
                }
            }
        } finally {
            // Let the sampling layer shut down
            samplingLayer.layerThread.endOfWork();
        }
    }

    private Bitmap lastBackground = null;
    private int lastBackgroundColor = -1;
    private final Object backgroundLock = new Object();

    private static final int BackImageSize = 16;
}