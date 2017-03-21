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

/**
 * This object parses Mapbox Vector Tile format one tile at a time.
 * Set it up and then ask it to parse the given blob of data into vector objects.
 * <br>
 * Just supports vector data for now.
 */
public class MapboxVectorTileParser
{
    MapboxVectorTileParser()
    {
        initialise();
    }

    public final static int GeomTypeUnknown = 0;
    public final static int GeomTypePoint = 1;
    public final static int GeomTypeLineString = 2;
    public final static int GeomTypePolygon = 3;

    /**
     * Data returned from a single parsed tile.
     * For now it's just vector objects, but eventually it may be images as well.
     */
    public static class DataReturn
    {
        public VectorObject[] vectorObjects;
    }

    /**
     * Parse the data from a single tile.
     * This returns a collection of vector objects in DataReturn.
     *
     * @param data The input data to parse.  You should have fetched this on your own.
     * @return Returns null on failure to parse.
     */
    public DataReturn parseData(byte[] data,Mbr mbr)
    {
        DataReturn dataReturn = new DataReturn();
        dataReturn.vectorObjects = parseDataNative(data,mbr.ll.getX(),mbr.ll.getY(),mbr.ur.getX(),mbr.ur.getY());

        return dataReturn;
    }

    native VectorObject[] parseDataNative(byte[] data,double minX,double minY,double maxX,double maxY);

    public void finalize()
    {
        dispose();
    }

    static
    {
        nativeInit();
    }
    native void initialise();
    native void dispose();
    private static native void nativeInit();
    protected long nativeHandle;
}
