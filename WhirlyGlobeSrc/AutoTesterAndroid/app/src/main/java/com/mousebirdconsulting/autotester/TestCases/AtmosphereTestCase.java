/*
 *  AtmosphereTestCase.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2014 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */
package com.mousebirdconsulting.autotester.TestCases;

import android.app.Activity;
import android.graphics.Color;

import com.mousebird.maply.Atmosphere;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.LayerThread;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MaplyStarModel;
import com.mousebird.maply.MultiplexTileSource;
import com.mousebird.maply.Point3d;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebird.maply.RemoteTileInfo;
import com.mousebird.maply.RemoteTileSource;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.io.File;
import java.io.IOException;


public class AtmosphereTestCase extends MaplyTestCase {

    private ShapesSphereThreadAdapter adapter;
    private LayerThread thread;

    public AtmosphereTestCase(Activity activity) {
        super(activity);
        setTestName("Atmosphere Test Case");
        setDelay(1000);
        this.implementation = TestExecutionImplementation.Globe;
    }

    // Day-time data source
    RemoteTileInfo daySource(MaplyBaseController baseC)
    {
        return new RemoteTileInfo("http://otile1.mqcdn.com/tiles/1.0.0/sat/",
                "png",1,8);
    }

    // Night-time data source
    RemoteTileInfo nightSource(MaplyBaseController baseC)
    {
        return new RemoteTileInfo("http://map1.vis.earthdata.nasa.gov/wmts-webmerc/VIIRS_CityLights_2012/default/2015-05-07/GoogleMapsCompatible_Level8/{z}/{y}/{x}",
                "jpg",1,8);
    }

    // The night images have gone missing
    boolean UseMultipleSources = false;

    LayerThread particleThread = null;
    MaplyStarModel particleAdapter = null;

    @Override
    public boolean setUpWithGlobe(final GlobeController globeVC) throws Exception {

        globeVC.addPostSurfaceRunnable(new Runnable() {
            @Override
            public void run() {
                globeVC.setPositionGeo(0, 0, 5.0);

                Atmosphere atmosphere = new Atmosphere((GlobeController) globeVC, MaplyBaseController.ThreadMode.ThreadAny);
                atmosphere.setWaveLength(new float[]{0.650f, 0.570f, 0.475f});
                atmosphere.setSunPosition(new Point3d(1.0,0.0,0.0));

                String cacheDirName = "day_night2";
                File cacheDir = new File(getActivity().getCacheDir(), cacheDirName);
                cacheDir.mkdir();

                SphericalMercatorCoordSystem coordSystem = new SphericalMercatorCoordSystem();
                QuadImageTileLayer.TileSource tileSource = null;
                if (UseMultipleSources) {
                    RemoteTileInfo sources[] = new RemoteTileInfo[2];
                    sources[0] = daySource(globeVC);
                    sources[1] = nightSource(globeVC);

                    MultiplexTileSource fullTileSource = new MultiplexTileSource(globeVC, sources, coordSystem);
                    fullTileSource.setCacheDir(cacheDir);
                    tileSource = fullTileSource;
                } else {
                    RemoteTileInfo tileInfo = daySource(globeVC);
                    RemoteTileSource singleSource = new RemoteTileSource(tileInfo);
//                    singleSource.setCacheDir(cacheDir);
                    tileSource = singleSource;
                }

                QuadImageTileLayer baseLayer = new QuadImageTileLayer(globeVC, coordSystem, tileSource);
//                baseLayer.setImageDepth(2);
                baseLayer.setCoverPoles(true);
                baseLayer.setHandleEdges(true);
//                baseLayer.setShaderName("Default Triangle;nightday=yes;multitex=yes;lighting=yes");
                baseLayer.setShaderName(atmosphere.getGroundShader().getName());
                globeVC.addLayer(baseLayer);
            }
        });

        globeVC.addPostSurfaceRunnable(new Runnable() {
            @Override
            public void run() {
                globeVC.setClearColor(Color.BLACK);
                particleThread = globeVC.makeLayerThread(false);
                try {
                    particleAdapter = new MaplyStarModel("starcatalog_orig.txt", "star_background.png", getActivity());
                    particleAdapter.addToViewc(globeVC, MaplyBaseController.ThreadMode.ThreadCurrent);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        });

        return true;
    }
}
