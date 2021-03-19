package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.net.Uri
import android.util.Log
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import okio.Okio
import java.io.IOException

class MapTilerTestCase(activity: Activity) :
        MaplyTestCase(activity, "MapTiler", TestExecutionImplementation.Both)
{
    // Maptiler token
    // Go to maptiler.com, setup an account and get your own
    private val token = "GetYerOwnToken"
    
    // Set up the loader (and all the stuff it needs) for the map tiles
    private fun setupLoader(control: BaseController, whichMap: Int) {
        getStyleJson(whichMap)?.let { json ->
            map = MapboxKindaMap(json, control).apply {
                mapboxURLFor = { Uri.parse(it.toString().replace("MapTilerKey", token)) }
                backgroundAllPolys = (control is GlobeController)
                imageVectorHybrid = true
                start()
            }
        }
    }

    private fun getStyleJson(whichMap: Int): String? {
        return when(whichMap) {
            0 -> "maptiler_basic.json"
            1 -> "maptiler_streets.json"
            else -> null
        }?.let {
            Log.i(javaClass.name, "Loading $it")
            try {
                Okio.buffer(Okio.source(getActivity().assets.open(it))).readUtf8()
            } catch (e: IOException) {
                Log.w(javaClass.simpleName, "Failed to load style $it", e)
                return null
            }
        } ?: customStyle
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
        map = null
        currentMap = (currentMap + 1) % 3
        baseViewC?.let { setupLoader(it, currentMap) }
    }
    
    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        baseViewC = globeVC?.also {
            setupLoader(it, currentMap)
            it.animatePositionGeo(Point2d.FromDegrees(-100.0, 40.0), 0.5, 0.0, 0.5)
        }
        return true
    }

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        baseViewC = mapVC?.also {
            setupLoader(it, currentMap)
            it.animatePositionGeo(Point2d.FromDegrees(-100.0, 40.0), 0.5, 0.0, 0.5)
        }
        return true
    }

    private var currentMap = 0
    private var map: MapboxKindaMap? = null
    private var baseViewC : BaseController? = null
    private val customStyle = """
        {  "name":"test","version":8,"layers":[
              {  "id":"background","type":"background",
                 "#comment": "bg is currently dynamic per-level, not per-frame",
                 "paint":{"background-color":{"stops":[[0,"rgba(255,255,255,1)"],[16,"rgba(255,0,0,1)"]]}}
              },{"id":"landcover_cropland","type":"fill","source":"openmaptiles","source-layer":"globallandcover",
                 "filter":["all", ["==", "class", "crop"] ],
                 "paint":{
                   "fill-color":{"base":1,"stops":[[4,"#00f"],[5,"#0f0"]]},
                   "fill-opacity":{"base":1, "stops":[[5,1],[6,0]]}
                 }
              },{"id":"road_motorway","type":"line","source":"openmaptiles","source-layer":"transportation",
                 "filter":["all",["==","class","motorway"]],
                 "paint":{
                   "line-color":{"base":1,"stops":[[5,"hsl(26,87%,62%)"],[16,"#0f0"]]},
                   "line-width":{"base":1.2,"stops":[[5,0],[16,60]]}
                 }
              }
           ],"sources":{"openmaptiles":{"type":"vector",
                 "url":"https://api.maptiler.com/tiles/v3/tiles.json?key=MapTilerKey"}}}
    """.trimIndent()
}