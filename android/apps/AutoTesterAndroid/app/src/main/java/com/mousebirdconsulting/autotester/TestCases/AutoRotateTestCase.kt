package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.graphics.BitmapFactory
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import com.mousebirdconsulting.autotester.R

class AutoRotateTestCase(activity: Activity?) :
        MaplyTestCase(activity, "Auto Rotate Test Case", TestExecutionImplementation.Globe) {

    @Throws(Exception::class)
    override fun setUpWithGlobe(globeVC: GlobeController): Boolean {
        baseCase.setUpWithGlobe(globeVC)
        globeVC.setPositionGeo(Point2d.FromDegrees(0.0, 30.0), 2.0)
        globeVC.setAutoRotate(5f, 45f)
    
        // Test marker updating without user interaction (See #296)
        val icon = BitmapFactory.decodeResource(activity.resources, R.drawable.teardrop_stroked)
        val iconTex = globeVC.addTexture(icon, null, ThreadMode.ThreadCurrent)
        val info = MarkerInfo().apply {
            drawPriority = 1000
        }
        objs.add(globeVC.addMarkers(listOf(
                Marker().apply {
                    image = icon
                    loc = Point2d.FromDegrees(90.0, 40.0)
                    size = Point2d(0.2, 0.2)
                    selectable = true
                },
                Marker().apply {
                    image = icon
                    loc = Point2d.FromDegrees(-90.0, 40.0)
                    size = Point2d(0.2, 0.2)
                    selectable = true
                }), info, ThreadMode.ThreadCurrent))

        objs.add(globeVC.addScreenMarkers(listOf(
                ScreenMarker().apply {
                    tex = iconTex
                    loc = Point2d.FromDegrees(0.0, 40.0)
                    size = Point2d(128.0, 128.0)
                    rotation = 0.0
                    selectable = true
                    layoutImportance = 1.0f
                },
                ScreenMarker().apply {
                    tex = iconTex
                    loc = Point2d.FromDegrees(180.0, 40.0)
                    size = Point2d(128.0, 128.0)
                    rotation = 0.0
                    selectable = true
                    layoutImportance = Float.MAX_VALUE
                }), info, ThreadMode.ThreadCurrent))

        return true
    }
    
    override fun shutdown() {
        controller.removeObjects(objs, ThreadMode.ThreadCurrent)
        objs.clear()

        baseCase.shutdown()
        super.shutdown()
    }

    private val objs = ArrayList<ComponentObject>()
    private val baseCase = StamenRemoteTestCase(getActivity())
}