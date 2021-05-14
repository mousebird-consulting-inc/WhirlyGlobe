package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.net.Uri
import android.util.Log
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import okio.*
import java.io.IOException
import kotlin.math.min

open class MapTilerTestCase : MaplyTestCase
{
    constructor(activity: Activity) :
            this(activity, "MapTiler", TestExecutionImplementation.Both) {
    }
    
    protected constructor(activity: Activity, name: String, impl: TestExecutionImplementation = TestExecutionImplementation.Both) :
            super(activity, name, impl) {
    }
    
    // Maptiler token
    // Go to maptiler.com, setup an account and get your own
    private val token = "GetYerOwnToken"
    
    // Set up the loader (and all the stuff it needs) for the map tiles
    private fun setupLoader(control: BaseController, whichMap: Int) {
        getStyleJson(whichMap)?.let { json ->
            loader?.shutdown()
            loader = null
            map?.stop()
            map = null
            
            map = MapboxKindaMap(json, control).apply {
                mapboxURLFor = { Uri.parse(it.toString().replace("MapTilerKey", token)) }
                backgroundAllPolys = (control is GlobeController)
                imageVectorHybrid = true
                setup(this)
                start()
            }
            map?.postSetup = {
                // Set up an overlay with the same parameters showing the
                // tile boundaries, for debugging/troubleshooting purposes
                loader = QuadPagingLoader(it.sampleParams, OvlDebugImageLoaderInterpreter(), control)
            }
        }
    }

    protected open fun setup(map: MapboxKindaMap) {
        map.styleSettings.drawPriorityPerLevel = 100
    }

    protected open fun getStyleJson(whichMap: Int): String? =
        getMaps().elementAt(whichMap)?.let {
            Log.i(javaClass.name, "Loading $it")
            try {
                activity.assets.open(it).use { stream ->
                    stream.source().use { source ->
                        source.buffer().use { buffer ->
                            buffer.readUtf8()
                        }
                    }
                }
            } catch (e: IOException) {
                Log.w(javaClass.simpleName, "Failed to load style $it", e)
                return null
            }
        } ?: customStyle
    
    // don't switch on long-press, it's too easy to do accidentally when pinching
    override fun userDidLongPress(globeControl: GlobeController?, selObjs: Array<SelectedObject?>?, loc: Point2d?, screenLoc: Point2d?) {
    }
    override fun userDidLongPress(mapController: MapController?, selObjs: Array<SelectedObject?>?, loc: Point2d?, screenLoc: Point2d?) {
    }
    
    // Switch maps on long tap
    override fun userDidTap(globeControl: GlobeController?, loc: Point2d?, screenLoc: Point2d?) {
        super.userDidTap(globeControl, loc, screenLoc)
        switchMaps()
    }
    
    override fun userDidTap(mapControl: MapController?, loc: Point2d?, screenLoc: Point2d?) {
        super.userDidTap(mapControl, loc, screenLoc)
        switchMaps()
    }
    
    private fun switchMaps() {
        currentMap = (currentMap + 1) % getMaps().size
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
            it.animatePositionGeo(Point2d.FromDegrees(-100.0, 40.0), 1.5, 0.0, 0.5)
        }
        return true
    }
    
    protected open fun getMaps(): Collection<String?> = listOf(
        "maptiler_basic.json",
        "maptiler_streets.json",
        "maptiler_topo.json",
        "maptiler_hybrid_satellite.json",
        "maptiler_expr_test.json",
        null        // placeholder for custom stylesheet below
    )

    private var currentMap = 0
    private var map: MapboxKindaMap? = null
    private var baseViewC : BaseController? = null
    private var loader : QuadPagingLoader? = null
    
    // Use this to test out a single or small number of elements alone.
    // This can be helpful when you want to set a breakpoint on something that's normally used by many styles.
    private val customStyle = """
        {  "name":"test","version":8,"layers":[
              {  "id":"background","type":"background","paint":{"background-color":"#aaa"}
              },{"id":"road_motorway","type":"line","source":"openmaptiles",
                 "source-layer":"transportation","filter":["all",["==","class","motorway"]],
                 "paint":{
                   "line-color":{"base":1,"stops":[[5,"hsl(26,87%,62%)"],[16,"#0f0"]]},
                   "line-width":{"base":1.2,"stops":[[5,0],[16,60]]}
                 }
              },
    {
      "id": "road_minor",
      "type": "line",
      "source": "openmaptiles",
      "source-layer": "transportation",
      "filter": [
        "all",
        [
          "==",
          "${"$"}type",
          "LineString"
        ],
        [
          "in",
          "class",
          "minor",
          "service"
        ]
      ],
      "layout": {
        "line-cap": "butt",
        "line-join": "round"
      },
      "paint": {
        "line-color": "rgba(247, 247, 247, 0.12)",
        "Xline-color": "rgba(247, 0, 0, 1.0)",
        "line-width": {
          "base": 1.55,
          "stops": [
            [
              4,
              0.25
            ],
            [
              20,
              10
            ]
          ]
        }
      }
    }
           ],"sources":{"openmaptiles":{"type":"vector",
                 "url":"https://api.maptiler.com/tiles/v3/tiles.json?key=MapTilerKey"}}}
    """.trimIndent()
}