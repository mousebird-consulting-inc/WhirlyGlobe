package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase

/**
 * Find Height just tests the findHeight logic for the globe and map.
 */
class FindHeightTestCase(activity: Activity) : MaplyTestCase(activity, "Find Height") {
    
    // Run a findHeight
    private fun runDelayedFind(vc: BaseController) {
        vc.addPostSurfaceRunnable {
            val bbox = Mbr(Point2d.FromDegrees(7.05090689853, 47.7675500593),
                           Point2d.FromDegrees(8.06813647023, 49.0562323851))
            val center = Point2d.FromDegrees((7.05090689853+8.06813647023)/2,
                                              (47.7675500593+49.0562323851)/2)
            if (vc is GlobeController) {
                val height = vc.findHeightToViewBounds(bbox, center)
                vc.animatePositionGeo(center.x,center.y,height,5.0)
            } else if (vc is MapController) {
                val height = vc.findHeightToViewBounds(bbox, center)
                vc.animatePositionGeo(center.x,center.y,height,5.0)
            }
        }
    }

    var baseCase = CartoLightTestCase(activity)

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        if (mapVC == null) {
            return false
        }
        baseCase.setUpWithMap(mapVC)

        val loc = Point2d.FromDegrees(-98.58, 39.83)
        mapVC.setPositionGeo(loc.x,loc.y,1.0)

        runDelayedFind(mapVC)

        return true
    }

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        if (globeVC == null) {
            return false
        }
        baseCase.setUpWithGlobe(globeVC)

        val loc = Point2d.FromDegrees(-98.58, 39.83)
        globeVC.setPositionGeo(loc.x,loc.y,1.5)

        runDelayedFind(globeVC)

        return true
    }
    
    override fun shutdown() {
        baseCase.shutdown()
        super.shutdown()
    }
}