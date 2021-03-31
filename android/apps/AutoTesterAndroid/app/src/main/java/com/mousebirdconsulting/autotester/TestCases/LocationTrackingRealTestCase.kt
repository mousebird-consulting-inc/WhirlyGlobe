package com.mousebirdconsulting.autotester.TestCases

import android.Manifest
import android.annotation.SuppressLint
import android.app.Activity
import android.content.pm.PackageManager
import android.graphics.Color
import android.widget.Button
import android.widget.FrameLayout
import android.widget.LinearLayout
import androidx.annotation.RequiresPermission
import androidx.core.app.ActivityCompat
import com.mousebird.maply.*
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase
import com.mousebirdconsulting.autotester.R

open class LocationTrackingRealTestCase(activity: Activity) :
        MaplyTestCase(activity, "Location Tracking - Real"),
        LocationTrackerDelegate {
    
    override fun onPreExecute() {

        activity.findViewById<FrameLayout>(R.id.content_frame)?.let { frame ->

            val lin = LinearLayout(activity)
            lin.setBackgroundColor(Color.WHITE)
            lin.layoutParams = LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT)

            val btnLayout = LinearLayout.LayoutParams(frame.width / 5, LinearLayout.LayoutParams.WRAP_CONTENT)

            val noneBtn = Button(activity.applicationContext)
            noneBtn.text = "No Lock"
            noneBtn.layoutParams = btnLayout
            noneBtn.textSize = frame.width / 120f
            noneBtn.isAllCaps = false
            noneBtn.setOnClickListener { _ -> onTrackNone() }
            lin.addView(noneBtn)

            val northBtn = Button(activity.applicationContext)
            northBtn.text = "North Up"
            northBtn.layoutParams = btnLayout
            northBtn.textSize = frame.width / 120f
            northBtn.isAllCaps = false
            northBtn.setOnClickListener { _ -> onTrackNorth() }
            lin.addView(northBtn)

            val hdgBtn = Button(activity.applicationContext)
            hdgBtn.text = "Heading Up"
            hdgBtn.layoutParams = btnLayout
            hdgBtn.textSize = frame.width / 120f
            hdgBtn.isAllCaps = false
            hdgBtn.setOnClickListener { _ -> onTrackHeading() }
            lin.addView(hdgBtn)

            val hdgFwdBtn = Button(activity.applicationContext)
            hdgFwdBtn.text = "Heading Up Forward"
            hdgFwdBtn.layoutParams = btnLayout
            hdgFwdBtn.textSize = frame.width / 120f
            hdgFwdBtn.isAllCaps = false
            hdgFwdBtn.setOnClickListener { _ -> onTrackHeadingForward() }
            lin.addView(hdgFwdBtn)

            frame.addView(lin)
        }
    }

    open fun setUp() {
        val context = activity.applicationContext
        if (ActivityCompat.checkSelfPermission(context, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED &&
                ActivityCompat.checkSelfPermission(context, Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {

            ActivityCompat.requestPermissions(activity, arrayOf(Manifest.permission.ACCESS_FINE_LOCATION, Manifest.permission.ACCESS_COARSE_LOCATION), 0)
            return
        }

        setUpTracker()
    }

    @RequiresPermission(anyOf = [Manifest.permission.ACCESS_COARSE_LOCATION, Manifest.permission.ACCESS_FINE_LOCATION])
    open fun setUpTracker() {
        tracker?.stop()
        tracker = null

        val context = activity.applicationContext
        baseViewC?.let { vc ->
            tracker = LocationTracker(vc, this, useHeading = true).apply {
                lockType = MaplyLocationLockType.MaplyLocationLockNorthUp
                markerSize = 48
                start(context)
            }
        }
    }

    // results of `ActivityCompat.requestPermissions`
    @Suppress("UNUSED_PARAMETER")
    @SuppressLint("MissingPermission")
    fun onRequestPermissionsResult(requestCode: Int, permissions: Array<String>, grantResults: Array<Int>) {
        if (grantResults.contains(PackageManager.PERMISSION_GRANTED)) {
            setUpTracker()
        }
    }

    override fun setUpWithMap(mapVC: MapController?): Boolean {
        baseViewC = mapVC
        baseCase = CartoLightTestCase(getActivity()).apply {
            setUpWithMap(mapVC)
        }
        mapVC?.setPositionGeo(-100 * Math.PI / 180.0, 40 * Math.PI / 180.0, 0.00005)
        setUp()
        return true
    }

    override fun setUpWithGlobe(globeVC: GlobeController?): Boolean {
        baseViewC = globeVC
        baseCase = CartoLightTestCase(getActivity()).apply {
            setUpWithGlobe(globeVC)
        }
        globeVC?.keepNorthUp = false
        globeVC?.setPositionGeo(-100 * Math.PI / 180.0, 40 * Math.PI / 180.0, 0.0001)
        setUp()
        return true
    }

    override fun shutdown() {
        tracker?.stop()
        tracker = null
        super.shutdown()
    }

    private fun onTrackNone() {
        tracker?.let {
            it.lockType = MaplyLocationLockType.MaplyLocationLockNone
        }
    }
    private fun onTrackNorth() {
        tracker?.let {
            it.lockType = MaplyLocationLockType.MaplyLocationLockNorthUp
        }
    }
    private fun onTrackHeading() {
        tracker?.let {
            it.lockType = MaplyLocationLockType.MaplyLocationLockHeadingUp
        }
    }
    private fun onTrackHeadingForward() {
        tracker?.let {
            it.lockType = MaplyLocationLockType.MaplyLocationLockHeadingUpOffset
            it.forwardTrackOffset = 500
        }
    }

    var baseCase: MaplyTestCase? = null
    var baseViewC: BaseController? = null
    var tracker: LocationTracker? = null
}