package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.graphics.Color
import android.os.Handler
import android.util.Log
import android.widget.Toast
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import okio.buffer
import okio.source
import java.util.*

class VectorsTestCase(activity: Activity?) :
        MaplyTestCase(activity, "Vectors Test", TestExecutionImplementation.Both) {

    private val compObjs = ArrayList<ComponentObject>()

    @Throws(Exception::class)
    private fun overlayCountries(baseVC: BaseController) {
        val vectorInfo = WideVectorInfo().apply {
            setColor(Color.RED)
            setLineWidth(4f)
        }
        val assetMgr = activity.assets
        for (path in assetMgr.list("country_json_50m")!!) {
            val json = assetMgr.open("country_json_50m/$path").use { stream ->
                stream.source().use { source ->
                    source.buffer().use { buf ->
                        buf.readUtf8()
                    }
                }
            }
            Log.d("Maply", "Loading $path")
            VectorObject.createFromGeoJSON(json)?.apply {
                selectable = true
            }?.let { vec ->
                vectors.add(vec)
                baseVC.addWideVector(vec, vectorInfo, ThreadMode.ThreadAny)?.let { co ->
                    compObjs.add(co)
                    onVectorLoaded?.invoke(vec,co)
                }
            }
        }
        
        // Build a really big vector for testing
//		VectorObject bigVecObj = new VectorObject();
//		Point2d pts[] = new Point2d[20000];
//		for (int ii=0;ii<20000;ii++)
//			pts[ii] = new Point2d(Math.random(), Math.random());
//		bigVecObj.addAreal(pts);
        
        // Then change to white
        val newVectorInfo = WideVectorInfo().apply {
            setColor(Color.WHITE)
            setLineWidth(4f)
        }
        // todo: doesn't work yet
        for (compObj in compObjs) {
            baseVC.changeWideVector(compObj, newVectorInfo, ThreadMode.ThreadAny)
        }
    }

    var onVectorsLoaded: ((Collection<VectorObject>)->Unit)? = null
    var onVectorLoaded: ((VectorObject,ComponentObject)->Unit)? = null
    
    @Throws(Exception::class)
    override fun setUpWithMap(mapVC: MapController): Boolean {
        baseCase.setUpWithMap(mapVC)
        baseCase.setForwardMapDelegate(this)
        mapVC.addPostSurfaceRunnable {
            Handler(activity.mainLooper).postDelayed({
                overlayCountries(mapVC)
                onVectorsLoaded?.invoke(vectors)
            }, 500)
        }
        return true
    }
    
    @Throws(Exception::class)
    override fun setUpWithGlobe(globeVC: GlobeController): Boolean {
        baseCase.setUpWithGlobe(globeVC)
        baseCase.setForwardGlobeDelegate(this)
        globeVC.addPostSurfaceRunnable {
            Handler(activity.mainLooper).postDelayed({
                overlayCountries(globeVC)
                onVectorsLoaded?.invoke(vectors)
            }, 500)
        }
        return true
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
                val name = it.attributes.getString("NAME_FORMA") ?:
                    it.attributes.getString("NAME") ?:
                    it.attributes.getString("ADMIN") ?:
                    it.attributes.getString("title") ?: "?"
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