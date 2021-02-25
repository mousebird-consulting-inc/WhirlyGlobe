/*
 *  RemoteTileFetcher.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 3/21/19.
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
import android.service.quicksettings.Tile;
import android.util.Log;

import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.lang.ref.WeakReference;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.TreeSet;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

public class RemoteTileFetcher extends HandlerThread implements TileFetcher
{
    protected boolean valid = false;

    String name = null;

    /**
     * Set this to get way too much debugging output.
     */
    public boolean debugMode = false;

    /**
     * Number of connections we'll allow at once.
     * You can change this later, but it'll take a little time to update.
     */
    int numConnections = 8;

    /**
     * Name of this tile fetcher.  Used for coordinating tile sources.
     */
    public String getFetcherName() {
        return name;
    }

    enum TileInfoState {ToLoad,Loading,None}

    /**
     * A single tile that we're supposed to be loading.
     */
    public class TileInfo implements Comparable<TileInfo>
    {
        TileInfoState state;

        // Set if we already know the tile is cached
        boolean isLocal = false;

        // Used to uniquely identify a group of requests
        long tileSource = 0;

        // Priority before importance
        int priority = 0;

        // Importance of this tile request as passed in by the fetch request
        float importance = 0.0f;

        // Group last.  Used for tiles with multiple sources.
        int group;

        // The request that came from the tile fetcher
        TileFetchRequest request = null;

        // Simple description of where we get the thing we're fetching
        RemoteTileFetchInfo fetchInfo = null;

        // OKHTTP call that can be cancelled
        Call task = null;

        void clear() {
            state = TileInfoState.None;
            isLocal = false;
            request = null;
            fetchInfo = null;
        }

        @Override public int compareTo(TileInfo that)
        {
            if (isLocal == that.isLocal) {
                if (priority == that.priority) {
                    if (importance == that.importance) {
                        if (group == that.group) {
                            if (tileSource == that.tileSource) {
                                if (fetchInfo.uniqueID == that.fetchInfo.uniqueID)
                                    return 0;
                                return (fetchInfo.uniqueID < that.fetchInfo.uniqueID) ? -1 : 1;
                            }
                            return tileSource < that.tileSource ? -1 : 1;
                        }
                        return group < that.group ? -1 : 1;
                    }
                    return importance < that.importance ? -1 : 1;
                }
                return priority < that.priority ? 1 : -1;
            }
            return (isLocal ? 1 : 0) < (that.isLocal ? 1 : 0) ? -1 : 1;
        }

        @Override public boolean equals(Object that)
        {
            if (this == that)
                return true;

            if (!(that instanceof TileInfo))
                return false;

            TileInfo lhs = (TileInfo)that;
            return isLocal == lhs.isLocal && priority == lhs.priority &&
                    importance == lhs.importance && group == lhs.group &&
                    tileSource == lhs.tileSource && fetchInfo.uniqueID == lhs.fetchInfo.uniqueID;
        }

        @Override public int hashCode()
        {
            int result = 17;
            result = 31 * result + (isLocal ? 1 : 0);
            result = 31 * result + priority;
            result = 31 * result + (int)importance;
            result = 31 * result + group;
            result = 31 * result + (int)tileSource;
            result = 31 * result + (int)fetchInfo.uniqueID;

            return result;
        }
    }

    /**
     * Stats collected by the fetcher
     */
    public class Stats {
        // Start of stats collection
        public Date startDate;

        // Total requests, remote and cached
        public int totalRequests;

        // Requests that resulted in a remote HTTP call
        public int remoteRequests;

        // Total requests cancelled
        public int totalCancels;

        // Requests failed
        public int totalFails;

        // Bytes of remote data loaded
        public long remoteData;

        // Bytes of cached data loaded
        public long localData;

        // Total time spent waiting for successful remote data requests
        public double totalLatency;

        // The maximum number of requests we've had at once (since the last reset)
        public int maxActiveRequests;

        // Current number of active requests
        public int activeRequests;

        // Add the given stats to ours
        public void addStats(Stats that) {
            totalRequests += that.totalRequests;
            remoteRequests += that.remoteRequests;
            totalCancels += that.totalCancels;
            totalFails += that.totalFails;
            remoteData += that.remoteData;
            localData += that.localData;
            totalLatency += that.totalLatency;
        }

        // Print out the stats
        public void dump(String name) {
            Log.v("Maply", String.format("---MaplyTileFetcher %s Stats---",name) );
            Log.v("Maply", String.format("   Active Requests = %d",activeRequests) );
            Log.v("Maply", String.format("   Max Active Requests = %d",maxActiveRequests) );
            Log.v("Maply", String.format("   Total Requests = %d",totalRequests) );
            Log.v("Maply", String.format("   Canceled Requests = %d",totalCancels) );
            Log.v("Maply", String.format("   Failed Requests = %d",totalFails) );
            Log.v("Maply", String.format("   Data Transferred = %.2fMB",(float)remoteData) );
            if (remoteRequests > 0) {
                Log.v("Maply", String.format("   Latency per request = %.2fms",totalLatency / remoteRequests * 1000.0) );
                Log.v("Maply", String.format("   Average request size = %.2fKB",remoteData / remoteRequests / 1024.0) );
            }
            Log.v("Maply", String.format("   Cached Data = %.2fMB",localData / (1024.0*1024.0)) );
        }
    }

    protected Stats allStats = null;
    protected Stats recentStats = null;

    /** Return the stats (recent or for all time
     */
    public Stats getStats(boolean allTime)
    {
        if (allTime)
            return allStats;
        else
            return recentStats;
    }

    /**
     * Reset the stats keeping back to zero
     */
    public void resetStats()
    {
        recentStats = new Stats();
    }

    /**
     * Reset the periodic active stats.  These are useful for progress bars.
     */
    public void resetActiveStats() {
        if (!valid)
            return;


        Handler handler = new Handler(getLooper());
        handler.post(new Runnable() {
            @Override
            public void run() {
                recentStats.activeRequests = toLoad.size() + loading.size();
                recentStats.maxActiveRequests = recentStats.activeRequests;
            }
        });
    }

    public void updateActiveStats() {
        recentStats.activeRequests = loading.size()+toLoad.size();
        if (recentStats.activeRequests > recentStats.maxActiveRequests)
            recentStats.maxActiveRequests = recentStats.activeRequests;
    }

    WeakReference<BaseController> control = null;

    RemoteTileFetcher(BaseController baseController, String name)
    {
        super(name);

        control = new WeakReference<BaseController>(baseController);
        client = baseController.getHttpClient();
        valid = true;

        allStats = new Stats();
        recentStats = new Stats();

        // Kick off the thread
        start();
    }

    // Tiles sorted by priority, importance etc...
    TreeSet<TileInfo> loading = new TreeSet<TileInfo>();
    TreeSet<TileInfo> toLoad = new TreeSet<TileInfo>();
    // Tiles sorted by fetch request
    HashMap<TileFetchRequest,TileInfo> tilesByFetchRequest = new HashMap<TileFetchRequest, TileInfo>();

    /**
     * Add a whole group of requests at once.
     * This is useful if we want to avoid low priority tiles grabbing the slots first
     */
    public void startTileFetches(final TileFetchRequest[] requests)
    {
        if (!valid)
            return;

        for (TileFetchRequest request : requests) {
            if (!(request.fetchInfo instanceof RemoteTileFetchInfo)) {
                Log.e("Maply", "RemoteTileFetcher expecting RemoteTileFetchInfo objects.  Aborting.");
                return;
            }
        }

        if (debugMode)
            Log.d("RemoteTileFetcher","Starting (number) tile requests: " + requests.length);

        // Have to run on our own thread
        Handler handler = new Handler(getLooper());
        handler.post(new Runnable() {
            @Override
            public void run() {
                allStats.totalRequests = allStats.totalRequests + requests.length;
                recentStats.totalRequests = recentStats.totalRequests + requests.length;

                for (TileFetchRequest request : requests) {
                    // Set up a new request
                    TileInfo tile = new TileInfo();
                    tile.tileSource = request.tileSource;
                    tile.importance = request.importance;
                    tile.priority = request.priority;
                    tile.group = request.group;
                    tile.state = TileInfoState.ToLoad;
                    tile.request = request;
                    tile.fetchInfo = (RemoteTileFetchInfo)request.fetchInfo;

                    if (debugMode)
                        Log.d("RemoteTileFetcher","Requesting fetch for " + tile.fetchInfo.urlReq);

                    // If it's already cached, let's mark that
                    tile.isLocal = tile.fetchInfo.cacheFile != null && tile.fetchInfo.cacheFile.exists();

                    tilesByFetchRequest.put(request,tile);
                    toLoad.add(tile);
                }

                if (debugMode)
                    Log.d("RemoteTileFetcher","Added (number) tile requests: " + requests.length);

                scheduleLoading();
            }
        });
    }

    boolean scheduled = false;

    // Schedule the next loading update
    protected void scheduleLoading()
    {
        if (!valid)
            return;

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

    OkHttpClient client = null;

    // Load a tile and pass off the results
    protected void updateLoading()
    {
        scheduled = false;

        if (!valid)
            return;

        while (loading.size() < numConnections) {
            updateActiveStats();

            if (toLoad.isEmpty())
                break;

            // Move one over to loading
            final TileInfo tile = toLoad.last();
            toLoad.remove(tile);
            tile.state = TileInfoState.Loading;
            loading.add(tile);

            if (debugMode)
                Log.d("RemoteTileFetcher","Starting load of request: " + tile.fetchInfo.urlReq);

            // Set up the fetching task
            tile.task = client.newCall(tile.fetchInfo.urlReq);

            if (tile.isLocal) {
                // Try reading the data in the background
                new AsyncTask<Void, Void, Void>() {
                    protected Void doInBackground(Void... unused) {
                        handleCache(tile);

                        return null;
                    }
                }.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, (Void)null);
            } else {
                startFetch(tile);
            }
        }

        updateActiveStats();
    }

    // Kick off a network fetch with the appropriate callbacks
    protected void startFetch(final TileInfo tile)
    {
        final double fetchStartTime = System.currentTimeMillis() /1000.0;

        tile.task.enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                if (!valid)
                    return;

                // Ignore cancels, because we do those a lot
                if (e != null) {
                    String mess = e.getLocalizedMessage();
                    if (mess != null && mess.contains("Canceled")) {
                        if (debugMode)
                            Log.d("RemoteTileFetcher","Ignoring a cancel for: " + call.request());
                        return;
                    }
                }

                finishedLoading(tile,null,e, fetchStartTime);
            }

            @Override
            public void onResponse(Call call, Response response) throws IOException {
                if (!valid) {
                    response.body().close();
                    return;
                }

                finishedLoading(tile,response,null, fetchStartTime);
            }
        });
    }

    // Got response back, may be good, may be bad.
    // On a random thread, perhaps
    protected void finishedLoading(final TileInfo inTile, final Response response, final Exception inE,final double fetchStartTile)
    {
        if (!valid)
            return;

        // Have to run on our own thread
        Handler handler = new Handler(getLooper());
        handler.post(new Runnable() {
            @Override
            public void run() {
                // Make sure we still care
                TileInfo tile = tilesByFetchRequest.get(inTile.request);
                if (tile == null) {
                    if (debugMode)
                        Log.d("RemoteTileFetcher","Dropping a tile request because it was cancelled: " + inTile.fetchInfo.urlReq);

                    try {
                        if (response != null)
                            response.body().close();
                    }
                    catch (Exception fooE) {
                    }

                    return;
                }

                boolean success = true;
                Exception e = inE;

                if (debugMode)
                    Log.d("RemoteTileFetcher","Got response for: " + response.request());

                if (response != null) {
                    try {
                        long length = response.body().contentLength();
                        allStats.remoteRequests = allStats.remoteRequests + 1;
                        recentStats.remoteRequests = recentStats.remoteRequests + 1;
                        allStats.remoteData = allStats.remoteData + length;
                        recentStats.remoteData = recentStats.remoteData + length;
                        double howLong = System.currentTimeMillis()/1000.0 - fetchStartTile;
                        allStats.totalLatency = allStats.totalLatency + howLong;
                        recentStats.totalLatency = recentStats.totalLatency + howLong;

                        handleFinishLoading(tile, response.body().bytes(), null);
                    }
                    catch (Exception thisE)
                    {
                        success = false;
                        e = thisE;
                    }
                } else {
                    success = false;
                }

                if (!success) {
                    allStats.totalFails = allStats.totalFails + 1;
                    recentStats.totalFails = recentStats.totalFails + 1;

                    handleFinishLoading(tile, null, e);
                }

                try {
                    if (response != null)
                        response.body().close();
                }
                catch (Exception fooE)
                {
                }
            }
        });
    }

    // Read cache data on a random thread
    protected void handleCache(final TileInfo tile)
    {
        if (!valid)
            return;

        boolean success = true;
        final int size = (int) tile.fetchInfo.cacheFile.length();
        final byte[] data = new byte[size];
        try {
            BufferedInputStream buf = new BufferedInputStream(new FileInputStream(tile.fetchInfo.cacheFile));
            buf.read(data, 0, data.length);
            buf.close();
        } catch (Exception e) {
            success = false;
        }

        if (success) {
            if (!valid)
                return;

            Handler handler = new Handler(getLooper());
            handler.post(new Runnable() {
                @Override
                public void run() {
                    allStats.localData = allStats.localData + size;
                    recentStats.localData = recentStats.localData + size;

                    handleFinishLoading(tile,data,null);
                }
            });

            if (debugMode)
                Log.d("RemoteTileFetcher","Read from cache: " + tile.fetchInfo.urlReq);
        } else {
            // Didn't read it, so go get it
            startFetch(tile);

            if (debugMode)
                Log.d("RemoteTileFetcher","Failed to reach from cache: " + tile.fetchInfo.urlReq);
        }
    }

    // Deal with a tile that was or was not loaded.
    // On our own thread
    protected void handleFinishLoading(TileInfo inTile,final byte[] data,final Exception error)
    {
        // Make sure we still want it
        final TileInfo tile = tilesByFetchRequest.get(inTile.request);
        if (tile == null)
            return;
        // Might have been cancelled
        if (tile.state == TileInfoState.None)
            return;

        BaseController theControl = control.get();

        // Let the caller know on a random thread because parsing may take a while
        // Has to be a worker thread because we need an OpenGL context
        LayerThread backThread = theControl.getWorkingThread();
        backThread.addTask(new Runnable() {
            @Override
            public void run() {
                if (!valid)
                    return;

                if (debugMode)
                    Log.d("RemoteTileFetcher","Returning fetch: " + tile.fetchInfo.urlReq);

                if (error == null) {
                    writeToCache(tile,data);
                    tile.request.callback.success(tile.request, data);
                } else
                    tile.request.callback.failure(tile.request,error.toString());

                if (!valid)
                    return;

                // Now get rid of the tile and kick off a new request
                Handler handler = new Handler(getLooper());
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        finishTile(tile);
                        scheduleLoading();
                    }
                });
            }
        });
    }

    // Write to the local cache.  Called on a random thread.
    protected void writeToCache(TileInfo tile,byte[] data)
    {
        if (tile.fetchInfo.cacheFile == null)
            return;

        OutputStream fOut = null;
        try {
            fOut = new FileOutputStream(tile.fetchInfo.cacheFile);
            fOut.write(data);
        }
        catch (Exception e)
        {
        }
        finally {
            try {
                fOut.close();
            }
            catch (Exception e)
            {
            }
        }
    }

    protected void finishTile(TileInfo inTile)
    {
        // Make sure we still want it
        final TileInfo tile = tilesByFetchRequest.get(inTile.request);
        if (tile == null) {
            if (debugMode)
                Log.d("RemoteTileFetcher","Dropping fetch: " + inTile.fetchInfo.urlReq);
            return;
        }

        tilesByFetchRequest.remove(tile.request);
        loading.remove(tile);
        toLoad.remove(tile);

        updateActiveStats();
    }

    /**
     * Update an active request with a new priority and importance.
     */
    public Object updateTileFetch(final Object fetchRequest, final int priority, final float importance)
    {
        if (!valid)
            return null;

        // Have to run on our own thread
        Handler handler = new Handler(getLooper());
        handler.post(new Runnable() {
            @Override
            public void run() {
                TileInfo tile = tilesByFetchRequest.get(fetchRequest);
                if (tile == null)
                    return;
                // Only mess with tiles that are actually loading
                if (tile.state == TileInfoState.ToLoad) {
                    toLoad.remove(tile);
                    tile.priority = priority;
                    tile.importance = importance;
                    toLoad.add(tile);
                }
            }
        });

        return fetchRequest;
    }

    /**
     * Cancel a group of requests at once
     * Use the object returned by the startTileFetch call (which is just a Request object)
     */
    public void cancelTileFetches(final Object[] fetchRequests)
    {
        if (!valid)
            return;

        if (debugMode)
            Log.d("RemoteTileFetcher","Cancelling (number) fetches: " + fetchRequests.length);

        // Have to run on our own thread
        Handler handler = new Handler(getLooper());
        handler.post(new Runnable() {
            @Override
            public void run() {
                allStats.totalCancels = allStats.totalCancels + 1;
                recentStats.totalCancels = recentStats.totalCancels + 1;

                for (Object fetchRequest : fetchRequests) {
                    TileInfo tile = tilesByFetchRequest.get(fetchRequest);
                    if (tile == null)
                        continue;
                    if (tile.task != null)
                        tile.task.cancel();
                    tile.state = TileInfoState.None;
                    toLoad.remove(tile);
                    loading.remove(tile);
                    tilesByFetchRequest.remove(fetchRequest);
                }
            }
        });
    }

    /**
     * Kill all outstanding connections and clean up.
     */
    public void shutdown()
    {
        valid = false;
        quitSafely();

        loading.clear();
        toLoad.clear();
        tilesByFetchRequest.clear();
    }
}
