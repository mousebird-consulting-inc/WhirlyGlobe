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

import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.AsyncTask;
import android.util.Log;

import java.io.File;

/**
 * The MBTiles Source reads Mapbox style MBTiles files.
 */
public class MBTilesSource implements QuadImageTileLayer.TileSource
{

    //***********************************************************************//
    //                           Inner classes                               //
    //***********************************************************************//

    private class FetchTileAsyncTask extends AsyncTask<MaplyTileID, Integer, Boolean> {


        private QuadImageTileLayerInterface layer;
        private int frame;

        public FetchTileAsyncTask(QuadImageTileLayerInterface layer, int frame) {
            this.layer = layer;
            this.frame = frame;
        }

        @Override
        protected Boolean doInBackground(MaplyTileID... tileIDs) {

            Log.v(TAG, String.format("Fetching %d tiles...", tileIDs.length));

            for (MaplyTileID tileID : tileIDs) {

                String[] params = new String[3];
                params[0] = Integer.toString(tileID.level);
                params[1] = Integer.toString(tileID.x);
                params[2] = Integer.toString(tileID.y);

                Cursor c = mbTileDb.rawQuery(GET_TILE_SQL, params);

                int tileDataIdx = c.getColumnIndex(TILE_DATA);

                if (c.getCount() > 0) {
                    c.moveToFirst();

                    byte[] image = c.getBlob(tileDataIdx);
                    Bitmap bm = BitmapFactory.decodeByteArray(image, 0, image.length);

                    MaplyImageTile tile = new MaplyImageTile(bm);

                    layer.loadedTile(tileID, frame, tile);

                    Log.v(TAG, String.format("Returned tile for Z=%s, X=%d, Y=%d", tileID.level, tileID.x, tileID.y));

                } else {
                    Log.i(TAG, String.format("Could not find tile for Z=%s, X=%d, Y=%d", tileID.level, tileID.x, tileID.y));
                }

                c.close();
            }

            return null;
        }
    }


    //***********************************************************************//
    //                          Class variables                              //
    //***********************************************************************//

    private static String TAG = MBTilesSource.class.getSimpleName();

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







    //***********************************************************************//
    //                         Instance variables                            //
    //***********************************************************************//

    private int minZoom = -1, maxZoom = -1;
    private int pixelsPerSide = 256;

    private SQLiteDatabase mbTileDb;

    private boolean initialized;    // Have we been correctly initialized
    private boolean oldStyleDB;     // Are we managing an old style database
    private boolean isJpg;          // Are we containing jpg tiles (or png tiles)

    private String name = "UNSET";            // Name of the tile dataset
    private String description = "UNSET";     // Description of the tile dataset
    private String type = "UNSET";            // Type of tile (overlay | baselayer)


    //***********************************************************************//
    //                            Constructors                               //
    //***********************************************************************//


    public MBTilesSource(String mbTileFileName)
    {
        // Note: Do the initial read from the database to get the header info
        //       This includes min and max zoom as well as anything you need
        //       to know about reading the rest of it.

        // We look for the file into assets
    }

    public MBTilesSource(File mbTileFile) {

        if (mbTileFile == null || !mbTileFile.exists() || !mbTileFile.canRead()) {
            String message = String.format("MBTileSource must be initialized with an existing file. \"%s\" does not exists or is null...",
                    (mbTileFile == null ? "#NULL" : mbTileFile.getAbsolutePath())) ;
            Log.e(TAG, message);
            assert true : message;
        }

        this.init(mbTileFile);

    }


    //***********************************************************************//
    //                         Getters and setters                           //
    //***********************************************************************//

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


    //***********************************************************************//
    //                           Public methods                              //
    //***********************************************************************//

    /**
     * This tells you when to start fetching a given tile. When you've fetched
     * the image you'll want to call loadedTile().  If you fail to fetch an image
     * call that with nil.
     *
     * @param layer The layer asking for the fetch.
     * @param tileID The tile ID to fetch.
     * @param frame If the source support multiple frames, this is the frame.  Otherwise -1.
     */
    public void startFetchForTile(QuadImageTileLayerInterface layer, MaplyTileID tileID, int frame)
    {
        // Note: Start the fetch for a single tile, ideally on another thread.
        //       When the tile data is in, call the layer loadedTile() method.

        Log.v(TAG, String.format("Requested to fetch tile for Z=%s, X=%d, Y=%d", tileID.level, tileID.x, tileID.y));

        String[] params = new String[3];
        params[0] = Integer.toString(tileID.level);
        params[1] = Integer.toString(tileID.x);
        params[2] = Integer.toString(tileID.y);

        Cursor c = mbTileDb.rawQuery(GET_TILE_SQL, params);

        int tileDataIdx = c.getColumnIndex(TILE_DATA);

        if (c.getCount() > 0) {
            c.moveToFirst();

            byte[] image = c.getBlob(tileDataIdx);
            Bitmap bm = BitmapFactory.decodeByteArray(image, 0, image.length);

            MaplyImageTile tile = new MaplyImageTile(bm);

            layer.loadedTile(tileID, frame, tile);

            Log.v(TAG, String.format("Returned tile for Z=%s, X=%d, Y=%d", tileID.level, tileID.x, tileID.y));

        } else {
            Log.i(TAG, String.format("Could not find tile for Z=%s, X=%d, Y=%d", tileID.level, tileID.x, tileID.y));
        }

        c.close();

    }


    //***********************************************************************//
    //                           Private methods                             //
    //***********************************************************************//

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

            sql = "SELECT MIN(zoom) AS minzoom, MAX(zoom) as maxzoom FROM tiles;";

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
}
