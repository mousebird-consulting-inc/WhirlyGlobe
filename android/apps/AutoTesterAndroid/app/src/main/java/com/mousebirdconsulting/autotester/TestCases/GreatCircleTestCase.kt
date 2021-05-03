package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.graphics.Color
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase

class GreatCircleTestCase(activity: Activity) :
        MaplyTestCase(activity, "Great Circles", TestExecutionImplementation.Both) {
    
    fun buildGreatCircle(control: BaseController, a: Point2d, b: Point2d, color: Int,
                         width: Float = 12.0f, precise: Boolean = false, epsilon: Double = 0.001,
                         priority: Int = 1000) : ComponentObject {
        var vecObj = VectorObject()
        vecObj.addLinear(arrayOf(a,b))

        if (epsilon > 0) {
            if (control is GlobeController) {
                if (precise) {
                    vecObj.subdivideToGlobeGreatCirclePrecise(epsilon)
                } else {
                    vecObj.subdivideToGlobeGreatCircle(epsilon)
                }
            } else {
                if (precise) {
                    vecObj.subdivideToFlatGreatCircle(epsilon)
                } else {
                    vecObj.subdivideToFlatGreatCirclePrecise(epsilon)
                }
            }?.let { vecObj = it }
        }

        val vecInfo = WideVectorInfo().apply {
            setColor(color)
            setLineWidth(width)
            drawPriority = priority
        }
        return control.addWideVector(vecObj,vecInfo,ThreadMode.ThreadAny)
    }

    // Add a series of great circles that test various problems
    private fun addGreatCircles(vc: BaseController) {
        buildGreatCircle(vc,
                Point2d.FromDegrees(2.548, 49.010),
                Point2d.FromDegrees(151.177, -33.946),
                Color.RED, 30.0f, false, 0.005, 1000)
        buildGreatCircle(vc,
                Point2d.FromDegrees(2.548, 49.010),
                Point2d.FromDegrees(151.177, -33.946),
                Color.BLACK, 20.0f, false, 0.0025, 1001)
        buildGreatCircle(vc,
                Point2d.FromDegrees(2.548, 49.010),
                Point2d.FromDegrees(151.177, -33.946),
                Color.MAGENTA, 10.0f, true, 0.001, 1002)
        buildGreatCircle(vc,
                Point2d.FromDegrees(2.548, 49.010),
                Point2d.FromDegrees(151.177, -33.946),
                Color.GRAY, 5.0f, true, 0.01, 1003)
        
        buildGreatCircle(vc,
                Point2d.FromDegrees(150.0, 0.0),
                Point2d.FromDegrees(-150.0, 0.0),
                Color.BLUE)
        buildGreatCircle(vc,
                Point2d.FromDegrees(-176.4624, -44.3040),
                Point2d.FromDegrees(171.2303, 44.3040),
                Color.GREEN)
        buildGreatCircle(vc,
                Point2d.FromDegrees(-179.686999,-24.950296),
                Point2d.FromDegrees(179.950328,-22.180086),
                Color.MAGENTA)
        buildGreatCircle(vc,
                Point2d.FromDegrees(-175.0,-33.092222),
                Point2d.FromDegrees(177.944183,-34.845333),
                Color.DKGRAY)
        buildGreatCircle(vc,
                Point2d.FromDegrees(177.747519,-34.672406),
                Point2d.FromDegrees(-175.0,-31.833547),
                Color.DKGRAY)
        buildGreatCircle(vc,
                Point2d.FromDegrees(-175.0,-31.833547),
                Point2d.FromDegrees(178.394161,-35.357856),
                Color.CYAN)
        buildGreatCircle(vc,
                Point2d.FromDegrees(180.0, -89.9),
                Point2d.FromDegrees(180.0, 89.9),
                Color.CYAN)
    }

    var baseCase : CartoLightTestCase? = null

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        baseCase = CartoLightTestCase(getActivity())

        baseCase?.setUpWithMap(mapVC)

        addGreatCircles(mapVC!!)

        return true
    }

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        baseCase = CartoLightTestCase(getActivity())

        baseCase?.setUpWithGlobe(globeVC)

        addGreatCircles(globeVC!!)

        return true
    }
}