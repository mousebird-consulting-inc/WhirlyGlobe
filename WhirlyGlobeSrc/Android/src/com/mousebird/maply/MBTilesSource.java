/*
 *  MBTilesSource.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/11/16.
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
 * The MBTiles Source reads Mapbox style MBTiles files.
 */
public class MBTilesSource implements QuadImageTileLayer.TileSource
{
    int minZoom=0,maxZoom=0;
    int pixelsPerSide=256;

    public MBTilesSource(String mbTileFileName)
    {
        // Note: Do the initial read from the database to get the header info
        //       This includes min and max zoom as well as anything you need
        //       to know about reading the rest of it.
    }

    /**
     * The minimum zoom level you'll be called about to create a tile for.
     */
    public int minZoom()
    {
        return minZoom;
    }

    /**
     * The maximum zoom level you'll be called about to create a tile for.
     */
    public int maxZoom()
    {
        return maxZoom;
    }

    /**
     * The number of pixels square for each tile.
     */
    public int pixelsPerSide()
    {
        return pixelsPerSide;
    }

    // The coordinate system for these is almost always spherical mercator
    public CoordSystem coordSys = new SphericalMercatorCoordSystem();

    /**
     * This tells you when to start fetching a given tile. When you've fetched
     * the image you'll want to call loadedTile().  If you fail to fetch an image
     * call that with nil.
     *
     * @param layer The layer asking for the fetch.
     * @param tileID The tile ID to fetch.
     * @param frame If the source support multiple frames, this is the frame.  Otherwise -1.
     */
    public void startFetchForTile(QuadImageTileLayerInterface layer,MaplyTileID tileID,int frame)
    {
        // Note: Start the fetch for a single tile, ideally on another thread.
        //       When the tile data is in, call the layer loadedTile() method.
    }
}
