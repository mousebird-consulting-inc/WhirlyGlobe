/*  VectorStyleTestCase.kt
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford.
 *  Copyright 2011-2022 mousebird consulting
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
 */
package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import okio.*

/**
 * Loads the USA geojson and applies a simple style.
 */
class VectorStyleTestCase(activity: Activity) :
        MaplyTestCase(activity, "Vector Style Test", TestExecutionImplementation.Both) {

    private fun setupOverlay(control: BaseController) {
        val json = activity.assets.open("country_json_50m/USA.geojson").use { stream ->
            stream.source().use { source ->
                source.buffer().use { buffer ->
                    buffer.readUtf8()
                }
            }
        }

        val vecObj = VectorObject.createFromGeoJSON(json)
        val styleGen = VectorStyleSimpleGenerator(control)
        VectorStyleProcessor.UseStyle(arrayOf(vecObj),styleGen,control)?.let { cos ->
            objs.addAll(cos)
        }
    }

    override fun setUpWithMap(mapVC: MapController): Boolean {
        baseCase.setUpWithMap(mapVC)
        setupOverlay(mapVC)
        return true
    }

    override fun setUpWithGlobe(globeVC: GlobeController): Boolean {
        baseCase.setUpWithGlobe(globeVC)
        setupOverlay(globeVC)
        return true
    }
    
    override fun shutdown() {
        controller?.removeObjects(objs, ThreadMode.ThreadCurrent)
        objs.clear()
        baseCase.shutdown()
        super.shutdown()
    }
    
    private var baseCase = CartoLightTestCase(activity)
    private val objs = ArrayList<ComponentObject>()
}
