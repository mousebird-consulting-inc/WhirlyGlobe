package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.graphics.Color
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import okio.Okio
import java.nio.charset.Charset

class LoftedPolyTestCase : MaplyTestCase {

    constructor(activity: Activity) : super(activity) {
        setTestName("Lofted Polys")
        implementation = TestExecutionImplementation.Both
    }

    fun addLoftedPolysSpain(vc: BaseController) {
        val stream = getActivity().assets.open("country_json_50m/ESP.geojson")
        val json = Okio.buffer(Okio.source(stream)).readUtf8()

        val vecObj = VectorObject()
        vecObj.fromGeoJSON(json)

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