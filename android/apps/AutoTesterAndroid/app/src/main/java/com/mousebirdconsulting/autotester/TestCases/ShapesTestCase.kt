/*
 *  ShapesTestCase.kt
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
import android.location.Location
import com.mousebird.maply.*

import com.mousebirdconsulting.autotester.Framework.MaplyTestCase

import java.util.ArrayList
import kotlin.math.PI


class ShapesTestCase(activity: Activity) : MaplyTestCase(activity) {

    init {
        setTestName("Shape Test Case")
        setDelay(1000)
        this.implementation = MaplyTestCase.TestExecutionImplementation.Globe
    }

    @Throws(Exception::class)
    override fun setUpWithGlobe(globeVC: GlobeController): Boolean {
        val baseView = StamenRemoteTestCase(getActivity())
        baseView.setUpWithGlobe(globeVC)

        addShapes(globeVC)

        return true
    }

    private inner class LocationInfo(internal var name: String, internal var x: Double, internal var y: Double)

    private fun addShapes(viewC: BaseController) {
        val locations = ArrayList<LocationInfo>()
        locations.add(LocationInfo("Kansas City", 39.1, -94.58))
        locations.add(LocationInfo("Washington, DC", 38.895111, -77.036667))
        locations.add(LocationInfo("Manila", 14.583333, 120.966667))
        locations.add(LocationInfo("Moscow", 55.75, 37.616667))
        locations.add(LocationInfo("London", 51.507222, -0.1275))
        locations.add(LocationInfo("Caracas", 10.5, -66.916667))
        locations.add(LocationInfo("Lagos", 6.453056, 3.395833))
        locations.add(LocationInfo("Sydney", -33.859972, 151.211111))
        locations.add(LocationInfo("Seattle", 47.609722, -122.333056))
        locations.add(LocationInfo("Tokyo", 35.689506, 139.6917))
        locations.add(LocationInfo("McMurdo Station", -77.85, 166.666667))
        locations.add(LocationInfo("Tehran", 35.696111, 51.423056))
        locations.add(LocationInfo("Santiago", -33.45, -70.666667))
        locations.add(LocationInfo("Pretoria", -25.746111, 28.188056))
        locations.add(LocationInfo("Perth", -31.952222, 115.858889))
        locations.add(LocationInfo("Beijing", 39.913889, 116.391667))
        locations.add(LocationInfo("New Delhi", 28.613889, 77.208889))
        locations.add(LocationInfo("San Francisco", 37.7793, -122.4192))
        locations.add(LocationInfo("Pittsburgh", 40.441667, -80.0))
        locations.add(LocationInfo("Freetown", 8.484444, -13.234444))
        locations.add(LocationInfo("Windhoek", -22.57, 17.083611))
        locations.add(LocationInfo("Buenos Aires", -34.6, -58.383333))
        locations.add(LocationInfo("Zhengzhou", 34.766667, 113.65))
        locations.add(LocationInfo("Bergen", 60.389444, 5.33))
        locations.add(LocationInfo("Glasgow", 55.858, -4.259))
        locations.add(LocationInfo("Bogota", 4.598056, -74.075833))
        locations.add(LocationInfo("Haifa", 32.816667, 34.983333))
        locations.add(LocationInfo("Puerto Williams", -54.933333, -67.616667))
        locations.add(LocationInfo("Panama City", 8.983333, -79.516667))
        locations.add(LocationInfo("Niihau", 21.9, -160.166667))

        addCylinders(viewC,locations,5,0)
        addGreatCircles(viewC,locations,5,2)
        addSpheres(viewC,locations,5,1)
        addCircles(viewC,locations,5,3)
        addArrows(viewC,locations,5,4)
    }

    private fun addSpheres(vc: BaseController, locs: ArrayList<LocationInfo>, stride: Int, offset: Int) {
        val shapes = ArrayList<Shape>()

        for (ii in offset until locs.size step stride) {
            val newShape = ShapeSphere()
            newShape.setLoc(Point2d.FromDegrees(locs[ii].y, locs[ii].x))
            newShape.setRadius(0.04f)
            newShape.isSelectable = true
            shapes.add(newShape)
        }

        val shapeInfo = ShapeInfo()
        shapeInfo.setColor(1f, 0f, 0f, 0.8f)
        shapeInfo.drawPriority = 1000000
        //        shapeInfo.setFade(1.0f);
        vc.addShapes(shapes, shapeInfo, RenderControllerInterface.ThreadMode.ThreadAny)
    }

    private fun addCylinders(vc: BaseController, locs: ArrayList<LocationInfo>, stride: Int, offset: Int) {
        val shapes = ArrayList<Shape>()

        for (ii in offset until locs.size step stride) {
            val cyl = ShapeCylinder()
            cyl.setBaseCenter(Point2d.FromDegrees(locs[ii].y, locs[ii].y))
            cyl.setRadius(0.01)
            cyl.setHeight(0.06)
            cyl.setSample(20)
            cyl.isSelectable = true
            shapes.add(cyl)
        }

        val shapeInfo = ShapeInfo()
        shapeInfo.setColor(0f, 0f, 1f, 0.8f)
        shapeInfo.drawPriority = 1000000
        //        shapeInfo.setFade(1.0f);
        vc.addShapes(shapes, shapeInfo, RenderControllerInterface.ThreadMode.ThreadAny)
    }

    private fun addGreatCircles(vc: BaseController, locs: ArrayList<LocationInfo>, stride: Int, offset: Int) {
        val shapes = ArrayList<Shape>()

        for (ii in offset until locs.size step stride) {
            val circle = ShapeGreatCircle()
            val loc0 = locs[ii];
            val loc1 = locs[(ii+1)%locs.size]
            circle.setPoints(Point2d.FromDegrees(loc0.y,loc0.x),Point2d.FromDegrees(loc1.y,loc1.x))
            circle.isSelectable = true
            val angle = circle.angleBetween()
            circle.setHeight(0.3 * angle / PI)
            shapes.add(circle)
        }

        val shapeInfo = ShapeInfo()
        shapeInfo.setColor(1.0f, 0.1f, 0f, 1.0f)
        shapeInfo.drawPriority = 1000000
        shapeInfo.setLineWidth(8.0f)
        //        shapeInfo.setFade(1.0f);
        vc.addShapes(shapes, shapeInfo, RenderControllerInterface.ThreadMode.ThreadAny)
    }

    private fun addCircles(vc: BaseController, locs: ArrayList<LocationInfo>, stride: Int, offset: Int) {
        val shapes = ArrayList<Shape>()

        for (ii in offset until locs.size step stride) {
            val circle = ShapeCircle()
            circle.setLoc(Point2d.FromDegrees(locs[ii].y,locs[ii].x))
            circle.setRadius(0.04)
            circle.setSample(40)
            circle.isSelectable = true
            shapes.add(circle)
        }

        val shapeInfo = ShapeInfo()
        shapeInfo.setColor(0f, 1.00f, 0f, 0.8f)
        shapeInfo.drawPriority = 1000000
        //        shapeInfo.setFade(1.0f);
        vc.addShapes(shapes, shapeInfo, RenderControllerInterface.ThreadMode.ThreadAny)
    }

    private fun addArrows(vc: BaseController, locs: ArrayList<LocationInfo>, stride: Int, offset: Int) {
        val shapes = ArrayList<Shape>()

        for (ii in offset until locs.size step stride) {
            val arrow = ShapeExtruded()
            val size = 100000.0
            val coords = arrayOf(Point2d(-0.25*size,-0.75*size),
                    Point2d(-0.25*size,0.25*size),
                    Point2d(-0.5*size,0.25*size),
                    Point2d(0.0*size,1.0*size),
                    Point2d(0.5*size,0.25*size),
                    Point2d(0.25*size,0.25*size),
                    Point2d(0.25*size,-0.75*size))
            arrow.setLoc(Point2d.FromDegrees(locs[ii].y,locs[ii].x))
            arrow.setOutline(coords)
            arrow.setThickness(size*1.0)
            arrow.setHeight(0.0)
            // Scale it down to meters
            arrow.setScale(1.0/6371000.0)
            shapes.add(arrow)
        }

        val shapeInfo = ShapeInfo()
        shapeInfo.setColor(1.0f, 0.1f, 0f, 0.8f)
        shapeInfo.drawPriority = 1000000
        //        shapeInfo.setFade(1.0f);
        vc.addShapes(shapes, shapeInfo, RenderControllerInterface.ThreadMode.ThreadAny)
    }
}
