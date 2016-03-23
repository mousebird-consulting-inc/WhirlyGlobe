/*
 *  LightingAdapter.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2016 mousebird consulting
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
package com.mousebirdconsulting.autotester.TestCases;

import com.mousebird.maply.Light;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.Point3d;

import java.util.ArrayList;
import java.util.List;


public class LightingAdapter {

    private class LocationInfo {
        String name;
        double x,y,z;

        public LocationInfo(String inName, double x, double y) {
            this.name = inName;
            this.x = x;
            this.y = y;
            this.z = Math.random()*10;
        }
    }

    private MaplyBaseController viewC;
    private List<LocationInfo> locations;

    public LightingAdapter(MaplyBaseController viewC) {
        this.viewC = viewC;
        locations = new ArrayList<>();
        locations.add(new LocationInfo("Kansas City",39.1, -94.58));
        locations.add(new LocationInfo("Washington, DC",38.895111,-77.036667));
        locations.add(new LocationInfo("Manila",14.583333,120.966667));
        locations.add(new LocationInfo("Moscow",55.75, 37.616667));
        locations.add(new LocationInfo("London",51.507222, -0.1275));
        locations.add(new LocationInfo("Caracas",10.5, -66.916667));
        locations.add(new LocationInfo("Lagos",6.453056, 3.395833));
        locations.add(new LocationInfo("Sydney",-33.859972, 151.211111));
        locations.add(new LocationInfo("Seattle",47.609722, -122.333056));
        locations.add(new LocationInfo("Tokyo",35.689506, 139.6917));
        locations.add(new LocationInfo("McMurdo Station",-77.85, 166.666667));
        locations.add(new LocationInfo("Tehran",35.696111, 51.423056));
        locations.add(new LocationInfo("Santiago",-33.45, -70.666667));
        locations.add(new LocationInfo("Pretoria",-25.746111, 28.188056));
        locations.add(new LocationInfo("Perth",-31.952222, 115.858889));
        locations.add(new LocationInfo("Beijing",39.913889, 116.391667));
        locations.add(new LocationInfo("New Delhi",28.613889, 77.208889));
        locations.add(new LocationInfo("San Francisco",37.7793, -122.4192));
        locations.add(new LocationInfo("Pittsburgh",40.441667, -80));
        locations.add(new LocationInfo("Freetown",8.484444, -13.234444));
        locations.add(new LocationInfo("Windhoek",-22.57, 17.083611));
        locations.add(new LocationInfo("Buenos Aires",-34.6, -58.383333));
        locations.add(new LocationInfo("Zhengzhou",34.766667, 113.65));
        locations.add(new LocationInfo("Bergen",60.389444, 5.33));
        locations.add(new LocationInfo("Glasgow",55.858, -4.259));
        locations.add(new LocationInfo("Bogota",4.598056, -74.075833));
        locations.add(new LocationInfo("Haifa",32.816667, 34.983333));
        locations.add(new LocationInfo("Puerto Williams",-54.933333, -67.616667));
        locations.add(new LocationInfo("Panama City",8.983333, -79.516667));
        locations.add(new LocationInfo("Niihau",21.9, -160.166667));

        addLights();
    }


    private void addLights() {
        Light light;

        for (LocationInfo locationInfo : this.locations) {
            light = new Light();
            light.setPos(new Point3d(locationInfo.x, locationInfo.y, locationInfo.z));
            light.setAmbient(0.1f, 0.1f , 0.1f, 1.0f);
            light.setDiffuse(0.8f, 0.8f, 0.8f, 1.0f);
            light.setViewDependent(true);
            this.viewC.addLight(light);
        }
    }
}
