/*  GlobeRotationTestCase.kt
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
 */

package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import com.mousebird.maply.GlobeController
import com.mousebird.maply.MapController
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase

class AnimationDelegateTestCase(activity: Activity) : MaplyTestCase(activity, "Animating Position", TestExecutionImplementation.Both) {
    
    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        if (!baseCase.setUpWithGlobe(globeVC)) {
            return false
        }
        globeVC?.keepNorthUp = false
        globeVC?.height = 1.0
        globeVC?.animatePositionGeo(degToRad(-0.1275), degToRad(51.507222), 0.01, degToRad(45.0), 5.0)
        return true
    }
    
    override fun setUpWithMap(mapVC: MapController?): Boolean {
        if (!baseCase.setUpWithMap(mapVC)) {
            return false
        }
        mapVC?.height = 1.0
        mapVC?.animatePositionGeo(degToRad(-0.1275), degToRad(51.507222), 0.01, degToRad(45.0), 5.0)
        return true
    }

    override fun shutdown() {
        baseCase.shutdown()
        super.shutdown()
    }

    private var baseCase = VectorsTestCase(activity)
}
