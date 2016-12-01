package com.mousebird.maply;

import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;

import java.io.File;
import java.io.FileNotFoundException;

import static android.R.attr.data;

/**
 * The LAZ Quad Reader will page a Lidar (LAZ or LAS) database organized
 into tiles in a sqlite database.
 */
public class LAZQuadReader implements QuadPagingLayer.PagingInterface
{
    GlobeController globeController;
    SQLiteDatabase tileDB;
    public int minPoints,maxPoints;
    public CoordSystem coordSys;
    String srs;
    public Shader shader;

    /**
     * Settings used to override what's in the database.
     */
    static public class Settings
    {
        public CoordSystem coordSystem;
        public double zOffset = 0.0;
        public int colorScale = 1<<16-1;
        public float pointSize = 0.f;
    }

    private LAZQuadReader()
    {
    }

    /**
     *
     */
    public LAZQuadReader(File sqliteDB, Settings settings, GlobeController inController)
    throws FileNotFoundException
    {
        initialise();

        globeController = inController;

        setZOffset(0.0);
        setPointSize(6.f);
        setColorScale(255);

        // Open the SQLite database
        tileDB = SQLiteDatabase.openDatabase(sqliteDB.getAbsolutePath(), null, SQLiteDatabase.OPEN_READONLY);
        if (tileDB == null)
            throw new FileNotFoundException("SQLite database not found: " + sqliteDB);

        // Read the metadata
        Cursor c = tileDB.rawQuery("SELECT * FROM manifest", null);
        if (c.getCount() > 0)
        {
            c.moveToFirst();

            double minX = c.getDouble(c.getColumnIndexOrThrow("minx"));
            double minY = c.getDouble(c.getColumnIndexOrThrow("miny"));
            double minZ = c.getDouble(c.getColumnIndexOrThrow("minz"));
            double maxX = c.getDouble(c.getColumnIndexOrThrow("maxx"));
            double maxY = c.getDouble(c.getColumnIndexOrThrow("maxy"));
            double maxZ = c.getDouble(c.getColumnIndexOrThrow("maxz"));
            setBounds(minX,minY,minZ,maxX,maxY,maxZ);

            int minLevel = c.getInt(c.getColumnIndexOrThrow("minlevel"));
            int maxLevel = c.getInt(c.getColumnIndexOrThrow("maxlevel"));
            setZoomLevels(minLevel,maxLevel);

            minPoints = c.getInt(c.getColumnIndexOrThrow("minpoints"));
            maxPoints = c.getInt(c.getColumnIndexOrThrow("maxpoints"));
            setTilePoints(minPoints,maxPoints);

            srs = c.getString(c.getColumnIndexOrThrow("srs"));

            int pointType = 0;
            if (c.getColumnIndex("pointtype") >= 0)
                pointType = c.getInt(c.getColumnIndex("pointtype"));
            setPointType(pointType);
            if (c.getColumnIndex("maxcolor") >= 0) {
                int maxColor = c.getInt(c.getColumnIndex("maxcolor"));
                if (maxColor > 300)
                    setColorScale(1<<16-1);
            }

            if (settings != null && settings.coordSystem != null)
                setCoordSystem(settings.coordSystem);
            else {
                Proj4CoordSystem coordSys = new Proj4CoordSystem(srs);
                Mbr mbr = new Mbr(new Point2d(getLL().getX(),getLL().getY()),new Point2d(getUR().getX(), getUR().getY()));
                coordSys.setBounds(mbr);
                setCoordSystem(coordSys);
            }

            if (settings != null && settings.pointSize > 0.0)
                setPointSize(settings.pointSize);

            if (settings != null && settings.zOffset > 0.0)
                setZOffset(settings.zOffset);

            if (settings != null && settings.colorScale != 0)
                setColorScale(settings.colorScale);

            // Note: Intersection handler
        }
    }

    public boolean hasColor()
    {
        return getPointType() > 1;
    }

    private void setCoordSystem(CoordSystem inCoordSys)
    {
        coordSys = inCoordSys;
        setCoordSystemNative(coordSys);
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

    public native void setPointType(int pointType);
    public native int getPointType();

    public native void setCoordSystemNative(CoordSystem coordSys);

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

    public void startFetchForTile(final QuadPagingLayer layer,final MaplyTileID tileID)
    {
        LayerThread layerThread = globeController.getWorkingThread();
        layerThread.addTask(new Runnable() {
            @Override
            public void run() {
                // Put together the precalculated quad index.  This is faster
                //  than x,y,level
                int quadIdx = 0;
                for (int iq=0;iq<tileID.level;iq++)
                    quadIdx += (1<<iq)*(1<<iq);
                quadIdx += tileID.y*(1<<tileID.level)+tileID.x;

                synchronized (tileDB)
                {
                    Cursor c = tileDB.rawQuery("SELECT data FROM lidartiles WHERE quadindex=" + quadIdx + ";", null);
                    if (c.getCount() > 0)
                    {
                        c.moveToFirst();

                        byte[] data = c.getBlob(c.getColumnIndexOrThrow("data"));
                        if (data != null)
                        {
                            Points points = new Points();

                            Point3d tileCenter = new Point3d(0,0,0);
                            processTileNative(globeController.coordAdapter, data, points.rawPoints, tileCenter);

                            Matrix4d mat = Matrix4d.translate(tileCenter.getX(),tileCenter.getY(),tileCenter.getZ());
                            points.setMatrix(mat);

                            GeometryInfo geomInfo = new GeometryInfo();
                            geomInfo.setPointSize(getPointSize());
                            geomInfo.setZBufferWrite(true);
                            geomInfo.setZBufferRead(true);
                            geomInfo.setShader(shader);
                            geomInfo.setDrawPriority(10000000);
                            globeController.addPoints(points,geomInfo, MaplyBaseController.ThreadMode.ThreadCurrent);
                        }
                    }
                }

                layer.tileDidLoad(tileID);
            }
        });
    }

    private native void processTileNative(CoordSystemDisplayAdapter coordAdapter, byte[] data, GeometryRawPoints points, Point3d tileCenter);

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
