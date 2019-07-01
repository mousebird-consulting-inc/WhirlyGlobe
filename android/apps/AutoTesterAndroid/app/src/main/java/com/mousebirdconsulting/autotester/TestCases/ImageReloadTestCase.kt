/*
*  ImageReloadTestCase.kt
*  WhirlyGlobeLib
*
*  Created by sjg
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
import android.os.Handler
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.ConfigOptions
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase

/**
 * This tests loading one image layer and then switching to another.
 */
class ImageReloadTestCase : MaplyTestCase {

    constructor(activity: Activity) : super(activity)
    {
        setTestName("Image Reload")
        implementation = TestExecutionImplementation.Both
    }

    var loader : QuadImageLoader? = null
    val maxZoom = 16
    var handler : Handler? = null

    fun setupImageLoader(control: BaseController,testType: ConfigOptions.TestType) {
        // Where we're getting the tile from
        val tileInfo = RemoteTileInfoNew("http://tile.stamen.com/watercolor/{z}/{x}/{y}.png",
                0, maxZoom)

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
        val theLoader = loader

        handler = Handler(getActivity().mainLooper)
        handler?.postDelayed(Runnable {
            val newTileInfo = RemoteTileInfoNew("http://basemaps.cartocdn.com/rastertiles/voyager/{z}/{x}/{y}@2x.png", 0, maxZoom)
            theLoader?.changeTileInfo(newTileInfo)
        }, 10*1000)

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