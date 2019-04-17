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
    VectorStyleInterface styleGen;
    WeakReference<BaseController> vc;
    MapboxVectorTileParser parser;

    public MapboxVectorInterpreter(VectorStyleInterface inStyleInter,BaseController inVC) {
        styleGen = inStyleInter;
        vc = new WeakReference<BaseController>(inVC);
        parser = new MapboxVectorTileParser(inStyleInter,inVC);
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
        VectorTileData tileData = new VectorTileData();
        parser.parseData(data,tileData);

        // Merge the results into the loadReturn
        if (objectLoader != null) {
            ObjectLoaderReturn objLoadReturn = (ObjectLoaderReturn)loadReturn;
            objLoadReturn.addComponentObjects(tileData.getComponentObjects());
        } else if (imageLoader != null) {
            ImageLoaderReturn imgLoadReturn = (ImageLoaderReturn)loadReturn;
            imgLoadReturn.addComponentObjects(tileData.getComponentObjects());
        }
    }

}
