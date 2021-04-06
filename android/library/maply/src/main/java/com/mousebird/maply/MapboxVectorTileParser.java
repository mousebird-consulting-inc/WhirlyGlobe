/*
 *  MapboxVectorTileParser.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/24/16.
 *  Copyright 2011-2016 mousebird consulting
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

import androidx.annotation.Nullable;

import org.jetbrains.annotations.NotNull;

import java.lang.ref.WeakReference;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * This object parses Mapbox Vector Tile format one tile at a time.
 * Set it up and then ask it to parse the given blob of data into vector objects.
 * <br>
 * Just supports vector data for now.
 */
public class MapboxVectorTileParser
{
    VectorStyleInterface styleDelegate = null;
    //VectorStyleWrapper vecStyleWrap = null;
    WeakReference<RenderControllerInterface> viewC;

    private MapboxVectorTileParser() { }

    MapboxVectorTileParser(VectorStyleInterface inStyleDelegate,RenderControllerInterface inViewC)
    {
        styleDelegate = inStyleDelegate;
        viewC = new WeakReference<>(inViewC);

        // If the style delegate is backed by a C++ object, we
        //  can just use that directly.
        if (inStyleDelegate instanceof MapboxVectorStyleSet) {
            initialise(inStyleDelegate,true);
        } else {
            // If not, then the C++ needs to build a wrapper for it
            VectorStyleWrapper vecStyleWrap = new VectorStyleWrapper(inStyleDelegate,inViewC);
            initialise(vecStyleWrap,false);
        }
    }

    public final static int GeomTypeUnknown = 0;
    public final static int GeomTypePoint = 1;
    public final static int GeomTypeLineString = 2;
    public final static int GeomTypePolygon = 3;

    /**
     * Parse the data from a single tile.
     * This returns a collection of vector objects in DataReturn.
     *
     * @param data The input data to parse.  You should have fetched this on your own.
     * @param tileData A container for the data we parse and the styles create.
     * @return Returns null on failure to parse.
     */
    public boolean parseData(byte[] data,VectorTileData tileData)
    {
        return parseDataNative(data, tileData, null);
    }

    /**
     * Parse the data from a single tile.
     * This returns a collection of vector objects in DataReturn.
     *
     * @param data The input data to parse.  You should have fetched this on your own.
     * @param tileData A container for the data we parse and the styles create.
     * @param cancel Cancellation signal, if set the parser will stop at the next feature
     * @return Returns null on failure to parse.
     */
    public boolean parseData(@NotNull byte[] data, @NotNull VectorTileData tileData, @Nullable AtomicBoolean cancel)
    {
        return parseDataNative(data, tileData, cancel);
    }

    native boolean parseDataNative(@NotNull byte[] data,@NotNull VectorTileData tileData,@Nullable AtomicBoolean cancel);

    /// If set, we'll parse into local coordinates as specified by the bounding box, rather than geo coords
    native void setLocalCoords(boolean localCoords);

    public void finalize()
    {
        dispose();
    }

    static
    {
        nativeInit();
    }
    native void initialise(Object vectorStyleDelegate,boolean isMapboxStyle);
    native void dispose();
    private static native void nativeInit();
    protected long nativeHandle;
}
