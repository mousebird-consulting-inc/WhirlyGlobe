package com.mousebird.maply;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.util.zip.GZIPInputStream;
import java.util.zip.ZipEntry;

/**
 * The MapboxVectorTiles class is used to load Mapbox format vector tiles
 * on demand over a certain area.  You'll need to use this in combination with
 * a QuadPagingLayer.
 */
public class MapboxVectorTileSource implements QuadPagingLayer.PagingInterface
{
    MBTiles mbTiles = null;

    MapboxVectorTileParser tileParser = null;

    /**
     * Construct with a initialized MBTilesSource.  This version reads from a local database.
     */
    public MapboxVectorTileSource(MBTiles dataSource)
    {
        mbTiles = dataSource;
        coordSys = mbTiles.coordSys;
        tileParser = new MapboxVectorTileParser();
    }

    public CoordSystem coordSys = null;

    /**
     * Minimum zoom level supported.
     */
    public int minZoom()
    {
        return mbTiles.minZoom();
    }

    /**
     * Maximum zoom level supported.
     */
    public int maxZoom()
    {
        return mbTiles.maxZoom();
    }

    /**
     * Used internally to start fetching data.
     * @param layer The quad paging layer asking you to start fetching.
     * @param tileID The tile to start fetching
     */
    public void startFetchForTile(final QuadPagingLayer layer,final MaplyTileID tileID)
    {
        Thread thread = new Thread() {
            @Override
            public void run() {
                // Load the data, if it's there
                byte[] tileData = mbTiles.getDataTile(tileID);

                if (tileData != null) {
                    try {
                        // Unzip if it's compressed
                        ByteArrayInputStream bin = new ByteArrayInputStream(tileData);
                        GZIPInputStream in = new GZIPInputStream(bin);
                        ByteArrayOutputStream bout = new ByteArrayOutputStream(tileData.length * 2);

                        ZipEntry ze;
                        byte[] buffer = new byte[1024];
                        int count;
                        while ((count = in.read(buffer)) != -1)
                            bout.write(buffer, 0, count);

                        tileData = bout.toByteArray();
                    }
                    catch (Exception ex)
                    {
                        // We'll try the raw data if we can't decompress it
                    }

                    // Parse the data
                    Mbr mbr = layer.boundsForTile(tileID);
                    MapboxVectorTileParser.DataReturn dataObjs = tileParser.parseData(tileData,mbr);

                    // Work through the vector objects
                    if (dataObjs != null && dataObjs.vectorObjects != null)
                        for (VectorObject vecObj : dataObjs.vectorObjects)
                        {

                        }

                    layer.tileDidLoad(tileID);
                } else
                    layer.tileFailedToLoad(tileID);
            }
        };

        thread.run();
    }

    /**
     * Notification that a tile unloaded.
     */
    public void tileDidUnload(MaplyTileID tileID)
    {
    }
}
