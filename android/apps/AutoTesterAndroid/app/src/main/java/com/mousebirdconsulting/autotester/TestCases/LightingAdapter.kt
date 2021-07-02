/*  LightingAdapter.kt
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

import com.mousebird.maply.*
import java.lang.ref.WeakReference

class LightingAdapter(viewC: BaseController) {
    private class LocationInfo(val name: String, lat: Double, lon: Double) {
        val x: Double = lon * Math.PI / 180
        val y: Double = lat * Math.PI / 180
        val z: Double = Math.random() * 1
    
    }
    
    //private val viewC: WeakReference<BaseController> = WeakReference(viewC)
    
    private val locations = arrayOf(
        //LocationInfo("Kansas City",39.1, -94.58),
        //LocationInfo("Washington, DC",38.895111,-77.036667),
        //LocationInfo("Manila",14.583333,120.966667),
        //LocationInfo("Kansas City",39.1, -94.58),
        //LocationInfo("Washington, DC",38.895111,-77.036667),
        //LocationInfo("Manila",14.583333,120.966667),
        LocationInfo("Moscow", 55.75, 37.616667),
        LocationInfo("London", 51.507222, -0.1275),
        //LocationInfo("Caracas",10.5, -66.916667),
        //LocationInfo("Lagos",6.453056, 3.395833),
        //LocationInfo("Sydney",-33.859972, 151.211111),
        //LocationInfo("Caracas",10.5, -66.916667),
        //LocationInfo("Lagos",6.453056, 3.395833),
        //LocationInfo("Sydney",-33.859972, 151.211111),
        LocationInfo("Seattle", 47.609722, -122.333056),
        //LocationInfo("Tokyo",35.689506, 139.6917),
        //LocationInfo("McMurdo Station",-77.85, 166.666667),
        //LocationInfo("Tehran",35.696111, 51.423056),
        //LocationInfo("Santiago",-33.45, -70.666667),
        //LocationInfo("Pretoria",-25.746111, 28.188056),
        //LocationInfo("Perth",-31.952222, 115.858889),
        //LocationInfo("Beijing",39.913889, 116.391667),
        //LocationInfo("New Delhi",28.613889, 77.208889),
        //LocationInfo("San Francisco",37.7793, -122.4192),
        //LocationInfo("Pittsburgh",40.441667, -80),
        //LocationInfo("Freetown",8.484444, -13.234444),
        //LocationInfo("Windhoek",-22.57, 17.083611),
        //LocationInfo("Buenos Aires",-34.6, -58.383333),
        //LocationInfo("Zhengzhou",34.766667, 113.65),
        //LocationInfo("Bergen",60.389444, 5.33),
        //LocationInfo("Glasgow",55.858, -4.259),
        //LocationInfo("Bogota",4.598056, -74.075833),
        //LocationInfo("Haifa",32.816667, 34.983333),
        //LocationInfo("Puerto Williams",-54.933333, -67.616667),
        //LocationInfo("Panama City",8.983333, -79.516667),
        //LocationInfo("Niihau",21.9, -160.166667),
    )

    private fun addLights(vc: BaseController) {
        vc.clearLights()
        for (locationInfo in locations) {
            vc.addLight(Light().apply {
                pos = Point3d(locationInfo.x, locationInfo.y, locationInfo.z)
                setAmbient(0.1f, 0.1f, 0.1f, 0.1f)
                setDiffuse(0.8f, 0.8f, 0.8f, 0.1f)
                isViewDependent = true
            })
        }
    }
    
    init {
        addLights(viewC)
    }
}