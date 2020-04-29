package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import com.mousebird.maply.BaseController
import com.mousebird.maply.GlobeController
import com.mousebirdconsulting.autotester.ConfigOptions
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase

class MapTilerTestCase : MaplyTestCase {
    constructor(activity: Activity) : super(activity) {
        setTestName("MapTiler")
        implementation = TestExecutionImplementation.Both
    }

    // Set up the loader (and all the stuff it needs) for the map tiles
    fun setupLoader(control: BaseController, testType: ConfigOptions.TestType) {

    }

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        setupLoader(globeVC!!, ConfigOptions.TestType.GlobeTest)
    }
}