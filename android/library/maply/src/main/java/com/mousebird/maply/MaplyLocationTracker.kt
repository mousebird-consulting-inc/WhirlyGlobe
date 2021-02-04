package com.mousebird.maply

import android.Manifest.permission
import android.content.Context
import android.graphics.*
import android.graphics.Shader
import android.os.Handler
import android.os.Looper
import androidx.annotation.ColorInt
import androidx.annotation.RequiresPermission
import com.google.android.gms.location.*
import com.google.android.gms.tasks.Task
import java.lang.ref.WeakReference
import kotlin.math.abs


class MaplyLocationTracker : LocationCallback {

    /**
     * MaplyLocationTracker constructor
     *
     * @param mapController The globe or map view controller
     * @param trackerDelegate Delegate for location tracking
     * @param useHeading Use location services heading information (requires physical magnetometer)
     * @param useCourse Use location services course information as fallback if heading unavailable
     */
    constructor(mapController: BaseController,
                trackerDelegate: MaplyLocationTrackerDelegate? = null,
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
                trackerDelegate: MaplyLocationTrackerDelegate? = null,
                simulatorDelegate: MaplyLocationSimulatorDelegate? = null,
                updateInterval: Double = 1.0,
                useHeading: Boolean = true, useCourse: Boolean = true)
    {
        this.baseController = WeakReference(mapController)
        this.mapController = WeakReference<MapController>(mapController as? MapController)
        this.globeController = WeakReference<GlobeController>(mapController as? GlobeController)
        this.trackerDelegate = WeakReference<MaplyLocationTrackerDelegate>(trackerDelegate)
        this.simulatorDelegate = WeakReference<MaplyLocationSimulatorDelegate>(simulatorDelegate)
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
        if (simulatorDelegate != null) {
            mainHandler.post(simTask)
        } else {
            locationClient = LocationServices.getFusedLocationProviderClient(context)?.also { client ->
                val req = LocationRequest()
                req.interval = (1000 * updateInterval).toLong()
                locationTask = client.requestLocationUpdates(req, this, looper ?: Looper.myLooper())
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

//        if (updateLocationScheduled) {
//            [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(updateLocationInternal:) object:nil];
//        }
//        if (!_simulate)
//            [self teardownLocationManager];
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
        this.lockType = lockType;
        this.forwardTrackOffset = forwardTrackOffset;
    }

    /**
    Get the current device location

    @return The coordinate if valid, else kMaplyNullCoordinate
     */
    fun getLocation(): MaplyLocationTrackerPoint? {
        return null
    }

    /**
    Set the current simulated location.
     */
    fun setLocation(location: MaplyLocationTrackerPoint) {

    }

    override fun onLocationAvailability(p0: LocationAvailability?) {
        super.onLocationAvailability(p0)
    }

    override fun onLocationResult(p0: LocationResult?) {
        super.onLocationResult(p0)
    }

    private final val locationTrackerPositionMarkerSize = 32;

    private fun setupMarkerImages() {
        if (markerImages == null || directionalImages == null) {
            val size = locationTrackerPositionMarkerSize * 2

            val color0 = Color.WHITE
            val color1 = Color.argb(255,0,192,255)

            baseController.get()?.also { vc ->
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

//    - (MaplyShapeCircle *)shapeCircleForCoord:(MaplyCoordinate)coord AndHorizontalAccuracy:(int)horizontalAccuracy {
//
//        MaplyShapeCircle *shapeCircle = [[MaplyShapeCircle alloc] init];
//        shapeCircle.center = coord;
//
//        const MaplyCoordinate coord1 = [self coordOfPointAtTrueCourse:0.0 andDistanceMeters:horizontalAccuracy fromCoord:coord];
//        const MaplyCoordinate coord2 = [self coordOfPointAtTrueCourse:90.0 andDistanceMeters:horizontalAccuracy fromCoord:coord];
//
//        const auto __strong vc = _theViewC;
//        const auto __strong mvc = _mapVC;
//        const auto __strong gvc = _globeVC;
//
//        const MaplyCoordinate3d dispPt0 = [vc displayPointFromGeo:coord];
//        const MaplyCoordinate3d dispPt1 = [vc displayPointFromGeo:coord1];
//        const MaplyCoordinate3d dispPt2 = [vc displayPointFromGeo:coord2];
//
//        const float d1 = sqrtf(powf(dispPt1.x-dispPt0.x, 2.0) + powf(dispPt1.y-dispPt0.y, 2.0));
//        const float d2 = sqrtf(powf(dispPt2.x-dispPt0.x, 2.0) + powf(dispPt2.y-dispPt0.y, 2.0));
//        shapeCircle.radius = (d1 + d2) / 2.0;
//
//        float minHeight = 0.0;
//        if (gvc)
//            minHeight = [gvc getZoomLimitsMin];
//        else {
//            float maxHeight;
//            [mvc getZoomLimitsMin:&minHeight max:&maxHeight];
//        }
//        shapeCircle.height = minHeight * 0.01;
//
//        return shapeCircle;
//    }

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

    private var trackerDelegate: WeakReference<MaplyLocationTrackerDelegate>? = null
    private var simulatorDelegate: WeakReference<MaplyLocationSimulatorDelegate>? = null
    private var updateInterval = 1.0

    private var locationClient: FusedLocationProviderClient? = null
    private var locationTask: Task<Void>? = null

    private val mainHandler: Handler by lazy { Handler(Looper.getMainLooper()) }
}