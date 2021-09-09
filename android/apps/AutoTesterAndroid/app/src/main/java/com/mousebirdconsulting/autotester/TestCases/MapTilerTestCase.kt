package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.content.Context.MODE_PRIVATE
import android.net.Uri
import android.util.Log
import android.widget.Toast
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import okio.*
import java.io.IOException

open class MapTilerTestCase : MaplyTestCase
{
    // Maptiler token
    // Go to maptiler.com, setup an account, get your token, and paste it here
    private var token: String = "GetYerOwnToken"
    
    constructor(activity: Activity) :
            this(activity, "MapTiler", TestExecutionImplementation.Both)
    
    protected constructor(activity: Activity, name: String,
                          impl: TestExecutionImplementation = TestExecutionImplementation.Both) :
            super(activity, name, impl)
    
    // Set up the loader (and all the stuff it needs) for the map tiles
    private fun setupLoader(control: BaseController, whichMap: Int) {
        val prefs = activity.getSharedPreferences("com.mousebird.autotester.prefs", MODE_PRIVATE)
        if (token.isEmpty() || token == "GetYerOwnToken") {
            token = prefs.getString("MapTilerToken", null) ?:
                    System.getenv("MAPTILER_TOKEN") ?: "GetYerOwnToken"
        }
        if (token.isEmpty() || token == "GetYerOwnToken") {
            Toast.makeText(activity.applicationContext, "Missing MapTiler Token", Toast.LENGTH_LONG).show()
        } else {
            prefs.edit().putString("MapTilerToken",token).apply()
        }

        control.layoutFadeEnabled = true
        control.setPerfInterval(60)

        getStyleJson(whichMap)?.let { (json,img) ->
            loader?.shutdown()
            loader = null
            map?.stop()
            map = null
            
            MapboxKindaMap(json, control).apply {
                map = this
                mapboxURLFor = { Uri.parse(it.toString().replace("MapTilerKey", token)) }
                backgroundAllPolys = !img && (control is GlobeController)
                imageVectorHybrid = true
                postSetup = {
                    // Set up an overlay with the same parameters showing the
                    // tile boundaries, for debugging/troubleshooting purposes
                    this@MapTilerTestCase.loader =
                            QuadPagingLoader(it.sampleParams, OvlDebugImageLoaderInterpreter(), control)
                }
                setup(this)
                start()
            }
        }
    }

    protected open fun setup(map: MapboxKindaMap) {
        map.styleSettings.drawPriorityPerLevel = 100
    }

    protected open fun getStyleJson(whichMap: Int): Pair<String,Boolean>? =
        getMaps().elementAt(whichMap)?.let {
            Toast.makeText(activity.applicationContext, "Loading ${it.first}", Toast.LENGTH_SHORT).show()
            try {
                activity.assets.open(it.first).use { stream ->
                    stream.source().use { source ->
                        source.buffer().use { buffer ->
                            Pair(buffer.readUtf8(), it.second)
                        }
                    }
                }
            } catch (e: IOException) {
                Log.w(javaClass.simpleName, "Failed to load style $it", e)
                return null
            }
        } ?: run {
            Toast.makeText(activity.applicationContext, "Loading custom stylesheet", Toast.LENGTH_SHORT).show()
            Pair(customStyle,false)
        }
    
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
    
    override fun shutdown() {
        loader?.shutdown()
        loader = null
        map?.stop()
        map = null
        super.shutdown()
    }

    protected open fun getMaps(): Collection<Pair<String,Boolean>?> = listOf(
        Pair("maptiler_basic.json", false),
        Pair("maptiler_streets.json", false),
        Pair("maptiler_topo.json", false),
        Pair("maptiler_hybrid_satellite.json", true),
        Pair("maptiler_expr_test.json", false),
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