/*
 *  BillboardTestCase.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2016 mousebird consulting
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
import com.mousebird.maply.MaplyBaseController;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;


public class BillboardTestCase extends MaplyTestCase {

    public BillboardTestCase(Activity activity) {
        super(activity);
        setTestName("Billboard Test Case");
        setDelay(200);
        this.implementation = TestExecutionImplementation.Globe;
    }

    @Override
    public boolean setUpWithGlobe(final GlobeController globeVC) throws Exception {
        StamenRemoteTestCase baseView = new StamenRemoteTestCase(getActivity());
        baseView.setUpWithGlobe(globeVC);
        globeVC.addPostSurfaceRunnable(new Runnable() {
            @Override
            public void run() {
                BillboardAdapter adapter = new BillboardAdapter(globeVC, getActivity(), MaplyBaseController.ThreadMode.ThreadAny);
                adapter.start();
            }
        });

        return true;
    }
}
