/*
 *  TextureVectorTestCase.java
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
import android.graphics.BitmapFactory
import android.graphics.Color
import android.util.Log
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import com.mousebirdconsulting.autotester.R
import okio.Okio
import java.lang.Exception
import java.nio.charset.Charset
import java.util.*
import kotlin.math.PI
import kotlin.math.abs

class TextureVectorTestCase(activity: Activity) : MaplyTestCase(activity, "Textured Vectors") {
    
    // Grid size to use for clipping
    // Smaller is going to be more triangles, but look better
    // Might be better to use a bigger size for the poles
    private val ClipGridSize = 2.0/180.0* PI

    private fun buildCountries(control: BaseController) : ComponentObject {
        val assetMgr = activity.assets
        val isGlobe = control is GlobeController
        
        val icon = BitmapFactory.decodeResource(activity.resources, R.drawable.testtarget)
        val tex = control.addTexture(icon, null, ThreadMode.ThreadCurrent)
        val paths = assetMgr.list("country_json_50m")?.sortedDescending() ?: emptyList()

        val tessObjs = ArrayList<VectorObject>()

        // Load in 20 countries to apply a texture to
        for (path in paths.take(20)) {
            try {
                assetMgr.open("country_json_50m/$path").use { stream ->
                    Okio.source(stream).use { source ->
                        Okio.buffer(source).use { buffer ->
                            VectorObject.createFromGeoJSON(buffer.readUtf8())?.let { vecObj ->
                                vecObj.selectable = true
                                
                                // Work through each individual loop
                                for (thisVecObj in vecObj) {
                                    val loopObj = thisVecObj.deepCopy()
                    
                                    // Center of the texture application
                                    val center = loopObj.center()
                                    val attrs = loopObj.attributes
                                    attrs.setDouble("veccenterx", center.x)
                                    attrs.setDouble("veccentery", center.y)
    
                                    var thisClipGridLon = ClipGridSize
                                    if (isGlobe) {
                                        // We adjust the grid clipping size based on the latitude
                                        // This helps a lot near the poles.  Otherwise we're way oversampling
                                        if (abs(center.x) > 60.0 / 180.0 * PI)
                                            thisClipGridLon *= 4.0
                                        else if (abs(center.y) > 45.0 / 180.0 * PI)
                                            thisClipGridLon *= 2.0;
                        
                                    }
                                    loopObj.clipToGrid(Point2d(thisClipGridLon, ClipGridSize))?.tesselate()?.let {
                                        tessObjs.add(it)
                                    }
                                }
                            }
                        }
                    }
                }
            } catch (e: Exception) {
                Log.e(javaClass.simpleName, "Failed to load $path", e)
            }
        }

        val vecInfo = VectorInfo().apply {
            setFilled(true)
            setTexture(tex)
            // todo: TextureProjection.Screen doesn't work?
            setTextureProjection(if (isGlobe) VectorInfo.TextureProjection.TangentPlane else VectorInfo.TextureProjection.None)
            setTexScale(16.0, 16.0)
            setColor(Color.WHITE)
        }

        // Add all the vectors at once to be more efficient
        // The geometry gets grouped together, which is nice and fast
        return control.addVectors(tessObjs, vecInfo, ThreadMode.ThreadAny)
    }

    var baseCase : CartoLightTestCase? = null

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        baseCase = CartoLightTestCase(activity)
        baseCase?.setUpWithMap(mapVC)

        buildCountries(mapVC!!)
        mapVC.animatePositionGeo(-Math.PI/2,0.0,1.0,1.0)

        return true
    }

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        baseCase = CartoLightTestCase(activity)
        baseCase?.setUpWithGlobe(globeVC)

        buildCountries(globeVC!!)
        globeVC.animatePositionGeo(-Math.PI/2,0.0,1.0,1.0)

        return true
    }
}