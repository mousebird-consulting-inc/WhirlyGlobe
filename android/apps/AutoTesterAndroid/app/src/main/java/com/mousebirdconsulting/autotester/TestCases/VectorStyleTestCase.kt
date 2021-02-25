/*
 *  VectorStyleTestCase.kt
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
import android.content.ContextWrapper
import android.graphics.Color
import okio.Okio
import java.io.FileOutputStream
import java.io.IOException
import java.nio.charset.Charset
import java.util.*


/**
 * Loads the USA geojon and applies a simple style.
 */
class VectorStyleTestCase : MaplyTestCase {

    constructor(activity: Activity) : super(activity) {
        setTestName("Vector Style Test")
        implementation = TestExecutionImplementation.Both
    }

    fun setupOverlay(control: BaseController) {
        val stream = getActivity().assets.open("country_json_50m/USA.geojson")
        val json = Okio.buffer(Okio.source(stream)).readUtf8()

        val vecObj = VectorObject()
        vecObj.fromGeoJSON(json)

        val styleGen = VectorStyleSimpleGenerator(control)
        VectorStyleProcessor.UseStyle(arrayOf(vecObj),styleGen,control)
    }

    var baseCase: CartoLightTestCase? = null

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        baseCase = CartoLightTestCase(mapVC!!.getActivity())
        baseCase?.setUpWithMap(mapVC)

        setupOverlay(mapVC)

        return true
    }

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        baseCase = CartoLightTestCase(globeVC!!.getActivity())
        baseCase?.setUpWithGlobe(globeVC)

        setupOverlay(globeVC)

        return true
    }

    private val MBTILES_DIR = "mbtiles"

}
