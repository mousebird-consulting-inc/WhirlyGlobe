/*
 *  MovingScreenMarkersTestCase.kt
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
import android.graphics.BitmapFactory
import android.os.Handler
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import com.mousebirdconsulting.autotester.R

public class MovingScreenMarkersTestCase : MaplyTestCase {

    constructor(activity: Activity) : super(activity) {
        testName = "Moving Screen Markers"
        implementation = TestExecutionImplementation.Both

        baseCase = VectorsTestCase(activity)
    }

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        if (!baseCase.setUpWithGlobe(globeVC)) {
            return false
        }
        setUp()
        globeVC?.animatePositionGeo(0.1, 0.1, 0.5, 0.0, 0.5)
        return true
    }

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        if (!baseCase.setUpWithMap(mapVC)) {
            return false
        }
        setUp()
        mapVC?.animatePositionGeo(0.0, 0.1, 0.5, 0.0, 0.5)
        return true
    }

    private fun setUp() {
        val icon0 = BitmapFactory.decodeResource(getActivity().resources, R.drawable.teardrop)
        val icon1 = BitmapFactory.decodeResource(getActivity().resources, R.drawable.teardrop_stroked)
        textures = arrayOf(
                controller.addTexture(icon0, null, threadCurrent)!!,
                controller.addTexture(icon1, null, threadCurrent)!!)

        timerRunnable = Runnable {
            clearMarkers()
            markerObj = makeMarkers()
            if (timerRunnable != null) {
                timerHandler.postDelayed(timerRunnable, (1000 * duration).toLong())
            }
        }
        timerHandler.postDelayed(timerRunnable, 0)
    }

    override fun shutdown() {
        timerRunnable.also {
            timerRunnable = null
            timerHandler.removeCallbacks(it)
        }

        baseCase.shutdown()
        super.shutdown()
    }

    private fun clearMarkers() {
        markerObj?.also {
            controller.removeObject(it, threadCurrent)
            markerObj = null
        }
    }

    private fun makeMarkers(): ComponentObject? {
        val pts = arrayOf(
                Point2d.FromDegrees(0.0, 0.0),
                Point2d.FromDegrees(10.0, 10.0),
                Point2d.FromDegrees(0.0, 20.0),
                Point2d.FromDegrees(-10.0, 10.0)
        )

        val markers = pts.indices.map {
            ScreenMovingMarker().apply {
                duration = this@MovingScreenMarkersTestCase.duration
                period = this@MovingScreenMarkersTestCase.duration / 2
                loc = pts[it]
                endLoc = pts[(it + 1) % pts.size]
                size = Point2d(32.0, 32.0)
                images = textures
                layoutImportance = Float.MAX_VALUE
            }
        }
        return controller.addScreenMovingMarkers(markers, MarkerInfo(), threadCurrent)
    }

    private val threadCurrent = RenderControllerInterface.ThreadMode.ThreadCurrent

    private var baseCase: VectorsTestCase
    private val duration = 5.0

    private var timerRunnable: Runnable? = null
    private val timerHandler = Handler()

    private var textures: Array<MaplyTexture>? = null
    private var markerObj: ComponentObject? = null
}
