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
 *
 */

package com.mousebird.maply

import android.Manifest.permission
import android.content.Context
import android.graphics.*
import android.graphics.Shader
import android.os.Build
import android.os.Handler
import android.os.Looper
import androidx.annotation.ColorInt
import androidx.annotation.RequiresPermission
import com.google.android.gms.location.*
import com.google.android.gms.location.LocationRequest.PRIORITY_BALANCED_POWER_ACCURACY
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
    fun start(context: Context, looper: Looper? = null) {
        stop()

        this.looper = looper ?: Looper.myLooper()
        this.handler = Handler(looper)

        simulatorDelegate.get()?.let {
            mainHandler.post(simTask)
        } ?: trackerDelegate.get()?.let {
            locationClient = LocationServices.getFusedLocationProviderClient(context)?.also { client ->
                val req = LocationRequest()
                req.priority = 	PRIORITY_BALANCED_POWER_ACCURACY
                req.interval = (1000.0 * updateInterval).toLong()
                req.maxWaitTime = 2 * req.interval - 1
                req.numUpdates = Int.MAX_VALUE
                locationTask = client.requestLocationUpdates(req, this, this.looper)
            }
        }
    }

    /**
     * Stop location tracking/simulation
     */
    fun stop() {
        locationClient?.also {
            it.removeLocationUpdates(this)
        }
        mainHandler.removeCallbacks(simTask)

        if (locationTask != null) {
            locationClient?.removeLocationUpdates(this)
            locationTask = null
            locationClient = null
        }

        baseController.get()?.also {
            it.removeObjects(arrayOf(markerObj, movingMarkerObj, circleObj).filterNotNull(),
                    RenderControllerInterface.ThreadMode.ThreadCurrent)
            markerObj = null
            movingMarkerObj = null
            circleObj = null
        }
    }

    /**
     * Min visibility for the marker assigned to follow location.
     */
    var markerMinVis = 0.0

    /**
     * Max visibility for the marker assigned to follow location.
     */
    var markerMaxVis = 1.0

    /**
     * Draw priority for the marker assigned to follow location.
     */
    var markerDrawPriority: Int = RenderController.VectorDrawPriorityDefault + 1
        set(value) {
            if (field != value) {
                field = value
                // Recreate the descriptors on next use
                this.markerInfo = null
                this.movingMarkerInfo = null
                this.circleInfo = null
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

        // Call the delegate if its present, allowing it to update the result
        val tracker = trackerDelegate.get()
        val a = if (tracker != null) tracker.locationManagerDidChangeAuthorizationStatus(this, availability) else availability
        if (a?.isLocationAvailable != true) {
            updateLocation(null)
        }
    }

    override fun onLocationResult(location: LocationResult?) {
        super.onLocationResult(location)
        updateLocation(convertIf(location?.lastLocation))
    }

    fun convertIf(loc: android.location.Location?): LocationTrackerPoint? {
        return if (loc != null) convert(loc) else null
    }
    fun convert(loc: android.location.Location): LocationTrackerPoint {
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

    private val threadCurrent = RenderControllerInterface.ThreadMode.ThreadCurrent

    private fun updateLocationInternal(location: LocationTrackerPoint?) {
        locationUpdatePending = false

        val vc = baseController.get() ?: return

        // Call the delegate, allowing it to update the result
        val tracker = trackerDelegate.get()
        val endLoc = if (tracker != null) tracker.locationManagerDidUpdateLocation(this, location) else location

        // Remove previous objects
        vc.removeObjects(arrayOf(markerObj, movingMarkerObj, circleObj).filterNotNull(), threadCurrent)
        markerObj = null
        movingMarkerObj = null
        circleObj = null

        prevLocation = lastLocation
        lastLocation = endLoc

        // If the latest is no location, we're done
        if (endLoc == null) {
            return
        }

        // Create textures and info objects, if they don't already exist
        setupMarkerImages()
        val circleInfo = circleInfo ?: return
        val markerInfo = markerInfo ?: return
        val movingMarkerInfo = movingMarkerInfo ?: return

        // If we have a previous location, animate starting there
        val startLoc: LocationTrackerPoint = prevLocation ?: endLoc

        val circle = circleForLocation(endLoc) ?: return
        circleObj = vc.addShapes(arrayOf(circle).asList(), circleInfo, threadCurrent)

        val orientation = if (useHeading) endLoc.headingDeg else null

        val movingMarker = ScreenMovingMarker()
        movingMarker.loc = Point2d.FromDegrees(startLoc.lonDeg, startLoc.latDeg)
        movingMarker.endLoc = Point2d.FromDegrees(endLoc.lonDeg, endLoc.latDeg)
        movingMarker.duration = updateInterval / 2.0
        movingMarker.period = updateInterval
        movingMarker.size = Point2d(locationTrackerPositionMarkerSize.toDouble(),
                                    locationTrackerPositionMarkerSize.toDouble())
        movingMarker.rotation = degToRad(if (orientation != null) 90.0 - orientation else 0.0)
        movingMarker.images = if (orientation != null) directionalImages else markerImages
        movingMarker.layoutImportance = Float.MAX_VALUE

        val marker = ScreenMarker()
        marker.loc = movingMarker.endLoc
        marker.period = updateInterval
        marker.size = movingMarker.size
        marker.rotation = movingMarker.rotation
        marker.images = movingMarker.images
        marker.layoutImportance = Float.MAX_VALUE

        //marker.images = directionalImages
        //movingMarker.images = directionalImages

        val time = System.currentTimeMillis() / 1000.0 + updateInterval / 2.0
        markerInfo.setEnableTimes(time, Double.MAX_VALUE)
        movingMarkerInfo.setEnableTimes(0.0, time)

        markerObj = vc.addScreenMarker(marker, markerInfo, threadCurrent)
        movingMarkerObj = vc.addScreenMovingMarkers(listOf(movingMarker), movingMarkerInfo, threadCurrent)

        lockToLocation(endLoc)
    }

    private fun lockToLocation(loc: LocationTrackerPoint) {
        val map = (baseController.get() as? MapController)
        val globe = (baseController.get() as? GlobeController)
        if (map == null && globe == null) {
            return
        }

        val gp = Point2d.FromDegrees(loc.lonDeg, loc.latDeg)
        val animTime = updateInterval / 2.0
        val hdg = degToRad((180 + (loc.headingDeg ?: 0.0)) % 360)

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
                // TODO: Add heading to animatePositionGeo
                if (map != null) {
                    map.animatePositionGeo(gp.x, gp.y, map.positionGeo.z, animTime)
                } else {
                    globe?.animatePositionGeo(gp.x, gp.y, globe.viewState.height, animTime)
                }
            }
            MaplyLocationLockType.MaplyLocationLockHeadingUpOffset -> {
                // TODO: Add animateToPosition:onScreen
                if (map != null) {
                    map.animatePositionGeo(gp.x, gp.y, map.positionGeo.z, animTime)
                } else {
                    globe?.animatePositionGeo(gp.x, gp.y, globe.viewState.height, animTime)
                }
            }
            else -> {}
        }
    }

    private fun setupMarkerImages() {
        if (markerImages == null || directionalImages == null) {
            val size = locationTrackerPositionMarkerSize * 2

            val color0 = Color.WHITE
            val color1 = Color.argb(255,0,192,255)

            baseController.get()?.let { vc ->
                markerImages = (0..16).map {
                        radialGradientMarker(vc, size, color0, color1, it, false)
                    }.toTypedArray()
                directionalImages = (0..16).map {
                        radialGradientMarker(vc, size, color0, color1, it, true)
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
                setColor(0.06f, 0.06f, 0.1f, 0.2f)
                setFade(0.0)
                drawPriority = markerDrawPriority - 1
                // TODO: kMaplySampleX: @(100)
                // TODO: kMaplyZBufferRead: @(false)
            }
        }
    }

    private fun markerGradLoc(n: Int, radius: Int, fraction: Int = 4): Float {
        return (radius / fraction - abs(radius / fraction - n)) * fraction.toFloat() / radius
    }
    private fun markerGradRad(n: Int, size: Int, radius: Int, fraction: Int = 4): Float {
        return (size - radius - abs((radius / fraction) - n)) / 2.0f
    }

    private fun radialGradientMarker(vc: BaseController, size: Int,
                                     @ColorInt color0: Int, @ColorInt color1: Int,
                                     idx: Int, directional: Boolean): MaplyTexture {
        val gradLoc = markerGradLoc(idx, locationTrackerPositionMarkerSize, 4)
        val gradRad = markerGradRad(idx, size, locationTrackerPositionMarkerSize, 4)
        val outlineWidth = 4f
        val image = radialGradientMarkerImage(size, color0, color1, gradLoc, gradRad, outlineWidth, directional)
        return vc.addTexture(image, RenderControllerInterface.TextureSettings(), RenderControllerInterface.ThreadMode.ThreadCurrent)
    }

    private fun radialGradientMarkerImage(size: Int,
                                          @ColorInt color0: Int, @ColorInt color1: Int,
                                          gradLocation: Float, gradRadius: Float,
                                          outlineWidth: Float,
                                          directional: Boolean): Bitmap {

        val image = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888)
        image.eraseColor(Color.TRANSPARENT)

        val canvas = Canvas(image)

        val paint = Paint(Paint.ANTI_ALIAS_FLAG + Paint.FILTER_BITMAP_FLAG)
        paint.color = Color.argb(48, 0, 0, 0)
        canvas.drawOval(0f, 0f, size.toFloat(), size.toFloat(), paint)

        val radius = size / 2.0f

        if (directional) {
            val len = size * 5 / 16f
            val width = size * 3 / 16f
            paint.color = color1
            paint.alpha = 255
            val vertexes = floatArrayOf(radius,         radius - gradRadius - len,
                                        radius - width, radius - gradRadius,
                                        radius + width, radius - gradRadius)
            val indexes = shortArrayOf(0, 1, 2)
            val colors = intArrayOf(color1, color1, color1)
            canvas.drawVertices(Canvas.VertexMode.TRIANGLES,
                    vertexes.size, vertexes, 0,
                    null, 0,
                    colors, 0,
                    indexes, 0, indexes.size,
                    paint)
        }

        paint.color = Color.WHITE
        canvas.drawOval(radius - gradRadius - outlineWidth, radius - gradRadius - outlineWidth,
                  radius + gradRadius + outlineWidth,  radius + gradRadius + outlineWidth,
                       paint)

        val gradientPaint = Paint(Paint.ANTI_ALIAS_FLAG + Paint.FILTER_BITMAP_FLAG)
        gradientPaint.shader = RadialGradient(radius, radius,
                (gradLocation * radius).coerceAtLeast(0.1f),
                color0, color1, Shader.TileMode.CLAMP)
        canvas.drawOval(radius - gradRadius, radius - gradRadius,
                radius + gradRadius,  radius + gradRadius,
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
            mainHandler.postDelayed(this, (updateInterval * 1000).toLong())
        }
    }

    private val baseController: WeakReference<BaseController>
    private val useHeading: Boolean

    private var markerImages: Array<MaplyTexture>? = null
    private var directionalImages: Array<MaplyTexture>? = null

    private var markerInfo: MarkerInfo? = null
    private var movingMarkerInfo: MarkerInfo? = null
    private var circleInfo: ShapeInfo? = null

    private var markerObj: ComponentObject? = null
    private var movingMarkerObj: ComponentObject? = null
    private var circleObj: ComponentObject? = null

    private var trackerDelegate = WeakReference<LocationTrackerDelegate>(null)
    private var simulatorDelegate = WeakReference<LocationSimulatorDelegate>(null)
    private var updateInterval = 1.0
        set (value) {
            field = value.coerceAtLeast(0.1)
        }

    private var locationClient: FusedLocationProviderClient? = null
    private var locationTask: Task<Void>? = null
    private var locationUpdatePending = false

    private var prevLocation: LocationTrackerPoint? = null

    private var looper: Looper? = null
    private var handler: Handler? = null
    private val mainHandler: Handler by lazy { Handler(Looper.getMainLooper()) }

    private val locationTrackerPositionMarkerSize = 32
}