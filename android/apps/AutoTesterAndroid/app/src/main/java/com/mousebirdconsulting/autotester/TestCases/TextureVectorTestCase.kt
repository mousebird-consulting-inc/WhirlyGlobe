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
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import com.mousebirdconsulting.autotester.R
import okio.Okio
import java.lang.Exception
import java.nio.charset.Charset
import java.util.*
import kotlin.math.PI
import kotlin.math.abs

class TextureVectorTestCase : MaplyTestCase {

    constructor(activity: Activity) : super(activity) {
        setTestName("Textured Vectors")
        implementation = TestExecutionImplementation.Both
    }

    // Grid size to use for clipping
    // Smaller is going to be more triangles, but look better
    // Might be better to use a bigger size for the poles
    val ClipGridSize = 2.0/180.0* PI

    fun buildCountries(control: BaseController) : ComponentObject {
        val icon = BitmapFactory.decodeResource(getActivity().resources, R.drawable.testtarget)
        val tex = control.addTexture(icon, null, RenderControllerInterface.ThreadMode.ThreadCurrent)

        val assetMgr = getActivity().assets
        val paths = assetMgr.list("country_json_50m")!!

        var tessObjs = ArrayList<VectorObject>()

        // Load in 20 contries to apply a texture to
        var count = 0
        for (path in paths) {
            val stream = assetMgr.open("country_json_50m/" + path)
            try {
                val vecObj = VectorObject()
                vecObj.selectable = true
                val json = Okio.buffer(Okio.source(stream)).readUtf8()
                if (vecObj.fromGeoJSON(json)) {
                    // Work through each individual loop
                    for (thisVecObj in vecObj) {
                        val loopObj = thisVecObj.deepCopy()

                        // Center of the texture application
                        val center = loopObj.center()
                        val attrs = loopObj.attributes
                        attrs.setDouble("veccenterx", center.x)
                        attrs.setDouble("veccentery", center.y)

                        var tessObj : VectorObject?
                        if (control is GlobeController) {
                            // We adjust the grid clipping size based on the latitude
                            // This helps a lot near the poles.  Otherwise we're way oversampling
                            var thisClipGridLon = ClipGridSize
                            if (abs(center.x) > 60.0/180.0 * PI)
                                thisClipGridLon *= 4.0
                            else if (abs(center.y) > 45.0/180.0 * PI)
                                thisClipGridLon *= 2.0;

                            // We clip the vector to a grid and then tesselate the results
                            // This forms the vector closer to the globe, make it look nicer
                            tessObj = loopObj.clipToGrid(Point2d(thisClipGridLon,ClipGridSize)).tesselate()
                        } else {
                            tessObj =  loopObj.tesselate();
                        }

                        if (tessObj != null)
                            tessObjs.add(tessObj)
                    }
                }
            }
            catch (e: Exception)
            {
            }

            if (count++ > 20)
                break
        }

        val vecInfo = VectorInfo()
        vecInfo.setFilled(true)
        vecInfo.setTexture(tex)
        vecInfo.setTextureProjection(VectorInfo.TextureProjection.TangentPlane)
        vecInfo.setTexScale(16.0, 16.0)
        vecInfo.setColor(Color.WHITE)

        // Add all the vectors at once to be more efficient
        // The geometry gets grouped together, which is nice and fast
        return control.addVectors(tessObjs, vecInfo, RenderControllerInterface.ThreadMode.ThreadAny)
    }

    var baseCase : CartoLightTestCase? = null

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        baseCase = CartoLightTestCase(getActivity())
        baseCase?.setUpWithMap(mapVC)

        buildCountries(mapVC!!)

        return true
    }

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        baseCase = CartoLightTestCase(getActivity())
        baseCase?.setUpWithGlobe(globeVC)

        buildCountries(globeVC!!)

        return true
    }
}