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

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.lang.ref.WeakReference;
import java.util.zip.GZIPInputStream;
import java.util.zip.ZipEntry;

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

        parser = new MapboxVectorTileParser(styleGen,inVC);
        if (inTileRender != null)
            imageParser = new MapboxVectorTileParser(imageStyleGen,inTileRender);
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

        // If we have a tile renderer, draw the data into that
        Bitmap tileBitmap = null;
        if (tileRender != null) {
            Mbr imageBounds = new Mbr(new Point2d(0.0,0.0), tileRender.frameSize);
            VectorTileData imageTileData = new VectorTileData(tileID,imageBounds,imageBounds);
            synchronized (tileRender) {
                imageParser.parseData(data,imageTileData);
                tileBitmap = tileRender.renderToBitmap();
            }
        }

        // Merge the results into the loadReturn
        if (objectLoader != null) {
            ObjectLoaderReturn objLoadReturn = (ObjectLoaderReturn)loadReturn;
            objLoadReturn.addComponentObjects(tileData.getComponentObjects());
        } else if (imageLoader != null) {
            ImageLoaderReturn imgLoadReturn = (ImageLoaderReturn)loadReturn;
            imgLoadReturn.addComponentObjects(tileData.getComponentObjects());
            if (tileBitmap != null)
                imgLoadReturn.addBitmap(tileBitmap);
        }
    }

}
