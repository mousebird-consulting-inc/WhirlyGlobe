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

import android.annotation.SuppressLint;

import org.jetbrains.annotations.NotNull;

import java.util.Objects;

/**
 *  Generic Tile fetcher request.
 *
 *  A single request for a single tile of data from a single source.
 *  The tile fetcher will... fetch this and call the success or failure callback.
 */
public class TileFetchRequest implements Comparable<TileFetchRequest>
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
        void success(TileFetchRequest fetchRequest,byte[] data);

        /**
         *  Tile Fetcher failure callback.
         */
        void failure(TileFetchRequest fetchRequest,String errorStr);
    };

    /**
     * Fill in these callbacks to get status back from a tile fetch.
     */
    public Callback callback = null;

    /**
     * Convenience helper for calling the success callback
     */
    public Boolean success(byte[] data) {
        if (callback != null) {
            callback.success(this, data);
            return true;
        }
        return false;
    }

    /**
     * Convenience helper for calling the failure callback
     */
    public Boolean failure(String errorStr) {
        if (callback != null) {
            callback.failure(this, errorStr);
            return true;
        }
        return false;
    }

    @Override
    public int compareTo(TileFetchRequest other) {
        int res = priority - other.priority;
        if (res == 0) res = -Float.compare(importance, other.importance);
        if (res == 0) res = group - other.group;
        if (res == 0) res = (int)(tileSource - other.tileSource);
        return res;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof TileFetchRequest)) return false;
        TileFetchRequest that = (TileFetchRequest)o;
        return priority == that.priority &&
                Float.compare(that.importance, importance) == 0 &&
                group == that.group &&
                tileSource == that.tileSource;
    }

    @Override
    public int hashCode() {
        return Objects.hash(priority, importance, group, tileSource, fetchInfo);
    }

    @NotNull
    @SuppressLint("DefaultLocale")
    @Override
    public String toString() {
        return String.format("TileFetchRequest{%d,%f,%d,%d,%s}",
                priority, importance, group, tileSource,
                (fetchInfo != null) ? fetchInfo.toString() : "null");
    }
}
