/*
 *  TileFetcher.java
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
 * Tile Fetcher protocol.
 *
 * The tile fetcher interacts with loaders that want tiles, as demanded by samplers.
 * A given data source (e.g. remote URL, MBTiles) needs one of these to fetch and return data.
 */
public interface TileFetcher
{
    /**
     * Name of this tile fetcher.  Used for coordinating tile sources.
     */
    public String getFetcherName();

    /**
     * Add a whole group of requests at once.
     * This is useful if we want to avoid low priority tiles grabbing the slots first
     */
    public void startTileFetches(TileFetchRequest[] requests);

    /**
     * Update an active request with a new priority and importance.
     */
    public Object updateTileFetch(Object fetchID,int priority,float importance);

    /**
     * Cancel a group of requests at once
     * Use the object returned by the startTileFetch call (which is just a Request object)
     */
    public void cancelTileFetches(Object[] fetchIDs);

    /**
     * Kill all outstanding connections and clean up.
     */
    public void shutdown();
}
