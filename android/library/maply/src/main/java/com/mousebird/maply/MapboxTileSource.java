package com.mousebird.maply;

/**
 * Simple interface for fetching a particular MBTiles tile.
 * These may come from MBTiles or GeoPackage.
 */
public interface MapboxTileSource {
    /**
     * Return the block of data associated with the given tile.
     */
    public byte[] getDataTile(MaplyTileID tileID);

    /**
     * Return the coordinate system.
     */
    CoordSystem getCoordSystem();

    // Minimum zoom level
    int getMinZoom();

    // Maximum zoom level
    int getMaxZoom();
}
