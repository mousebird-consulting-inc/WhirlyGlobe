package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import android.graphics.BitmapFactory
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import com.mousebirdconsulting.autotester.R

class AutoRotateTestCase(activity: Activity?) :
        MaplyTestCase(activity, "Auto Rotate", TestExecutionImplementation.Globe) {

    @Throws(Exception::class)
    override fun setUpWithGlobe(globeVC: GlobeController): Boolean {
        baseCase.setUpWithGlobe(globeVC)
        globeVC.setPositionGeo(Point2d.FromDegrees(0.0, 30.0), 2.0)
        globeVC.setAutoRotate(5f, 30f)
    
        // Marker/ScreenMarker updates (particularly w/ Layout) without user interaction (See #296)
        val icon = BitmapFactory.decodeResource(activity.resources, R.drawable.teardrop_stroked)
        val info = MarkerInfo()
        objs.add(globeVC.addMarkers(listOf(
                Marker().apply {
                    image = icon
                    loc = Point2d.FromDegrees(90.0, 40.0)
                    size = Point2d(0.2, 0.2)
                }, Marker().apply {
                    image = icon
                    loc = Point2d.FromDegrees(-90.0, 40.0)
                    size = Point2d(0.2, 0.2)
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
    private val baseCase = ScreenMarkersTestCase(getActivity())
}