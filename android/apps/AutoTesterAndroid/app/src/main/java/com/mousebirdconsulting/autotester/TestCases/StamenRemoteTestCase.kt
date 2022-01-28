/*  StamenRemoteTestCase.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/22/19.
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
import android.graphics.Color
import android.os.Handler
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.ConfigOptions.TestType
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import java.io.File

class StamenRemoteTestCase(activity: Activity?) :
        MaplyTestCase(activity, "Stamen Watercolor Remote", TestExecutionImplementation.Both) {

    var doColorChange = true

    private fun setupImageLoader(testType: TestType, baseController: BaseController): QuadImageLoader {
        val cacheDirName = "stamen_watercolor6"
        val cacheDir = File(activity.cacheDir, cacheDirName)
        cacheDir.mkdirs()
        
        val url = "http://tile.stamen.com/watercolor/{z}/{x}/{y}.png"
        val tileInfo = RemoteTileInfoNew(url, 0, 18)
        tileInfo.cacheDir = cacheDir
        
        val params = SamplingParams()
        params.coordSystem = SphericalMercatorCoordSystem()
        params.coverPoles = true
        params.edgeMatching = true
        params.minZoom = tileInfo.minZoom
        params.maxZoom = tileInfo.maxZoom
        params.singleLevel = true
        
        val loader = QuadImageLoader(params, tileInfo, baseController)
        
        // Store image tiles as RGB 5/6/5 when creating textures
        loader.setImageFormat(RenderController.ImageFormat.MaplyImageUShort565)
        
        if (doColorChange) {
            // Change the color and then change it back
            Handler().postDelayed({
                loader.setColor(Color.RED)
                Handler().postDelayed({ loader.setColor(Color.WHITE) }, 4000)
            }, 4000)
        }

        return loader
    }
    
    override fun setUpWithGlobe(globeVC: GlobeController): Boolean {
        imageLoader = setupImageLoader(TestType.GlobeTest, globeVC)
        globeVC.keepNorthUp = true
        globeVC.setClearColor(Color.argb(32,32,0,0));
        globeVC.animatePositionGeo(-3.6704803, 40.5023056, 1.0, 1.0)
        return true
    }
    
    override fun setUpWithMap(mapVC: MapController): Boolean {
        imageLoader = setupImageLoader(TestType.MapTest, mapVC)
        mapVC.setAllowRotateGesture(false)
        mapVC.setClearColor(Color.argb(32,32,0,0));
        mapVC.animatePositionGeo(-3.6704803, 40.5023056, 2.0, 1.0)
        return true
    }
    
    override fun shutdown() {
        imageLoader?.let {
            it.shutdown()
        }
        imageLoader = null

        super.shutdown()
    }
    
    private var imageLoader: QuadImageLoader? = null
    
    init {
        setDelay(4)
    }
}