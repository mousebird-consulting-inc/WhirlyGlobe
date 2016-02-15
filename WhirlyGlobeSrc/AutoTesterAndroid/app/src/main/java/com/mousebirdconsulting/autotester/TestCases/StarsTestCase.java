/*
 *  StarsTestCase.java
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

import com.mousebird.maply.GlobeController;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebirdconsulting.autotester.MaplyStarModel;


public class StarsTestCase extends MaplyTestCase {

    public StarsTestCase(Activity activity) {
        super(activity);
        setTestName("Stars Test Case");
        setDelay(5);
    }

    @Override
    public boolean setUpWithGlobe(GlobeController globeVC) throws Exception {
        MaplyStarModel maplyStarModel = new MaplyStarModel("starcatalog_orig.txt", "star_background.png", getActivity());
        return true;
    }
}
