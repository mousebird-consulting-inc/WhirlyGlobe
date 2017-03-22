package com.mousebird.maply;

/**
 * Interface implemented by the quad image tile-like layers.
 */
public interface QuadImageTileLayerInterface
{
    /**
     * When a tile source finishes loading a given image tile,
     * it calls this method to let the quad image tile layer know
     * about it.  You can call this on any thread.
     *
     * @param imageTile The image tile we've just loaded.  Pass in null on failure.
     */
    public void loadedTile(final MaplyTileID tileID,final int frame,final MaplyImageTile imageTile);

    /**
     * Returns the layer thread.
     */
    public LayerThread getLayerThread();
}
