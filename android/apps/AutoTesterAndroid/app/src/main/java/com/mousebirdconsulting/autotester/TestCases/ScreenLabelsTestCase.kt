package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.graphics.Color
import android.graphics.Typeface
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import java.util.*

import kotlin.jvm.Throws

class ScreenLabelsTestCase(activity: Activity?) :
        MaplyTestCase(activity, "Screen Labels", TestExecutionImplementation.Both) {

    @Throws(Exception::class)
    override fun setUpWithGlobe(globeVC: GlobeController): Boolean {
        baseCase.setUpWithGlobe(globeVC)
        baseCase.baseCase.setForwardGlobeDelegate(this)
        insertLabels(baseCase.vectors, globeVC)
        val loc = Point2d.FromDegrees(-74.075833, 4.598056)
        globeVC.animatePositionGeo(loc.x, loc.y, 3.0, 1.0)
        globeVC.keepNorthUp = false
        return true
    }
    
    @Throws(Exception::class)
    override fun setUpWithMap(mapVC: MapController): Boolean {
        baseCase.setUpWithMap(mapVC)
        insertLabels(baseCase.vectors, mapVC)
        val loc = Point2d.FromDegrees(-74.075833, 4.598056)
        mapVC.setPositionGeo(loc.x, loc.y, 3.0)
        mapVC.setAllowRotateGesture(true)
        return true
    }

    override fun userDidTap(mapControl: MapController?, loc: Point2d?, screenLoc: Point2d?) {
        super.userDidTap(mapControl, loc, screenLoc)
        controller.showDebugLayoutBoundaries = !controller.showDebugLayoutBoundaries
    }

    override fun userDidTap(globeControl: GlobeController?, loc: Point2d?, screenLoc: Point2d?) {
        super.userDidTap(globeControl, loc, screenLoc)
        controller.showDebugLayoutBoundaries = !controller.showDebugLayoutBoundaries
    }
    
    override fun shutdown() {
        controller.removeObjects(componentObjects, ThreadMode.ThreadCurrent)
        componentObjects.clear()
        baseCase.shutdown()
        super.shutdown()
    }
    
    // Make a screen label at a given location (in degrees)
    private fun makeLabel(lonDeg: Double, latDeg: Double,
                          inText: String?, importance: Float, rot: Float = 0.0f) =
        ScreenLabel().apply {
            loc = Point2d.FromDegrees(lonDeg, latDeg)
            text = inText
            layoutImportance = importance
            rotation = rot.toDouble()
        }
    
    private fun insertLabels(objects: ArrayList<VectorObject>, baseVC: BaseController) {
        val labelInfo = LabelInfo().apply {
            fontSize = 32f
            textColor = Color.BLACK
            backgroundColor = Color.BLUE
            typeface = Typeface.DEFAULT
            outlineColor = Color.WHITE
            outlineSize = 2f
            layoutImportance = 1f //Float.MAX_VALUE

            setLayoutPlacement(LabelInfo.LayoutRight or LabelInfo.LayoutCenter)
            setTextJustify(LabelInfo.TextJustify.TextLeft)

            //setMinVis(0.f);
            //setMaxVis(2.5f);
        }
        
        val markerInfo = MarkerInfo().apply {
            drawPriority = labelInfo.drawPriority + 1
        }

        val labels = ArrayList<ScreenLabel>()
        val markers = ArrayList<ScreenMarker>()
        var i = 0
        for (vecObj in objects) {
            vecObj.attributes?.let { attrs ->
                val labelName = attrs.getString("ADMIN")
                if (labelName != null && labelName.isNotEmpty()) {
                    (vecObj.centroid() ?: vecObj.center())?.let { center ->
                        labels.add(ScreenLabel().apply {
                            text = labelName
                            loc = center
                            selectable = true
                            //layoutImportance = 1.f
                            //layoutImportance = Float.MAX_VALUE
                            //layoutSize = new Point2d(1000.0f, 256.f )
                            offset = Point2d(10 * center.x / Math.PI / 2,
                                             10 * center.y / Math.PI)
                            if (center.y > 0) {
                                rotation = center.x / 8
                            }
                            if ((i % 2) == 0) {
                                // shadow and outline stuff requires separate LabelInfo objects
                                //shadowColor =
                                //shadowSize =
                                layoutPlacement = LabelInfo.LayoutCenter
                            } else {
                                //outline
                                layoutPlacement = LabelInfo.LayoutBelow
                            }
                        })
                        if (addMarkers) {
                            markers.add(ScreenMarker().apply {
                                loc = center
                                size = Point2d(8.0, 8.0)
                            })
                        }
                    }
                }
            }
            i += 1
        }
        
        // Toss in a few with explicit diacritics
        labels.add(makeLabel(-74.075833, 4.4, "Bogotá", 2f, Math.PI.toFloat() / 4.0f))
        labels.add(makeLabel(-74.075833, 4.598056, "Bogotá2", 2f))
        labels.add(makeLabel(6.0219, 47.2431, "Besançon", 2f))
        labels.add(makeLabel(4.361, 43.838, "Nîmes", 2f))
        labels.add(makeLabel(4.9053, 43.9425, "Morières-lès-Avignon", 2f))
        labels.add(makeLabel(11.616667, 44.833333, "Ferrara", 2f))
        labels.add(makeLabel(7.0, 49.233333, "Saarbrücken", 2f))
        labels.add(makeLabel(0.0, 0.0, "abcdef\nghijklmn\nopqrstu\nvwxyzA\nBCDEF\nGHIJKLMN\nOPQRTST\nUVWXYZ", 10f))
        if (addMarkers) {
            markers.add(ScreenMarker().apply {
                loc = Point2d.FromDegrees(0.0, 0.0)
                size = Point2d(8.0, 8.0)
            })
        }
        labels.add(makeLabel(1.0, -5.0, "abcdef\nghijklmn\nopqrstu\nvwxyz", 10f))
        if (addMarkers) {
            markers.add(ScreenMarker().apply {
                loc = Point2d.FromDegrees(1.0, -5.0)
                size = Point2d(8.0, 8.0)
            })
        }
        labels.add(makeLabel(4.361, 43.838, "Nîmes", Float.MAX_VALUE).also {
            // Tall and skinny for testing
            it.layoutSize = Point2d(1.0, 200.0)
        })
        
        // Test the layout engine very explicitly
        //int i = 0;
        //for (double o = 0.0; o < 0.5; o += 0.05) {
        //  labels.add(makeLabel(4.361 + o, 43.838 + o, "Nîmes " + ++i, Float.MAX_VALUE));
        //}

        baseVC.addScreenLabels(labels, labelInfo, ThreadMode.ThreadAny)?.let {
            componentObjects.add(it)
        }
        if (addMarkers) {
            baseVC.addScreenMarkers(markers, markerInfo, ThreadMode.ThreadAny)?.let {
                componentObjects.add(it)
            }
        }
    }
    
    private val componentObjects = ArrayList<ComponentObject>()
    private val baseCase = VectorsTestCase(getActivity())
    
    companion object {
        // If set, we'll put markers around the points for debugging
        var addMarkers = true
    }
}