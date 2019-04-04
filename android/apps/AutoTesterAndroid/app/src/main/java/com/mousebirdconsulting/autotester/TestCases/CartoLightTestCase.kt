package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.ConfigOptions
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import java.io.File

/**
 * The Carto light basemap used in a bunch of test cases since it's not too
 * intrusive.
 */
class CartoLightTestCase : MaplyTestCase {

    constructor(activity: Activity) : super(activity)
    {
        setTestName("Carto Light")
        implementation = TestExecutionImplementation.Both
    }

    var loader : QuadImageLoader? = null

    fun setupImageLoader(control: BaseController,testType: ConfigOptions.TestType) {
        val cacheDirName = "carto_light"
        val cacheDir = File(getActivity().cacheDir, cacheDirName)
        cacheDir.mkdir()

        // Where we're getting the tile from
        val tileInfo = RemoteTileInfoNew("http://basemaps.cartocdn.com/rastertiles/voyager/{z}/{x}/{y}@2x.png",
                0, 16);
        tileInfo.cacheDir = cacheDir

        // Sampling params define how the globe is broken up, including the depth
        var params = SamplingParams()
        params.coordSystem = SphericalMercatorCoordSystem()
        if (testType == ConfigOptions.TestType.GlobeTest) {
            params.coverPoles = true
            params.edgeMatching = true
        }
        params.singleLevel = true
        params.minZoom = tileInfo.minZoom
        params.maxZoom = tileInfo.maxZoom

        loader = QuadImageLoader(params,tileInfo,control)
    }

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        setupImageLoader(mapVC!!, ConfigOptions.TestType.MapTest)

        return true
    }

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        setupImageLoader(globeVC!!, ConfigOptions.TestType.GlobeTest)

        return true
    }
}