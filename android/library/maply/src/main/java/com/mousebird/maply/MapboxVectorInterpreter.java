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
import android.graphics.BitmapFactory;
import android.util.Log;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Collections;
import java.util.concurrent.Callable;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.zip.GZIPInputStream;
import java.util.zip.InflaterInputStream;

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

    private interface StreamMaker {
        FilterInputStream make(InputStream rawStream, int length) throws IOException;
    }

    /**
     * Try decoding the data using the specified filter stream, return it as-is if anything goes wrong
     */
    private byte[] decodeStream(byte[] data, LoaderReturn loadReturn, StreamMaker maker) {
        FilterInputStream in = null;
        ByteArrayOutputStream bout = null;
        ByteArrayInputStream bin = null;
        try {
            // Unzip if it's compressed
            bin = new ByteArrayInputStream(data);
            in = maker.make(bin, data.length);
            // Bail as soon as possible if there's a problem
            if (in.available() != 0) {
                bout = new ByteArrayOutputStream(data.length * 2);

                byte[] buffer = new byte[1024];
                for (int count; (count = in.read(buffer)) != 0; ) {
                    if (loadReturn.isCanceled()) {
                        return null;
                    }
                    if (count < 1) {
                        break;
                    }
                    bout.write(buffer, 0, count);
                }
                return bout.toByteArray();
            }
        } catch (Exception ignored) {
            // No good, try something else
        }  finally  {
            // Clean everything up in the opposite order they were created.
            if (bout != null) try { bout.close (); } catch (IOException ignore){}
            if (in != null) try { in.close (); } catch (IOException ignore){}
            if (bin != null) try { bin.close (); } catch (IOException ignore){}
        }
        return data;
    }

    /**
     * Decode compressed data, or return it as-is if not decode-able
     */
    private byte[] decodeStream(byte[] data, LoaderReturn loadReturn) {
        if (data.length > 2) {
            if (data[0] == (byte)0x1F && data[1] == (byte)0x8B) {
                return decodeStream(data, loadReturn, GZIPInputStream::new);
            } else if (data[0] == (byte)0x78) {
                return decodeStream(data, loadReturn, (b,len) -> new InflaterInputStream(b));
            }
        }
        return data;
    }

    public void dataForTile(LoaderReturn loadReturn,QuadLoaderBase loader)
    {
        BaseController theVC = vc.get();
        if (theVC == null) {
            return;
        }

        if (styleGen != null) styleGen.setZoomSlot(loader.getZoomSlot());
        if (imageStyleGen != null) imageStyleGen.setZoomSlot(loader.getZoomSlot());

        ArrayList<Bitmap> images = new ArrayList<>();
        ArrayList<byte[]> pbfData = new ArrayList<>();
        for (byte[] data : loadReturn.getTileData())
        {
            // Decode compressed data
            data = decodeStream(data, loadReturn);
            if (data == null || data.length < 1) {
                if (loadReturn.isCanceled()) {
                    return;
                }
                loadReturn.errorString = "Decode Failed";
                continue;
            }

            // See if it's an image
            Bitmap image = BitmapFactory.decodeByteArray(data, 0, data.length);
            if (image != null) {
                images.add(image);
            } else {
                pbfData.add(data);
            }
        }

        if (images.isEmpty() && pbfData.isEmpty()) {
            loadReturn.errorString = "No usable data";
            return;
        }

        // Don't let the sampling layer shut down while we're working
        QuadSamplingLayer samplingLayer = loader.samplingLayer.get();
        if (samplingLayer == null || !samplingLayer.layerThread.startOfWork()) {
            return;
        }

        try {   // ensure endOfWork is called
            // Parse the data into vectors
            // This will skip layers we don't care about
            TileID tileID = loadReturn.getTileID();
            Mbr locBounds = loader.geoBoundsForTile(tileID);
            locBounds.ll = toMerc(locBounds.ll);
            locBounds.ur = toMerc(locBounds.ur);

            VectorTileData tileData = new VectorTileData(tileID, locBounds, loader.geoBoundsForTile(tileID));

            for (byte[] data : pbfData) {
                if (!parser.parseData(data, tileData, loadReturn) || loadReturn.isCanceled()) {
                    if (loadReturn.isCanceled()) {
                        return;
                    } else {
                        Log.w(getClass().getSimpleName(), "Tile parsing failed for " + tileID);
                    }
                }
            }

            ArrayList<ComponentObject> ovlObjs = new ArrayList<>();
            ComponentObject[] thisOvjObjs = tileData.getComponentObjects("overlay");
            if (thisOvjObjs != null) {
                Collections.addAll(ovlObjs, thisOvjObjs);
            }

            loadReturn.mergeChanges(tileData.getChangeSet());

            if (loadReturn.isCanceled()) {
                return;
            }

            // If we have a tile renderer, draw the data into that
            Bitmap tileBitmap = null;
            // multiple checks and synchronization are ok, tileRender is never set to null
            if (tileRender != null) {
                synchronized (tileRender) {
                    tileRender.setClearColor(imageStyleGen.backgroundColorForZoom(tileID.level));
                    Mbr imageBounds = new Mbr(new Point2d(0.0, 0.0), tileRender.frameSize);
                    VectorTileData imageTileData = new VectorTileData(tileID, imageBounds, locBounds);

                    // Need to activate the renderer, add the data, enable the objects and then clean it all up
                    // We need to use a specific context that comes with the tile renderer
                    RenderControllerInterface.ContextInfo cInfo = RenderController.getEGLContext();
                    tileRender.setEGLContext(null);

                    for (byte[] data : pbfData) {
                        if (!imageParser.parseData(data, imageTileData, loadReturn) || loadReturn.isCanceled()) {
                            if (loadReturn.isCanceled()) {
                                return;
                            } else {
                                Log.w(getClass().getSimpleName(), "Tile parsing failed for " + tileID);
                            }
                        }
                    }

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

            if (loadReturn.isCanceled()) {
                return;
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

            if (loadReturn.isCanceled()) {
                return;
            }

            // Merge the results into the loadReturn
            if (!ovlObjs.isEmpty()) {
                loadReturn.addOverlayComponentObjects(ovlObjs.toArray(new ComponentObject[1]));
            }
            loadReturn.addComponentObjects(regObjs);

            if (loadReturn instanceof ImageLoaderReturn) {
                ImageLoaderReturn imgLoadReturn = (ImageLoaderReturn)loadReturn;

                if (tileBitmap != null) {
                    images.add(tileBitmap);
                } else if (images.isEmpty()) {
                    // Make a single color background image
                    // We have to do this each time because it can change per level
                    final int bgColor = styleGen.backgroundColorForZoom(tileID.level);

                    // The color will be the same for all the tiles in a level, try to reuse it
                    final Bitmap image;
                    synchronized (backgroundLock) {
                        if (lastBackground != null && lastBackgroundColor == bgColor) {
                            image = lastBackground;
                        } else {
                            image = Bitmap.createBitmap(BackImageSize, BackImageSize, Bitmap.Config.ARGB_8888);
                            image.eraseColor(bgColor);
                            lastBackground = image;
                            lastBackgroundColor = bgColor;
                        }
                    }
                    images.add(image);
                }

                for (Bitmap image : images) {
                    ImageTile tile = new ImageTile(image);
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