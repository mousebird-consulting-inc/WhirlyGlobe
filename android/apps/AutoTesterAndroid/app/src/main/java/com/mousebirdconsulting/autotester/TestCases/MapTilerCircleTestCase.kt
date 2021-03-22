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
    
    override fun getStyleJson(whichMap: Int) = """
     {  "name":"test","version":8,"layers":[
       { "id":"background","type":"background","paint":{"background-color":"#eee"}},{
         "id":"point-label","type":"symbol","source":"openmaptiles","source-layer":"place",
         "filter":["all",["==","${'$'}type","Point"]],
         "layout":{
            "text-field":"{name:latin}",
            "text-font":["Roboto Regular"],
            "text-max-width":5,
            "text-size":12,
            "text-offset":[1,1]
         },
         "paint":{"text-color":"rgba(76, 125, 173, 1)"}
      },{"id":"point-circle","type":"circle","source":"openmaptiles","source-layer":"place",
         "paint":{
            "circle-color": "rgba(255, 219, 133, 1)",
            "circle-stroke-color": "rgba(0, 0, 0, 1)",
            "circle-stroke-width": 1,
            "circle-stroke-opacity": 0.8
         }
      }],"sources":{"openmaptiles":{"type":"vector","url":"https://api.maptiler.com/tiles/v3/tiles.json?key=MapTilerKey"}}}
    """.trimIndent()
}