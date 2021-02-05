package com.mousebird.maply

import com.google.android.gms.location.LocationAvailability

interface LocationTrackerDelegate {
    fun locationManagerDidFailWithError(tracker: LocationTracker, error: String) {
    }
    fun locationManagerDidChangeAuthorizationStatus(tracker: LocationTracker, availability: LocationAvailability?): LocationAvailability? {
        return availability
    }
    fun locationManagerDidUpdateLocation(tracker: LocationTracker, location: LocationTrackerPoint?): LocationTrackerPoint? {
        return location
    }
}