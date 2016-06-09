package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.util.Log;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MBTiles;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MapboxVectorTileSource;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.QuadPagingLayer;
import com.mousebird.maply.VectorStyleSimpleGenerator;
import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Created by sjg on 5/25/16.
 */
public class LocalVectorTileTestCase extends MaplyTestCase {

    private static String TAG = "AutoTester";
    private static String MBTILES_DIR = "mbtiles";

    private Activity activity;

    public LocalVectorTileTestCase(Activity activity) {
        super(activity);
        setTestName("Local Vector Tile Test");
        setDelay(1000);

        this.activity = activity;
    }

    private QuadPagingLayer setupVectorLayer(MaplyBaseController baseController, ConfigOptions.TestType testType) throws Exception
    {

        // We need to copy the file from the asset so that it can be used as a file
        File mbTiles = this.getMbTileFile("mbtiles/France.mbtiles", "France.mbtiles");

        if (!mbTiles.exists()) {
            throw new FileNotFoundException(String.format("Could not copy MBTiles asset to \"%s\"", mbTiles.getAbsolutePath()));
        }

        Log.d(TAG, String.format("Obtained MBTiles SQLLite database \"%s\"", mbTiles.getAbsolutePath()));

        MBTiles mbTileSource = new MBTiles(mbTiles);
        VectorStyleSimpleGenerator simpleStyles = new VectorStyleSimpleGenerator(baseController);
        MapboxVectorTileSource tileSource = new MapboxVectorTileSource(mbTileSource,simpleStyles);

        QuadPagingLayer layer = new QuadPagingLayer(baseController,tileSource.coordSys,tileSource);
        layer.setImportance(1024*1024);

        return layer;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        CartoDBMapTestCase baseCase = new CartoDBMapTestCase(getActivity());
        baseCase.setUpWithGlobe(globeVC);
        globeVC.addLayer(setupVectorLayer(globeVC, ConfigOptions.TestType.GlobeTest));

        Point2d loc = Point2d.FromDegrees(2.3508, 48.8567);
        globeVC.setPositionGeo(loc.getX(),loc.getY(),0.15);

        return true;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        CartoDBMapTestCase baseCase = new CartoDBMapTestCase(getActivity());
        baseCase.setUpWithMap(mapVC);
        mapVC.addLayer(setupVectorLayer(mapVC, ConfigOptions.TestType.MapTest));

        Point2d loc = Point2d.FromDegrees(2.3508, 48.8567);
        mapVC.setPositionGeo(loc.getX(),loc.getY(),0.15);

        return true;
    }

    private File getMbTileFile(String assetMbTile, String mbTileFilename) throws IOException {

        ContextWrapper wrapper = new ContextWrapper(activity);
        File mbTilesDirectory =  wrapper.getDir(MBTILES_DIR, Context.MODE_PRIVATE);

        InputStream is = activity.getAssets().open(assetMbTile);
        File of = new File(mbTilesDirectory, mbTileFilename);

        if (of.exists()) {
            return of;
        }

        OutputStream os = new FileOutputStream(of);
        byte[] mBuffer = new byte[1024];
        int length;
        while ((length = is.read(mBuffer))>0) {
            os.write(mBuffer, 0, length);
        }
        os.flush();
        os.close();
        is.close();

        return of;

    }
}
