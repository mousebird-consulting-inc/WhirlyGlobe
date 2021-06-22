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
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase

class AnimationDelegateTestCase(activity: Activity) :
        MaplyTestCase(activity, "Animating Position", TestExecutionImplementation.Both) {
    
    val loc = Point2d.FromDegrees(-0.1275, 51.507222)
    val fromHeight = 0.5
    val toHeight = 0.05
    
    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        if (!baseCase.setUpWithGlobe(globeVC)) {
            return false
        }
        globeVC?.apply {
            keepNorthUp = false
            height = fromHeight
            // animate
            animatePositionGeo(loc, toHeight, degToRad(45.0), 5.0)
            // Animate heading only
            addPostSurfaceRunnable({ animatePositionGeo(loc, toHeight, degToRad(-45.0), 2.0) }, 6000)
            // and back
            addPostSurfaceRunnable({ animatePositionGeo(loc, fromHeight, degToRad(0.0), 2.0) }, 8000)
            // repeat with `keepNorthUp` set
            addPostSurfaceRunnable({
                keepNorthUp = true
                animatePositionGeo(loc, toHeight, degToRad(45.0), 2.0)
            }, 10000)
        }
        return true
    }
    
    override fun setUpWithMap(mapVC: MapController?): Boolean {
        if (!baseCase.setUpWithMap(mapVC)) {
            return false
        }
        mapVC?.apply {
            height = fromHeight
            // Note that, unlike `GlobeViewController.keepNorthUp`, this option only affects
            // gestures, not programmatic changes through `animatePositionGeo` and friends
            setAllowRotateGesture(false)
            // animate
            animatePositionGeo(loc, toHeight, degToRad(45.0), 5.0)
            // Animate heading only
            addPostSurfaceRunnable({
                animatePositionGeo(loc, toHeight, degToRad(-45.0), 2.0)
            }, 6000)
        }
        return true
    }

    override fun shutdown() {
        baseCase.shutdown()
        super.shutdown()
    }

    private var baseCase = VectorsTestCase(activity)
}
