/*
 *  TileInfoNew.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 3/20/19.
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

/**
 * TileInfo New Base class.
 * <br>
 * This describes a single source of data tiles.  A uses these to
 * figure out what to load when and where.  The loader passes the result of
 * fetchInfoForTile to a TileFetcher to get the data it wants.
 */
public class TileInfoNew
{
    // Use a subclass, not this
    protected TileInfoNew()
    {}

    /**
     * A unique ID used for sorting tile info objects when loading.
     */
    public long uniqueID = Identifiable.genID();

    /**
     *  The minimum zoom level available.
     *
     *  This is the lowest level we'll try to fetch.
     *  Any levels below that will be filled in with placeholders.
     *  Those are empty, but they allow us to load tiles beneath.
     */
    public int minZoom = -1;

    /**
     *  The maximum zoom level available.
     *
     *  This is the highest level (e.g. largest) that we'll
     *  fetch for a given pyramid tile source.  The source can sparse,
     *  so you are not required to have these tiles available, but this
     *  is as high as the loader will fetch.
     */
    public int maxZoom = -1;

    /**
     *   FetchInfo object for a given tile.
     *
     *   The FetchInfo object is specific to the type of TileFetcher you're using and
     *   tells the fetcher how to get the data you wawnt.
     *   RemoteTileFetchers want a RemoteTileInfoNew object.
     */
    public Object fetchInfoForTile(TileID tileID,boolean flipY)
    {
        return null;
    }
}
