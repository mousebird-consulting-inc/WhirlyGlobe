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
import com.google.android.gms.tasks.Task
import java.lang.ref.WeakReference
import kotlin.concurrent.thread
import kotlin.math.*


class LocationTracker : LocationCallback {

    /**
     * MaplyLocationTracker constructor
     *
     * @param mapController The globe or map view controller
     * @param trackerDelegate Delegate for location tracking
     * @param useHeading Use location services heading information (requires physical magnetometer)
     * @param useCourse Use location services course information as fallback if heading unavailable
     */
    constructor(mapController: BaseController,
                trackerDelegate: LocationTrackerDelegate? = null,
                useHeading: Boolean = true, useCourse: Boolean = true) :
            this(mapController, trackerDelegate, null, 0.0, useHeading, useCourse)
    {
    }

    /**
     * MaplyLocationTracker constructor
     *
     * @param mapController The globe or map view controller
     * @param trackerDelegate Delegate for location tracking
     * @param simulatorDelegate Delegate for simulated location
     * @param updateInterval Seconds between simulation updates
     * @param useHeading Use location services heading information (requires physical magnetometer)
     * @param useCourse Use location services course information as fallback if heading unavailable
     */
    constructor(mapController: BaseController,
                trackerDelegate: LocationTrackerDelegate? = null,
                simulatorDelegate: LocationSimulatorDelegate? = null,
                updateInterval: Double = 1.0,
                useHeading: Boolean = true, useCourse: Boolean = true)
    {
        this.baseController = WeakReference(mapController)
        this.mapController = WeakReference<MapController>(mapController as? MapController)
        this.globeController = WeakReference<GlobeController>(mapController as? GlobeController)
        this.trackerDelegate = WeakReference<LocationTrackerDelegate>(trackerDelegate)
        this.simulatorDelegate = WeakReference<LocationSimulatorDelegate>(simulatorDelegate)
        this.updateInterval = updateInterval
        this.useHeading = useHeading
        this.useCourse = useCourse
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
                req.interval = (1000 * updateInterval).toLong()
                locationTask = client.requestLocationUpdates(req, this, this.looper)
            }
        }
    }

    /**
     * Stop location tracking/simulation
     */
    fun stop() {
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
    Change lock type

    @param lockType The MaplyLocationLockType value for lock behavior
    @param forwardTrackOffset The vertical offset if using MaplyLocationLockHeadingUpOffset (positive values are below the view center)
     */
    fun changeLockType(lockType: MaplyLocationLockType, forwardTrackOffset: Int = 0) {
        this.lockType = lockType
        this.forwardTrackOffset = forwardTrackOffset
    }

    /**
    Get the current device location

    @return The coordinate if valid, else kMaplyNullCoordinate
     */
    fun getLocation(): LocationTrackerPoint? {
        return lastLocation
    }

    /**
    Set the current simulated location.
     */
    fun setLocation(location: LocationTrackerPoint?) {
        if (!locationUpdatePending) {
            locationUpdatePending = true
            handler?.post { updateLocationInternal(location) }
        }
    }

    override fun onLocationAvailability(availability: LocationAvailability?) {
        availability.let { super.onLocationAvailability(it) }

        // Call the delegate if its present, allowing it to update the result
        var a = availability
        trackerDelegate.get()?.let {
            a = it.locationManagerDidChangeAuthorizationStatus(this, availability)
        }

        if (a?.isLocationAvailable != true) {
            setLocation(null)
        }
    }

    override fun onLocationResult(location: LocationResult?) {
        super.onLocationResult(location)
        setLocation(convertIf(location?.lastLocation))
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

    private final val threadCurrent = RenderControllerInterface.ThreadMode.ThreadCurrent

    private fun updateLocationInternal(location: LocationTrackerPoint?) {
        locationUpdatePending = false

        val vc = baseController.get() ?: return

        setupMarkerImages()
        if (circleInfo == null) return

        var endLoc = location

        // Call the delegate, allowing it to update the result
        trackerDelegate.get()?.let {
            endLoc = it.locationManagerDidUpdateLocation(this, location)
        }

        var startLoc = prevLocation ?: endLoc

        vc.removeObjects(arrayOf(markerObj, movingMarkerObj, circleObj).filterNotNull(), threadCurrent)
        markerObj = null
        movingMarkerObj = null
        circleObj = null

        prevLocation = lastLocation
        lastLocation = endLoc

        val circle = circleForLocation(endLoc) ?: return
        circleObj = vc.addShapes(arrayOf(circle).asList(), circleInfo, threadCurrent)

//            NSNumber *orientation;
//            if (_useHeading && _latestHeading)
//                orientation = _latestHeading;
//            else if (_useCourse && location.course >= 0)
//                orientation = @(location.course);
//
//            NSArray *markerImages;
//            if (orientation)
//                markerImages = _markerImgsDirectional;
//            else
//                markerImages = _markerImgs;
//
//            MaplyMovingScreenMarker *movingMarker = [[MaplyMovingScreenMarker alloc] init];
//            movingMarker.loc = startLoc;
//            movingMarker.endLoc = endLoc;
//            movingMarker.duration = 0.5;
//
//            movingMarker.period = 1.0;
//            movingMarker.size = CGSizeMake(LOC_TRACKER_POS_MARKER_SIZE, LOC_TRACKER_POS_MARKER_SIZE);
//            if (orientation)
//                movingMarker.rotation = -M_PI/180.0 * orientation.doubleValue;
//            movingMarker.images = markerImages;
//            movingMarker.layoutImportance = MAXFLOAT;
//
//            MaplyScreenMarker *marker = [[MaplyScreenMarker alloc] init];
//            marker.loc = endLoc;
//
//            marker.period = 1.0;
//            marker.size = CGSizeMake(LOC_TRACKER_POS_MARKER_SIZE, LOC_TRACKER_POS_MARKER_SIZE);
//            if (orientation)
//                marker.rotation = -M_PI/180.0 * orientation.doubleValue;
//            marker.images = markerImages;
//            marker.layoutImportance = MAXFLOAT;
//
//            NSTimeInterval ti = [NSDate timeIntervalSinceReferenceDate]+0.5;
//            _markerDesc[kMaplyEnableStart] = _movingMarkerDesc[kMaplyEnableEnd] = @(ti);
//
//            _movingMarkerObj = [theViewC addScreenMarkers:@[movingMarker] desc:_movingMarkerDesc mode:MaplyThreadCurrent];
//            _markerObj = [theViewC addScreenMarkers:@[marker] desc:_markerDesc mode:MaplyThreadCurrent];
//
//            [self lockToLocation:endLoc heading:(orientation ? orientation.floatValue : 0.0)];
//
//            _prevLoc = endLoc;
//        }
//
//        __strong NSObject<MaplyLocationTrackerDelegate> *delegate = _delegate;
//        if ([delegate respondsToSelector:@selector(updateLocation:)]) {
//            [delegate updateLocation:location];
//        }

    }

    private fun setupMarkerImages() {
        if (markerImages == null || directionalImages == null) {
            val size = locationTrackerPositionMarkerSize * 2

            val color0 = Color.WHITE
            val color1 = Color.argb(255,0,192,255)

            baseController.get()?.let { vc ->
                markerImages = (0..16).map { radialGradientMarker(vc, size, color0, color1, it, false) }
                directionalImages = (0..16).map { radialGradientMarker(vc, size, color0, color1, it, true) }
            }
        }

        if (markerInfo == null) {
            markerInfo = MarkerInfo().also {
                it.setMinVis(markerMinVis)
                it.setMaxVis(markerMaxVis)
                it.setFade(0.0f)
                it.drawPriority = markerDrawPriority
                it.setEnableTimes(0.0, Double.MAX_VALUE)
            }
        }
        if (movingMarkerInfo == null) {
            movingMarkerInfo = MarkerInfo().also {
                it.setMinVis(markerMinVis)
                it.setMaxVis(markerMaxVis)
                it.setFade(0.0f)
                it.drawPriority = markerDrawPriority
                it.setEnableTimes(0.0, Double.MAX_VALUE)
            }
        }
        if (circleInfo == null) {
            circleInfo = ShapeInfo().also {
                it.setColor(0.06f, 0.06f, 0.1f, 0.2f)
                it.setFade(0.0f)
                it.drawPriority = markerDrawPriority - 1
                // TODO: kMaplySampleX: @(100)
                // TODO: kMaplyZBufferRead: @(false)
            }
        }
    }

    private fun markerGradLoc(n: Int): Float {
        return (8 - abs(8 - n)) / 8.0f
    }
    private fun markerGradRad(n: Int, size: Int): Float {
        return (size - locationTrackerPositionMarkerSize - abs(8 - n)) / 2.0f
    }

    private fun radialGradientMarker(vc: BaseController, size: Int,
                                     @ColorInt color0: Int, @ColorInt color1: Int,
                                     idx: Int, directional: Boolean): MaplyTexture {
        val image = radialGradientMarkerImage(size, color0, color1, markerGradLoc(idx), markerGradRad(idx, size), directional)
        return vc.addTexture(image, RenderControllerInterface.TextureSettings(), RenderControllerInterface.ThreadMode.ThreadCurrent)
    }

    private fun radialGradientMarkerImage(size: Int, @ColorInt color0: Int, @ColorInt color1: Int, gradLocation: Float, radius: Float, directional: Boolean): Bitmap {

        val image = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888)
        image.eraseColor(Color.TRANSPARENT)

        val canvas = Canvas(image)

        val outlinePaint = Paint(Paint.ANTI_ALIAS_FLAG)
        outlinePaint.color = Color.argb(127, 255, 255, 255)
        canvas.drawOval(0f, 0f, size.toFloat(), size.toFloat(), outlinePaint)

        // TODO: Draw direction indicator triangle
//        if (directional) {
//            float len = 20.0;
//            float height = 12.0;
//            CGPathMoveToPoint(path, NULL,    size/2, size/2-radius-len);
//            CGPathAddLineToPoint(path, NULL, size/2-height, size/2-radius);
//            CGPathAddLineToPoint(path, NULL, size/2+height, size/2-radius);
//            CGContextSetFillColorWithColor(ctx, color1.CGColor);
//            CGContextAddPath(ctx, path);
//            CGContextFillPath(ctx);
//        }

        outlinePaint.color = Color.WHITE
        canvas.drawOval(size / 2.0f - radius - 4, size / 2.0f - radius - 4, 2.0f * radius + 8, 2.0f * radius + 8, outlinePaint)

        val gradientPaint = Paint(Paint.ANTI_ALIAS_FLAG)
        gradientPaint.isDither = true
        gradientPaint.shader = RadialGradient(size / 2.0f, size / 2.0f,
                gradLocation * size / 2.0f,
                color0, color1, Shader.TileMode.CLAMP)
        canvas.drawOval(0f, 0f, size.toFloat(), size.toFloat(), gradientPaint)

        return image
    }

    private fun circleForLocation(location: LocationTrackerPoint?, defRadius: Double? = null): ShapeCircle? {
        if (location == null) return null
        val vc = baseController.get() ?: return null
        val radius = location.horizontalAccuracy ?: defRadius ?: return null
        val center = Point2d.FromDegrees(location.lonDeg, location.latDeg)

        val circle = ShapeCircle()
        circle.setLoc(center)

        val top = coordOfPointAtTrueCourse(center, courseDeg = 0.0, distanceMeters = radius)
        val right = coordOfPointAtTrueCourse(center, courseDeg = 90.0, distanceMeters = radius)

        val dispPtCenter: Point2d = vc.displayPointFromGeo(center)
        val dispPtTop: Point2d = vc.displayPointFromGeo(center)
        val dispPtRight: Point2d = vc.displayPointFromGeo(center)

        val vRad = hypot(dispPtTop.x - dispPtCenter.x, dispPtTop.y - dispPtCenter.y)
        val hRad = hypot(dispPtRight.x - dispPtCenter.x, dispPtRight.y - dispPtCenter.y)
        circle.setRadius((vRad + hRad) / 2.0)

        val minHeight = (vc as? GlobeController)?.also { it.getZoomLimitsMin() } ?:
                            (vc as? MapController).also { it.getZoomLimitsMin() } ?: 0.0
        circle.setHeight(minHeight / 100.0)

        return circle
    }

    private fun degToRad(deg: Double): Double { return deg * Math.PI / 180.0 }
    private fun radToDeg(rad: Double): Double { return rad * 180.0 / Math.PI }

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
    private var mapController: WeakReference<MapController> = WeakReference<MapController>(null)
    private var globeController: WeakReference<GlobeController> = WeakReference<GlobeController>(null)
    private val useHeading: Boolean
    private val useCourse: Boolean

    private var markerImages: List<MaplyTexture>? = null
    private var directionalImages: List<MaplyTexture>? = null

    private var markerInfo: MarkerInfo? = null
    private var movingMarkerInfo: MarkerInfo? = null
    private var circleInfo: ShapeInfo? = null

    private var markerObj: ComponentObject? = null
    private var movingMarkerObj: ComponentObject? = null
    private var circleObj: ComponentObject? = null

    private var trackerDelegate = WeakReference<LocationTrackerDelegate>(null)
    private var simulatorDelegate = WeakReference<LocationSimulatorDelegate>(null)
    private var updateInterval = 1.0

    private var locationClient: FusedLocationProviderClient? = null
    private var locationTask: Task<Void>? = null
    private var locationUpdatePending = false

    private var lastLocation: LocationTrackerPoint? = null
    private var prevLocation: LocationTrackerPoint? = null

    private var looper: Looper? = null
    private var handler: Handler? = null
    private val mainHandler: Handler by lazy { Handler(Looper.getMainLooper()) }

    private final val locationTrackerPositionMarkerSize = 32
}