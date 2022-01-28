/*  ShapesTestCase.kt
 *  WhirlyGlobeLib
 *
 *  Created by sjg
 *  Copyright 2011-2022 mousebird consulting
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
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import com.mousebird.maply.*

import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import com.mousebirdconsulting.autotester.R

import java.util.ArrayList
import kotlin.math.PI
import kotlin.math.sin

@Suppress("SameParameterValue")
class ShapesTestCase(activity: Activity) :
        MaplyTestCase(activity, "Shapes", TestExecutionImplementation.Both) {

    @Throws(Exception::class)
    override fun setUpWithGlobe(globeVC: GlobeController): Boolean {
        baseView.setUpWithGlobe(globeVC)
        addShapes(globeVC)
        globeVC.animatePositionGeo(-3.67, 40.5, 1.5, 1.0)
        return true
    }
    
    override fun setUpWithMap(mapVC: MapController): Boolean {
        baseView.setUpWithMap(mapVC)
        addShapes(mapVC)
        return true
    }

    override fun shutdown() {
        baseView.shutdown()
        super.shutdown()
    }

    private inner class LocationInfo(var name: String, var lat: Double, var lon: Double) {
        val point = Point2d.FromDegrees(lon, lat)
    }

    private val locations = listOf(
        LocationInfo("Kansas City", 39.1, -94.58),
        LocationInfo("Washington, DC", 38.895111, -77.036667),
        LocationInfo("Manila", 14.583333, 120.966667),
        LocationInfo("Moscow", 55.75, 37.616667),
        LocationInfo("London", 51.507222, -0.1275),
        LocationInfo("Caracas", 10.5, -66.916667),
        LocationInfo("Lagos", 6.453056, 3.395833),
        LocationInfo("Sydney", -33.859972, 151.211111),
        LocationInfo("Seattle", 47.609722, -122.333056),
        LocationInfo("Tokyo", 35.689506, 139.6917),
        LocationInfo("McMurdo Station", -77.85, 166.666667),
        LocationInfo("Tehran", 35.696111, 51.423056),
        LocationInfo("Santiago", -33.45, -70.666667),
        LocationInfo("Pretoria", -25.746111, 28.188056),
        LocationInfo("Perth", -31.952222, 115.858889),
        LocationInfo("Beijing", 39.913889, 116.391667),
        LocationInfo("New Delhi", 28.613889, 77.208889),
        LocationInfo("San Francisco", 37.7793, -122.4192),
        LocationInfo("Pittsburgh", 40.441667, -80.0),
        LocationInfo("Freetown", 8.484444, -13.234444),
        LocationInfo("Windhoek", -22.57, 17.083611),
        LocationInfo("Buenos Aires", -34.6, -58.383333),
        LocationInfo("Zhengzhou", 34.766667, 113.65),
        LocationInfo("Bergen", 60.389444, 5.33),
        LocationInfo("Glasgow", 55.858, -4.259),
        LocationInfo("Bogota", 4.598056, -74.075833),
        LocationInfo("Haifa", 32.816667, 34.983333),
        LocationInfo("Puerto Williams", -54.933333, -67.616667),
        LocationInfo("Panama City", 8.983333, -79.516667),
        LocationInfo("Niihau", 21.9, -160.166667),
    )

    private fun addShapes(viewC: BaseController) {
        addCylinders(viewC,locations,5,0)
        addGreatCircles(viewC,locations,5,1)
        addLinears(viewC,locations,5,2)
        addSpheres(viewC,locations,5,3)
        addCircles(viewC,locations,5,3)
        addArrows(viewC,locations,5,4)
        addRectangles(viewC,locations,5,5)
    }

    private fun addSpheres(vc: BaseController, locs: List<LocationInfo>, stride: Int, offset: Int) {
        val shapes = ArrayList<Shape>()

        for (ii in offset until locs.size step stride) {
            shapes.add(ShapeSphere().apply {
                setLoc(Point2d.FromDegrees(locs[ii].lon, locs[ii].lat))
                setHeight(0.05f)
                setRadius(0.04f)
                isSelectable = true
            })
        }

        val shapeInfo = ShapeInfo().apply {
            setColor(1f, 0f, 0f, 0.8f)
            drawPriority = 1000000
            //setFade(1.0f);
            setZBufferRead(globeController != null)
        }
        vc.addShapes(shapes, shapeInfo, ThreadMode.ThreadAny)
    }

    private fun addCylinders(vc: BaseController, locs: List<LocationInfo>, stride: Int, offset: Int) {
        val shapes = ArrayList<Shape>()

        for (ii in offset until locs.size step stride) {
            val cyl = ShapeCylinder()
            cyl.setBaseCenter(Point2d.FromDegrees(locs[ii].lon, locs[ii].lon))
            cyl.setRadius(0.01)
            cyl.setBaseHeight(0.01)
            cyl.setHeight(0.06)
            cyl.setSample(20)
            cyl.isSelectable = true
            shapes.add(cyl)
        }

        val shapeInfo = ShapeInfo().apply {
            setColor(0f, 0f, 1f, 0.8f)
            drawPriority = 1000000
            setZBufferRead(globeController != null)
            //setFade(1.0f);
        }
        vc.addShapes(shapes, shapeInfo, ThreadMode.ThreadAny)
    }

    private fun addGreatCircles(vc: BaseController, locs: List<LocationInfo>, stride: Int, offset: Int) {
        val shapes = ArrayList<Shape>()

        for (ii in offset until locs.size step stride) {
            val circle = ShapeGreatCircle()
            val loc0 = locs[ii]
            val loc1 = locs[(ii+1)%locs.size]
            circle.setPoints(Point2d.FromDegrees(loc0.lon,loc0.lat),Point2d.FromDegrees(loc1.lon,loc1.lat))
            circle.isSelectable = true
            val angle = circle.angleBetween()
            circle.setHeight(0.3 * angle / PI)
            shapes.add(circle)
        }

        val shapeInfo = ShapeInfo().apply {
            setColor(1.0f, 0.1f, 0f, 1.0f)
            drawPriority = 1000000
            setLineWidth(8.0f)
            //setFade(1.0f);
            setZBufferRead(globeController != null)
        }
        vc.addShapes(shapes, shapeInfo, ThreadMode.ThreadAny)
    }

    private fun addCircles(vc: BaseController, locs: List<LocationInfo>, stride: Int, offset: Int) {
        val shapes = ArrayList<Shape>()
        for (ii in offset until locs.size step stride) {
            shapes.add(ShapeCircle().apply {
                setLoc(Point2d.FromDegrees(locs[ii].lon, locs[ii].lat))
                setHeight(0.01)
                setRadius(0.03)
                setSample(50)
                isSelectable = true
            })
        }

        val shapeInfo = ShapeInfo().apply {
            setColor(0f, 1.00f, 0f, 0.8f)
            drawPriority = 1000000
            //setFade(1.0f);
            setZBufferRead(globeController != null)
        }
        vc.addShapes(shapes, shapeInfo, ThreadMode.ThreadAny)
    }
    
    private fun addRectangles(vc: BaseController, locs: List<LocationInfo>, stride: Int, offset: Int) {
        val bmp = BitmapFactory.decodeResource(activity.resources, R.drawable.teardrop)
        val tex = vc.addTexture(bmp, null, ThreadMode.ThreadCurrent)
        
        val shapes = ArrayList<Shape>()
        for (ii in offset until locs.size step stride) {
            shapes.add(ShapeRectangle().apply {
                val p = Point2d.FromDegrees(locs[ii].lon, locs[ii].lat)
                setPoints(Point3d(p.x-0.05, p.y-0.05, 0.01),
                          Point3d(p.x+0.05, p.y+0.05, 0.1))
                addTexture(tex)
                isSelectable = true
            })
        }
        
        val shapeInfo = ShapeInfo().apply {
            setColor(0f, 1.00f, 0f, 0.8f)
            drawPriority = 1000000
            //setFade(1.0f);
            setZBufferRead(globeController != null)
        }
        vc.addShapes(shapes, shapeInfo, ThreadMode.ThreadAny)
    }
    
    private fun addArrows(vc: BaseController, locs: List<LocationInfo>, stride: Int, offset: Int) {
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
            arrow.setLoc(Point2d.FromDegrees(locs[ii].lon,locs[ii].lat))
            arrow.setOutline(coords)
            arrow.setThickness(size*1.0)
            arrow.setHeight(0.0)
            // Scale it down to meters
            arrow.scale = 1.0/6371000.0
            shapes.add(arrow)
        }

        val shapeInfo = ShapeInfo().apply {
            setColor(1.0f, 0.1f, 0f, 0.8f)
            drawPriority = 1000000
            //        shapeInfo.setFade(1.0f);
            setZBufferRead(globeController != null)
        }
        vc.addShapes(shapes, shapeInfo, ThreadMode.ThreadAny)
    }
    
    private fun addLinears(vc: BaseController, locs: List<LocationInfo>, stride: Int, offset: Int) {
        val n = 50
        val maxH = 0.1
        // todo: doesn't handle anti-meridian, use GeographicLib instead of lerp
        val shapes = listOf(1,8,17) //indexes(locs.size, stride, offset)
                .zipWithNext { a, b ->
            val pa = locs[a].point
            val pb = locs[b].point
            ShapeLinear().apply {
                setCoords(IntRange(0, 2 * n).map {
                    val p = pa.addTo(pb.subtract(pa).multiplyBy(it.toDouble() / n / 2))
                    val s = sin(it * 4 * Math.PI / n) * 0.2
                    val h = (s + 1 - (it - n) * (it - n).toDouble() / n / n) * maxH
                    Point3d(p.x, p.y, h)
                }.toTypedArray())
            }
        }
        val shapeInfo = ShapeInfo().apply {
            setColor(1.0f, 0.5f, 0.2f, 0.9f)
            drawPriority = 1000000
            setZBufferRead(globeController != null)
        }
        vc.addShapes(shapes.toList(), shapeInfo, ThreadMode.ThreadAny)
    }

    private fun indexes(count: Int, stride: Int, offset: Int) = sequence {
        for (ii in offset until count step stride) yield(ii)
    }

    private val baseView = StamenRemoteTestCase(activity).apply {
        doColorChange = false
    }
}
