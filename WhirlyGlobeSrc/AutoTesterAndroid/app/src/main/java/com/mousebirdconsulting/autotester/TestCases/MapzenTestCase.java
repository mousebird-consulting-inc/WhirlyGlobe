package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MapboxVectorTileSource;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.QuadPagingLayer;
import com.mousebird.maply.RemoteTileInfo;
import com.mousebird.maply.VectorStyleSimpleGenerator;
import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

public class MapzenTestCase extends MaplyTestCase {
    private static String TAG = "AutoTester";

    private Activity activity;

    public MapzenTestCase(Activity activity) {
        super(activity);
        setTestName("Mapzen Test Case");
        setDelay(1000);
        this.implementation = MaplyTestCase.TestExecutionImplementation.Both;

        this.activity = activity;
    }

    private QuadPagingLayer setupVectorLayer(MaplyBaseController baseController, ConfigOptions.TestType testType) throws Exception {
        VectorStyleSimpleGenerator simpleStyles = new VectorStyleSimpleGenerator(baseController);

        RemoteTileInfo tileInfo =
                new RemoteTileInfo(
                        "https://tile.mapzen.com/mapzen/vector/v1/all/{z}/{x}/{y}.mvt?api_key=vector-tiles-ejNTZ28",
                        null,
                        0, 19);
        MapboxVectorTileSource tileSource = new MapboxVectorTileSource(tileInfo, simpleStyles);
        tileSource.debugOutput = true;

        QuadPagingLayer layer = new QuadPagingLayer(baseController, tileSource.coordSys, tileSource);
        layer.setSimultaneousFetches(4);
        layer.setSingleLevelLoading(true);
        layer.setImportance(1024 * 1024);

        return layer;
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        CartoDBMapTestCase baseCase = new CartoDBMapTestCase(getActivity());
        baseCase.setUpWithGlobe(globeVC);
        globeVC.addLayer(setupVectorLayer(globeVC, ConfigOptions.TestType.GlobeTest));

        Point2d loc = Point2d.FromDegrees(2.3508, 48.8567);
        globeVC.setPositionGeo(loc.getX(), loc.getY(), 0.15);

        return true;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC) throws Exception {
        CartoDBMapTestCase baseCase = new CartoDBMapTestCase(getActivity());
        baseCase.setUpWithMap(mapVC);
        mapVC.addLayer(setupVectorLayer(mapVC, ConfigOptions.TestType.MapTest));

        Point2d loc = Point2d.FromDegrees(2.3508, 48.8567);
        mapVC.setPositionGeo(loc.getX(), loc.getY(), 0.15);

        return true;
    }
}