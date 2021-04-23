package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.graphics.Color
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import okio.*
import java.nio.charset.Charset

class LoftedPolyTestCase : MaplyTestCase {

    constructor(activity: Activity) : super(activity) {
        setTestName("Lofted Polys")
        implementation = TestExecutionImplementation.Both
    }

    fun addLoftedPolysSpain(vc: BaseController) {
        val json = activity.assets.open("country_json_50m/ESP.geojson").use { stream ->
            stream.source().use { source ->
                source.buffer().use { buffer ->
                    buffer.readUtf8()
                }
            }
        }

        val vecObj = VectorObject.createFromGeoJSON(json)

        val loftInfo = LoftedPolyInfo()
        loftInfo.setHeight(0.01)
        loftInfo.setColor(Color.argb(128, 255, 0, 0))
        vc.addLoftedPoly(vecObj,loftInfo,RenderControllerInterface.ThreadMode.ThreadAny)
    }

    var baseCase : CartoLightTestCase? = null

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        baseCase = CartoLightTestCase(getActivity())
        baseCase?.setUpWithGlobe(globeVC)

        addLoftedPolysSpain(globeVC!!)

        return true
    }

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        baseCase = CartoLightTestCase(getActivity())
        baseCase?.setUpWithMap(mapVC)

        addLoftedPolysSpain(mapVC!!)

        return true
    }
}