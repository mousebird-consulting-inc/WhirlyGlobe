/*
 *  TileFetchRequest.java
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
 *  Generic Tile fetcher request.
 *
 *  A single request for a single tile of data from a single source.
 *  The tile fetcher will... fetch this and call the success or failure callback.
 */
public class TileFetchRequest
{
    /**
     * Priority before importance.  Less is more important
     */
    public int priority;

    /**
     * How important this is to us.  Probably screen space.
     */
    public float importance;

    /**
     * If all other values are equal, sort by this.
     * It keeps requests we're waiting for grouped together
     */
    public int group;

    /**
     * A unique ID used for sorting.  Not accessed by the fetcher.
     */
    public long tileSource;

    /**
     * This is requested from a TileInfo object by a loader and then passed
     *  along to the TileFetcher.  TileFetchers expect certain objects.
     *  The RemoteTileFetcher wants a RemoteFetchInfo object and will check.
     *  Other fetchers will want other things.
     */
    public Object fetchInfo;

    public interface Callback {
        /**
         *  Tile Fetcher success callback.
         *
         *  Called on a random thread and won't be marked as loaded until it returns.
         *  This is a good way to limit how many things are loading/parsing at the same time.
         */
        public void success(TileFetchRequest fetchRequest,byte[] data);

        /**
         *  Tile Fetcher failure callback.
         */
        public void failure(TileFetchRequest fetchRequest,String errorStr);
    };

    /**
     * Fill in these callbacks to get status back from a tile fetch.
     */
    public Callback callback = null;
}
