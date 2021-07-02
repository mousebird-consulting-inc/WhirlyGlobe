package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.graphics.Color
import android.widget.Toast
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import okio.buffer
import okio.source
import java.util.*

class VectorsTestCase(activity: Activity?) :
        MaplyTestCase(activity, "Vectors Test", TestExecutionImplementation.Both) {

    val compObjs = ArrayList<ComponentObject>()

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
            VectorObject.createFromGeoJSON(json)?.apply {
                selectable = true
            }?.let {
                vectors.add(it)
                baseVC.addWideVector(it, vectorInfo, ThreadMode.ThreadAny)?.let {
                    compObjs.add(it)
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
    
    @Throws(Exception::class)
    override fun setUpWithMap(mapVC: MapController): Boolean {
        baseCase.setUpWithMap(mapVC)
        baseCase.setForwardMapDelegate(this)
        mapVC.addPostSurfaceRunnable {
            overlayCountries(mapVC)
        }
        return true
    }
    
    @Throws(Exception::class)
    override fun setUpWithGlobe(globeVC: GlobeController): Boolean {
        baseCase.setUpWithGlobe(globeVC)
        baseCase.setForwardGlobeDelegate(this)
        globeVC.addPostSurfaceRunnable {
            overlayCountries(globeVC)
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

    val vectors = ArrayList<VectorObject>()
}