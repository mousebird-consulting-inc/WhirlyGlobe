package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.os.Handler;

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.MapController;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebird.maply.RemoteTileInfo;
import com.mousebird.maply.RemoteTileSource;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.io.File;

/**
 * Repeatedly add and remove layers
 */
public class LayerShutdownTestCase extends MaplyTestCase  {
    public LayerShutdownTestCase(Activity activity) {
        super(activity);

        setTestName("Layer Startup/Shutdown Test");
        setDelay(4);
        this.implementation = MaplyTestCase.TestExecutionImplementation.Both;
    }

    QuadImageTileLayer stamenLayer = null;

    private QuadImageTileLayer setupImageLayer(ConfigOptions.TestType testType, MaplyBaseController baseController) {
        String cacheDirName = "stamen_watercolor3";
        File cacheDir = new File(getActivity().getCacheDir(), cacheDirName);
        cacheDir.mkdir();
        RemoteTileSource remoteTileSource = new RemoteTileSource(new RemoteTileInfo("http://tile.stamen.com/watercolor/", "png", 0, 18));
        remoteTileSource.setCacheDir(cacheDir);
        // Note: Turn this on to get more information from the tile source
//		remoteTileSource.debugOutput = true;
        SphericalMercatorCoordSystem coordSystem = new SphericalMercatorCoordSystem();
        QuadImageTileLayer baseLayer = new QuadImageTileLayer(baseController, coordSystem, remoteTileSource);
        if (testType == ConfigOptions.TestType.MapTest)
        {
//			baseLayer.setSingleLevelLoading(true);
//			baseLayer.setUseTargetZoomLevel(true);
//			baseLayer.setMultiLevelLoads(new int[]{-3});
            baseLayer.setCoverPoles(false);
            baseLayer.setHandleEdges(false);
        } else {
            baseLayer.setCoverPoles(true);
            baseLayer.setHandleEdges(true);
        }

        return baseLayer;
    }

    public void cycleLayer(final ConfigOptions.TestType testType,final MaplyBaseController baseVC)
    {
        final Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                if (stamenLayer != null) {
                    baseVC.removeLayer(stamenLayer);
                    stamenLayer = null;
                }
                stamenLayer = setupImageLayer(testType,baseVC);
                baseVC.addLayer(stamenLayer);

                cycleLayer(testType,baseVC);
            }
        }, 2000);
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        cycleLayer(ConfigOptions.TestType.GlobeTest,globeVC);
        globeVC.animatePositionGeo(-3.6704803, 40.5023056, 5, 1.0);
//		globeVC.setZoomLimits(0.0,1.0);
        return true;
    }

    @Override
    public boolean setUpWithMap(MapController mapVC)
    {
        cycleLayer(ConfigOptions.TestType.MapTest,mapVC);
        mapVC.animatePositionGeo(-3.6704803, 40.5023056, 5, 1.0);
        mapVC.setAllowRotateGesture(true);
//		mapVC.setZoomLimits(0.0,1.0);
        return true;
    }}
