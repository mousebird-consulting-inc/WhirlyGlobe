/*
 *  QuadParticleSystemTestCase.java
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
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.LayerThread;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;

public class ComplexParticleSystemTestCase extends MaplyTestCase {

    public ComplexParticleSystemTestCase(Activity activity) {
        super(activity);

        this.setTestName("Complex Particle System Test");
        this.setDelay(2000);
    }

    ComplexParticleThreadAdapter particleAdapter = null;
    LayerThread particleThread = null;

    @Override
    public boolean setUpWithGlobe(final GlobeController globeVC) throws Exception {
        CartoDBDarkTestCase baseView = new CartoDBDarkTestCase(getActivity());
        baseView.setUpWithGlobe(globeVC);

        // Kick off the particle thread
        // Note: Need to shut this down
        globeVC.onSurfaceCreatedTask(new Runnable() {
            @Override
            public void run() {
                particleThread = globeVC.makeLayerThread();
                // Note: minZoom should be 5
                particleAdapter = new ComplexParticleThreadAdapter(globeVC, particleThread,
                        "http://tilesets.s3-website-us-east-1.amazonaws.com/wind_test/{dir}_tiles/{z}/{x}/{y}.png", 0, 18);
            }
        });

        return true;
    }
}
