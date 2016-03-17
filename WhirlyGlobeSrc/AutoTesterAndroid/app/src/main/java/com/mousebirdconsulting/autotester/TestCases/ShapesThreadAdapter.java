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


public class ShapesThreadAdapter {

    private MaplyBaseController viewC;
    private ComponentObject componentObject;
    private LayerThread shapeThread;

    public ShapesThreadAdapter(MaplyBaseController viewC, LayerThread thread) {
        this.viewC = viewC;
        this.shapeThread = thread;
        addShapes(this.viewC);
    }

    private void addShapes(MaplyBaseController viewC) {
        List<Shape> shapes = new ArrayList<>();
        int numShapes = 10000;

        ShapeSphere newShape;
        for (int ii = 0; ii < numShapes; ii++) {
            newShape = new ShapeSphere();
            newShape.setColor(0,0,0,0);
            newShape.setHeight(1);
            newShape.setRadius(1);
            newShape.setLoc(new Point2d(-3.6704803, 40.5023056));
            shapes.add(newShape);
        }

        ShapeInfo shapeInfo = new ShapeInfo();
        shapeInfo.setColor(0,0,0,0);
        shapeInfo.setLineWidth(2);
        shapeInfo.setEnable(true);
        viewC.addShapes(shapes, shapeInfo, MaplyBaseController.ThreadMode.ThreadAny);
    }
}
