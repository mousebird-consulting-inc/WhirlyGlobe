/*
 *  ParticleSystemTestCase.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 26/1/16
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

import com.mousebird.maply.GlobeController;
import com.mousebird.maply.LayerThread;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

public class SimpleParticleSystemTestCase extends MaplyTestCase {

    public SimpleParticleSystemTestCase(Activity activity) {
        super(activity);

        this.setTestName("Simple Particle System Test");
        this.setDelay(2000);
        this.implementation = TestExecutionImplementation.Globe;
    }

    SimpleParticleThreadAdapter particleAdapter = null;
    LayerThread particleThread = null;

    @Override
    public boolean setUpWithGlobe(final GlobeController globeVC) throws Exception {

        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithGlobe(globeVC);

        // Kick off the particle thread
        // Note: Need to shut this down
        globeVC.addPostSurfaceRunnable(new Runnable() {
            @Override
            public void run() {
                particleThread = globeVC.makeLayerThread(false);
                particleAdapter = new SimpleParticleThreadAdapter(globeVC, particleThread);
            }
        });

        return true;
    }
}
