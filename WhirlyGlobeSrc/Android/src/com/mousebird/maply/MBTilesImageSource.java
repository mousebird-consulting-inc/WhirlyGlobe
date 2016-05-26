package com.mousebird.maply;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

/**
 * Created by sjg on 5/25/16.
 */
public class MBTilesImageSource implements QuadImageTileLayer.TileSource
{
    MBTiles mbTileDb;

    public MBTilesImageSource(MBTiles tileDB)
    {
        mbTileDb = tileDB;
        coordSys = mbTileDb.coordSys;
    }

    public CoordSystem coordSys = null;

    /**
     * The minimum zoom level you'll be called about to create a tile for.
     */
    public int minZoom()
    {
        return mbTileDb.minZoom();
    }

    /**
     * The maximum zoom level you'll be called about to create a tile for.
     */
    public int maxZoom()
    {
        return mbTileDb.maxZoom();
    }

    public int pixelsPerSide = 256;

    /**
     * The number of pixels square for each tile.
     */
    public int pixelsPerSide()
    {
        return pixelsPerSide;
    }

    /**
     * This tells you when to start fetching a given tile. When you've fetched
     * the image you'll want to call loadedTile().  If you fail to fetch an image
     * call that with nil.
     *
     * @param layer The layer asking for the fetch.
     * @param tileID The tile ID to fetch.
     * @param frame If the source support multiple frames, this is the frame.  Otherwise -1.
     */
    public void startFetchForTile(final QuadImageTileLayerInterface layer, final MaplyTileID tileID, final int frame)
    {
        // Note: Start the fetch for a single tile, ideally on another thread.
        //       When the tile data is in, call the layer loadedTile() method.

        Thread thread = new Thread() {
            @Override
            public void run() {

                byte[] image = mbTileDb.getDataTile(tileID);

                if (image != null) {
                    Bitmap bm = BitmapFactory.decodeByteArray(image, 0, image.length);
                    MaplyImageTile tile = new MaplyImageTile(bm);

                    layer.loadedTile(tileID, frame, tile);
                } else
                    layer.loadedTile(tileID, frame, null);
            }
        };

        thread.run();
    }
}
