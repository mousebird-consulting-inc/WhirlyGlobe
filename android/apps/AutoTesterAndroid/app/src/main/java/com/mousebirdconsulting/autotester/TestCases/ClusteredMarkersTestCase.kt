/*  ClusteredMarkersTestCase.kt
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
import android.graphics.*
import android.util.Log
import com.mousebird.maply.*
import com.mousebird.maply.RenderControllerInterface.TextureSettings
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import java.util.*

class ClusteredMarkersTestCase(activity: Activity?) :
        MaplyTestCase(activity, "Clustered Markers", TestExecutionImplementation.Both) {

    @Throws(Exception::class)
    override fun setUpWithMap(mapVC: MapController): Boolean {
        baseCase.setUpWithMap(mapVC)
        baseCase.baseCase.setForwardMapDelegate(this)  // Get delegate calls from the underlying case
        insertClusteredMarkers(baseCase.vectors, mapVC)
        mapVC.setPositionGeo(pos.x, pos.y, 2.0)
        mapVC.setAllowRotateGesture(true)
        return true
    }
    
    @Throws(Exception::class)
    override fun setUpWithGlobe(globeVC: GlobeController): Boolean {
        baseCase.setUpWithGlobe(globeVC)
        baseCase.baseCase.setForwardGlobeDelegate(this)    // Get delegate calls from the underlying case
        insertClusteredMarkers(baseCase.vectors, globeVC)
        globeVC.animatePositionGeo(pos.x, pos.y, 0.9, 1.0)
        globeVC.keepNorthUp = false
        globeVC.setPerfInterval(20)
        return true
    }
    
    val pos: Point2d = Point2d.FromDegrees(-3.6704803, 40.5023056)

    private fun loadIcon(name: String): Bitmap =
        activity.assets.open("maki icons/$name.png").use { BitmapFactory.decodeStream(it) }
    private fun loadTex(name: String): MaplyTexture? =
        controller.addTexture(loadIcon(name), TextureSettings(), ThreadMode.ThreadCurrent)

    private fun insertClusteredMarkers(vectors: List<VectorObject>, inController: BaseController) {
        val iconSize = Point2d(64.0, 64.0)
        val clusterMarkers: MutableList<ScreenMarker> = ArrayList(vectors.size)
        val pointMarkers: MutableList<ScreenMarker> = ArrayList(vectors.size)
        val markerIcon = loadTex("marker-stroked-24@2x")
        val pointIcon = loadTex("circle-24@2x")
        for (v in vectors) {
            (v.centroid() ?: v.center())?.let { c ->
                pointMarkers.add(ScreenMarker().apply {
                    loc = c
                    tex = pointIcon
                    size = Point2d(8.0, 8.0)
                    selectable = false
                    layoutImportance = Float.MAX_VALUE
                })
                // Note: Increase this to test capacity
                for (ii in 0..0) {
                    clusterMarkers.add(ScreenMarker().apply {
                        loc = c
                        tex = markerIcon
                        size = iconSize
                        selectable = true
                        // Offset based on hemispheres
                        offset = Point2d(iconSize.x * (c.x / Math.PI / 2),
                                      iconSize.y * (c.y / Math.PI))
                        // Some fixed to map rotation, some fixed to screen
                        rotation = if (Math.random() < 0.5) 1.0e-8 else 0.0
                        layoutImportance = ii.toFloat()
                    })
                }
            }
        }
        val info = MarkerInfo().apply {
            setClusterGroup(0)
        }
        inController.addScreenMarkers(clusterMarkers, info, ThreadMode.ThreadAny)?.let {
            objs.add(it)
        }
        inController.addScreenMarkers(pointMarkers, info, ThreadMode.ThreadAny)?.let {
            objs.add(it)
        }
    }
    
    override fun userDidSelect(globeControl: GlobeController?, selObjs: Array<out SelectedObject>?, loc: Point2d?, screenLoc: Point2d?) {
        super.userDidSelect(globeControl, selObjs, loc, screenLoc)
        updateClusterGen()
    }
    override fun userDidTap(globeControl: GlobeController, loc: Point2d, screenLoc: Point2d) {
        super.userDidTap(globeControl, loc, screenLoc)
        updateClusterGen()
    }
    
    override fun userDidSelect(mapControl: MapController?, selObjs: Array<out SelectedObject>?, loc: Point2d?, screenLoc: Point2d?) {
        super.userDidSelect(mapControl, selObjs, loc, screenLoc)
        updateClusterGen()
    }
    override fun userDidTap(mapControl: MapController, loc: Point2d, screenLoc: Point2d) {
        super.userDidTap(mapControl, loc, screenLoc)
        updateClusterGen()
    }
    
    override fun shutdown() {
        controller.removeObjects(objs, ThreadMode.ThreadCurrent)
        objs.clear()
        baseCase.shutdown()
        super.shutdown()
    }
    
    private fun updateClusterGen() {
        controller.clearClusterGenerators()
        n = (n + 1) % 4
        when (n) {
            0 -> {
                Log.d("Maply", "Default Clustering")
                controller.addDefaultClusterGenerator()
            }
            1 -> {
                Log.d("Maply", "Custom 1")
                controller.addClusterGenerator(BasicClusterGenerator(intArrayOf(
                        Color.argb(165, 255, 255, 0)),
                        0, Point2d(64.0, 64.0), controller))
            }
            2 -> {
                Log.d("Maply", "Custom 2")
                controller.addClusterGenerator(BasicClusterGenerator(intArrayOf(
                        Color.argb(255, 0, 0, 0),
                        Color.argb(255, 0, 255, 255),
                        Color.argb(255, 255, 255, 255)),
                        0, Point2d(96.0, 64.0), controller))
            }
            3 -> Log.d("Maply", "No clustering")
        }
    }
    
    private var n = -1
    private val objs = ArrayList<ComponentObject>()
    private var baseCase = VectorsTestCase(activity)
}