/*
 *  CartoTestCase.kt
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
import android.graphics.Color
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import okhttp3.Request
import java.net.URL
import java.net.URLEncoder
import kotlin.math.PI

/**
 * Pull the NY city parcel data onto a map.
 * Makes a nice test case for the paging loader.
 */
class CartoTestCase : MaplyTestCase {

    constructor(activity: Activity) : super(activity) {
        setTestName("Carto New York")
        implementation = TestExecutionImplementation.Both
    }

    var interp : CartoInterp? = null
    var loader : QuadPagingLoader? = null

    fun setupCartoLayer(vc: BaseController) {
        val params = SamplingParams()
        params.minZoom = 0
        params.maxZoom = 15
        params.minImportance = 1024.0*1024.0
        params.singleLevel = true
        params.coordSystem = SphericalMercatorCoordSystem()

        val interp = CartoInterp("SELECT the_geom,address,ownername,numfloors FROM mn_mappluto_13v1 WHERE the_geom && ST_SetSRID(ST_MakeBox2D(ST_Point(%f, %f), ST_Point(%f, %f)), 4326) LIMIT 2000;")
        interp.minZoom = params.maxZoom
        interp.maxZoom = params.maxZoom

        loader = QuadPagingLoader(params,interp,interp,vc)
    }

    var baseCase : CartoLightTestCase? = null

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        baseCase = CartoLightTestCase(getActivity())
        baseCase?.setUpWithGlobe(globeVC)

        setupCartoLayer(globeVC!!)

        val loc = Point2d.FromDegrees(-73.99,40.75)
        globeVC.setPositionGeo(loc.x,loc.y,0.0001)

        return true
    }

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        baseCase = CartoLightTestCase(getActivity())
        baseCase?.setUpWithMap(mapVC)

        setupCartoLayer(mapVC!!)

        val loc = Point2d.FromDegrees(-73.99,40.75)
        mapVC.setPositionGeo(loc.x,loc.y,0.0002)

        return true
    }

    /**
     * We've got both a LoaderInterpreter (builds geometry) and a TileInfo (says where to get
     * stuff) rolled into the same object.  For laziness.
     */
    class CartoInterp : LoaderInterpreter, TileInfoNew {

        val search: String

        constructor(srch: String) {
            search = srch

            vecInfoFill.setFilled(true)
            vecInfoFill.setColor(0.25f,0.0f,0.0f,0.25f)

            vecInfoOutline.setColor(Color.RED)
        }

        var theLoader: QuadLoaderBase? = null
        val vecInfoFill = VectorInfo()
        val vecInfoOutline = VectorInfo()

        override fun setLoader(inLoader: QuadLoaderBase) {
            theLoader = inLoader
        }

        // Generate the fetch request for the chunk of data we want
        override fun fetchInfoForTile(tileID: TileID?, flipY: Boolean): Any? {
            val bbox = theLoader?.geoBoundsForTile(tileID)

            // Construct the query string
            val fetchInfo = RemoteTileFetchInfo()
            val toDeg = 180.0/ PI
            val query = String.format(search,bbox!!.ll.x*toDeg,bbox.ll.y*toDeg,bbox.ur.x*toDeg,bbox.ur.y*toDeg)
            val encodeQuery = URLEncoder.encode(query,"utf-8")
            val fullURLStr = String.format("https://pluto.cartodb.com/api/v2/sql?format=GeoJSON&q=%s",encodeQuery)
            fetchInfo.urlReq = Request.Builder().url(URL(fullURLStr)).build()

            return fetchInfo
        }

        // Parse the GeoJSON coming back and turn it into geometry
        override fun dataForTile(loadReturn: LoaderReturn, loader: QuadLoaderBase) {
            if (!(loadReturn is ObjectLoaderReturn))
                return

            val data = loadReturn.firstData
            if (data == null)
                return

            val geojson = String(data)
            val vecObj = VectorObject()
            vecObj.fromGeoJSON(geojson)

            val vc = loader.controller
            loadReturn.addComponentObject(vc.addVector(vecObj,vecInfoFill,RenderControllerInterface.ThreadMode.ThreadCurrent))
            loadReturn.addComponentObject(vc.addVector(vecObj,vecInfoOutline,RenderControllerInterface.ThreadMode.ThreadCurrent))
        }
    }
}