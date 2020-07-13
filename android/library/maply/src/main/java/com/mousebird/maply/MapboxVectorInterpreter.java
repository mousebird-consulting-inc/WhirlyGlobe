/*
 *  MapboxVectorInterpreter.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/16/19.
 *  Copyright 2011-2019 mousebird consulting
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
 *
 */

package com.mousebird.maply;

import android.graphics.Bitmap;
import android.graphics.Color;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
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
        vc = new WeakReference<BaseController>(inVC);
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
        vc = new WeakReference<BaseController>(inVC);
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
            objectLoader = new WeakReference<QuadPagingLoader>((QuadPagingLoader)inLoader);
        } else if (inLoader instanceof QuadImageLoaderBase) {
            imageLoader = new WeakReference<QuadImageLoaderBase>((QuadImageLoaderBase)inLoader);
        }
    }

    static double MAX_EXTENT = 20037508.342789244;

    // Convert to spherical mercator directly
    Point2d toMerc(Point2d pt)
    {
        Point2d newPt = new Point2d();
        newPt.setValue(Math.toDegrees(pt.getX()) * MAX_EXTENT / 180.0,
                3189068.5 * Math.log((1.0 + Math.sin(pt.getY())) / (1.0 - Math.sin(pt.getY()))));

        return newPt;
    }

    public void dataForTile(LoaderReturn loadReturn,QuadLoaderBase loader)
    {
        byte[] data = loadReturn.getFirstData();
        if (data == null)
            return;

        try {
            // Unzip if it's compressed
            ByteArrayInputStream bin = new ByteArrayInputStream(data);
            GZIPInputStream in = new GZIPInputStream(bin);
            ByteArrayOutputStream bout = new ByteArrayOutputStream(data.length * 2);

            ZipEntry ze;
            byte[] buffer = new byte[1024];
            int count;
            while ((count = in.read(buffer)) != -1)
                bout.write(buffer, 0, count);

            data = bout.toByteArray();
        } catch (Exception ex) {
            // We'll try the raw data if we can't decompress it
        }

        // Parse the data into vectors
        // This will skip layers we don't care about
        TileID tileID = loadReturn.getTileID();
        Mbr locBounds = loader.geoBoundsForTile(tileID);
        locBounds.ll = toMerc(locBounds.ll);
        locBounds.ur = toMerc(locBounds.ur);
        VectorTileData tileData = new VectorTileData(tileID,locBounds,loader.geoBoundsForTile(tileID));
        parser.parseData(data,tileData);
        BaseController theVC = vc.get();
        ArrayList<ComponentObject> ovlObjs = new ArrayList<ComponentObject>();
        if (theVC != null) {
            ComponentObject[] thisOvjObjs = tileData.getComponentObjects("overlay");
            if (thisOvjObjs != null)
                Collections.addAll(ovlObjs,thisOvjObjs);
            loadReturn.mergeChanges(tileData.getChangeSet());
        }

        // If we have a tile renderer, draw the data into that
        Bitmap tileBitmap = null;
        if (tileRender != null) {
            synchronized (tileRender) {
                tileRender.setClearColor(imageStyleGen.backgroundColorForZoom(tileID.level));
                Mbr imageBounds = new Mbr(new Point2d(0.0,0.0), tileRender.frameSize);
                VectorTileData imageTileData = new VectorTileData(tileID,imageBounds,locBounds);

                // Need to activate the renderer, add the data, enable the objects and then clean it all up
                // We need to use a specific context that comes with the tile renderer
                RenderControllerInterface.ContextInfo cInfo = RenderController.getEGLContext();
                tileRender.setEGLContext(null);
                imageParser.parseData(data,imageTileData);
                ChangeSet changes = imageTileData.getChangeSet();
                changes.process(tileRender,tileRender.getScene());
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
            ArrayList<ComponentObject> minusOvls = new ArrayList<ComponentObject>();
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
            ObjectLoaderReturn objLoadReturn = (ObjectLoaderReturn)loadReturn;
            if (!ovlObjs.isEmpty())
                objLoadReturn.addOverlayComponentObjects(ovlObjs.toArray(new ComponentObject[1]));
            objLoadReturn.addComponentObjects(regObjs);
        } else if (imageLoader != null) {
            ImageLoaderReturn imgLoadReturn = (ImageLoaderReturn)loadReturn;
            if (!ovlObjs.isEmpty())
                imgLoadReturn.addOverlayComponentObjects(ovlObjs.toArray(new ComponentObject[1]));
            imgLoadReturn.addComponentObjects(regObjs);
            if (tileBitmap != null)
                imgLoadReturn.addBitmap(tileBitmap);
        }
    }

}
