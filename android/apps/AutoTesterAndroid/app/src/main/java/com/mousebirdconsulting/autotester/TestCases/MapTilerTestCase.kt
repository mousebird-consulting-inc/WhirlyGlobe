package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.graphics.Color
import android.net.Uri
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.ConfigOptions
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import okio.Okio
import java.io.File
import java.io.IOException

class MapTilerTestCase : MaplyTestCase {
    constructor(activity: Activity) : super(activity) {
        setTestName("MapTiler")
        implementation = TestExecutionImplementation.Both
    }

    var map: MapboxKindaMap? = null

    // Set up the loader (and all the stuff it needs) for the map tiles
    fun setupLoader(control: BaseController, testType: ConfigOptions.TestType) {
        val assetMgr = getActivity().assets
        val stream = assetMgr.open("maptiler_basic.json")
        var polyStyle: MapboxVectorStyleSet?

        // Maptiler token
        // Go to maptiler.com, setup an account and get your own
        val token = "GetYerOwnToken"

        try {
            val json = Okio.buffer(Okio.source(stream)).readUtf8()
            map = object: MapboxKindaMap(json,control) {
                override fun mapboxURLFor(file: Uri): Uri {
                    val str = file.toString()
                    return Uri.parse(str.replace("MapTilerKey",token))
                }
            }
            if (map == null)
                return
            map?.backgroundAllPolys = true
            map?.start()
        }
        catch (e: IOException) {
            return
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