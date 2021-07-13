/*  CustomBNGCoordAdapter.kt
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/13/16.
 *  Copyright 2016-2021 mousebird consulting
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
import com.mousebird.maply.MapController

class CustomBNGCoordAdapter(activity: Activity?) :
        CustomBNGTileSource(activity, "British National Grid Custom Map", TestExecutionImplementation.Map) {

    override fun makeMapController(): MapController {
        val bngCoordSys = makeBNGCoordSystem(activity, true)
        val settings = MapController.Settings().apply {
            coordSys = bngCoordSys
        }
        return MapController(activity, settings).also {
            it.gestureDelegate = this
        }
    }
}