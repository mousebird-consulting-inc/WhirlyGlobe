/*
 *  PagingLayerTestCase.kt
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
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase

class PagingLayerTestCase(activity: Activity) : MaplyTestCase(activity) {

    init {
        setTestName("Paging Layer")
        this.implementation = TestExecutionImplementation.Both
    }

    var interp : OvlDebugImageLoaderInterpreter? = null
    var loader : QuadPagingLoader? = null

    fun setupLayer(vc: BaseController) {
        // Describes how to break down the space
        val params = SamplingParams()
        params.minZoom = 4
        params.maxZoom = 22
        params.minImportance = 256.0*256.0
        params.singleLevel = true
        params.coordSystem = SphericalMercatorCoordSystem()

        // This will put an outline around a tile and a number in the middle
        interp = OvlDebugImageLoaderInterpreter()

        // The paging loader isn't assuming an image.  More generic.
        loader = QuadPagingLoader(params,interp,vc)
    }

    var baseCase : CartoLightTestCase? = null

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        baseCase = CartoLightTestCase(getActivity())
        baseCase?.setUpWithGlobe(globeVC)

        setupLayer(globeVC!!)

        return true
    }

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        baseCase = CartoLightTestCase(getActivity())
        baseCase?.setUpWithMap(mapVC)

        setupLayer(mapVC!!)

        return true
    }
}