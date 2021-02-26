/*
 *  GlobeRotationTestCase.kt
 *  WhirlyGlobeLib
 *
 *  Created by Tim Sylvester on 9 Feb 2021.
 *  Copyright 2021 mousebird consulting
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

package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import com.mousebird.maply.GlobeController
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase

public class GlobeRotationTestCase : MaplyTestCase {

    constructor(activity: Activity) : super(activity) {
        testName = "Globe Rotation (#1286)"
        implementation = TestExecutionImplementation.Globe

        baseCase = VectorsTestCase(activity)
    }

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        if (!baseCase.setUpWithGlobe(globeVC)) {
            return false
        }
        globeVC?.animatePositionGeo(0.0, Math.PI/8, 0.75, 0.0, 0.5)
        return true
    }

    override fun shutdown() {
        baseCase.shutdown()
        super.shutdown()
    }

    private var baseCase: VectorsTestCase
}
