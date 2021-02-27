package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.net.Uri
import android.util.Log
import com.mousebird.maply.*
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
    private fun setupLoader(control: BaseController, whichMap: Int, backgroundAll: Boolean) {
        // Third map is no map
        if (whichMap > 1)
            return;

        val mapName = if (whichMap == 0) "maptiler_basic.json" else "maptiler_streets.json"

        Log.i(javaClass.name, String.format("Loading %s bg=%s", mapName, backgroundAll.toString()));
        
        val assetMgr = getActivity().assets
        val stream = assetMgr.open(mapName)
        var polyStyle: MapboxVectorStyleSet?

        // Maptiler token
        // Go to maptiler.com, setup an account and get your own
        val token = "GetYerOwnToken"

        try {
            val json = Okio.buffer(Okio.source(stream)).readUtf8()
            map = MapboxKindaMap(json, control)
            //map?.styleSettings?.setZBufferRead(false)
            //map?.styleSettings?.setZBufferWrite(false)
            map?.mapboxURLFor = { url: Uri ->
                val urlStr = url.toString()
                var newStr = urlStr
                if (urlStr.contains("MapTilerKey"))
                    newStr = urlStr.replace("MapTilerKey", token)
                Uri.parse(newStr)
            }
            if (map == null)
                return
            map?.backgroundAllPolys = backgroundAll
            map?.start()
        }
        catch (e: IOException) {
            return
        }
    }

    // Switch maps on long press
    override fun userDidLongPress(globeControl: GlobeController?, selObjs: Array<SelectedObject?>?, loc: Point2d?, screenLoc: Point2d?) {
        switchMaps()
    }
    override fun userDidLongPress(mapController: MapController?, selObjs: Array<SelectedObject?>?, loc: Point2d?, screenLoc: Point2d?) {
        switchMaps()
    }

    private fun switchMaps() {
        map?.stop()
    
        currentMap = (currentMap + 1) % 3
        if (currentMap == 0) {
            bgAll = !bgAll
        }
        setupLoader(baseViewC!!, currentMap, bgAll)
    }
    
    var currentMap = 0
    var bgAll = true
    var baseViewC : BaseController? = null

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        baseViewC = globeVC
        setupLoader(baseViewC!!, currentMap, bgAll)

        globeVC?.animatePositionGeo(Point2d.FromDegrees(-100.0, 40.0), 0.5, 0.0, 0.5)
        return true
    }

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        baseViewC = mapVC
        setupLoader(baseViewC!!, currentMap, bgAll)

        return true
    }

}