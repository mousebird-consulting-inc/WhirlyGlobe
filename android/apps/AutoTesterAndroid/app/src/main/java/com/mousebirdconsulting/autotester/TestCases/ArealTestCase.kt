package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.graphics.Color
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase

/**
 * Created by sjg on 5/31/16.
 */
class ArealTestCase(activity: Activity?) : MaplyTestCase(activity, "Areal Vector Test", TestExecutionImplementation.Both) {
    @Throws(Exception::class)
    private fun addAreal(baseVC: BaseController) {
        val vectorInfo = VectorInfo().apply {
            setColor(Color.RED)
            setLineWidth(4f)
            setFilled(true)
        }

        val outer = arrayOf(
            Point2d.FromDegrees(0.0, 0.0),
            Point2d.FromDegrees(10.0, 0.0),
            Point2d.FromDegrees(10.0, 10.0),
            Point2d.FromDegrees(0.0, 10.0))
        val innerLoops = arrayOf(arrayOf(
            Point2d.FromDegrees(0.1, 0.1),
            Point2d.FromDegrees(0.1, 0.2),
            Point2d.FromDegrees(0.2, 0.2),
            Point2d.FromDegrees(0.2, 0.1)))
        val vecObj = VectorObject().apply {
            addAreal(outer, innerLoops)
        }
        baseVC.addVector(vecObj, vectorInfo, ThreadMode.ThreadAny)
    }
    
    @Throws(Exception::class)
    override fun setUpWithMap(mapVC: MapController): Boolean {
        baseCase.setUpWithMap(mapVC)
        addAreal(mapVC)
        mapVC.setPositionGeo(0.0, 0.0, 0.5)
        return true
    }
    
    @Throws(Exception::class)
    override fun setUpWithGlobe(globeVC: GlobeController): Boolean {
        baseCase.setUpWithGlobe(globeVC)
        addAreal(globeVC)
        globeVC.setPositionGeo(0.0, 0.0, 0.5)
        return true
    }
    
    override fun shutdown() {
        baseCase.shutdown()
        super.shutdown()
    }
    
    private val baseCase = CartoLightTestCase(getActivity())
    
    init {
        setDelay(1000)
    }
}