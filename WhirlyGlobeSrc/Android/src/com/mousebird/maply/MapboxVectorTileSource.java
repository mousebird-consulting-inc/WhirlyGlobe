package com.mousebird.maply;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.util.ArrayList;
import java.util.HashMap;
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
    VectorStyleInterface vecStyleFactory = null;

    /**
     * Construct with a initialized MBTilesSource.  This version reads from a local database.
     */
    public MapboxVectorTileSource(MBTiles dataSource,VectorStyleInterface inVecStyleFactor)
    {
        mbTiles = dataSource;
        coordSys = mbTiles.coordSys;
        tileParser = new MapboxVectorTileParser();
        vecStyleFactory = inVecStyleFactor;
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

    static double MAX_EXTENT = 20037508.342789244;

    // Convert to spherical mercator directly
    Point2d toMerc(Point2d pt)
    {
        Point2d newPt = new Point2d();
        newPt.setValue(Math.toDegrees(pt.getX()) * MAX_EXTENT / 180.0,
                3189068.5 * Math.log((1.0 + Math.sin(pt.getY())) / (1.0 - Math.sin(pt.getY()))));

        return newPt;
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
                ArrayList<ComponentObject> tileCompObjs = new ArrayList<ComponentObject>();

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
                    // Note: Eventually short circuit parsing with layer test and UUID test
                    Mbr mbr = layer.geoBoundsForTile(tileID);
                    mbr.ll = toMerc(mbr.ll);
                    mbr.ur = toMerc(mbr.ur);
                    MapboxVectorTileParser.DataReturn dataObjs = tileParser.parseData(tileData,mbr);

                    // Work through the vector objects
                    if (vecStyleFactory != null) {
                        HashMap<String,ArrayList<VectorObject>> vecObjsPerStyle = new HashMap<String,ArrayList<VectorObject>>();

                        // Sort the vector objects into bins based on their styles
                        if (dataObjs != null && dataObjs.vectorObjects != null)
                            for (VectorObject vecObj : dataObjs.vectorObjects) {
                                AttrDictionary attrs = vecObj.getAttributes();
                                VectorStyle[] styles = vecStyleFactory.stylesForFeature(attrs,tileID,attrs.getString("layer_name"),layer.maplyControl);
                                for (VectorStyle style : styles)
                                {
                                    ArrayList<VectorObject> vecObjsForStyle = vecObjsPerStyle.get(style.getUuid());
                                    if (vecObjsForStyle == null) {
                                        vecObjsForStyle = new ArrayList<VectorObject>();
                                        vecObjsPerStyle.put(style.getUuid(),vecObjsForStyle);
                                    }
                                    vecObjsForStyle.add(vecObj);
                                }
                            }

                        // Work through the various styles
                        for (String uuid : vecObjsPerStyle.keySet())
                        {
                            ArrayList<VectorObject> vecObjs = vecObjsPerStyle.get(uuid);
                            VectorStyle style = vecStyleFactory.styleForUUID(uuid,layer.maplyControl);

                            // This makes the objects
                            ComponentObject[] compObjs = style.buildObjects(vecObjs,tileID,layer.maplyControl);
                            if (compObjs != null)
                                for (int ii=0;ii<compObjs.length;ii++)
                                    tileCompObjs.add(compObjs[ii]);
                        }
                    }

                    // Add all the component objects we created
                    if (tileCompObjs.size() > 0)
                        layer.addData(tileCompObjs,tileID);

                    layer.tileDidLoad(tileID);
                } else
                    layer.tileFailedToLoad(tileID);
            }
        };

        thread.start();
    }

    /**
     * Notification that a tile unloaded.
     */
    public void tileDidUnload(MaplyTileID tileID)
    {
    }
}
