package com.mousebird.maply

import androidx.annotation.Keep

@Keep
class GeographicLib private constructor() {

    companion object {
        @JvmStatic @Keep
        external fun calcDirect(lon: Double, lat: Double, azimuth: Double, distance: Double): Point2d?
        @JvmStatic @Keep
        external fun calcDirectPt(origin: Point2d, azimuth: Double, distance: Double): Point2d?

        @JvmStatic @Keep
        external fun calcInverse(lon1: Double, lat1: Double, lon2: Double, lat2: Double): Inverse?
        @JvmStatic @Keep
        external fun calcInversePt(pt1: Point2d, pt2: Point2d): Inverse?

//        bool MaplyCoordinateInPolygon(MaplyCoordinate p, const MaplyCoordinate polygon[], unsigned count);
//        bool MaplyCoordinateDInPolygon(MaplyCoordinateD p, const MaplyCoordinate polygon[], unsigned count);
//        bool MaplyCoordinateInPolygonD(MaplyCoordinate p, const MaplyCoordinateD polygon[], unsigned count);
//        bool MaplyCoordinateDInPolygonD(MaplyCoordinateD p, const MaplyCoordinateD polygon[], unsigned count);
//        double GeoLibDistanceD(MaplyCoordinateD startPt, MaplyCoordinateD endPt);
//        bool GeoLibLineDIntersectsPolygonD(MaplyCoordinateD startPt, MaplyCoordinateD endPt, const MaplyCoordinateD[], unsigned count);
//        GeoLibInt GeoLibIntersectD(MaplyCoordinateD a, MaplyCoordinateD b, MaplyCoordinateD c, MaplyCoordinateD d);
//        GeoLibIntPair GeoLibLineDIntersectCircleD(MaplyCoordinateD startPt, MaplyCoordinateD endPt, MaplyCoordinateD center, double radiusMeters);
//        bool GeoLibLineDIntersectsCircleD(MaplyCoordinateD startPt, MaplyCoordinateD endPt, MaplyCoordinateD center, double radiusMeters);
//        GeoLibOrthoDist GeoLibOrthoDistD(MaplyCoordinateD a, MaplyCoordinateD b, MaplyCoordinateD c);
//        double GeoLibSampleArcD(MaplyCoordinateD center, double radiusMeters,
//        double beginAzimuthRad, double endAziumthRad, bool clockwise,
//        MaplyCoordinateD points[], unsigned count);

        @JvmStatic @Keep
        private external fun nativeInit()
        init { nativeInit() }
    }

    @Keep
    data class Inverse(inline val distance: Double, inline val azimuth1: Double, inline val azimuth2: Double)
    @Keep
    data class Intersection(val intersects: Boolean, val lon: Double, val lat: Double)
    @Keep
    data class OrthoDist(val downTrackDist: Double, val crossTrackDist: Double, val length: Double)
}