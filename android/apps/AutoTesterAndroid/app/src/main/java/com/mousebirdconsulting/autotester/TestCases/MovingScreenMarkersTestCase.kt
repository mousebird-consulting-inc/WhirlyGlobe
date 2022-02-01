/*
 *  MovingScreenMarkersTestCase.kt
 *  WhirlyGlobeLib
 *
 *  Created by Tim Sylvester on 9 Feb 2021.
 *  Copyright 2021-2022 mousebird consulting
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
import android.graphics.Color
import android.os.Handler
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import com.mousebirdconsulting.autotester.R

class MovingScreenMarkersTestCase(activity: Activity) :
        MaplyTestCase(activity, "Moving Screen Markers", TestExecutionImplementation.Both) {
    
    override fun setUpWithGlobe(globeVC: GlobeController): Boolean {
        if (!baseCase.setUpWithGlobe(globeVC)) {
            return false
        }
        setUp()
        globeVC.animatePositionGeo(0.1, 0.1, 0.5, 0.0, 0.5)
        return true
    }

    override fun setUpWithMap(mapVC: MapController): Boolean {
        if (!baseCase.setUpWithMap(mapVC)) {
            return false
        }
        setUp()
        mapVC.animatePositionGeo(0.0, 0.1, 0.5, 0.0, 0.5)
        return true
    }

    private fun setUp() {
        val icon0 = BitmapFactory.decodeResource(activity.resources, R.drawable.teardrop)
        val icon1 = BitmapFactory.decodeResource(activity.resources, R.drawable.teardrop_stroked)
        textures = arrayOf(
                controller.addTexture(icon0, null, threadCurrent)!!,
                controller.addTexture(icon1, null, threadCurrent)!!)

        timerRunnable = Runnable {
            clear()
            objs.addAll(makeMarkers())
            timerRunnable?.let {
                timerHandler.postDelayed(it, (1000 * duration).toLong())
            }
        }.also {
            timerHandler.postDelayed(it, 0)
        }
    }

    override fun shutdown() {
        timerRunnable?.also {
            timerRunnable = null
            timerHandler.removeCallbacks(it)
        }

        clear()

        baseCase.shutdown()
        super.shutdown()
    }

    private fun clear() {
        controller.removeObjects(objs, threadCurrent)
        objs.clear()
    }

    private fun makeMarkers(): Collection<ComponentObject> {
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
        val labels = pts.indices.map {
            ScreenMovingLabel().apply {
                duration = this@MovingScreenMarkersTestCase.duration
                //period = this@MovingScreenMarkersTestCase.duration / 2
                loc = pts[it]
                endLoc = pts[(it + 1) % pts.size]
                layoutImportance = Float.MAX_VALUE
                offset = Point2d(20.0, -10.0)
                text = "<=="
            }
        }
        val labelInfo = LabelInfo().apply {
            textColor = Color.RED
            fontSize = 20.0f
        }
        return listOfNotNull(
            controller.addScreenMovingMarkers(markers, MarkerInfo(), threadCurrent),
            controller.addScreenMovingLabels(labels, labelInfo, threadCurrent)
        )
    }
    
    private val threadCurrent = ThreadMode.ThreadCurrent

    private var baseCase = VectorsTestCase(activity)
    private val duration = 5.0

    private var timerRunnable: Runnable? = null
    private val timerHandler = Handler()

    private var textures: Array<MaplyTexture>? = null
    private val objs = ArrayList<ComponentObject>()
}
