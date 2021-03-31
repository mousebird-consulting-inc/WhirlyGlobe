/* SimpleStyleTestCase.kt
 * AutoTesterAndroid.app
 *
 * Created by Tim Sylvester on 12/02/2021
 * Copyright © 2021 mousebird consulting, inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations under the License.
 */

package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.util.Log
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase

class SimpleStyleTestCase(activity: Activity) : MaplyTestCase(activity, "Simple Vector Styles") {
    
    override fun setUpWithGlobe(globeVC: GlobeController): Boolean {
        baseCase.setUpWithGlobe(globeVC)
        runExamples(globeVC)
        globeVC.animatePositionGeo(Point2d.FromDegrees(145.0, -33.0), 0.2, 0.0, 0.5)
        return true
    }
    
    override fun setUpWithMap(mapVC: MapController): Boolean {
        baseCase.setUpWithMap(mapVC)
        runExamples(mapVC)
        mapVC.animatePositionGeo(Point2d.FromDegrees(145.0, -33.0), 0.2, 0.0, 0.5)
        return true
    }

    override fun shutdown() {
        cleanup()
        super.shutdown()
    }
    
    private fun cleanup() {
        componentObjects?.let {
            controller?.removeObjects(it, threadCurrent)
            componentObjects = null
        }
        styleManager?.shutdown()
        styleManager = null
    }
    
    private fun runExamples(vc: BaseController) {
        cleanup()
        styleManager = SimpleStyleManager(activity.applicationContext, vc).apply {
            medSize = Point2d(42.0, 36.0)
            largeSize = Point2d(64.0, 80.0)
            objectLocator = object : SimpleStyleManager.StyleObjectLocator {
                override fun locate(name: String): Collection<String> {
                    return listOf("$name.png", "maki icons/$name-24@2x.png")
                }
            }
            onAddMarker = { obj, marker, info, style ->
                marker.userObject = obj
                if (style.labelOffset != null) {
                    info.setLayoutImportance(Float.MAX_VALUE)
                }
                true
            }
        }.also { styleMan ->
            componentObjects = arrayOf(vectorGeoJson1, vectorGeoJson2).flatMap { json ->
                VectorObject().let { obj ->
                    if (obj.fromGeoJSON(json)) {
                        styleMan.addFeatures(obj, threadAny)
                    } else {
                        Log.e(javaClass.name, "Failed to parse JSON")
                        sequenceOf()
                    }
                }.toList()
            }
        }
    }
    
    private fun prop(name: String, value: String?, quote: Boolean): String? {
        return if (value == null) null else ("\"$name\": " + (if (quote) "\"$value\"" else value))
    }
    
    private fun marker(title: String, lat: Double, lon: Double, m: String? = null, bg: String? = null,
                       c: Boolean? = null, mC: String? = null, mA: Double? = null, fC: String? = null,
                       fA: Double? = null, s: Double? = null, sC: String? = null, sA: Double? = null,
                       mSz: String? = null, ox: Double? = null, oy: Double? = null, lC: String? = null,
                       lSz: Double? = null, lx: Double? = null, ly: Double? = null): String {
        return """
        {
          "type": "Feature",
          "properties": {
        """ +
            arrayOf(prop("title", title, true),
                    prop("marker-size", mSz ?: "large", true),
                    prop("marker-color", mC, true),
                    prop("marker-opacity", "$mA", false),
                    prop("marker-symbol", m, true),
                    prop("marker-background-symbol", bg, true),
                    prop("marker-circle", "${!(c ?: true)}", false),
                    prop("marker-offset-x", if (ox != null) "$ox" else null, false),
                    prop("marker-offset-y", if (oy != null) "$oy" else null, false),
                    prop("fill", fC, true),
                    prop("fill-opacity", if (fA != null) "$fA" else null, false),
                    prop("stroke-width", if (s != null) "$s" else null, false),
                    prop("stroke", sC, true),
                    prop("stroke-opacity", if (sA != null) "$sA" else null, false),
                    prop("label", lC, true),
                    prop("label-size", if (lSz != null) "$lSz" else null, false),
                    prop("label-offset-x", if (lx != null) "$lx" else null, false),
                    prop("label-offset-y", if (ly != null) "$ly" else null, false)
            ).filterNotNull().joinToString(",") +
        """
          },
          "geometry": { "type": "Point", "coordinates": [ $lon, $lat ] }
        }
        """
    }
    
    private fun markers1(): String {
        val startLon = 142.0
        var lat = -30.0
        var lon = startLon
        val latStep = 0.05
        val lonStep = 0.05
        var n = 0
        val rowSize = 64
        return arrayOf(null, "bar").flatMap { m ->
               arrayOf(null, "marker-stroked").flatMap { bg ->
               arrayOf("small", "medium", "large").flatMap { mSz ->
               arrayOf(0.0, 2.0, 5.0).flatMap { sWidth ->
               arrayOf(0.0, 0.5, 1.0).flatMap { fillA ->
               arrayOf(true, false).flatMap { circle ->
               arrayOf("f0f", "#0f0").flatMap { mColor ->
               arrayOf("0fa", "#a0f").flatMap { fColor ->
               arrayOf("fa0", "#0af").map { sColor ->
                    lon += lonStep
                    if ((n++ % rowSize) == 0) { lat -= latStep; lon = startLon }
                    marker("", lat, lon, m, bg, circle, mColor, null, fColor,
                           fillA, sWidth, sColor, 0.8, mSz)
               } } } } } } } }
            }.joinToString(",")
    }

    private fun fmt(v: Double): String {
        return Regex("\\.0+$").replace("$v", "")
    }
    private fun markers2(): String {
        return arrayOf(-4.0, -1.5, 0.0, 1.0).flatMap { ox ->
               arrayOf(-5.0, -1.5, 0.0, 1.0).map { oy ->
                    marker("${fmt(ox)},${fmt(oy)}", -30.0, 140.0,null, "marker-stroked",
                            false, "f0f", null, "0fa", 0.7, 0.2, "#0af", 0.8,
                            "medium", ox, oy, "#f50", 5.0, ox * 45 / 5, oy * 38 / 5)
               }
            }.joinToString(",")
    }

    private fun markers3(): String {
        return arrayOf(
                marker("",-33.0,140.0,"square","wide",false,"#f00",0.7,"#00f",null,
                        null,null,null,"large",null,null,null,null,null,null),
                marker("",-33.1,140.0,"square","tall",false,"#00f",0.7,"#00f",null,
                        null,null,null,"large",null,null,null,null,null,null),
                marker("",-33.0,140.1,"wide","square",false,"#0f0",0.7,"#00f",null,
                        null,null,null,"large",null,null,null,null,null,null),
                marker("",-33.1,140.1,"tall","square",false,"#f0f",0.7,"#00f",null,
                        null,null,null,"large",null,null,null,null,null,null)
        ).joinToString(",")
    }
    
    private val vectorGeoJson1 = """
    { "type": "FeatureCollection",
      "features": [{
          "type": "Feature",
          "properties": {
            "title": "line",
            "stroke": "#0000ff",
            "stroke-width": 10.0
          }, "geometry": {
            "type": "LineString",
            "coordinates": [
              [ 140.0677490234375,  -31.97871614600332  ],
              [ 130.55389404296875, -31.764181375100804 ]
      ] } } ]
    }"""

    private val vectorGeoJson2 = """
    { "type": "FeatureCollection",
      "features": [
        ${markers1()},
        ${markers2()},
        ${markers3()},
        { "type": "Feature",
          "properties": {
            "title": "poly",
            "fill": "#ff0000",
            "stroke": "#ffffff"
          }, "geometry": {
            "type": "Polygon",
            "coordinates": [ [
                [ 150.40283203125, -33.504759069226075 ],
                [ 151.10595703125, -33.504759069226075 ],
                [ 151.10595703125, -33.16974360021616  ],
                [ 150.40283203125, -33.16974360021616  ],
                [ 150.40283203125, -33.504759069226075 ]
          ] ] } }, {
          "type": "Feature",
          "properties": {
            "title": "line",
            "stroke": "#0000ff",
            "stroke-width": 10.0
          }, "geometry": {
            "type": "LineString",
            "coordinates": [
              [ 150.0677490234375,  -32.97871614600332  ],
              [ 150.14190673828125, -32.97641208290518  ],
              [ 150.5731201171875,  -32.95797741405951  ],
              [ 150.98236083984375, -33.01096671579776  ],
              [ 150.75164794921875, -33.08463802391686  ],
              [ 149.91943359375,    -33.27084277265288  ],
              [ 149.77935791015625, -32.9049563191375   ],
              [ 150.24078369140625, -32.669436832605314 ],
              [ 150.55389404296875, -32.764181375100804 ]
      ] } } ]
    }"""

    private val baseCase: MaplyTestCase
    
    private var styleManager: SimpleStyleManager? = null
    private var componentObjects: List<ComponentObject>? = null
    
    private val threadAny = ThreadMode.ThreadAny
    private val threadCurrent = ThreadMode.ThreadCurrent
    
    init {
        baseCase = StamenRemoteTestCase(activity)
    }
}
