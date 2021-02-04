package com.mousebird.maply

import com.google.android.gms.location.LocationAvailability

interface MaplyLocationTrackerDelegate {
    fun locationManagerDidFailWithError(tracker: MaplyLocationTracker, error: String) {
    }
    fun locationManagerDidChangeAuthorizationStatus(tracker: MaplyLocationTracker, availability: LocationAvailability) {
    }
    fun locationManagerDidUpdateLocation(tracker: MaplyLocationTracker, location: MaplyLocationTrackerPoint) {
    }
}