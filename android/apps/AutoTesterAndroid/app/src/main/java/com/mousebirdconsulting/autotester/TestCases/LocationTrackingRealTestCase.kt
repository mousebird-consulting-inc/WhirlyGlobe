package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import com.mousebird.maply.GlobeController
import com.mousebird.maply.MapController
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase

class LocationTrackingRealTestCase : MaplyTestCase {

    constructor(activity: Activity) : super(activity)
    {
        setTestName("Location Tracking - Real")
        implementation = TestExecutionImplementation.Both
    }

    var baseCase : MaplyTestCase? = null

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        baseCase = CartoLightTestCase(getActivity())
        baseCase?.setUpWithMap(mapVC)

        return true
    }

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        baseCase = CartoLightTestCase(getActivity())
        baseCase?.setUpWithGlobe(globeVC)

        return true
    }
}