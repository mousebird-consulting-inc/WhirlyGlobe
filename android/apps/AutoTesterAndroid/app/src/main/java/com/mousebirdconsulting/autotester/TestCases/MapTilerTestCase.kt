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
    
    // Maptiler token
    // Go to maptiler.com, setup an account and get your own
    private val token = "GetYerOwnToken"
    
    // Set up the loader (and all the stuff it needs) for the map tiles
    private fun setupLoader(control: BaseController, whichMap: Int, backgroundAll: Boolean) {
        // Third map is no map
        if (whichMap > 1)
            return;

        val mapName = if (whichMap == 0) "maptiler_basic.json" else "maptiler_streets.json"

        Log.i(javaClass.name, String.format("Loading %s bg=%s", mapName, backgroundAll.toString()));
        
        val assetMgr = getActivity().assets
        val stream = assetMgr.open(mapName)

        try {
            val json = Okio.buffer(Okio.source(stream)).readUtf8()
            map = MapboxKindaMap(json, control).apply {
                mapboxURLFor = this@MapTilerTestCase::urlMap
                // backgroundAll is only needed for globes
                backgroundAllPolys = (control is GlobeController)
                imageVectorHybrid = true
                start()
            }
        }
        catch (e: IOException) {
            return
        }
    }

    private fun urlMap(url: Uri): Uri {
        val urlStr = url.toString()
        var newStr = urlStr
        if (urlStr.contains("MapTilerKey"))
            newStr = urlStr.replace("MapTilerKey", token)
        return Uri.parse(newStr)
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
    
        currentMap = (currentMap + 1) % 2
        if (currentMap == 0) {
            bgAll = !bgAll
        }
        setupLoader(baseViewC!!, currentMap, bgAll)
    }
    
    var currentMap = 0
    var bgAll = false
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
    
    private var map: MapboxKindaMap? = null
}