/*
 *  MBTileFetcher.java
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

import java.io.File;
import java.util.HashMap;
import java.util.TreeSet;
import java.util.concurrent.Semaphore;

import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;

/**
 * MBTiles tile fetcher.
 * <br>
 * This tile fetcher focuses on a single MBTiles file.  You mate this
 * with a QuadImageLoader to do the actual work.
 * <br>
 * Will work for image or vector MBTiles files.
 */
public class MBTileFetcher extends HandlerThread implements TileFetcher
{
    protected boolean valid = false;

    /**
     * Min zoom read from file
     */
    public int minZoom;

    /**
     * Max zoom read from file
     */
    public int maxZoom;

    /**
     * Coordinate system (probably Spherical Mercator)
     */
    CoordSystem coordSys = null;

    /**
     * Construct with the location of an MBTiles file.
     */
    MBTileFetcher(File mbTileFile)
    {
        super("MBTiles Fetcher");

        coordSys = new SphericalMercatorCoordSystem();

        if (mbTileFile == null || !mbTileFile.exists() || !mbTileFile.canRead()) {
            String message = String.format("MBTileSource must be initialized with an existing file. \"%s\" does not exists or is null...",
                    (mbTileFile == null ? "#NULL" : mbTileFile.getAbsolutePath())) ;
            Log.e(TAG, message);
            assert true : message;
        }

        this.init(mbTileFile);

        // Kicks off our thread
        valid = true;
        start();
    }

    /**
     * We don't need to describe a remote URL, so this is
     * basically a sub.
     */
    protected class MBTileInfo extends TileInfoNew
    {
        MBTileInfo(int inMinZoom,int inMaxZoom)
        {
            minZoom = inMinZoom;
            maxZoom = inMaxZoom;
        }

        Object fetchInfoForTile(TileID tileID)
        {
            return null;
        }
    }

    /**
     * No remote URLs to track, so we just keep the
     * tile ID.
     */
    protected class MBTileFetchInfo
    {
        MBTileFetchInfo(TileID inTileID)
        {
            tileID = inTileID;
        }

        public TileID tileID = null;
    }

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
        MBTileFetchInfo fetchInfo = null;

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
        return "MBTiles";
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
                    tileInfo.fetchInfo = (MBTileFetchInfo)request.fetchInfo;
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

    // Load a tile and pass off the results
    protected void updateLoading()
    {
        scheduled = false;

        if (!valid)
            return;

        if (toLoad.isEmpty())
            return;

        TileInfo tileInfo = toLoad.first();

        // Load the data tile
        byte[] data = getDataTile(tileInfo.fetchInfo.tileID);

        if (data != null)
            tileInfo.request.callback.success(tileInfo.request,data);
        else
            tileInfo.request.callback.failure(tileInfo.request,"Failed to read MBTiles File");

        finishTile(tileInfo);

        scheduleLoading();
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

    /** SQLite interface logic below **/

    // Query parameters and such

    private static String TAG = MBTiles.class.getSimpleName();

    private static String BOUNDS = "bounds";
    private static String MINZOOM = "minzoom";
    private static String MAXZOOM = "maxzoom";
    private static String TYPE = "type";
    private static String DESCRIPTION = "description";
    private static String FORMAT = "format";

    private static String NAME = "name";
    private static String VALUE = "value";
    private static String PNG = "png";
    private static String JPG = "jpg";

    private static String GET_TILE_SQL = "SELECT tile_data from tiles where zoom_level = ? AND tile_column = ? AND tile_row= ?;";
    private static String TILE_DATA = "tile_data";

    private SQLiteDatabase mbTileDb;

    private boolean initialized;    // Have we been correctly initialized
    private boolean oldStyleDB;     // Are we managing an old style database
    private boolean isJpg;          // Are we containing jpg tiles (or png tiles)

    private String name = "UNSET";            // Name of the tile dataset
    private String description = "UNSET";     // Description of the tile dataset
    private String type = "UNSET";            // Type of tile (overlay | baselayer)


    /**
     * Initializes the MBTilesSource with an SQLite database.
     * @param sqliteDb a File mapped to an <b>existing</b> SQLite database. The file must exists beforehand.
     */
    private void init(File sqliteDb)
    {
        mbTileDb = SQLiteDatabase.openDatabase(sqliteDb.getAbsolutePath(), null, SQLiteDatabase.OPEN_READONLY);

        // We read metadata
        String sql = "SELECT name, value FROM metadata;";

        Cursor c = mbTileDb.rawQuery(sql, null);

        if (c.getCount() > 0) {
            c.moveToFirst();

            int nameIdx = c.getColumnIndexOrThrow(NAME);
            int valueIdx = c.getColumnIndexOrThrow(VALUE);

            while (!c.isAfterLast()) {

                // What parameter are we reading
                String meta = c.getString(nameIdx);

                if (MAXZOOM.equals(meta)) {
                    maxZoom = c.getInt(valueIdx);
                }

                if (MINZOOM.equals(meta)) {
                    minZoom = c.getInt(valueIdx);
                }

                if (FORMAT.equals(meta)) {
                    String format = c.getString(valueIdx);
                    if (JPG.equals(format)) {
                        isJpg = true;
                    }
                }

                if (NAME.equals(meta)) {
                    name = c.getString(valueIdx);
                }

                if (DESCRIPTION.equals(meta)) {
                    description =  c.getString(valueIdx);
                }

                c.moveToNext();
            }
        }

        c.close();


        // If we did not get a minZoom and maxZoom, we need to get them the hard way
        if (minZoom == -1 || maxZoom == -1) {

            sql = "SELECT MIN(zoom_level) AS minzoom, MAX(zoom_level) as maxzoom FROM tiles;";

            c = mbTileDb.rawQuery(sql, null);

            if (c.getCount() > 0) {

                c.moveToFirst();

                minZoom = c.getInt(c.getColumnIndexOrThrow(MINZOOM));
                maxZoom = c.getInt(c.getColumnIndexOrThrow(MAXZOOM));
            }

            c.close();
            Log.i(TAG, "Got MIN & MAX zoom the hard way...");
        }

        Log.v(TAG, String.format("Initialized MBTilesSource %s (%s)", name, description));
        Log.v(TAG, String.format("  > Zoom %d -> %d", minZoom, maxZoom));
        Log.v(TAG, String.format("  > Type \"%s\"", type));
        Log.v(TAG, String.format("  > Format \"%s\"", (isJpg ? "jpg" : "png")));
    }

    /**
     * Fetch the data blog for a given tile.  This blocks.
     */
    public byte[] getDataTile(TileID tileID)
    {
        String[] params = new String[3];
        params[0] = Integer.toString(tileID.level);
        params[1] = Integer.toString(tileID.x);
        params[2] = Integer.toString(tileID.y);

        Cursor c = mbTileDb.rawQuery(GET_TILE_SQL, params);

        int tileDataIdx = c.getColumnIndex(TILE_DATA);

        if (c.getCount() > 0) {
            c.moveToFirst();

            byte[] data = c.getBlob(tileDataIdx);
            return data;
//                    Log.v(TAG, String.format("Returned tile for Z=%s, X=%d, Y=%d", tileID.level, tileID.x, tileID.y));
        }

        c.close();

        return null;
    }
}
