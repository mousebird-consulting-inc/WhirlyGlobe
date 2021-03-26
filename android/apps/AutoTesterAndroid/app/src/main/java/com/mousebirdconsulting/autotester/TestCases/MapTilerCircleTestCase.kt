package com.mousebirdconsulting.autotester.TestCases

import android.app.Activity

class MapTilerCircleTestCase(activity: Activity) :
        MapTilerTestCase(activity, "MapTiler Circles")
{
    override fun getMaps() = listOf("maptiler_test_circles.json")
    
    //override fun getStyleJson(whichMap: Int) =
    //override fun setup(map: MapboxKindaMap) {
    //    super.setup(map)
    //    map.styleSettings.markerImportance = 200000.0 //Float.MAX_VALUE.toDouble()
    //}
}