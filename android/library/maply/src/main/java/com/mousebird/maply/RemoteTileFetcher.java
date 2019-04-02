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

import com.squareup.okhttp.Call;
import com.squareup.okhttp.Callback;
import com.squareup.okhttp.OkHttpClient;
import com.squareup.okhttp.Request;
import com.squareup.okhttp.Response;

import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.HashMap;
import java.util.HashSet;
import java.util.TreeSet;

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
                return priority < that.priority ? -1 : 1;
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

    RemoteTileFetcher(BaseController baseController, String name)
    {
        super(name);

        client = baseController.getHttpClient();
        valid = true;

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

        // Have to run on our own thread
        Handler handler = new Handler(getLooper());
        handler.post(new Runnable() {
            @Override
            public void run() {
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

    OkHttpClient client = null;

    // Load a tile and pass off the results
    protected void updateLoading()
    {
        scheduled = false;

        if (!valid)
            return;

        while (loading.size() < numConnections) {
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
    }

    // Kick off a network fetch with the appropriate callbacks
    protected void startFetch(final TileInfo tile)
    {
        tile.task.enqueue(new Callback() {
            @Override
            public void onFailure(Request request, IOException e) {
                if (!valid)
                    return;
                // Ignore cancels, because we do those a lot
                if (e != null) {
                    String mess = e.getLocalizedMessage();
                    if (mess != null && mess.contains("Canceled")) {
                        if (debugMode)
                            Log.d("RemoteTileFetcher","Ignoring a cancel for: " + request);
                        return;
                    }
                }

                finishedLoading(tile,null,e);
            }

            @Override
            public void onResponse(Response response) throws IOException {
                if (!valid)
                    return;

                finishedLoading(tile,response,null);
            }
        });
    }

    // Got response back, may be good, may be bad.
    // On a random thread, perhaps
    protected void finishedLoading(final TileInfo inTile, final Response response, final Exception inE)
    {
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

                    return;
                }

                boolean success = true;
                Exception e = inE;

                if (debugMode)
                    Log.d("RemoteTileFetcher","Got response for: " + response.request());

                if (response != null) {
                    try {
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
                    handleFinishLoading(tile, null, e);
                }

                try {
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
        boolean success = true;
        int size = (int) tile.fetchInfo.cacheFile.length();
        final byte[] data = new byte[size];
        try {
            BufferedInputStream buf = new BufferedInputStream(new FileInputStream(tile.fetchInfo.cacheFile));
            buf.read(data, 0, data.length);
            buf.close();
        } catch (Exception e) {
            success = false;
        }

        if (success) {
            Handler handler = new Handler(getLooper());
            handler.post(new Runnable() {
                @Override
                public void run() {
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

        // Let the caller know on a random thread because parsing may take a while
        AsyncTask<Void,Void,Void> task = new AsyncTask<Void, Void, Void>() {
            protected Void doInBackground(Void... unused) {

                if (debugMode)
                    Log.d("RemoteTileFetcher","Returning fetch: " + tile.fetchInfo.urlReq);

                if (error == null) {
                    writeToCache(tile,data);
                    tile.request.callback.success(tile.request, data);
                } else
                    tile.request.callback.failure(tile.request,error.toString());

                // Now get rid of the tile and kick off a new request
                Handler handler = new Handler(getLooper());
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        finishTile(tile);
                        scheduleLoading();
                    }
                });

                return null;
            }
        };
        task.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, (Void)null);
    }

    // Write to the local cache.  Called on a random thread.
    protected void writeToCache(TileInfo tile,byte[] data)
    {
        try {
            OutputStream fOut;
            fOut = new FileOutputStream(tile.fetchInfo.cacheFile);
            fOut.write(data);
            fOut.close();
        }
        catch (Exception e)
        {
        }
    }

    protected void finishTile(TileInfo inTile)
    {
        // Make sure we still want it
        final TileInfo tile = tilesByFetchRequest.get(inTile.request);
        if (tile == null)
            return;

        tilesByFetchRequest.remove(tile.request);
        loading.remove(tile);
        toLoad.remove(tile);
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

        // Have to run on our own thread
        Handler handler = new Handler(getLooper());
        handler.post(new Runnable() {
            @Override
            public void run() {
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
