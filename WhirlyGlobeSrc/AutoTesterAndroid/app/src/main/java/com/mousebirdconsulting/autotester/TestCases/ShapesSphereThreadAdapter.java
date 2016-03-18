/*
 *  ShapesThreadAdapter.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2014 mousebird consulting
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

import com.mousebird.maply.ComponentObject;
import com.mousebird.maply.GlobeController;
import com.mousebird.maply.LayerThread;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.Point2d;
import com.mousebird.maply.Shape;
import com.mousebird.maply.ShapeInfo;
import com.mousebird.maply.ShapeSphere;

import java.util.ArrayList;
import java.util.List;


public class ShapesSphereThreadAdapter {

    private class LocationInfo {
        String name;
        double x,y;

        public LocationInfo(String inName, double x, double y){
            this.name = inName;
            this.x = x;
            this.y = y;
        }
    }

    private MaplyBaseController viewC;
    private ComponentObject componentObject;
    private LayerThread shapeThread;
    private List<LocationInfo> locations;

    public ShapesSphereThreadAdapter(MaplyBaseController viewC, LayerThread thread) {
        this.viewC = viewC;
        this.shapeThread = thread;
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

        addShapes(this.viewC);
    }

    private void addShapes(MaplyBaseController viewC) {
        List<Shape> shapes = new ArrayList<>();
        int numShapes = locations.size();

        ShapeSphere newShape;
        for (int ii = 0; ii < numShapes; ii++) {
            newShape = new ShapeSphere();
            newShape.setLoc(new Point2d(locations.get(ii).x, locations.get(ii).y));
            newShape.setRadius(0.04f);
            newShape.setSelectable(true);
            shapes.add(newShape);
        }

        ShapeInfo shapeInfo = new ShapeInfo();
        shapeInfo.setColor(1,0,0,0.8f);
        shapeInfo.setDrawPriority(1000);
        shapeInfo.setFade(1.0f);
        componentObject =viewC.addShapes(shapes, shapeInfo, MaplyBaseController.ThreadMode.ThreadCurrent);
        viewC.enableObject(componentObject, MaplyBaseController.ThreadMode.ThreadAny);
    }
}
