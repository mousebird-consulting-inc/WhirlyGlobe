package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.graphics.Color
import android.os.Looper
import android.util.Log
import android.widget.Toast
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import okio.buffer
import okio.source
import java.util.*

class VectorsTestCase(activity: Activity?) :
        MaplyTestCase(activity, "Vectors", TestExecutionImplementation.Both) {

    private val compObjs = ArrayList<ComponentObject>()

    @Throws(Exception::class)
    private fun overlayCountries(baseVC: BaseController) {
        val vectorInfo = VectorInfo().apply {
            color = Color.RED
            drawPriority = RenderController.VectorDrawPriorityDefault+1
        }
        val wideInfo = WideVectorInfo().apply {
            color = Color.WHITE
            drawPriority = RenderController.VectorDrawPriorityDefault
            lineWidth = 5f
        }
        val assetMgr = activity.assets
        var loaded = 0
        var msg: Toast? = null
        val paths = assetMgr.list("country_json_50m")
        paths?.forEach { path ->
            val txt = "Loading %s : %.1f%%".format(
                path.removeSuffix(".geojson"),
                ++loaded * 100.0 / paths.size)
            activity.runOnUiThread {
                msg?.cancel()
                msg = Toast.makeText(activity.applicationContext, txt, Toast.LENGTH_SHORT)
                msg?.show()
            }
            val json = assetMgr.open("country_json_50m/$path").use { stream ->
                stream.source().use { source ->
                    source.buffer().use { buf ->
                        buf.readUtf8()
                    }
                }
            }
            Log.d("Maply", "Loading $path")
            if (!baseVC.isRunning || canceled) {
                return@forEach
            }
            VectorObject.createFromGeoJSON(json)?.apply {
                selectable = true
            }?.let { vec ->
                vectors.add(vec)
                // We use ThreadAny here so that it's handled correctly if the map is shut
                // down while we're running.
                val cos = listOfNotNull(
                    baseVC.addVector(vec, vectorInfo, ThreadMode.ThreadAny),
                    baseVC.addWideVector(vec, wideInfo, ThreadMode.ThreadAny))
                compObjs.addAll(cos)
                onVectorLoaded?.invoke(vec,cos)
            }
        }
        activity.runOnUiThread { msg?.cancel() }

        // Build a really big vector for testing
//		VectorObject bigVecObj = new VectorObject();
//		Point2d pts[] = new Point2d[20000];
//		for (int ii=0;ii<20000;ii++)
//			pts[ii] = new Point2d(Math.random(), Math.random());
//		bigVecObj.addAreal(pts);
        
        // Then change to white
        val newVectorInfo = WideVectorInfo().apply {
            color = Color.WHITE
            lineWidth = 4f
        }
        // todo: doesn't work yet
        for (compObj in compObjs) {
            baseVC.changeWideVector(compObj, newVectorInfo, ThreadMode.ThreadAny)
        }
    }

    var onVectorsLoaded: ((Collection<VectorObject>)->Unit)? = null
    var onVectorLoaded: ((VectorObject,Collection<ComponentObject>)->Unit)? = null
    
    @Throws(Exception::class)
    override fun setUpWithMap(mapVC: MapController): Boolean {
        controller = mapVC
        mapController = mapVC
        baseCase.setUpWithMap(mapVC)
        baseCase.setForwardMapDelegate(this)
        loadVectorsDelayed()
        return true
    }
    
    @Throws(Exception::class)
    override fun setUpWithGlobe(globeVC: GlobeController): Boolean {
        controller = globeVC
        globeController = globeVC
        baseCase.setUpWithGlobe(globeVC)
        baseCase.setForwardGlobeDelegate(this)
        loadVectorsDelayed()
        return true
    }

    private fun loadVectorsDelayed() {
        controller?.addPostSurfaceRunnable {
            // Load vectors on a separate thread so that you can
            // back out of the test case before they're all loaded.
            Thread({
                Looper.prepare()
                Thread.sleep(500)
                if (!canceled && controller != null) {
                    overlayCountries(controller)
                }
                if (!canceled && controller != null) {
                    onVectorsLoaded?.invoke(vectors)
                }
            }, "vector loader").start()
        }
    }
    
    override fun shutdown() {
        baseCase.shutdown()
        super.shutdown()
    }
    
    private fun didSelect(selObjs: Array<SelectedObject>) {
        // todo: place a marker or something at the center like iOS does
        val s = selObjs.mapNotNull { it.selObj as? VectorObject }
            .mapNotNull {
                val center = it.centroid() ?: it.center() ?: return@mapNotNull null
                val name = it.attributes?.getString("NAME_FORMA") ?:
                    it.attributes?.getString("NAME") ?:
                    it.attributes?.getString("ADMIN") ?:
                    it.attributes?.getString("title") ?: "?"
                mapController?.animatePositionGeo(center, mapController.height, mapController.heading, 1.0)
                globeController?.animatePositionGeo(center, globeController.height, globeController.heading, 1.0)
                String.format("$name (%.5f,%.5f)",
                        center.y * 180 / Math.PI,
                        center.x * 180 / Math.PI)
            }.joinToString("\n")
        Toast.makeText(activity.applicationContext, s, Toast.LENGTH_SHORT).show()
    }

    override fun userDidSelect(globeControl: GlobeController, selObjs: Array<SelectedObject>, loc: Point2d, screenLoc: Point2d) {
        super.userDidSelect(globeControl, selObjs, loc, screenLoc)
        didSelect(selObjs)
    }
    
    override fun userDidSelect(mapControl: MapController, selObjs: Array<SelectedObject>, loc: Point2d, screenLoc: Point2d) {
        super.userDidSelect(mapControl, selObjs, loc, screenLoc)
        didSelect(selObjs)
    }
    
    val baseCase: MaplyTestCase = GeographyClass(activity)

    private val vectors = ArrayList<VectorObject>()
}