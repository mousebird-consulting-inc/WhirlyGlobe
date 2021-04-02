/*  BillboardTestCase.kt
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2021 mousebird consulting
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

class BillboardTestCase(activity: Activity?) :
        MaplyTestCase(activity, "Billboard Test Case", TestExecutionImplementation.Globe) {
    @Throws(Exception::class)
    override fun setUpWithGlobe(globeVC: GlobeController): Boolean {
        val baseView = StamenRemoteTestCase(activity)
        baseView.setUpWithGlobe(globeVC)
        globeVC.addPostSurfaceRunnable {
            adapter = BillboardAdapter(globeVC, activity, ThreadMode.ThreadAny).also {
                it.start()
            }
        }
        return true
    }
    
    override fun shutdown() {
        adapter?.stop()
        adapter = null
        super.shutdown()
    }
    
    var adapter: BillboardAdapter? = null
    
    init {
        setDelay(200)
    }
}