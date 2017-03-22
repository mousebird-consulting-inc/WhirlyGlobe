/*
 *  MaplyStarModelTestCase.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2015 mousebird consulting
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

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.LayerThread;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MaplyStarModel;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

import java.io.IOException;


public class MaplyStarModelTestCase extends MaplyTestCase {

    public MaplyStarModelTestCase(Activity activity) {
        super(activity);
        setTestName("Maply Star Test Case");
        setDelay(10000);
        this.implementation = TestExecutionImplementation.Globe;
    }

    LayerThread particleThread = null;
    MaplyStarModel particleAdapter = null;


    @Override
    public boolean setUpWithGlobe(final GlobeController globeVC) throws Exception {
        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithGlobe(globeVC);

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
