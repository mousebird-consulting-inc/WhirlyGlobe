/*  BillboardAdapter.kt
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2021 mousebird consulting
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
import android.graphics.BitmapFactory
import com.mousebird.maply.*
import com.mousebird.maply.RenderControllerInterface.TextureSettings
import com.mousebird.maply.RenderControllerInterface.ThreadMode
import com.mousebirdconsulting.autotester.R
import java.util.*

class BillboardAdapter(private val viewC: GlobeController,
                       private val activity: Activity,
                       private val threadMode: ThreadMode) {

    fun start() {
        addBillboard()
    }
    
    fun stop() {
        atmosphere?.removeFromController()
        atmosphere = null

        sunObj?.let { viewC.removeObject(it, ThreadMode.ThreadCurrent) }
        sunObj = null

        moonObj?.let { viewC.removeObject(it, ThreadMode.ThreadCurrent) }
        moonObj = null
    }
    
    private fun addBillboard() {
        val sun = Sun()

        val light = sun.makeLight()
        viewC.clearLights()
        viewC.addLight(light)
        
        // Sun
        val billSun = Billboard().apply {
            val sunPosition = sun.asPosition()
            center = Point3d(sunPosition[0].toDouble(), sunPosition[1].toDouble(), 5.4 * EarthRadius)
            selectable = false
    
            screenObject = ScreenObject().apply {
                val sunBitmap = BitmapFactory.decodeResource(activity.resources, R.drawable.sun)
                val sunTex = viewC.addTexture(sunBitmap, TextureSettings(), ThreadMode.ThreadAny)
                addTexture(sunTex, floatArrayOf(1.0f, 1.0f, 1.0f, 1.0f), 0.9f, 0.9f)
            }
        }

        val info = BillboardInfo().apply {
            orient = BillboardInfo.Orient.Eye
            drawPriority = 2
        }

        sunObj = viewC.addBillboards(arrayListOf(billSun), info, threadMode)
        
        // Moon
        val billMoon = Billboard().apply {
            val cal = GregorianCalendar.getInstance()
            cal.timeZone = SimpleTimeZone(SimpleTimeZone.UTC_TIME, "UTC")
            
            val moon = Moon(cal)
            val moonPosition = moon.asPosition()

            center = Point3d(moonPosition.x, moonPosition.y, 5.4 * EarthRadius)
            screenObject = ScreenObject().apply {
                val bmMoon = BitmapFactory.decodeResource(activity.resources, R.drawable.moon)
                val moonTex = viewC.addTexture(bmMoon, TextureSettings(), ThreadMode.ThreadAny)
                addTexture(moonTex, floatArrayOf(1.0f, 1.0f, 1.0f, 1.0f), 0.75f, 0.75f)
            }
            selectable = false
        }

        info.drawPriority = 3
        moonObj = viewC.addBillboards(arrayListOf(billMoon), info, threadMode)
        
        atmosphere = Atmosphere(viewC, ThreadMode.ThreadCurrent).apply {
            waveLength = floatArrayOf(0.650f, 0.570f, 0.475f)
            setSunPosition(sun.direction)
        }
    }

    var atmosphere: Atmosphere? = null
    var sunObj: ComponentObject? = null
    var moonObj: ComponentObject? = null

    companion object {
        private const val EarthRadius = 6371000f
    }
}