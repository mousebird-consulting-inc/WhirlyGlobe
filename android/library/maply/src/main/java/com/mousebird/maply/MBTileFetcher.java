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
import android.os.AsyncTask;
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
public class MBTileFetcher extends  SimpleTileFetcher
{

    /**
     * Coordinate system (probably Spherical Mercator)
     */
    CoordSystem coordSys = null;

    /**
     * Construct with the location of an MBTiles file.
     */
    public MBTileFetcher(File mbTileFile)
    {
        super("MBTiles Fetcher");
        neverFail = true;

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

    /** SQLite interface logic below **/

    // Query parameters and such

    private static String TAG = MBTiles.class.getSimpleName();

    private static String BOUNDS = "bounds";
    private static String MINZOOM = "minzoom";
    private static String MAXZOOM = "maxzoom";
    private static String MINZOOMLEVEL = "minZoomLevel";
    private static String MAXZOOMLEVEL = "maxZoomLevel";
    private static String TYPE = "type";
    private static String DESCRIPTION = "description";
    private static String FORMAT = "format";

    private static String NAME = "name";
    private static String VALUE = "value";
    private static String PNG = "png";
    private static String JPG = "jpg";
    private static String JPEG = "jpeg";

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
                } else if (MAXZOOMLEVEL.equals(meta)) {
                    maxZoom = c.getInt(valueIdx);
                }

                if (MINZOOM.equals(meta)) {
                    minZoom = c.getInt(valueIdx);
                } else if (MINZOOMLEVEL.equals(meta)) {
                    minZoom = c.getInt(valueIdx);
                }

                if (FORMAT.equals(meta)) {
                    String format = c.getString(valueIdx);
                    if (JPG.equals(format)) {
                        isJpg = true;
                    } else if (JPEG.equals(format)) {
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
    @Override public byte[] dataForTile(Object fetchInfo,TileID tileID)
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
            c.close();
            return data;
//                    Log.v(TAG, String.format("Returned tile for Z=%s, X=%d, Y=%d", tileID.level, tileID.x, tileID.y));
        }

        c.close();

        return null;
    }
}
