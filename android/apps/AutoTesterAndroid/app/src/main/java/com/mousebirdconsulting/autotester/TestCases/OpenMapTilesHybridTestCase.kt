/*
 *  OpenMapTilesHybridTestCase.kt
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford.
 *  Copyright 2011-2019 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.ConfigOptions
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import java.io.File

class OpenMapTilesHybridTestCase : MaplyTestCase {

    constructor(activity: Activity) : super(activity) {
        setTestName("OpenMapTiles Hybrid")
        implementation = TestExecutionImplementation.Both
    }

    var loader: QuadImageLoader? = null
    var interp: MapboxVectorInterpreter? = null
    var polyStyleGen: VectorStyleSimpleGenerator? = null
    var lineStyleGen: VectorStyleSimpleGenerator? = null
    var tileRenderer: RenderController? = null

    fun setupLoader(control: BaseController, testType: ConfigOptions.TestType) {
        val cacheDirName = "openmaptiles"
        val cacheDir = File(getActivity().cacheDir, cacheDirName)
        cacheDir.mkdir()

        // TODO: Move these
        // Remote OpenMapTiles
        val tileInfo = RemoteTileInfoNew("http://public-mobile-data-stage-saildrone-com.s3-us-west-1.amazonaws.com/openmaptiles/{z}/{x}/{y}.png",
                0, 14)
        tileInfo.cacheDir = cacheDir

        // Sampling params define how the globe is broken up, including the depth
        var params = SamplingParams()
        params.coordSystem = SphericalMercatorCoordSystem()
        params.singleLevel = true
        params.minZoom = tileInfo.minZoom
        params.maxZoom = tileInfo.maxZoom
        if (testType == ConfigOptions.TestType.GlobeTest) {
            params.coverPoles = true
            params.edgeMatching = true
        }

        // Need a standalone renderer
        tileRenderer = RenderController(512,512)

        // Style sheets for image and overlay
        // TODO: Filter out the polygons
        polyStyleGen = VectorStyleSimpleGenerator(control)
        lineStyleGen = VectorStyleSimpleGenerator(control)

        // The interpreter renders some of the data into images and overlays the rest
        interp = MapboxVectorInterpreter(polyStyleGen, tileRenderer, lineStyleGen, control)

        // Finally the loader asks for tiles
        loader = QuadImageLoader(params,tileInfo,control)
        loader?.setLoaderInterpreter(interp)
    }

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        setupLoader(globeVC!!, ConfigOptions.TestType.GlobeTest)

        return true
    }

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        setupLoader(mapVC!!, ConfigOptions.TestType.MapTest)

        return true
    }
}