package com.mousebird.maply;

import com.mousebird.maply.CoordSystem;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MaplyTileID;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.Point3d;
import com.mousebird.maply.QuadPagingLayer;
import com.mousebird.maply.Shader;

/**
 * The LAZ Quad Reader will page a Lidar (LAZ or LAS) database organized
 into tiles in a sqlite database.
 */
public class LAZQuadReader implements QuadPagingLayer.PagingInterface
{
    GlobeController globeController;

    /**
     * Settings used to override what's in the database.
     */
    public class Settings
    {
        public CoordSystem coordSystem;
        double zOffset = 0.0;
        public int colorScale = 1<<16-1;
    }

    private LAZQuadReader()
    {
    }

    /**
     *
     */
    public LAZQuadReader(String fileName, Shader shader, Settings settings, GlobeController inController)
    {
        initialise();

        globeController = inController;

        setZOffset(0.0);
        setPointSize(6.f);
        setColorScale(255);

        // Open the SQLite database

    }

    /**
     * Return the center from the bounding box
     */
    public Point2d getCenter()
    {
        Point3d ll = getLL();
        Point3d ur = getUR();

        return new Point2d((ll.getX()+ur.getX())/2.0,(ll.getY()+ur.getY())/2.0);
    }

    /**
     * Based on the number of points we want to display, how many tiles should we fetch?
     */
    public int getNumTilesFromMaxPoints(int maxPoints)
    {
        int numTiles = maxPoints/getMinTilePoints();
        if (numTiles < 1)  numTiles = 1;
        if (numTiles > 256)  numTiles = 256;

        return numTiles;
    }

    public native void setBounds(double minX,double minY,double minZ,double maxX,double maxY,double maxZ);
    public native Point3d getLL();
    public native Point3d getUR();

    public native void setZOffset(double zOffset);
    public native double getZOffset();

    public native void setZoomLevels(int minZoom,int maxZoom);
    public native int getMinZoom();
    public native int getMaxZoom();

    public native void setTilePoints(int minPoints,int maxPoints);
    public native int getMinTilePoints();
    public native int getMaxTilePoints();

    public native void setPointSize(float pointSize);
    public native float getPointSize();

    public native void setColorScale(int colorScale);
    public native int getColorScale();

    //
    // Paging Interface methods
    //

    public int minZoom()
    {
        return getMinZoom();
    }

    public int maxZoom()
    {
        return getMaxZoom();
    }

    public void startFetchForTile(QuadPagingLayer layer,MaplyTileID tileID)
    {

    }

    public void tileDidUnload(MaplyTileID tileID)
    {

    }

    public void finalize()
    {
        dispose();
    }

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;

}
