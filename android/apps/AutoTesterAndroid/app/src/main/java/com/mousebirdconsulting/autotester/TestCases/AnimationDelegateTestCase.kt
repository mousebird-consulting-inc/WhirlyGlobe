/*  AnimationDelegateTestCase.kt
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
import android.util.Log
import android.widget.Toast
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import kotlin.math.exp
import kotlin.math.log

class AnimationDelegateTestCase(activity: Activity) :
        MaplyTestCase(activity, "Animation Delegate", TestExecutionImplementation.Both) {
    
    private val initLoc = Point2d.FromDegrees(12.5037997, 41.893988)
    private val loc = Point2d.FromDegrees(0.5508867, 51.5203505)
    private val fromHeight = 1.5
    private val toHeight = 0.01

    override fun setUpWithGlobe(vc: GlobeController): Boolean {
        if (!baseCase.setUpWithGlobe(vc)) {
            return false
        }
        vc.apply {
            keepNorthUp = false
            positionGeo = Point3d(initLoc.x, initLoc.y, fromHeight)
            heading = degToRad(0.0)
        }
        script(vc, sequenceOf(
            // animate
            ScriptItem(10.0, null) { vc.animatePositionGeo(loc, toHeight, degToRad(45.0), 9.0) },
        
            // Animate heading only
            ScriptItem(2.0, "Heading Only") { vc.animatePositionGeo(loc, toHeight, degToRad(-45.0), 2.0) },
            // and back
            ScriptItem(3.0, null) { vc.animatePositionGeo(loc, toHeight, degToRad(0.0), 2.0) },
        
            // repeat with `keepNorthUp` set
            ScriptItem(5.0, "Keep North Up") {
                vc.keepNorthUp = true
                vc.animatePositionGeo(initLoc, fromHeight, degToRad(45.0), 5.0)
            },
            ScriptItem(6.0, null) { vc.animatePositionGeo(loc, toHeight, degToRad(45.0), 5.0) },
        
            // Again with old, linear zoom scale
            ScriptItem(5.0, "Linear Zoom") {
                vc.zoomAnimationEasing = BaseController.ZoomAnimationEasing { z0, z1, t -> z0 + (z1 - z0) * t }
                vc.animatePositionGeo(loc, fromHeight, 0.0, 5.0)
            },
            ScriptItem(6.0, null) { vc.animatePositionGeo(loc, toHeight, 0.0, 5.0) },
        
            // Again, with flair!
            ScriptItem(6.0, "Flair") {
                vc.zoomAnimationEasing = BaseController.ZoomAnimationEasing(easeInOnly)
                vc.animatePositionGeo(loc, fromHeight, 0.0, 5.0)
            },
            ScriptItem(6.0, null) { vc.animatePositionGeo(loc, toHeight, 0.0, 5.0) },
        ))
        return true
    }
    
    override fun setUpWithMap(vc: MapController): Boolean {
        if (!baseCase.setUpWithMap(vc)) {
            return false
        }
        vc.apply {
            positionGeo = Point3d(initLoc.x, initLoc.y, fromHeight)
            height = fromHeight
            // Note that, unlike `GlobeViewController.keepNorthUp`, this option only affects
            // gestures, not programmatic changes through `animatePositionGeo` and friends
            allowRotateGesture = false
        }
        script(vc, sequenceOf(
            // animate
            ScriptItem(10.0, null) { vc.animatePositionGeo(loc, toHeight, degToRad(45.0), 9.0) },
        
            // Animate heading only
            ScriptItem(2.0, "Heading Only") { vc.animatePositionGeo(loc, toHeight, degToRad(-45.0), 2.0) },
            // and back
            ScriptItem(3.0, null) { vc.animatePositionGeo(loc, toHeight, degToRad(0.0), 2.0) },
        
            // Again with old, linear zoom scale
            ScriptItem(5.0, "Linear Zoom") {
                vc.zoomAnimationEasing = BaseController.ZoomAnimationEasing { z0, z1, t -> z0 + (z1 - z0) * t }
                vc.animatePositionGeo(loc, fromHeight, 0.0, 5.0)
            },
            ScriptItem(6.0, null) { vc.animatePositionGeo(loc, toHeight, 0.0, 5.0) },
        
            // Again, with flair!
            ScriptItem(6.0, "Flair") {
                vc.zoomAnimationEasing = BaseController.ZoomAnimationEasing(easeInOnly)
                // We have to target a lower zoom level for now because the Android
                // flat map doesn't handle zooming out out beyond about 2 well.
                vc.animatePositionGeo(loc, 1.0, 0.0, 5.0)
            },
            ScriptItem(6.0, null) { vc.animatePositionGeo(loc, toHeight, 0.0, 5.0) },
        ))
        return true
    }

    override fun shutdown() {
        baseCase.shutdown()
        super.shutdown()
    }
    
    private data class ScriptItem(
        val duration: Double,
        val toast: String?,
        val run: ()->Unit)
    private fun script(vc: BaseController, items: Sequence<ScriptItem>) {
        var t = 0.0
        items.forEach { (duration, toastStr, run) ->
            vc.addPostSurfaceRunnable({ toast(toastStr) }, (t * 1000.0).toLong())
            vc.addPostSurfaceRunnable(run, (t * 1000.0).toLong())
            t += duration
        }
    }
    
    // https://easings.net/#easeInBack
    private fun easeInBack(t: Double): Double {
        val c = 1.70158
        return ((c + 1) * t - c) * t * t;
    }
    private fun linearize(f: ((Double,Double,Double)->Double)): ((Double,Double,Double)->Double) {
        return { z0,z1,t -> exp(f(log(z0,Math.E),log(z1,Math.E),t)) }
    }
    // Apply easing only on zooming in to avoid going negative
    private val easeInOnly = linearize { z0,z1,t ->
        z0+(z1-z0)*(if (z0>z1) easeInBack(t) else (1-easeInBack(1-t)))
    }
    
    private var baseCase = VectorsTestCase(activity)
    
    private var prevToast: Toast? = null
    private fun toast(s: String?) {
        activity?.runOnUiThread {
            prevToast?.cancel()
            prevToast = s?.let {
                activity?.applicationContext?.let { c ->
                    Toast.makeText(c, s, Toast.LENGTH_SHORT).also {
                        it.show()
                    }
                }
            }
        }
    }
}
