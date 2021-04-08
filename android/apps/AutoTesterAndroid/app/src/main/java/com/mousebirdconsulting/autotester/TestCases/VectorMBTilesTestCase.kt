/*  VectorMBTilesTestCase.kt
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford.
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
import android.content.Context
import android.content.ContextWrapper
import android.graphics.Color
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import java.io.File
import java.io.FileNotFoundException
import java.io.FileOutputStream
import java.io.IOException


/**
 * Loads vector tiles over southern France.
 */
class VectorMBTilesTestCase(activity: Activity) : MaplyTestCase(activity, "Vector MBTiles") {
    
    var loader: QuadLoaderBase? = null

    fun setupCountriesRaster(control: BaseController) {
        val mbTiles: File

        // We need to copy the file from the asset so that it can be used as a file
        try {
            mbTiles = this.getFile("mbtiles", "mbtiles/countries-raster.mbtiles", "countries-raster.mbtiles")
        } catch (e: Exception) {
            return
        }

        if (!mbTiles.exists()) {
            throw FileNotFoundException(String.format("Could not copy MBTiles asset to \"%s\"", mbTiles.absolutePath))
        }

        // The fetcher fetches tile from the MBTiles file
        val mbTileFetcher = MBTileFetcher(control, mbTiles)

        // Set up the parameters to match the MBTile file
        val params = SamplingParams()
        params.coordSystem = SphericalMercatorCoordSystem()
        params.coverPoles = true
        params.edgeMatching = true
        params.singleLevel = true
        params.minZoom = 0
        params.maxZoom = mbTileFetcher.maxZoom

        val loader = QuadImageLoader(params, mbTileFetcher.tileInfo, control)
        loader.tileFetcher = mbTileFetcher
        this.loader = loader
    }

    fun setupContriesVector(control: BaseController) {
        val mbTiles: File

        // We need to copy the file from the asset so that it can be used as a file
        try {
            mbTiles = this.getFile("mbtiles", "mbtiles/countries.mbtiles", "countries.mbtiles")
        } catch (e: Exception) {
            return
        }

        if (!mbTiles.exists()) {
            throw FileNotFoundException(String.format("Could not copy MBTiles asset to \"%s\"", mbTiles.absolutePath))
        }

        // The fetcher fetches tile from the MBTiles file
        val mbTileFetcher = MBTileFetcher(control, mbTiles)

        // Set up the parameters to match the MBTile file
        val params = SamplingParams()
        params.coordSystem = SphericalMercatorCoordSystem()
        params.coverPoles = true
        params.edgeMatching = true
        params.singleLevel = true
        params.minZoom = 0
        params.maxZoom = mbTileFetcher.maxZoom

        // Simple style generator just picks random colors
        val styleGen = VectorStyleSimpleGenerator(control)

        // Mapbox interpreter uses the parser and style to create visual data
        val interp = MapboxVectorInterpreter(styleGen, control)

        // Overlay the tile number on top
        val debugInterp = OvlDebugImageLoaderInterpreter()
        debugInterp.setParentInterpreter(interp)

        val loader = QuadPagingLoader(params, mbTileFetcher.tileInfo, debugInterp, control)
        loader.setTileFetcher(mbTileFetcher)
        this.loader = loader
    }

    fun setupFranceVector(control: BaseController) {
        val mbTileFile = getFile("mbtiles", "mbtiles/France.mbtiles", "France.mbtiles")
        val fetcher = MBTileFetcher(control, mbTileFile)

        // Sampling params define how the globe is broken up, including the depth
        var params = SamplingParams()
        params.coordSystem = SphericalMercatorCoordSystem()
        params.singleLevel = true
        params.minZoom = 0
        params.maxZoom = fetcher.maxZoom

        // Simple style generator just picks random colors
        val styleGen = VectorStyleSimpleGenerator(control)

        // Mapbox interpreter uses the parser and style to create visual data
        val interp = MapboxVectorInterpreter(styleGen, control)

        // Overlay the tile number on top
//        val debugInterp = OvlDebugImageLoaderInterpreter()
//        debugInterp!!.setParentInterpreter(interp)

        val loader = QuadPagingLoader(params, fetcher.tileInfo, interp, control)
        loader.setTileFetcher(fetcher)
        this.loader = loader

        // Shut down the loader after a short period
        // Useful for debugging
//        val handler = Handler()
//        handler.postDelayed({
//            loader.shutdown()
//            this.loader = null
//
//            val handler = Handler()
//            handler.postDelayed({
//                this.setupFranceVector(control)
//            }, 1000)
//        }, 500)
    }

    fun setupShapefile(control: BaseController) {
        // Load a shapefile too
        val shpData = VectorObject()
        getFile("ne_10m_roads", "ne_10m_roads/ne_10m_roads.dbf", "ne_10m_roads.dbf")
        val shpFile = getFile("ne_10m_roads", "ne_10m_roads/ne_10m_roads.shp", "ne_10m_roads.shp")
        getFile("ne_10m_roads", "ne_10m_roads/ne_10m_roads.shx", "ne_10m_roads.shx")
        shpData.fromShapeFile(shpFile.absolutePath)
        val vecInfo = VectorInfo()
        vecInfo.setColor(Color.MAGENTA)
        vecInfo.drawPriority = 1000000
        control.addVector(shpData, vecInfo, ThreadMode.ThreadAny)
    }

    var baseCase: GeographyClass? = null

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        baseCase = GeographyClass(mapVC!!.getActivity())
        baseCase?.setUpWithMap(mapVC)
        val coord = Point2d.FromDegrees(3.1, 44.0)
        mapVC.setPositionGeo(coord.x, coord.y, 0.07)

//        setupCountriesRaster(mapVC as BaseController)
//        setupContriesVector(mapVC as BaseController)
        setupFranceVector(mapVC as BaseController)
//        setupShapefile(mapVC as BaseController)

        return true
    }

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        baseCase = GeographyClass(globeVC!!.getActivity())
        baseCase?.setUpWithGlobe(globeVC)
        val coord = Point2d.FromDegrees(3.1, 44.0)
        globeVC.setPositionGeo(coord.x, coord.y, 0.07)

//        setupCountriesRaster(globeVC as BaseController)
//        setupContriesVector(globeVC as BaseController)
        setupFranceVector(globeVC as BaseController)
//        setupShapefile(globeVC as BaseController)

        return true
    }

    // Copy an MBTiles file out of the package for direct use
    @Throws(IOException::class)
    private fun getFile(dir: String, fileName: String, outputFileName: String): File {

        val wrapper = ContextWrapper(activity)
        val destDir = wrapper.getDir(dir, Context.MODE_PRIVATE)

        val inStream = activity.assets.open(fileName)
        val of = File(destDir, outputFileName)

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

    // Switch maps on long press
    override fun userDidLongPress(mapController: MapController?, selObjs: Array<SelectedObject?>?, loc: Point2d?, screenLoc: Point2d?) {
        loader?.reload()
    }

}
