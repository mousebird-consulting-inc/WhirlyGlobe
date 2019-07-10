/*
 *  VectorMBTilesTestCase.kt
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
import android.content.Context
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.ConfigOptions
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import java.io.File
import android.content.Context.MODE_PRIVATE
import android.content.ContextWrapper
import java.io.FileOutputStream
import java.io.IOException


/**
 * Loads vector tiles over southern France.
 */
class VectorMBTilesTestCase : MaplyTestCase {

    constructor(activity: Activity) : super(activity) {
        setTestName("Vector MBTiles")
        implementation = TestExecutionImplementation.Both
    }

    var loader: QuadPagingLoader? = null
    var fetcher: MBTileFetcher? = null
    var interp: MapboxVectorInterpreter? = null
    var debugInterp: OvlDebugImageLoaderInterpreter? = null
    var styleGen: VectorStyleSimpleGenerator? = null

    fun setupLoader(control: BaseController) {
        val mbTileFile = getMbTileFile("mbtiles/France.mbtiles", "France.mbtiles")
        fetcher = MBTileFetcher(mbTileFile)

        // Sampling params define how the globe is broken up, including the depth
        var params = SamplingParams()
        params.coordSystem = SphericalMercatorCoordSystem()
        params.singleLevel = true
        params.minZoom = fetcher!!.minZoom
        params.maxZoom = fetcher!!.maxZoom

        // Simple style generator just picks random colors
        styleGen = VectorStyleSimpleGenerator(control)

        // Mapbox interpreter uses the parser and style to create visual data
        interp = MapboxVectorInterpreter(styleGen, control)

        // Overlay the tile number on top
        debugInterp = OvlDebugImageLoaderInterpreter()
        debugInterp!!.setParentInterpreter(interp)

        loader = QuadPagingLoader(params, fetcher!!.tileInfo, debugInterp, control)
        loader!!.setTileFetcher(fetcher)
    }

    var baseCase: VectorsTestCase? = null

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        baseCase = VectorsTestCase(mapVC!!.getActivity())
        baseCase?.setUpWithMap(mapVC)

        setupLoader(mapVC)

        return true
    }

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        baseCase = VectorsTestCase(globeVC!!.getActivity())
        baseCase?.setUpWithGlobe(globeVC)

        setupLoader(globeVC)

        return true
    }

    private val MBTILES_DIR = "mbtiles"

    // Copy an MBTiles file out of the package for direct use
    @Throws(IOException::class)
    private fun getMbTileFile(assetMbTile: String, mbTileFilename: String): File {

        val wrapper = ContextWrapper(activity)
        val mbTilesDirectory = wrapper.getDir(MBTILES_DIR, Context.MODE_PRIVATE)

        val inStream = activity.assets.open(assetMbTile)
        val of = File(mbTilesDirectory, mbTileFilename)

        if (of.exists()) {
            return of
        }

        val os = FileOutputStream(of)
        val mBuffer = ByteArray(1024)
        var length = inStream.read(mBuffer)
        while (length > 0) {
            os.write(mBuffer, 0, length)
            length = inStream.read(mBuffer)
        }
        os.flush()
        os.close()
        inStream.close()

        return of
    }
}
