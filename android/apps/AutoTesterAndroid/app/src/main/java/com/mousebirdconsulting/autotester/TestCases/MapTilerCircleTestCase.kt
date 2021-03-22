package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.net.Uri
import android.util.Log
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import okio.Okio
import java.io.IOException

class MapTilerCircleTestCase(activity: Activity) :
        MapTilerTestCase(activity, "MapTiler Circles", TestExecutionImplementation.Both)
{
    override fun getMaps() = listOf<String?>(null)
    
    override fun setup(map: MapboxKindaMap) {
        super.setup(map)
        //map.styleSettings.markerImportance = 200000.0 //Float.MAX_VALUE.toDouble()
        //map.styleSettings.drawPriorityPerLevel = 100
        //map.styleSettings.baseDrawPriority = 10000
    }
    
    private val placeLayer = """
        "source":"openmaptiles","source-layer":"place"
    """.trimIndent()
    private fun rankFilter(rank: Int): String = """
        "filter":["all",["==","${'$'}type","Point"],["==","rank",$rank]]
    """.trimIndent()
    private fun label(s: String): String = """
         "layout":{"text-field":"$s","text-font":["Roboto Regular"],"text-size":12,"text-offset":[1,1]},
         "paint":{"text-color":"rgba(76, 125, 173, 1)"}
    """.trimIndent()
    override fun getStyleJson(whichMap: Int) = """
     { "name":"test","version":8,"layers":[
      {"id":"background","type":"background","paint":{"background-color":"#eee"}},
      {"id":"landcover","type":"fill","source":"openmaptiles","source-layer":"globallandcover",
         "filter": ["all",["==","class","crop"]],"paint": {"fill-color": "#aaa"}},

      {"id":"l1","type":"symbol",$placeLayer,${rankFilter(1)},${label("1:no-rad")}},
      {"id":"p1","type":"circle",$placeLayer,${rankFilter(1)},
         "paint":{"circle-color":"#f00"}
      },

      {"id":"l2","type":"symbol",$placeLayer,${rankFilter(2)},${label("2:no-rad-stroke")}},
      {"id":"p2","type":"circle",$placeLayer,${rankFilter(2)},
         "paint":{ "circle-color":"#ff0","circle-stroke-color":"#f00","circle-stroke-width":2}
      },

      {"id":"l3","type":"symbol",$placeLayer,${rankFilter(3)},${label("3:no-stroke")}},
      {"id":"p3","type":"circle",$placeLayer,${rankFilter(3)},
         "paint":{ "circle-radius":15,"circle-color":"#0a0","circle-stroke-width":0 }
      },

      {"id":"l4","type":"symbol",$placeLayer,${rankFilter(4)},${label("4:wide-stroke")}},
      {"id":"p4","type":"circle",$placeLayer,${rankFilter(4)},
         "paint":{ "circle-radius":20,"circle-color":"#0e0","circle-stroke-color":"#707","circle-stroke-width":10 }
      },
      
      {"id":"l5","type":"symbol",$placeLayer,${rankFilter(5)},${label("5:fill-opacity-1")}},
      {"id":"p5","type":"circle",$placeLayer,${rankFilter(5)},
         "paint":{ "circle-radius":30,"circle-color":"rgba(0,255,0,0.5)" }
      },
      {"id":"l6","type":"symbol",$placeLayer,${rankFilter(6)},${label("6:fill-opacity-2")}},
      {"id":"p6","type":"circle",$placeLayer,${rankFilter(6)},
         "paint":{ "circle-radius":30,"circle-color":"#0f0","circle-opacity":0.5 }
      },
      
      {"id":"l7","type":"symbol",$placeLayer,${rankFilter(7)},${label("7:line-opacity-1")}},
      {"id":"p7","type":"circle",$placeLayer,${rankFilter(7)},
         "paint":{"circle-radius":10,"circle-color":"rgba(0,0,0,0)","circle-opacity":0.0,
                  "circle-stroke-width":20,"circle-stroke-color":"rgba(255,0,0,0.5)"}
      },
      {"id":"l8","type":"symbol",$placeLayer,${rankFilter(8)},${label("8:line-opacity-2")}},
      {"id":"p8","type":"circle",$placeLayer,${rankFilter(8)},
         "paint":{"circle-radius":10,"circle-color":"rgba(0,0,0,0)","circle-opacity":0.0,
                  "circle-stroke-width":20,"circle-stroke-color":"#f00","circle-stroke-opacity":0.5}
      },
      
      {"id":"l11","type":"symbol",$placeLayer,${rankFilter(11)},${label("11:opacity")}},
      {"id":"p11","type":"circle",$placeLayer,${rankFilter(11)},
         "paint":{"circle-radius":20,"circle-color":"#0f0","circle-opacity":0.5,
                  "circle-stroke-width":20,"circle-stroke-color":"#f00","circle-stroke-opacity":0.5}
      }


      ],"sources":{"openmaptiles":{"type":"vector","url":"https://api.maptiler.com/tiles/v3/tiles.json?key=MapTilerKey"}}}
    """.trimIndent()

}