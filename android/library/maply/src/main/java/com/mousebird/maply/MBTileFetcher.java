/*  MBTileFetcher.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 3/21/19.
 *  Copyright 2011-2021 mousebird consulting
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
 */

package com.mousebird.maply;

import java.io.File;
import java.lang.reflect.Field;

import android.database.Cursor;
import android.database.CursorWindow;
import android.database.sqlite.SQLiteBlobTooBigException;
import android.database.sqlite.SQLiteCursor;
import android.database.sqlite.SQLiteDatabase;
import android.os.Build;
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
    CoordSystem coordSys;

    /**
     * Construct with the location of an MBTiles file.
     */
    public MBTileFetcher(BaseController control,File mbTileFile)
    {
        super(control,"MBTiles Fetcher");
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

    private static final String TAG = MBTiles.class.getSimpleName();

    //private static final String BOUNDS = "bounds";
    private static final String MINZOOM = "minzoom";
    private static final String MAXZOOM = "maxzoom";
    private static final String MINZOOMLEVEL = "minZoomLevel";
    private static final String MAXZOOMLEVEL = "maxZoomLevel";
    //private static final String TYPE = "type";
    private static final String DESCRIPTION = "description";
    private static final String FORMAT = "format";

    private static final String NAME = "name";
    private static final String VALUE = "value";
    //private static final String PNG = "png";
    private static final String JPG = "jpg";
    private static final String JPEG = "jpeg";

    private static final String TILE_DATA = "tile_data";
    private static final String GET_TILE_SQL =
        "SELECT " + TILE_DATA + " from tiles where zoom_level=? AND tile_column=? AND tile_row=?";
    final static String GET_META_SQL = "SELECT name, value FROM metadata";
    final static String GET_ZOOM_LIMITS_SQL = "SELECT MIN(zoom_level) AS minzoom, MAX(zoom_level) as maxzoom FROM tiles;";
    final static String GET_TILE_SIZE_SQL = "select max(length(" + TILE_DATA + ")) from tiles";

    private SQLiteDatabase mbTileDb;

    private boolean isJpg;          // Are we containing jpg tiles (or png tiles)

    @SuppressWarnings("FieldCanBeLocal")
    private String name = "UNSET";            // Name of the tile dataset
    private String description = "UNSET";     // Description of the tile dataset
    //private String type = "UNSET";            // Type of tile (overlay | baselayer)


    /**
     * Initializes the MBTilesSource with an SQLite database.
     * @param sqliteDb a File mapped to an <b>existing</b> SQLite database. The file must exists beforehand.
     */
    private void init(File sqliteDb)
    {
        name = sqliteDb.getName();
        mbTileDb = SQLiteDatabase.openDatabase(sqliteDb.getAbsolutePath(), null, SQLiteDatabase.OPEN_READONLY);

        // We read metadata
        try (Cursor c = mbTileDb.rawQuery(GET_META_SQL, null)) {
            final int nameIdx = c.getColumnIndexOrThrow(NAME);
            final int valueIdx = c.getColumnIndexOrThrow(VALUE);
            while (c.moveToNext()) {
                // What parameter are we reading
                final String meta = c.getString(nameIdx);

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

                if (!isJpg && FORMAT.equals(meta)) {
                    String format = c.getString(valueIdx);
                    isJpg = (JPG.equals(format) || JPEG.equals(format));
                }

                if (NAME.equals(meta)) {
                    name = c.getString(valueIdx);
                }

                if (DESCRIPTION.equals(meta)) {
                    description = c.getString(valueIdx);
                }
            }
        }

        // If we did not get a minZoom and maxZoom, we need to get them the hard way
        if (minZoom == -1 || maxZoom == -1) {
            try (Cursor c = mbTileDb.rawQuery(GET_ZOOM_LIMITS_SQL, null)) {
                if (c.moveToNext()) {
                    minZoom = c.getInt(c.getColumnIndexOrThrow(MINZOOM));
                    maxZoom = c.getInt(c.getColumnIndexOrThrow(MAXZOOM));
                    Log.i(TAG, "Got MIN & MAX zoom the hard way");
                }
            }
        }

        super.initWithName(name, minZoom, maxZoom);

        Log.v(TAG, String.format("Initialized MBTilesSource %s (%s)", name, description));
        Log.v(TAG, String.format("  > Zoom %d -> %d", minZoom, maxZoom));
        //Log.v(TAG, String.format("  > Type \"%s\"", type));
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

        // try to handle large blobs on old builds
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.P) {
            if (!windowAdjusted) {
                try (Cursor c = mbTileDb.rawQuery(GET_TILE_SIZE_SQL, new String[0])) {
                    if (c.moveToNext()) {
                        final int rowSize = c.getInt(0) + 1024;
                        final Field field = CursorWindow.class.getDeclaredField("sCursorWindowSize");
                        if (field != null) {
                            field.setAccessible(true);
                            final int windowSize = field.getInt(null);
                            if (windowSize < rowSize) {
                                field.setInt(null, rowSize);
                                Log.v(tag, String.format("Adjusting default cursor window size from %d to %d", windowSize, rowSize));
                            }
                        }
                    }
                } catch (Exception ex) {
                    Log.w(tag, "Failed to adjust cursor window size", ex);
                }
                windowAdjusted = true;
            }
        }

        while (true) {
            boolean usingWindow = false;
            try (Cursor c = mbTileDb.rawQuery(GET_TILE_SQL, params)) {

                // Set up a custom cursor window, if necessary, and possible.
                if (maxWindowBytes > 0 &&
                        Build.VERSION.SDK_INT >= Build.VERSION_CODES.P &&
                        c instanceof SQLiteCursor) {
                    SQLiteCursor cc = (SQLiteCursor)c;
                    cc.setFillWindowForwardOnly(true);

                    // Can we reuse these?
                    CursorWindow window = new CursorWindow("mbtile", maxWindowBytes);
                    cc.setWindow(window);
                    usingWindow = true;
                }

                if (c.moveToNext()) {
                    final int tileDataIdx = c.getColumnIndexOrThrow(TILE_DATA);
                    final byte[] data = c.getBlob(tileDataIdx);
                    //Log.v(tag, String.format("Returned tile for Z=%s, X=%d, Y=%d (%d bytes)", tileID.level, tileID.x, tileID.y, data.length));
                    return data;
                }
            } catch (SQLiteBlobTooBigException ignored) {
                // Grow the size and try again, up to a reasonable limit
                if (usingWindow && maxWindowBytes < maxWindowSize) {
                    maxWindowBytes *= 2;
                    Log.w(tag, String.format("Adjusting cursor window size to %d", maxWindowBytes));
                    continue;
                } else if (maxWindowBytes < 0) {
                    // The default window size didn't work, so try setting one explicitly.
                    Log.w(tag, String.format("Adjusting cursor window size to %d", defWindowSize));
                    maxWindowBytes = defWindowSize;
                    continue;
                }
            }
            break;
        }

        return null;
    }

    private int maxWindowBytes = -1;
    private boolean windowAdjusted = false;

    // see `com.android.internal.R.integer.config_cursorWindowSize`
    private final static int defWindowSize = 4 * 1024 * 1024;
    private final static int maxWindowSize = 32 * defWindowSize;
    private final static String tag = MBTileFetcher.class.getSimpleName();
}
