/*
 *  SimpleTileFetcher.javae
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 6/4/19.
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

import android.os.AsyncTask;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Queue;
import java.util.TreeSet;

/**
 * Simple Tile Fetcher is meant for sub classing.
 *
 * Some data sources aren't all that complex.  You read from a local source,
 * you return the data.  Something else turns it into visible objects.  Simple.
 *
 * To implement one of those, subclass the Simple Tile Fetcher and let it do the
 * tricky bits.
 */
public class SimpleTileFetcher extends HandlerThread implements TileFetcher
{
    protected boolean valid = false;
    protected String name;
    public int minZoom = -1,maxZoom = -1;

    /**
     * Construct with the name, min and max zoom levels.
     */
    public SimpleTileFetcher(String inName)
    {
        super(inName);

        name = inName;
    }

    /**
     * We don't need to describe a remote URL, so this is
     * basically a stub that passes back the tile ID.
     */
    public class SimpleTileInfo extends TileInfoNew
    {
        SimpleTileInfo(int inMinZoom,int inMaxZoom)
        {
            minZoom = inMinZoom;
            maxZoom = inMaxZoom;
        }

        @Override public Object fetchInfoForTile(TileID tileID,boolean flipY)
        {
            return new SimpleTileFetchInfo(tileID);
        }
    }

    /**
     * The tile info is used by the quad paging logic to figure out
     * min/max zoom level and such.  This version is very simple
     * compared to the RemoteTileInfoNew object.
     */
    public SimpleTileInfo getTileInfo()
    {
        if (tileInfo == null) {
            tileInfo = new SimpleTileInfo(minZoom, maxZoom);
        }

        return tileInfo;
    }

    /**
     * No remote URLs to track, so we just keep the
     * tile ID.
     */
    protected class SimpleTileFetchInfo
    {
        SimpleTileFetchInfo(TileID inTileID)
        {
            tileID = inTileID;
        }

        public TileID tileID = null;
    }

    protected SimpleTileInfo tileInfo;

    /**
     * Wrapper around the fetch request so we can prioritize loads.
     */
    public class TileInfo implements Comparable<TileInfo>
    {
        // Priority before importance
        int priority = 0;

        // Importance of this tile request as passed in by the fetch request
        float importance = 0.0f;

        // The request that came from the tile fetcher
        TileFetchRequest request = null;

        // Simple description of where we get the thing we're fetching
        SimpleTileFetchInfo fetchInfo = null;

        @Override public int compareTo(TileInfo that)
        {
            if (priority == that.priority) {
                if (importance == that.importance) {
                    return fetchInfo.tileID.compareTo(that.fetchInfo.tileID);
                } else {
                    return (importance < that.importance) ? -1 : 1;
                }
            } else {
                return (priority < that.priority) ? -1 : 1;
            }
        }

        @Override public boolean equals(Object that)
        {
            if (this == that)
                return true;

            if (!(that instanceof TileInfo))
                return false;

            TileInfo lhs = (TileInfo)that;
            return priority == lhs.priority && importance == lhs.importance &&
                    fetchInfo.tileID.equals(lhs.fetchInfo.tileID);
        }

        @Override public int hashCode()
        {
            int result = 17;
            result = 31 * result + priority;
            result = 31 * result + (int)importance;
            result = 31 * result + fetchInfo.tileID.hashCode();

            return result;
        }
    }

    // Tiles sorted by priority, importance etc...
    TreeSet<TileInfo> toLoad = new TreeSet<TileInfo>();
    // Tiles sorted by fetch request
    HashMap<TileFetchRequest,TileInfo> tilesByFetchRequest = new HashMap<TileFetchRequest,TileInfo>();

    /**
     * Name of this tile fetcher.  Used for coordinating tile sources.
     */
    @Override public String getFetcherName()
    {
        return name;
    }

    /**
     * Add a bunch of requests to the queue and kick them off in a tick.
     */
    @Override public void startTileFetches(final TileFetchRequest[] requests)
    {
        if (!valid)
            return;

        Handler handler = new Handler(getLooper());
        handler.post(new Runnable() {
            @Override
            public void run() {
                for (TileFetchRequest request : requests) {
                    // Set up a new request
                    TileInfo tileInfo = new TileInfo();
                    tileInfo.priority = request.priority;
                    tileInfo.importance = request.importance;
                    tileInfo.request = request;
                    tileInfo.fetchInfo = (SimpleTileFetchInfo)request.fetchInfo;
                    toLoad.add(tileInfo);
                    tilesByFetchRequest.put(request,tileInfo);
                }

                scheduleLoading();
            }
        });
    }

    boolean scheduled = false;

    // Schedule the next loading update
    protected void scheduleLoading()
    {
        if (!scheduled) {
            Handler handler = new Handler(getLooper());
            handler.post(new Runnable() {
                @Override
                public void run() {
                    updateLoading();
                }
            });
        }
    }

    // Keep track of the number of tiles parsing at once
    // The ThreadPoolExecutor gets testy beyond a certain number
    private static final int MaxParsing = 8;
    private int numParsing = 0;
    protected Queue<AsyncTask<Void, Void, Void> > tasks = new LinkedList<>();
    // If set by the subclass, we'll just treat null data as valid
    // This is helpful when you have sparse data sets
    protected boolean neverFail = false;

    // Load a tile and pass off the results
    protected void updateLoading()
    {
        scheduled = false;

        if (!valid)
            return;

        if (toLoad.isEmpty())
            return;

        final TileInfo tileInfo = toLoad.last();
        toLoad.remove(tileInfo);

        // Load the data tile
        final byte[] data = dataForTile(tileInfo.fetchInfo,tileInfo.fetchInfo.tileID);

        // We assume they'll be parsing things which will take time
        tasks.add(new AsyncTask<Void, Void, Void>() {
            protected Void doInBackground(Void... unused) {
                if (data != null || neverFail)
                    tileInfo.request.callback.success(tileInfo.request, data);
                else
                    tileInfo.request.callback.failure(tileInfo.request,"Failed to read MBTiles File");

                // Add more tasks, if there are any
                Handler handler = new Handler(getLooper());
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        numParsing--;
                        updateTasks();
                    }
                });

                return null;
            }
        });

        finishTile(tileInfo);

        updateTasks();
        scheduleLoading();
    }

    // Keep a certain number of tasks running for parsing, but no more
    protected void updateTasks() {
        if (numParsing < MaxParsing && !tasks.isEmpty()) {
            AsyncTask<Void, Void, Void> task = tasks.remove();
            task.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, (Void)null);
            numParsing++;
        }
    }

    protected void finishTile(TileInfo tileInfo)
    {
        toLoad.remove(tileInfo);
        tilesByFetchRequest.remove(tileInfo.request);
    }

    /**
     * Update an active request with a new priority and importance.
     */
    @Override public Object updateTileFetch(final Object fetchRequest,final int priority,final float importance)
    {
        if (!valid)
            return null;

        Handler handler = new Handler(getLooper());
        handler.post(new Runnable() {
            @Override
            public void run() {
                TileInfo tileInfo = tilesByFetchRequest.get(fetchRequest);
                if (tileInfo != null) {
                    toLoad.remove(tileInfo);
                    tileInfo.priority = priority;
                    tileInfo.importance = importance;
                    toLoad.add(tileInfo);
                }
            }
        });

        return fetchRequest;
    }

    /**
     * Cancel a group of requests at once
     * Use the object returned by the startTileFetch call (which is just a Request object)
     */
    @Override public void cancelTileFetches(final Object[] fetches)
    {
        if (!valid)
            return;

        Handler handler = new Handler(getLooper());
        handler.post(new Runnable() {
            @Override
            public void run() {
                for (Object fetchInfo : fetches) {
                    TileInfo tileInfo = tilesByFetchRequest.get(fetchInfo);
                    if (tileInfo != null) {
                        tilesByFetchRequest.remove(fetchInfo);
                        toLoad.remove(tileInfo);
                    }
                }
            }
        });
    }

    /**
     * Kill all outstanding connections and clean up.
     */
    @Override public void shutdown()
    {
        valid = false;
        quitSafely();
    }

    /**
     * Fill this in in your subclass.
     * @return Return the data for a particular tile.  This is synchronous, but may be called
     * on a random thread.
     */
    public byte[] dataForTile(Object fetchInfo,TileID tileID)
    {
        Log.e("Maply", "You forgot to fill in dataForTile in your SimpleTileFetcher subclass.");
        return null;
    }
}
