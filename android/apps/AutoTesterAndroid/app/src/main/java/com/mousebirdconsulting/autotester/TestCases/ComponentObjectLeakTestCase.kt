/*  ComponentObjectLeakTestCase.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2022 mousebird consulting
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
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import com.mousebirdconsulting.autotester.R
import java.util.*

class ComponentObjectLeakTestCase(activity: Activity?) :
        MaplyTestCase(activity, "ComponentObject Leak Test") {

    @Throws(Exception::class)
    override fun setUpWithGlobe(globeVC: GlobeController): Boolean {
        baseCase.setUpWithGlobe(globeVC)
        globeVC.setPositionGeo(0.5, 0.5, 1.0)
        addAndRemove(COUNT, DURATION_SEC)
        return true
    }
    
    @Throws(Exception::class)
    override fun setUpWithMap(mapVC: MapController): Boolean {
        baseCase.setUpWithMap(mapVC)
        mapVC.setPositionGeo(0.5, 0.5, 1.0)
        addAndRemove(COUNT, DURATION_SEC)
        return true
    }
    
    override fun shutdown() {
        stop = true
        baseCase.shutdown()
        super.shutdown()
    }
    
    private fun addAndRemove(count: Int, durationSec: Double) {
        stop = false
        val icon = BitmapFactory.decodeResource(activity.resources, R.drawable.sticker)
        controller?.addPostSurfaceRunnable({
            if (!stop) {
                val endTime = System.nanoTime() + (durationSec * 1.0e9).toLong()
                addAndRemove(count, endTime, icon)
            }
        }, 5000)
    }
    
    private fun addAndRemove(count: Int, endTime: Long, image: Bitmap) {
        val viewC = controller
        if (System.nanoTime() < endTime && viewC != null && !stop) {
            viewC.addPostSurfaceRunnable({
                addAndRemove(count, image)
                addAndRemove(count, endTime, image)
            }, 1)
        }
    }
    
    private fun addAndRemove(count: Int, img: Bitmap) {
        val viewC = controller ?: return
        val markerInfo = MarkerInfo().apply {
            //setMinVis(0.f);
            //setMaxVis(1.f);
            //setColor(1);
            drawPriority = 100100
        }
        val markers = ArrayList<ComponentObject>(count)
        var i = 0
        while (i++ < count && !stop) {
            val marker = ScreenMarker().apply {
                image = img
                loc = Point2d(Math.random(), Math.random())
                size = Point2d(64.0, 64.0)
            }
            viewC.addScreenMarker(marker, markerInfo, ThreadMode.ThreadAny)?.let {
                markers.add(it)
            }
        }
        viewC.removeObjects(markers, ThreadMode.ThreadAny)
    }
    
    var stop = false
    private val baseCase = StamenRemoteTestCase(getActivity()).apply {
        doColorChange = false
    }
    
    companion object {
        private const val COUNT = 100
        private const val DURATION_SEC = 120.0
    }
}