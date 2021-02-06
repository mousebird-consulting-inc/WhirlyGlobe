package com.mousebirdconsulting.autotester.TestCases

import android.annotation.SuppressLint
import android.app.Activity
import android.os.Looper
import com.mousebird.maply.LocationSimulatorDelegate
import com.mousebird.maply.LocationTracker

class LocationTrackingSimTestCase : LocationTrackingRealTestCase, LocationSimulatorDelegate {

    constructor(activity: Activity) : super(activity)
    {
        setTestName("Location Tracking - Simulated")
        implementation = TestExecutionImplementation.Both
    }

    @SuppressLint("MissingPermission")
    override fun setUpTracker() {
        tracker?.also {
            it.stop()
        }
        tracker = null

        val context = activity.applicationContext
        baseViewC?.also { vc ->
            tracker = LocationTracker(vc, this, this,
                    updateInterval = 0.5, useHeading = true)?.also {
                it.start(context, Looper.myLooper())
            }
        }
    }
}