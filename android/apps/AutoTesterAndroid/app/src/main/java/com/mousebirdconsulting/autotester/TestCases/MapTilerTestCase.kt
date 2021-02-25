package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.net.Uri
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

    var map: MapboxKindaMap? = null

    // Set up the loader (and all the stuff it needs) for the map tiles
    private fun setupLoader(control: BaseController, whichMap: Int) {
        // Third map is no map
        if (whichMap > 1)
            return;

        val mapName = if (whichMap == 0) "maptiler_basic.json" else "maptiler_streets.json"

        val assetMgr = getActivity().assets
        val stream = assetMgr.open(mapName)
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

    // Switch maps on long press
    override fun userDidLongPress(mapController: MapController?, selObjs: Array<SelectedObject?>?, loc: Point2d?, screenLoc: Point2d?) {
        map?.stop()

        currentMap = currentMap + 1
        if (currentMap > 2)
            currentMap = 0
        setupLoader(baseViewC!!, currentMap)
    }

    var currentMap = 0
    var baseViewC : BaseController? = null

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        baseViewC = globeVC
        setupLoader(baseViewC!!, currentMap)

        return true
    }

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        baseViewC = mapVC
        setupLoader(baseViewC!!, currentMap)

        return true
    }

}