package com.mousebirdconsulting.autotester.TestCases

import okio.source
import okio.buffer
import android.app.Activity
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import android.graphics.Color
import android.util.Log
import com.mousebird.maply.*
import com.mousebird.maply.RenderControllerInterface.TextureSettings
import java.lang.Exception
import java.util.ArrayList

class WideVectorsTestCase(activity: Activity?) :
        MaplyTestCase(activity, "Wide Vectors", TestExecutionImplementation.Both) {
    
    init {
        setDelay(4)
    }
    
    private fun addGeoJSON(
        baseController: BaseController, name: String,
        width: Float, drawPriority: Int, color: Int,
        usePattern: Boolean, edge: Double
    ): Collection<ComponentObject> {
        val tex = if (usePattern) {
            // Build a dashed pattern
            val patternImage = LinearTextureBuilder().run {
                setPattern(intArrayOf(4, 4))
                makeImage()
            }
            val texSet = TextureSettings().apply {
                wrapU = true
                wrapV = true
            }
            baseController.addTexture(patternImage, texSet, ThreadMode.ThreadCurrent)
        } else null

        val wideVecInfo = WideVectorInfo().also {
            it.color = color
            it.lineWidth = width
            it.drawPriority = drawPriority
            if (tex != null) {
                it.setTexture(tex)
                it.setTextureRepeatLength(8.0)
            }
            it.edgeFalloff = edge
        }
        val vecInfo = VectorInfo().apply {
            lineWidth = 4.0f
            setColor(Color.BLACK)
        }

        try {
            val stream = activity.assets.open("wide_vecs/$name")
            VectorObject.createFromGeoJSON(stream.source().buffer().readUtf8())
                    ?.subdivideToGlobeGreatCircle(0.0001)?.let {
                return listOfNotNull(
                    baseController.addVector(it, vecInfo, ThreadMode.ThreadCurrent),
                    baseController.addWideVector(it, wideVecInfo, ThreadMode.ThreadCurrent))
            }
        } catch (e: Exception) {
            Log.e(javaClass.simpleName, "Failed", e)
        }
        return emptyList()
    }
    
    private fun wideVecTest(baseController: BaseController) {
        val pri = VectorInfo.VectorPriorityDefault
        componentObjects.addAll(addGeoJSON(baseController, "mowing-lawn.geojson", 20.0f, pri, Color.BLUE, true, 1.0))
        componentObjects.addAll(addGeoJSON(baseController, "spiral.geojson",      20.0f, pri, Color.BLUE, true, 1.0))
        componentObjects.addAll(addGeoJSON(baseController, "square.geojson",      20.0f, pri, Color.BLUE, true, 1.0))
        componentObjects.addAll(addGeoJSON(baseController, "line.geojson",       100.0f, pri, Color.BLUE, true, 1.0))
        componentObjects.addAll(addGeoJSON(baseController, "line.geojson",        60.0f, pri + 10, Color.RED, true, 1.0))
        componentObjects.addAll(addGeoJSON(baseController, "track.geojson",       20.0f, pri, Color.BLUE, true, 1.0))
        componentObjects.addAll(addGeoJSON(baseController, "uturn2.geojson",      20.0f, pri, Color.BLUE, true, 1.0))
        componentObjects.addAll(addGeoJSON(baseController, "testJson.geojson",    20.0f, pri, Color.BLUE, true, 1.0))
        componentObjects.addAll(addGeoJSON(baseController, "sawtooth.geojson",    50.0f, pri, Color.RED, false, 20.0))
        componentObjects.addAll(offsetTests(baseController))
    }
    
    private fun offsetTests(vc: BaseController): Collection<ComponentObject> {
        val sep = 15 // separation for when/if performance implementation is supported
        return (0..1).flatMap { k ->
            (0..0).flatMap { j ->
                (0..4).flatMap { i ->
                    val lat = 35
                    val lon = (j+k) * sep - 80
                    val coords = arrayOf(
                        Point2d.FromDegrees((-15 + i * 1 + lon).toDouble(), lat + 17.0),
                        Point2d.FromDegrees((-10 + i * 2 + lon).toDouble(), lat + 15.0),
                        Point2d.FromDegrees((  0 - i * 2 + lon).toDouble(), lat + 0.0),
                        Point2d.FromDegrees((-15 + i * 1 + lon).toDouble(), lat + 17.0),
                    )
            
                    val wideInfo = WideVectorInfo().also {
                        if (j == 0) it.setColor(0.0f, 0.0f, 1.0f, 0.5f)
                        else it.setColor(1.0f, 0.0f, 0.0f, 0.5f)
                        //it.filled = false
                        it.enable = true
                        it.setFade(0.0)
                        it.drawPriority = VectorInfo.VectorPriorityDefault
                        //it.centered = true
                        it.edgeFalloff = i.toDouble()
                        it.joinType = if (k == 0) WideVectorInfo.JoinType.BevelJoin else WideVectorInfo.JoinType.MiterJoin
                        it.offset = (2 * i).toDouble()
                        it.mitreLimit = 90.0
                        it.lineWidth = 20.0f
                        //it.wideVecImpl = if (j == 0) perf : default
                    }
                    val info = VectorInfo().also {
                        it.setColor(1.0f, 0.0f, 1.0f, 0.5f)
                        it.enable = true
                        it.drawPriority = wideInfo.drawPriority
                    }
            
                    val vecObj = VectorObject.createLineString(coords)?.subdivideToGlobe(0.0001)
                    listOfNotNull(
                        vc.addVector(vecObj, info, ThreadMode.ThreadCurrent),
                        vc.addWideVector(vecObj, wideInfo, ThreadMode.ThreadCurrent)
                    )
                }
            }
        }
    }
    
    override fun setUpWithMap(mapVC: MapController): Boolean {
        baseTestCase.setUpWithMap(mapVC)
        wideVecTest(mapVC)
        mapVC.animatePositionGeo(Point2d.FromDegrees(-100.0, 40.0), 0.5, 0.0, 1.0)
        return true
    }
    
    override fun setUpWithGlobe(globeVC: GlobeController): Boolean {
        baseTestCase.setUpWithGlobe(globeVC)
        wideVecTest(globeVC)
        globeVC.animatePositionGeo(Point2d.FromDegrees(-100.0, 40.0), 0.5, 0.0, 1.0)
        return true
    }
    
    override fun shutdown() {
        controller.removeObjects(componentObjects, ThreadMode.ThreadCurrent)
        componentObjects.clear()
        baseTestCase.shutdown()
        super.shutdown()
    }
    
    private val baseTestCase = CartoLightTestCase(getActivity())
    private val componentObjects: MutableList<ComponentObject> = ArrayList()
}