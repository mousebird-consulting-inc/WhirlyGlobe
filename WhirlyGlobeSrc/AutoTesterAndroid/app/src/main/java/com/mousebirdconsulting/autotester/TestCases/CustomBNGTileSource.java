package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.content.Context;
import android.os.Looper;

import com.mousebird.maply.CoordSystem;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.Mbr;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.Proj4CoordSystem;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebird.maply.TestImageSource;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Created by sjg on 2/13/16.
 */
public class CustomBNGTileSource extends MaplyTestCase
{
    public CustomBNGTileSource(Activity activity) {
        super(activity);
        this.setTestName("Custom BNG Tile Source");
        this.setDelay(100000);
    }

    public static String getFilePathFromAssets(String assetFilePath, Context context) {
        File file = new File(context.getCacheDir() + assetFilePath);

        // Copy file from assets if not already in cache directory
        if (!file.exists()) {
            try {
                InputStream inputStream = context.getAssets().open(assetFilePath);
                int bufferSize = 4 * 1024;
                byte[] buffer = new byte[bufferSize];
                inputStream.read(buffer);
                inputStream.close();

                FileOutputStream fileOutputStream = new FileOutputStream(file);
                fileOutputStream.write(buffer);
                fileOutputStream.close();
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }

        return file.getPath();
    }

    // Put together a British National Grid system
    static public CoordSystem MakeBNGCoordSystem(Activity activity, boolean displayVersion)
    {
        // Set up the proj4 string including the local grid file
        String outFileName = getFilePathFromAssets("OSTN02_NTv2.gsb", activity);

        String projStr = "+proj=tmerc +lat_0=49 +lon_0=-2 +k=0.9996012717 +x_0=400000 +y_0=-100000 +ellps=airy +nadgrids=" + outFileName + " +units=m +no_defs";
        Proj4CoordSystem coordSys = new Proj4CoordSystem(projStr);

        // Set the bounding box for validity.  It assumes it can go everywhere by default
        Mbr bbox = new Mbr();
        bbox.addPoint(new Point2d(1393.0196,13494.9764));
        bbox.addPoint(new Point2d(671196.3657,1230275.0454));

        // Now expand it out so we can see the whole of the UK
        if (displayVersion)
        {
            // Note: There may be a center/offset problem with the bounds.  Made them bigger to compensate
            double spanX = bbox.ur.getX() - bbox.ll.getX();
            double spanY = bbox.ur.getY() - bbox.ll.getY();
            double extra = 1.5;
            bbox.addPoint(new Point2d(-extra*spanX,-extra*spanY));
            bbox.addPoint(new Point2d(extra*spanX,extra*spanY));
        }

        coordSys.setBounds(bbox);

        return coordSys;
    }

    public QuadImageTileLayer makeTestLayer(MaplyBaseController viewC)
    {
        CoordSystem bngCoordSystem = MakeBNGCoordSystem(getActivity(),false);

        TestImageSource tileSource = new TestImageSource(Looper.getMainLooper(),0,14);

        QuadImageTileLayer baseLayer = new QuadImageTileLayer(viewC, bngCoordSystem, tileSource);
        baseLayer.setCoverPoles(false);
        baseLayer.setHandleEdges(false);
        baseLayer.setDrawPriority(1000);

        return baseLayer;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception
    {
        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithGlobe(globeVC);

        QuadImageTileLayer layer = makeTestLayer(globeVC);
        if (layer != null)
            globeVC.addLayer(layer);

        Point2d pt = Point2d.FromDegrees(-0.1275, 51.507222);
        globeVC.setPositionGeo(pt.getX(), pt.getY(), 0.4);

        return true;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception
    {
        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithMap(mapVC);

        QuadImageTileLayer layer = makeTestLayer(mapVC);
        if (layer != null)
            mapVC.addLayer(layer);

        Point2d pt = Point2d.FromDegrees(-0.1275, 51.507222);
        mapVC.setPositionGeo(pt.getX(), pt.getY(), 0.4);

        return true;
    }

}
