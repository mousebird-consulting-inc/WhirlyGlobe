package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.ConfigOptions
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import okio.Okio
import java.io.IOException

class MapTilerTestCase : MaplyTestCase {
    constructor(activity: Activity) : super(activity) {
        setTestName("MapTiler")
        implementation = TestExecutionImplementation.Both
    }

    // Set up the loader (and all the stuff it needs) for the map tiles
    fun setupLoader(control: BaseController, testType: ConfigOptions.TestType) {
        val assetMgr = getActivity().assets
        val stream = assetMgr.open("maptiler_basic.json")
        val paths = assetMgr.list("country_json_50m")
        try {
            val json = Okio.buffer(Okio.source(stream)).readUtf8()
            val style = MapboxVectorStyleSet(json, null, control)
        }
        catch (e: IOException) {
        }
    }

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        setupLoader(globeVC!!, ConfigOptions.TestType.GlobeTest)

        return true
    }

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        setupLoader(mapVC!!, ConfigOptions.TestType.GlobeTest)

        return true
    }
}