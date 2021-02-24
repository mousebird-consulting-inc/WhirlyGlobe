/*
 *  LocationTracker.kt
 *  WhirlyGlobeLib
 *
 *  Created by Tim Sylvester
 *  Copyright 2021 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

package com.mousebird.maply

import android.Manifest.permission
import android.content.Context
import android.graphics.*
import android.graphics.Shader
import android.os.*
import androidx.annotation.RequiresPermission
import androidx.core.graphics.*
import com.google.android.gms.location.*
import com.google.android.gms.location.LocationRequest.*
import com.google.android.gms.tasks.Task
import java.lang.ref.WeakReference
import kotlin.math.*

class LocationTracker : LocationCallback {

    /**
     * MaplyLocationTracker constructor
     *
     * @param mapController The globe or map view controller
     * @param trackerDelegate Delegate for location tracking
     * @param useHeading Use location services heading information
     */
    constructor(mapController: BaseController,
                trackerDelegate: LocationTrackerDelegate? = null,
                useHeading: Boolean = true) :
            this(mapController, trackerDelegate, null, 1.0, useHeading)

    /**
     * MaplyLocationTracker constructor
     *
     * @param mapController The globe or map view controller
     * @param trackerDelegate Delegate for location tracking
     * @param simulatorDelegate Delegate for simulated location
     * @param updateInterval Seconds between simulation updates
     * @param useHeading Use location services heading information (requires physical magnetometer)
     */
    constructor(mapController: BaseController,
                trackerDelegate: LocationTrackerDelegate? = null,
                simulatorDelegate: LocationSimulatorDelegate? = null,
                updateInterval: Double = 1.0,
                useHeading: Boolean = true)
    {
        this.baseController = WeakReference(mapController)
        this.trackerDelegate = WeakReference<LocationTrackerDelegate>(trackerDelegate)
        this.simulatorDelegate = WeakReference<LocationSimulatorDelegate>(simulatorDelegate)
        this.updateInterval = updateInterval
        this.useHeading = useHeading
    }

    /**
     * Start location tracking/simulation
     */
    @RequiresPermission(anyOf = [permission.ACCESS_COARSE_LOCATION, permission.ACCESS_FINE_LOCATION])
    fun start(context: Context, looper: Looper? = null, request: LocationRequest? = null) {
        stop()
    
        val defLooper = baseController.get()?.workingThread?.looper ?: Looper.myLooper()!!
        handler = Handler(looper ?: defLooper)
        
        simulatorDelegate.get()?.let {
            simulating = true
            handler?.post(simTask)
        } ?: trackerDelegate.get()?.let {
            locationClient = LocationServices.getFusedLocationProviderClient(context)?.also { client ->
                val req = request ?: LocationRequest().apply {
                    priority = PRIORITY_BALANCED_POWER_ACCURACY
                    interval = (1000.0 * updateInterval).toLong()
                    maxWaitTime = 2 * interval - 1
                    numUpdates = Int.MAX_VALUE
                }
                locationTask = client.requestLocationUpdates(req, this, handler?.looper)
            }
        }
    }

    /**
     * Stop location tracking/simulation
     */
    fun stop() {

        simulating = false
        handler?.removeCallbacks(simTask)
        handler = null

        if (locationTask != null) {
            locationClient?.removeLocationUpdates(this)
            locationTask = null
            locationClient = null
        }

        removeComponentObjects()
        clearInfoObjects()
        clearMarkerImages()
    }

    /**
     * Min visibility for the marker assigned to follow location.
     */
    var markerMinVis = 0.0
        set(value) {
            if (field != value) {
                field = value
                clearInfoObjects()
            }
        }

    /**
     * Max visibility for the marker assigned to follow location.
     */
    var markerMaxVis = 1.0
        set(value) {
            if (field != value) {
                field = value
                clearInfoObjects()
            }
        }

    var markerSize = 32
        set(value) {
            // Limit to at least 16 and ensure that it's even
            val newValue = (value.coerceAtLeast(8))
            if (newValue != field) {
                field = newValue;
                imagesInvalidated = true
            }
        }

    var markerColorInner = Color.WHITE
        set(value) {
            if (field != value) {
                field = value
                imagesInvalidated = true
            }
        }

    var markerColorOuter = Color.argb(255, 0, 192, 255)
        set(value) {
            if (field != value) {
                field = value
                imagesInvalidated = true
            }
        }

    var markerColorOutline = Color.WHITE
        set(value) {
            if (field != value) {
                field = value
                imagesInvalidated = true
            }
        }

    var markerColorShadow = Color.argb(48, 0, 0, 0)
        set(value) {
            if (field != value) {
                field = value
                imagesInvalidated = true
            }
        }

    var accuracyCircleColor = Color.argb(52, 15, 15, 26)
        set(value) {
            if (field != value) {
                field = value
                clearInfoObjects()
            }
        }
    
    var trackerDelegate = WeakReference<LocationTrackerDelegate>(null)
    
    var simulatorDelegate = WeakReference<LocationSimulatorDelegate>(null)
    
    /**
     * Draw priority for the marker assigned to follow location.
     */
    var markerDrawPriority: Int = RenderController.VectorDrawPriorityDefault + 1
        set(value) {
            if (field != value) {
                field = value
                clearInfoObjects()
            }
        }

    /**
     * Location lock type
     */
    var lockType: MaplyLocationLockType = MaplyLocationLockType.MaplyLocationLockNone

    /**
     * Forward track offset, for lock type MaplyLocationLockHeadingUpOffset
     */
    var forwardTrackOffset: Int = 0

    /**
     * Get the current device location
     */
    var lastLocation: LocationTrackerPoint? = null

    /**
     * Set the current location.
     */
    fun updateLocation(location: LocationTrackerPoint?) {
        if (!locationUpdatePending) {
            locationUpdatePending = true
            handler?.post {
                try {
                    updateLocationInternal(location)
                } catch (ex: Exception) {
                    println(ex.localizedMessage)
                }
            }
        }
    }

    override fun onLocationAvailability(availability: LocationAvailability?) {
        availability.let { super.onLocationAvailability(it) }
    
        if (baseController.get() == null) {
            stop()
            return
        }
        
        // Call the delegate if its present, allowing it to update the result
        val tracker = trackerDelegate.get()
        val a = if (tracker != null) tracker.locationManagerDidChangeAuthorizationStatus(this, availability) else availability
        if (a?.isLocationAvailable != true) {
            updateLocation(null)
        }
    }

    override fun onLocationResult(location: LocationResult?) {
        super.onLocationResult(location)
    
        if (baseController.get() != null) {
            updateLocation(convertIf(location?.lastLocation))
        } else {
            stop()
        }
    }

    private fun convertIf(loc: android.location.Location?): LocationTrackerPoint? {
        return if (loc != null) convert(loc) else null
    }
    private fun convert(loc: android.location.Location): LocationTrackerPoint {
        return LocationTrackerPoint().apply {
            latDeg = loc.latitude
            lonDeg = loc.longitude
            horizontalAccuracy = if (loc.hasAccuracy()) loc.accuracy.toDouble() else null
            elevation = if (loc.hasAltitude()) loc.altitude else null
            headingDeg = if (loc.hasBearing()) loc.bearing.toDouble() else null
            speed = if (loc.hasSpeed()) loc.speed.toDouble() else null

            if (Build.VERSION.SDK_INT > 26) {
                verticalAccuracy = if (loc.hasVerticalAccuracy()) loc.verticalAccuracyMeters.toDouble() else null
                headingAccuracy = if (loc.hasBearingAccuracy()) loc.bearingAccuracyDegrees.toDouble() else null
                speedAccuracy = if (loc.hasSpeedAccuracy()) loc.speedAccuracyMetersPerSecond.toDouble() else null
            }
        }
    }

    private fun updateLocationInternal(location: LocationTrackerPoint?) {
        locationUpdatePending = false

        val vc = baseController.get()
        if (vc == null) {
            stop()
            return
        }

        // Call the delegate, allowing it to update the result
        val tracker = trackerDelegate.get()
        val endLoc = if (tracker != null) tracker.locationManagerDidUpdateLocation(this, location) else location

        prevLocation = lastLocation
        lastLocation = endLoc

        // If the latest is no location, we're done
        if (endLoc == null) {
            removeComponentObjects()
            return
        }

        // If something has changed, we need to remove and re-create the marker image textures.
        // Hold onto them until after the objects using them are removed, though.
        var oldMarkerImages: Array<MaplyTexture>? = null
        var oldDirectionalImages: Array<MaplyTexture>? = null
        if (imagesInvalidated) {
            oldMarkerImages = markerImages
            oldDirectionalImages = directionalImages
            markerImages = null
            directionalImages = null
            imagesInvalidated = false;
        }

        try {
            // Create textures and info objects, if they don't already exist
            setupMarkerImages()
            val circleInfo = circleInfo ?: return
            val markerInfo = markerInfo ?: return
            val movingMarkerInfo = movingMarkerInfo ?: return

            val now = System.currentTimeMillis() / 1000.0

            // If we have a previous location, animate starting there
            val startLoc: LocationTrackerPoint = prevLocation ?: endLoc

            val circle = circleForLocation(endLoc) ?: return

            val orientation = if (useHeading) endLoc.headingDeg else null

            val movingMarker = ScreenMovingMarker()
            movingMarker.loc = Point2d.FromDegrees(startLoc.lonDeg, startLoc.latDeg)
            movingMarker.endLoc = Point2d.FromDegrees(endLoc.lonDeg, endLoc.latDeg)
            movingMarker.duration = updateInterval / 2.0
            movingMarker.period = 1.0
            movingMarker.size = Point2d(markerSize.toDouble(),
                    markerSize.toDouble())
            movingMarker.rotation = degToRad(if (orientation != null) -orientation else 0.0)
            movingMarker.images = if (orientation != null) directionalImages else markerImages
            movingMarker.layoutImportance = Float.MAX_VALUE
            movingMarkerInfo.setEnableTimes(now, now + movingMarker.duration)

            val marker = ScreenMarker()
            marker.loc = movingMarker.endLoc
            marker.period = 1.0
            marker.size = movingMarker.size
            marker.rotation = movingMarker.rotation
            marker.images = movingMarker.images
            marker.layoutImportance = Float.MAX_VALUE
            markerInfo.setEnableTimes(now + movingMarker.duration, 0.0)

            // Capture old object refs
            val objs = arrayOf(markerObj, movingMarkerObj, circleObj).filterNotNull()

            try {
                // Add new objects
                circleObj = vc.addShapes(listOf(circle), circleInfo, threadCurrent)
                movingMarkerObj = vc.addScreenMovingMarkers(listOf(movingMarker), movingMarkerInfo, threadCurrent)
                markerObj = vc.addScreenMarker(marker, markerInfo, threadCurrent)
            } finally {
                // Remove old objects
                if (objs.isNotEmpty()) {
                    vc.removeObjects(objs, threadCurrent)
                }
            }
        } finally {
            // Clean up the old versions of the marker image textures
            oldMarkerImages?.let {
                vc.removeTextures(it.toMutableList(), threadAny)
            }
            oldDirectionalImages?.let {
                vc.removeTextures(it.toMutableList(), threadAny)
            }
        }

        lockToLocation(endLoc)
    }

    private fun lockToLocation(loc: LocationTrackerPoint) {
        val map = (baseController.get() as? MapController)
        val globe = (baseController.get() as? GlobeController)
        if (map == null && globe == null) {
            stop()
            return
        }

        val gp = Point2d.FromDegrees(loc.lonDeg, loc.latDeg)
        val animTime = updateInterval / 2.0
        val hdg = -degToRad((360 + (loc.headingDeg ?: 0.0)) % 360)

        var lockType = lockType
        if (loc.headingDeg == null &&
                (lockType == MaplyLocationLockType.MaplyLocationLockHeadingUp ||
                 lockType == MaplyLocationLockType.MaplyLocationLockHeadingUpOffset)) {
            lockType = MaplyLocationLockType.MaplyLocationLockNorthUp
        }

        when (lockType) {
            MaplyLocationLockType.MaplyLocationLockNorthUp -> {
                if (map != null) {
                    map.animatePositionGeo(gp.x, gp.y, map.positionGeo.z, animTime)
                } else {
                    globe?.animatePositionGeo(gp.x, gp.y, globe.viewState.height, animTime)
                }
            }
            MaplyLocationLockType.MaplyLocationLockHeadingUp -> {
                if (map != null) {
                    map.animatePositionGeo(gp.x, gp.y, map.positionGeo.z, hdg, animTime)
                } else {
                    globe?.animatePositionGeo(gp.x, gp.y, globe.viewState.height, -hdg, animTime)
                }
            }
            MaplyLocationLockType.MaplyLocationLockHeadingUpOffset -> {
                val offset = Point2d(0.0, -forwardTrackOffset.toDouble())
                if (map != null) {
                    val target = Point3d(gp.x, gp.y, map.positionGeo.z)
                    map.animatePositionGeo(target, offset, hdg, animTime)
                } else {
                    // TODO: Add animateToPosition:onScreen
                    val target = Point3d(gp.x, gp.y, globe!!.viewState.height)
                    globe.animatePositionGeo(target, offset, -hdg, animTime)
                }
            }
            else -> {}
        }
    }

    private fun setupMarkerImages() {
        if (markerImages == null || directionalImages == null) {
            baseController.get()?.let { vc ->
                markerImages = (0..16).map {
                        radialGradientMarker(vc, markerSize * 2, it, false)
                    }.toTypedArray()
                directionalImages = (0..16).map {
                        radialGradientMarker(vc, markerSize * 2, it, true)
                }.toTypedArray()
            }
        }

        if (markerInfo == null) {
            markerInfo = MarkerInfo().apply {
                setVisibleHeightRange(markerMinVis, markerMaxVis)
                setFade(0.0)
                drawPriority = markerDrawPriority
                setEnableTimes(0.0, Double.MAX_VALUE)
            }
        }
        if (movingMarkerInfo == null) {
            movingMarkerInfo = MarkerInfo().apply {
                setVisibleHeightRange(markerMinVis, markerMaxVis)
                setFade(0.0)
                drawPriority = markerDrawPriority
                setEnableTimes(0.0, Double.MAX_VALUE)
            }
        }
        if (circleInfo == null) {
            circleInfo = ShapeInfo().apply {
                drawPriority = markerDrawPriority - 1
                setColor(accuracyCircleColor.red / 255f, accuracyCircleColor.green / 255f,
                        accuracyCircleColor.blue / 255f, accuracyCircleColor.alpha / 255f)
                setFade(0.0)
                // Prevent the circle from being occluded by other stuff
                setZBufferRead(false)
                // Use a no-light shader so the circle doesn't look different in different geographic locations
                setShader(baseController.get()?.getShader(com.mousebird.maply.Shader.NoLightTriangleShader))
            }
        }
    }

    private fun clearMarkerImages() {
        baseController.get()?.let { vc ->
            markerImages?.let {
                vc.removeTextures(it.toMutableList(), threadAny)
            }
            directionalImages?.let {
                vc.removeTextures(it.toMutableList(), threadAny)
            }
        }
        markerImages = null
        directionalImages = null
    }

    private fun clearInfoObjects() {
        markerInfo = null
        movingMarkerInfo = null
        circleInfo = null
    }

    private fun removeComponentObjects() {
        val objs = arrayOf(markerObj, movingMarkerObj, circleObj).filterNotNull()
        baseController.get()?.removeObjects(objs, threadCurrent)
        markerObj = null
        movingMarkerObj = null
        circleObj = null
    }

    @Suppress("SameParameterValue")
    private fun markerGradLoc(n: Int, radius: Int, fraction: Int = 4): Float {
        return (radius / fraction - abs(radius / fraction - n)) * fraction.toFloat() / radius
    }
    @Suppress("SameParameterValue")
    private fun markerGradRad(n: Int, size: Int, radius: Int, fraction: Int = 4): Float {
        return (size - radius - abs((radius / fraction) - n)) / 2.0f
    }

    @Suppress("SameParameterValue")
    private fun radialGradientMarker(vc: BaseController, size: Int,
                                     idx: Int, directional: Boolean): MaplyTexture {
        val gradLoc = markerGradLoc(idx, markerSize, 4)
        val gradRad = markerGradRad(idx, size, markerSize, 4)
        val outlineWidth = (markerSize / 8f).coerceAtLeast(1f).coerceAtMost(10f)
        val image = radialGradientMarkerImage(size, gradLoc, gradRad, outlineWidth, directional)
        return vc.addTexture(image, RenderControllerInterface.TextureSettings(), RenderControllerInterface.ThreadMode.ThreadCurrent)
    }

    @Suppress("SameParameterValue")
    private fun radialGradientMarkerImage(size: Int,
                                          gradLocation: Float, gradRadius: Float,
                                          outlineWidth: Float,
                                          directional: Boolean): Bitmap {

        val image = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888)
        image.eraseColor(Color.TRANSPARENT)

        val canvas = Canvas(image)

        val paint = Paint(Paint.ANTI_ALIAS_FLAG.or(Paint.FILTER_BITMAP_FLAG))
        paint.color = markerColorShadow
        canvas.drawOval(0f, 0f, size.toFloat(), size.toFloat(), paint)

        val radius = size / 2.0f

        if (directional) {
            val len = size * 5 / 16f
            val width = size * 3 / 16f
            //paint.color = markerColorOuter
            //paint.alpha = Color.alpha(markerColorOuter)
            val vertexes = floatArrayOf(radius, radius - gradRadius - len,
                    radius - width, radius - gradRadius,
                    radius + width, radius - gradRadius)
            val indexes = shortArrayOf(0, 1, 2)
            val colors = intArrayOf(markerColorOuter, markerColorOuter, markerColorOuter)
            canvas.drawVertices(Canvas.VertexMode.TRIANGLES,
                    vertexes.size, vertexes, 0,
                    null, 0,
                    colors, 0,
                    indexes, 0, indexes.size,
                    paint)
        }

        paint.color = markerColorOutline
        canvas.drawOval(radius - gradRadius - outlineWidth, radius - gradRadius - outlineWidth,
                radius + gradRadius + outlineWidth, radius + gradRadius + outlineWidth,
                paint)

        val gradientPaint = Paint(Paint.ANTI_ALIAS_FLAG.or(Paint.FILTER_BITMAP_FLAG))
        gradientPaint.shader = RadialGradient(radius, radius,
                (gradLocation * radius).coerceAtLeast(0.1f),
                markerColorInner, markerColorOuter, Shader.TileMode.CLAMP)
        canvas.drawOval(radius - gradRadius, radius - gradRadius,
                radius + gradRadius, radius + gradRadius,
                gradientPaint)

        return image
    }

    private fun circleForLocation(location: LocationTrackerPoint?, defRadius: Double? = null): ShapeCircle? {
        if (location == null) return null
        val vc = baseController.get() ?: return null
        val center = Point2d.FromDegrees(location.lonDeg, location.latDeg)

        val radius = location.horizontalAccuracy ?: defRadius ?: 0.0
        if (radius <= 0.0) {
            return null
        }

        val circle = ShapeCircle()
        circle.setLoc(center)
        circle.setSample(100)

        val top = coordOfPointAtTrueCourse(center, courseDeg = 0.0, distanceMeters = radius)
        val right = coordOfPointAtTrueCourse(center, courseDeg = 90.0, distanceMeters = radius)

        val displayPtCenter = vc.displayPointFromGeo(Point3d(center.x, center.y, 0.0))
        val displayPtTop = vc.displayPointFromGeo(Point3d(top.x, top.y, 0.0))
        val displayPtRight = vc.displayPointFromGeo(Point3d(right.x, right.y, 0.0))

        val vRad = hypot(displayPtTop.x - displayPtCenter.x, displayPtTop.y - displayPtCenter.y)
        val hRad = hypot(displayPtRight.x - displayPtCenter.x, displayPtRight.y - displayPtCenter.y)
        circle.setRadius((vRad + hRad) / 2.0)

        circle.setHeight(vc.zoomLimitMin / 100.0)

        return circle
    }

    private fun degToRad(deg: Double): Double { return deg * Math.PI / 180.0 }
    //private fun radToDeg(rad: Double): Double { return rad * 180.0 / Math.PI }

    private fun coordOfPointAtTrueCourse(center: Point2d, courseDeg: Double, distanceMeters: Double): Point2d {
        // http://www.movable-type.co.uk/scripts/latlong.html
        val tcRad = degToRad(courseDeg)
        val lat1 = center.y
        val lon1 = center.x
        val earthRadMeters = 6371008.7714
        val dRad = distanceMeters / earthRadMeters
        val latRad = asin(sin(lat1) * cos(dRad) + cos(lat1) * sin(dRad) * cos(tcRad))
        val cosLat = cos(latRad)
        val lonRad = if (cosLat == 0.0) lon1 else ((lon1 - asin(sin(tcRad) * sin(dRad) / cos(latRad)) + Math.PI) % (2.0 * Math.PI)) - Math.PI
        return Point2d(lonRad, latRad)
    }

    private val simTask = object : Runnable {
        override fun run() {
            if (baseController.get() == null) {
                stop()
                return
            }
            
            simulatorDelegate.get()?.let {
                updateLocation(it.locationSimulatorGetLocation())
            }
            if (simulating) {
                handler?.postDelayed(this, (updateInterval * 1000).toLong())
            }
        }
    }

    private val baseController: WeakReference<BaseController>
    private val useHeading: Boolean

    private var markerImages: Array<MaplyTexture>? = null
    private var directionalImages: Array<MaplyTexture>? = null
    private var imagesInvalidated = false

    private var markerInfo: MarkerInfo? = null
    private var movingMarkerInfo: MarkerInfo? = null
    private var circleInfo: ShapeInfo? = null

    private var markerObj: ComponentObject? = null
    private var movingMarkerObj: ComponentObject? = null
    private var circleObj: ComponentObject? = null

    private var simulating = false
    private var updateInterval = 1.0
        set (value) {
            field = value.coerceAtLeast(0.1)
        }

    private var locationClient: FusedLocationProviderClient? = null
    private var locationTask: Task<Void>? = null
    private var locationUpdatePending = false
    private var prevLocation: LocationTrackerPoint? = null

    private var handler: Handler? = null

    private val threadCurrent = RenderControllerInterface.ThreadMode.ThreadCurrent
    private val threadAny = RenderControllerInterface.ThreadMode.ThreadAny
}