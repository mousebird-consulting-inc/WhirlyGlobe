package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.os.Handler
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.ConfigOptions
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import java.io.File
import java.util.*
import kotlin.concurrent.timer

/**
 * Find Height just tests the findHeight logic for the globe and map.
 */
class FindHeightTestCase : MaplyTestCase {

    constructor(activity: Activity) : super(activity)
    {
        setTestName("Find Height")
        implementation = TestExecutionImplementation.Both
    }

    // Run a findHeight
    fun runDelayedFind(vc: BaseController) {
        Handler().postDelayed(Runnable {
            // TODO: Fill this in
        }, 10000)
    }

    var baseCase : MaplyTestCase? = null

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        baseCase = CartoLightTestCase(getActivity())
        baseCase?.setUpWithMap(mapVC)

        val loc = Point2d.FromDegrees(-98.58, 39.83)
        mapVC?.setPositionGeo(loc.x,loc.y,1.0)

        runDelayedFind(mapVC!!)

        return true
    }

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        baseCase = CartoLightTestCase(getActivity())
        baseCase?.setUpWithGlobe(globeVC)

        val loc = Point2d.FromDegrees(-98.58, 39.83)
        globeVC?.setPositionGeo(loc.x,loc.y,1.5)

        runDelayedFind(globeVC!!)

        return true
    }
}