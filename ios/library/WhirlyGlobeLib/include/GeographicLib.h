//
//  geowrap.h
//  WhirlyGlobeLib
//
//  Created by Tim Sylvester on 12/14/20.
//  Copyright © 2020 mousebird consulting. All rights reserved.
//

#ifndef GeographicLib_Wrapper_h
#define GeographicLib_Wrapper_h

#import "math/MaplyCoordinate.h"

typedef struct GeoLibInv_t {
    double distance;    // meters
    double azimuth1;    // radians
    double azimuth2;    // radians
} GeoLibInv;

typedef struct GeoLibInt_t {
    MaplyCoordinateD intersection;
    bool intersects;
} GeoLibInt;

typedef struct GeoLibIntPair_t {
    MaplyCoordinateD intersections[2];
    double distances[2];
    unsigned int count;
} GeoLibIntPair;

typedef struct GeoLibOrthoDist_t {
    double downtrackDistance;
    double crosstrackDistance;
    double segmentLength;
} GeoLibOrthoDist;

#if defined __cplusplus
extern "C" {
#endif

/// Solve the direct geodesic problem where the length of the geodesic is specified in terms of distance.
/// azimuth in radians, distance in meters
MaplyCoordinate GeoLibCalcDirectF(MaplyCoordinate origin, double azimuth, double distance);
MaplyCoordinateD GeoLibCalcDirectD(MaplyCoordinateD origin, double azimuth, double distance);

// Solve the inverse geodesic problem
GeoLibInv GeoLibCalcInverseF(MaplyCoordinate p1, MaplyCoordinate p2);
GeoLibInv GeoLibCalcInverseD(MaplyCoordinateD p1, MaplyCoordinateD p2);

// Test for a point lying inside the specified polygon
bool MaplyCoordinateInPolygon(MaplyCoordinate p, const MaplyCoordinate polygon[], unsigned count);
bool MaplyCoordinateDInPolygon(MaplyCoordinateD p, const MaplyCoordinate polygon[], unsigned count);
bool MaplyCoordinateInPolygonD(MaplyCoordinate p, const MaplyCoordinateD polygon[], unsigned count);
bool MaplyCoordinateDInPolygonD(MaplyCoordinateD p, const MaplyCoordinateD polygon[], unsigned count);

double GeoLibDistanceD(MaplyCoordinateD startPt, MaplyCoordinateD endPt);

// Test for a segment intersecting a polygon.
// Note that if the line is completely within the polygon the result is false.
bool GeoLibLineDIntersectsPolygonD(MaplyCoordinateD startPt, MaplyCoordinateD endPt, const MaplyCoordinateD[], unsigned count);

// Compute the intersection point of two geodesic segments
GeoLibInt GeoLibIntersectD(MaplyCoordinateD a, MaplyCoordinateD b, MaplyCoordinateD c, MaplyCoordinateD d);

// Determine where a great circle intersects a small circle
GeoLibIntPair GeoLibLineDIntersectCircleD(MaplyCoordinateD startPt, MaplyCoordinateD endPt, MaplyCoordinateD center, double radiusMeters);

// Determine whether there's an intersection without bothering to compute its location
bool GeoLibLineDIntersectsCircleD(MaplyCoordinateD startPt, MaplyCoordinateD endPt, MaplyCoordinateD center, double radiusMeters);

// Compute the orthogonal distances for a point.
//
// Given a segment and a point, find the perpendicular intersection point (the closest point along the
// segment) and compute the distance from that point to the segment starting point (down-track) and
// to the specified point (cross-track).
//
//  negative down-track distance          C
//                             v          |    <- negative cross-track distance
//                         - - - - - A--------B
//                         |              |    <- positive cross-track distance
//                         C              C
//                                    ----
//                                     ^ positive down-track-distance
//
// If the point lies "before" the segment start point, the down-track distance will be negative.
// If the point lies "after" the segment end point, the down-track distance will be greater than the segment length.
// If the point lies to the right of the segment, the cross-track distance will be positive.
// If the point lies to the left of the segment, the cross-track distance will be negative.
GeoLibOrthoDist GeoLibOrthoDistD(MaplyCoordinateD a, MaplyCoordinateD b, MaplyCoordinateD c);

// Generate points along an arc
double GeoLibSampleArcD(MaplyCoordinateD center, double radiusMeters,
                        double beginAzimuthRad, double endAziumthRad, bool clockwise,
                        MaplyCoordinateD points[], unsigned count);

#if defined __cplusplus
}   // extern "C"
#endif

#endif /* GeographicLib_Wrapper_h */
