package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MBTilesSource;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

/**
 * Created by sjg on 4/11/16.
 */
public class MBTilesImageTestCase extends MaplyTestCase {
    public MBTilesImageTestCase(Activity activity) {
        super(activity);
        setTestName("MBTiles Image Test");
        setDelay(1000);
    }

    private QuadImageTileLayer setupImageLayer(MaplyBaseController baseController, ConfigOptions.TestType testType) throws Exception
    {
        MBTilesSource tileSource = new MBTilesSource("TestFile.sqlite");
        QuadImageTileLayer imageLayer = new QuadImageTileLayer(baseController, tileSource.coordSys, tileSource);
        imageLayer.setCoverPoles(true);
        imageLayer.setHandleEdges(true);

        return imageLayer;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        globeVC.addLayer(setupImageLayer(globeVC, ConfigOptions.TestType.GlobeTest));
        return true;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        mapVC.addLayer(setupImageLayer(mapVC, ConfigOptions.TestType.MapTest));
        return true;
    }
}
