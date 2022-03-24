package com.mousebirdconsulting.autotester.TestCases

import okio.source
import okio.buffer
import android.app.Activity
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import android.graphics.Color
import android.os.Handler
import android.os.Looper
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
                it.textureRepeatLength = 8.0
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

        // Expressions require the loader to be set up so we can get a zoom slot... at least until
        // we have a non-loader-dependent zoom slot (See #1523).  Lacking a way to run code after
        // the loader setup completes, a la `addPostSurfaceRunnable`, just wait a while.
        Handler(Looper.getMainLooper()).postDelayed({
            componentObjects.addAll(expressionTests(baseController))
        }, 1000)
    }
    
    private fun offsetTests(vc: BaseController): Collection<ComponentObject> {
        val rsep = 1.5 // separation for when/if performance implementation is supported
        val csep = 1.2
        val cs = 0.1
        return (0..1).flatMap { k ->
            (0..1).flatMap { j ->
                (0..4).flatMap { i ->
                    val lat = 40 - (k * rsep)
                    val lon = j * csep - 90
                    val coords = arrayOf(
                        Point2d.FromDegrees((-11*cs + i*1*cs + lon), lat + 13*cs),
                        Point2d.FromDegrees(( -7*cs + i*2*cs + lon), lat + 11*cs),
                        Point2d.FromDegrees((  0*cs - i*2*cs + lon), lat +  0*cs),
                        Point2d.FromDegrees((-13*cs + i*1*cs + lon), lat + 13*cs),
                    )
            
                    val wideInfo = WideVectorInfo().also {
                        if (j == 0) it.setColor(0.0f, 0.0f, 1.0f, 0.75f)
                        else it.setColor(1.0f, 0.0f, 0.0f, 0.75f)
                        it.enable = true
                        it.setFade(0.0)
                        it.drawPriority = VectorInfo.VectorPriorityDefault
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
                        it.drawPriority = wideInfo.drawPriority + 10
                    }
            
                    val vecAttrs = AttrDictionary()
                    val clAttrs = AttrDictionary()
                    if (k > 0) {
                        val cc = 50 * i
                        vecAttrs.setInt("color", Color.argb(129, 0,cc,255 - cc))
                        clAttrs.setInt("color", Color.argb(192,0,255 - cc,cc))
                    }
                    val vecObj = VectorObject.createLineString(coords, vecAttrs)?.subdivideToGlobe(0.0001)
                    val clObj = VectorObject.createLineString(coords, clAttrs)?.subdivideToGlobe(0.0001)
                    
                    listOfNotNull(
                        vc.addVector(clObj, info, ThreadMode.ThreadCurrent),
                        vc.addWideVector(vecObj, wideInfo, ThreadMode.ThreadCurrent)
                    )
                }
            }
        }
    }
    
    private fun expressionTests(vc: BaseController): Collection<ComponentObject> {
        val coords = arrayOf(
            Point2d.FromDegrees(-100.0, 60.0),
            Point2d.FromDegrees(-110.0, 61.0),
            Point2d.FromDegrees(-120.0, 62.0),
        )
    
        val wideInfo = WideVectorInfo().also {
            it.setColor(1.0f, 0.0f, 0.0f, 1.0f)
            it.enable = true
            it.setFade(0.0)
            it.drawPriority = VectorInfo.VectorPriorityDefault
            it.edgeFalloff = 1.0
            it.offset = 2.0
            it.lineWidth = 20.0f

            it.zoomSlot = baseTestCase.loader?.zoomSlot ?: -1

            val c1 = Color.argb(1.0f, 0.0f, 1.0f, 0.8f)
            val c2 = Color.argb(0.0f, 0.0f, 1.0f, 0.8f)
            it.colorExp = ColorExpressionInfo.createLinear(2.0f, c1, 6.0f, c2)
            it.opacityExp = FloatExpressionInfo.createLinear(2.0f, 0.2f, 6.0f, 0.9f)
            it.widthExp = FloatExpressionInfo.createLinear(2.0f, 1.0f, 6.0f, 20.0f)
            it.offsetExp = FloatExpressionInfo.createLinear(2.0f, -20.0f, 6.0f, 20.0f)
        }
        val info = VectorInfo().also {
            it.setColor(1.0f, 0.0f, 1.0f, 0.8f)
            it.enable = true
            it.drawPriority = wideInfo.drawPriority + 10
        }
    
        val vecObj = VectorObject.createLineString(coords, null)?.subdivideToGlobe(0.0001)
        val clObj = VectorObject.createLineString(coords, null)?.subdivideToGlobe(0.0001)
    
        return listOfNotNull(
            vc.addVector(clObj, info, ThreadMode.ThreadCurrent),
            vc.addWideVector(vecObj, wideInfo, ThreadMode.ThreadCurrent)
        )
    }

    override fun setUpWithMap(mapVC: MapController): Boolean {
        baseTestCase.setUpWithMap(mapVC)
        mapVC.addPostSurfaceRunnable {
            wideVecTest(mapVC)
            mapVC.animatePositionGeo(Point2d.FromDegrees(-100.0, 40.0), 0.5, 0.0, 1.0)
        }
        return true
    }
    
    override fun setUpWithGlobe(globeVC: GlobeController): Boolean {
        baseTestCase.setUpWithGlobe(globeVC)
        globeVC.addPostSurfaceRunnable {
            wideVecTest(globeVC)
            globeVC.animatePositionGeo(Point2d.FromDegrees(-100.0, 40.0), 0.5, 0.0, 1.0)
        }
        return true
    }
    
    override fun shutdown() {
        controller.removeObjects(componentObjects, ThreadMode.ThreadCurrent)
        componentObjects.clear()
        baseTestCase.shutdown()
        super.shutdown()
    }
    
    private val baseTestCase = GeographyClass(getActivity())
    private val componentObjects: MutableList<ComponentObject> = ArrayList()
}