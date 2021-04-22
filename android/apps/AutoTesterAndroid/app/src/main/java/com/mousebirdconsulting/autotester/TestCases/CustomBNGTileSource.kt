/*  CustomBNGTileSource.kt
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
import com.mousebird.maply.*
import com.mousebird.maply.TestTileFetcher.TestTileInfo
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import kotlin.math.log10

open class CustomBNGTileSource : MaplyTestCase {
    constructor(activity: Activity?) :
            super(activity, "British National Grid Tile Source") {
        setDelay(2)
    }
    
    protected constructor(activity: Activity?, testName: String?, impl: TestExecutionImplementation?) :
            super(activity, testName, impl)
    
    @Throws(Exception::class)
    override fun setUpWithGlobe(globeVC: GlobeController): Boolean {
        baseCase = GeographyClass(activity).also {
            it.setUpWithGlobe(globeVC)
        }

        loader = makeTestLoader(activity, globeVC)

        val bound = geoBound(makeBNGCoordSystem(activity, false))
        val middle = bound.middle()
        val h = globeVC.findHeightToViewBounds(bound, middle)
        globeVC.setPositionGeo(middle, h/2)
        globeVC.animatePositionGeo(middle, h*1.1, 0.0, 1.0)

        return true
    }
    
    @Throws(Exception::class)
    override fun setUpWithMap(mapVC: MapController): Boolean {
        baseCase = GeographyClass(activity).also {
            it.setUpWithMap(mapVC)
        }

        loader = makeTestLoader(activity, mapVC)

        val bound = geoBound(makeBNGCoordSystem(activity, false))
        val middle = bound.middle()
        val h = mapVC.findHeightToViewBounds(bound, middle)
        mapVC.setPositionGeo(middle, h/3)
        mapVC.animatePositionGeo(middle, h, 0.0, 1.0)

        return true
    }
    
    private fun geoBound(coordSys: CoordSystem): Mbr {
        val bound = coordSys.bounds
        val ll = coordSys.localToGeographic(Point3d(bound.ll.x, bound.ll.y, 0.0))
        val ur = coordSys.localToGeographic(Point3d(bound.ur.x, bound.ur.y, 0.0))
        return Mbr(Point2d(ll.x, ll.y), Point2d(ur.x, ur.y))
    }
    
    override fun shutdown() {
        loader?.let {
            it.tileFetcher?.shutdown()
            it.shutdown()
            loader = null
        }
        baseCase?.let {
            it.shutdown()
            baseCase = null
        }
        super.shutdown()
    }

    // Put together a British National Grid system
    protected fun makeBNGCoordSystem(activity: Activity?, displayVersion: Boolean): CoordSystem {
        // Load up the grid adjustment file from the assets, and write it out where proj.4 can read it
        // todo: how do we quote/escape this if there are spaces?
        val bngFile = copyAssetFile(activity, "bng/OSTN02_NTv2.gsb", "bng", "OSTN02_NTv2.gsb")
        val projStr = "+proj=tmerc +lat_0=49 +lon_0=-2 +k=0.9996012717 +x_0=400000 +y_0=-100000 +ellps=airy +units=m +no_defs +nadgrids=$bngFile"
        
        // Set the bounding box for validity.  It assumes it can go everywhere by default
        val bound = Mbr().also {
            it.addPoint(Point2d(1393.0196, 13494.9764))
            it.addPoint(Point2d(671196.3657, 1230275.0454))
        }
        
        // Expand when setting the view bound so we can see the whole UK and the edge of the tile set
        if (displayVersion) {
            val extra = 0.5
            val extraX = extra * (bound.ur.x - bound.ll.x)
            val extraY = extra * (bound.ur.y - bound.ll.y)
            bound.addPoint(Point2d(bound.ll.x - extraX, bound.ll.y - extraY))
            bound.addPoint(Point2d(bound.ur.x + extraX, bound.ur.y + extraY))
        }

        return Proj4CoordSystem(projStr).apply {
            bounds = bound
        }
    }
    
    private fun makeTestLoader(activity: Activity, viewC: BaseController): QuadImageLoader {
        val bngCoordSystem = makeBNGCoordSystem(activity, false)
        val params = SamplingParams().apply {
            coordSystem = bngCoordSystem
            coverPoles = false
            edgeMatching = false
            minZoom = 0
            maxZoom = 10
            singleLevel = true
        }
        val info: TileInfoNew = TestTileInfo(0, 20)
        
        return QuadImageLoader(params, info, viewC).apply {
            setBaseDrawPriority(2000)
            tileFetcher = TestTileFetcher(viewC, "test").apply {
                alpha = 96
                start() // todo: does the loader clean this up?
            }
            debugMode = false
            // Additional tile boundaries for troubleshooting
            //setLoaderInterpreter(OvlDebugImageLoaderInterpreter().apply {
            //    setParentInterpreter(ImageLoaderInterpreter())
            //})
        }
    }
    
    private var baseCase: MaplyTestCase? = null
    private var loader: QuadImageLoader? = null
}