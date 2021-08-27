package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import com.mousebird.maply.MapController
import com.mousebird.maply.Point2d
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase

/**
 * Created by sjg on 8/23/16.
 */
class BoundsTestCase(activity: Activity?) :
        MaplyTestCase(activity, "Bounds Test", TestExecutionImplementation.Map) {

    @Throws(Exception::class)
    override fun setUpWithMap(mapVC: MapController): Boolean {
        baseCase.apply {
            doColorChange = false
            setUpWithMap(mapVC)
        }
        mapVC.apply {
            setPositionGeo(0.0, 0.0, 0.5)
            //allowPan = false
            //allowZoom = false
            allowRotateGesture = true
            setViewExtents(Point2d.FromDegrees(-60.0, -40.0),
                           Point2d.FromDegrees(60.0, 70.0))
        }
        return true
    }
    
    override fun shutdown() {
        baseCase.shutdown()
        super.shutdown()
    }

    init {
        setDelay(4)
    }
    
    private val baseCase = StamenRemoteTestCase(activity)
}