//
//  geowrap.c
//  WhirlyGlobeLib
//
//  Created by Tim Sylvester on 12/14/20.
//  Copyright © 2020 mousebird consulting. All rights reserved.
//

#import "GeographicLib/Geodesic.hpp"
#import "GeographicLib/Geocentric.hpp"

#import "../../../../common/WhirlyGlobeLib/include/GeographicLib.h"
#import "../include/GeographicLib.h"
#import "CoordSystem.h"
#import "WhirlyGeometry.h"
#import "WhirlyKitLog.h"

#import <tuple>
#import <cmath>

#if !defined(M_2PI)
# define M_2PI (2 * M_PI)
#endif

typedef WhirlyKit::Point3d Point3d;
using namespace WhirlyKit::detail;

MaplyCoordinate GeoLibCalcDirectF(MaplyCoordinate origin, double azimuth, double distance)
{
    return MaplyCoordinateMakeWithMaplyCoordinateD(
        GeoLibCalcDirectD(MaplyCoordinateD { origin.x, origin.y }, azimuth, distance));
}

MaplyCoordinateD GeoLibCalcDirectD(MaplyCoordinateD origin, double azimuthRadians, double distanceMeters)
{
    const auto lat1 = WhirlyKit::RadToDeg(origin.y);
    const auto lon1 = WhirlyKit::RadToDeg(origin.x);
    const auto azDeg = WhirlyKit::RadToDeg(azimuthRadians);

    double lat2 = 0.0, lon2 = 0.0;
    const auto res = wgs84Geodesic().Direct(lat1, lon1, azDeg, distanceMeters, lat2, lon2);

    return std::isfinite(res)
        ? MaplyCoordinateD { WhirlyKit::DegToRad(lon2), WhirlyKit::DegToRad(lat2) }
        : MaplyCoordinateD { std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN() };
}

namespace {
    template <typename T>
    static GeoLibInv CalcInv(T p1, T p2)
    {
        const auto lat1 = WhirlyKit::RadToDeg(p1.y);
        const auto lon1 = WhirlyKit::RadToDeg(p1.x);
        const auto lat2 = WhirlyKit::RadToDeg(p2.y);
        const auto lon2 = WhirlyKit::RadToDeg(p2.x);
        double dist = 0.0, az1 = 0.0, az2 = 0.0;
        wgs84Geodesic().Inverse(lat1, lon1, lat2, lon2, dist, az1, az2);
        return GeoLibInv { dist, WhirlyKit::DegToRad(az1), WhirlyKit::DegToRad(az2) };
    }
}

GeoLibInv GeoLibCalcInverseF(MaplyCoordinate p1, MaplyCoordinate p2) { return CalcInv(p1, p2); }
GeoLibInv GeoLibCalcInverseD(MaplyCoordinateD p1, MaplyCoordinateD p2) { return CalcInv(p1, p2); }

namespace {
    template <typename TPoint, typename TPolyPt>
    static bool InPoly(TPoint p, const TPolyPt poly[], unsigned count) {
        if (count < 2)
        {
            return false;
        }
        // TODO: do this without needing to make a copy of the points
        WhirlyKit::Point2dVector pts;
        pts.reserve(count + 1);
        for (unsigned i = 0; i < count; ++i) {
            pts.emplace_back(poly[i].x, poly[i].y);
        }
        // Ensure that the polygon is closed
        const auto &f = pts.front(), &b = pts.back();
        if (f.x() != b.x() || f.y() != b.y())
        {
            pts.push_back(f);
        }

        // TODO: this is using spherical assumptions
        return WhirlyKit::PointInPolygon(WhirlyKit::Point2d(p.x, p.y), pts);
    }

    static inline Point3d CoordToGeocentric(const MaplyCoordinateD &p, double geoidHeight = 0.0)
    {
        double x = 0, y = 0, z = 0;
        wgs84Geocentric().Forward(WhirlyKit::RadToDeg(p.y), WhirlyKit::RadToDeg(p.x), geoidHeight, x, y, z);
        return {x,y,z};
    }

    static inline MaplyCoordinateD GeocentricToCoord(const Point3d &p)
    {
        double lat = 0, lon = 0, height = 0;
        wgs84Geocentric().Reverse(p.x(), p.y(), p.z(), lat, lon, height);
        return { WhirlyKit::DegToRad(lon), WhirlyKit::DegToRad(lat) };
    }
}

bool MaplyCoordinateInPolygon(MaplyCoordinate p, const MaplyCoordinate polygon[], unsigned count) { return InPoly(p, polygon, count); }
bool MaplyCoordinateDInPolygon(MaplyCoordinateD p, const MaplyCoordinate polygon[], unsigned count) { return InPoly(p, polygon, count); }
bool MaplyCoordinateInPolygonD(MaplyCoordinate p, const MaplyCoordinateD polygon[], unsigned count) { return InPoly(p, polygon, count); }
bool MaplyCoordinateDInPolygonD(MaplyCoordinateD p, const MaplyCoordinateD polygon[], unsigned count) { return InPoly(p, polygon, count); }


GeoLibInt GeoLibIntersectD(MaplyCoordinateD a, MaplyCoordinateD b, MaplyCoordinateD c, MaplyCoordinateD d)
{
    const auto gca = CoordToGeocentric(a);
    const auto gcb = CoordToGeocentric(b);
    const auto gcc = CoordToGeocentric(c);
    const auto gcd = CoordToGeocentric(d);

    bool res;
    Point3d p;
    std::tie(res,p) = WhirlyKit::Geocentric::findIntersection(gca, gcb, gcc, gcd, wgs84Geodesic());

    return res ? GeoLibInt{ GeocentricToCoord(p), true } : GeoLibInt{ { 0, 0 }, false };
}

bool GeoLibLineDIntersectsPolygonD(MaplyCoordinateD startPt, MaplyCoordinateD endPt, const MaplyCoordinateD points[], unsigned count)
{
    if (count < 2)
    {
        return false;
    }

    const auto gcStart = CoordToGeocentric(startPt);
    const auto gcEnd = CoordToGeocentric(endPt);

    Point3d gca = CoordToGeocentric(points[0]);
    Point3d gcb;

    for (unsigned i = 0; i < count; ++i)
    {
        const auto& p = points[(i + 1) % count];

        // Alternate with each new point so we only convert each point once, and we don't care about direction
        ((i & 1) ? gca : gcb) = CoordToGeocentric(p);

        if (gca != gcb && WhirlyKit::Geocentric::checkIntersection(gcStart, gcEnd, gca, gcb, wgs84Geodesic()))
        {
            return true;
        }
    }
    return false;
}

double GeoLibDistanceD(MaplyCoordinateD startPt, MaplyCoordinateD endPt)
{
    double s12 = 0.0;
    wgs84Geodesic().Inverse(WhirlyKit::RadToDeg(startPt.y), WhirlyKit::RadToDeg(startPt.x),
                            WhirlyKit::RadToDeg(endPt.y), WhirlyKit::RadToDeg(endPt.x), s12);
    return s12;
}


double GeoLibInitialHeadingD(MaplyCoordinateD startPt, MaplyCoordinateD endPt)
{
    const auto gcStart = CoordToGeocentric(startPt);
    const auto gcEnd = CoordToGeocentric(endPt);
    return WhirlyKit::Geocentric::initialHeading(gcStart, gcEnd);
}

bool GeoLibLineDIntersectsCircleD(MaplyCoordinateD startPt, MaplyCoordinateD endPt, MaplyCoordinateD center, double radiusMeters)
{
    // If either endpoint of the line is within the circle, we're done.
    // TODO: Is this fast enough to be worthwhile, when we get the same info below?
    if (GeoLibDistanceD(startPt, center) <= radiusMeters ||
        GeoLibDistanceD(endPt, center) <= radiusMeters)
    {
        return true;
    }

    // Find the point on the line closest to the circle center.
    // If that point is within the length of the segment and within the radius, they intersect.
    // If it's within the radius, then the line intersects the circle.
    auto const res = GeoLibOrthoDistD(startPt, endPt, center);
    return (0 < res.downtrackDistance && res.downtrackDistance < res.segmentLength &&
            std::fabs(res.crosstrackDistance) <= radiusMeters);
}

GeoLibIntPair GeoLibLineDIntersectCircleD(MaplyCoordinateD startPt, MaplyCoordinateD endPt, MaplyCoordinateD center, double radiusMeters)
{
    const auto gcStart = CoordToGeocentric(startPt).normalized();
    const auto gcEnd = CoordToGeocentric(endPt).normalized();
    const auto gcCenter = CoordToGeocentric(center).normalized();

    GeoLibIntPair result = { {{ 0.0, 0.0 }, { 0.0, 0.0 }}, { 0.0, 0.0 }, 0 };

    double dt, ct, len;
    std::tie(dt, ct, len) = WhirlyKit::Geocentric::OrthoDist(gcStart, gcEnd, gcCenter);
    if (dt < -radiusMeters ||                    // preceeds the GC segment by more than the radius
        dt > len + radiusMeters || // follows the GC segment by more than the radius
        std::fabs(ct) > radiusMeters)            // to the side of the GC segment by more than the radius
    {
        return result;
    }

    // Use the right-triangle-simplified law of cosines for triangles on a sphere

    const double earthRad = wgs84Geodesic().EquatorialRadius();
    const double a = earthRad * std::acos(std::cos(radiusMeters / earthRad) /
                                          std::cos(std::fabs(ct) / earthRad));

    // The intersections must be symmetric around the perpendicular intercept
    const double dist1  = dt + a;
    const double dist2  = dt - a;

    const double hdg = WhirlyKit::Geocentric::initialHeading(gcStart, gcEnd);

    if (dist1 >= 0.0 && dist1 <= len)
    {
        result.distances[0] = dist1;
        result.intersections[0] = GeocentricToCoord(WhirlyKit::Geocentric::orthoDirect(gcStart, hdg, dist1));
        result.count += 1;
    }
    if (dist2 >= 0.0 && dist2 <= len)
    {
        result.distances[result.count] = dist2;
        result.intersections[result.count] = GeocentricToCoord(WhirlyKit::Geocentric::orthoDirect(gcStart, hdg, dist2));
        result.count += 1;
    }

    return result;
}

namespace {

}

GeoLibOrthoDist GeoLibOrthoDistD(MaplyCoordinateD a, MaplyCoordinateD b, MaplyCoordinateD c)
{
    const auto gca = CoordToGeocentric(a).normalized();
    const auto gcb = CoordToGeocentric(b).normalized();
    const auto gcc = CoordToGeocentric(c).normalized();
    const auto res = WhirlyKit::Geocentric::OrthoDist(gca, gcb, gcc);
    return { std::get<0>(res), std::get<1>(res), std::get<2>(res) };
}

namespace {

    static inline double normalizeAzimuth(double a)
    {
        a = std::fmod(a, M_2PI);
        while (a < 0.0)
        {
            a += M_2PI;
        }
        return a;
    }

    static inline double arcSpan(double beginAzimuthRad, double endAzimuthRad, bool clockwise)
    {
        if (beginAzimuthRad == endAzimuthRad)
        {
            // special case for a full circle
            return clockwise ? M_2PI : -M_2PI;
        }
        // Account for going the "long way around"
        return clockwise ? normalizeAzimuth(endAzimuthRad + M_2PI - beginAzimuthRad)
                         : -normalizeAzimuth(beginAzimuthRad + M_2PI - endAzimuthRad);
    }
}

double GeoLibSampleArcD(MaplyCoordinateD center, double radiusMeters,
                        double beginAzimuthRad, double endAzimuthRad, bool clockwise,
                        MaplyCoordinateD points[], unsigned count)
{
    if (count < 2)
    {
        return 0.0;
    }
    
    beginAzimuthRad = normalizeAzimuth(beginAzimuthRad);
    endAzimuthRad = normalizeAzimuth(endAzimuthRad);

    auto const span = arcSpan(beginAzimuthRad, endAzimuthRad, clockwise);
    auto const increment = span / (count - 1);

    double azimuth = beginAzimuthRad;
    for (unsigned i = 0; i < count; ++i, azimuth += increment)
    {
        // Use the exact end angle, in case we introduced any error
        if (i == count - 1)
        {
            azimuth = endAzimuthRad;
        }
        points[i] = GeoLibCalcDirectD(center, azimuth, radiusMeters);
    }

    return std::fabs(increment);
}

//#define RUN_UNIT_TEST 1
#if RUN_UNIT_TEST
namespace {
    static double d2r(double deg) { return WhirlyKit::DegToRad(deg); }
    static MaplyCoordinateD cd(double lat, double lon) { return MaplyCoordinateD{d2r(lon), d2r(lat)}; }
    static bool eq(double a, double b, double e) { return std::fabs(a-b) < e; }
    static void assertEq(double a, double b, double e) { assert(eq(a,b,e)); }
    static struct Test {
        Test() {
            TestDistance();
            TestLineLineInt();
            TestLinePolyInt();
            TestOrthoDist();
            TestLineCircleInt();
            TestSampleArc();
            wkLog("GeographicLib Tests Passed");
        }
        void TestDistance() {
            // Values from 10.5281/zenodo.32156
            assertEq(GeoLibDistanceD(cd(21.219268205986, 0), cd(21.210895095821835985, 0.000470899511575113)), 928.3575608, 1.0e-6);
            assertEq(GeoLibDistanceD(cd(27.095351435163, 0), cd(-27.095351435168394093, 179.462448686689092514)), 19977267.8165311, 1.0e-6);
        }
        void TestLineLineInt() {
            // trivial case
            const auto a = GeoLibIntersectD(cd(0, -10), cd(0,  10), cd(-10, 0), cd(10, 0));
            assert(a.intersects);
            assert(a.intersection.y == 0);
            assert(a.intersection.x == 0);
            // polar intersection
            const auto b = GeoLibIntersectD(cd(80, 180), cd(80, 0), cd(80, 90), cd(80, -90));
            assert(b.intersects);
            assert(b.intersection.y == d2r(90.));
            // simple case, reference value from Google Earth
            const auto c = GeoLibIntersectD(cd(45, -120), cd(40, -110), cd(45, -110), cd(40, -120));
            assert(c.intersects);
            assertEq(c.intersection.y, d2r(42.708939), 1e-6);
            assertEq(c.intersection.x, d2r(-115), 1e-6);
        }
        void TestLinePolyInt() {
            const MaplyCoordinateD poly[] = { cd(10, 0), cd(0, 10), cd(-10, 0), cd(0, -10 )};
            assert( GeoLibLineDIntersectsPolygonD(cd(0, 0), cd(10, 10), poly, 4));
            assert( GeoLibLineDIntersectsPolygonD(cd(0, 0), cd(-10, 10), poly, 4));
            assert( GeoLibLineDIntersectsPolygonD(cd(0, 0), cd(10, -10), poly, 4));
            assert( GeoLibLineDIntersectsPolygonD(cd(0, 0), cd(-10, -10), poly, 4));
            assert(!GeoLibLineDIntersectsPolygonD(cd(0, 0), cd(1, 1), poly, 4));
        }
        void TestOrthoDist() {
            const auto p0 = cd(0,0);
            const auto p1 = GeoLibCalcDirectD(p0, /*az=*/0, /*dist=*/1000);
            const auto p2 = GeoLibCalcDirectD(p0, /*az=*/0, /*dist=*/500);      // half way down
            const auto p3 = GeoLibCalcDirectD(p2, /*az=*/M_PI_2, /*dist=*/20);  // to the right
            
            auto res = GeoLibOrthoDistD(p0, p1, p3);
            assertEq(res.segmentLength, 1000, 1e-6);
            assertEq(res.downtrackDistance, 500, 1e-6);      // both positive
            assertEq(res.crosstrackDistance, 20, 1e-6);
            
            const auto p4 = GeoLibCalcDirectD(p2, /*az=*/3 * M_PI_2, /*dist=*/20);  // to the left
            res = GeoLibOrthoDistD(p0, p1, p4);
            assertEq(res.segmentLength, 1000, 1e-6);
            assertEq(res.downtrackDistance, 500, 1e-6);
            assertEq(res.crosstrackDistance, -20, 1e-6);     // crosstrack is negative
            
            const auto p5 = GeoLibCalcDirectD(p0, /*az=*/M_PI, /*dist=*/500);   // go backwards instead
            const auto p6 = GeoLibCalcDirectD(p5, /*az=*/M_PI_2, /*dist=*/20);
            res = GeoLibOrthoDistD(p0, p1, p6);
            assertEq(res.segmentLength, 1000, 1e-6);
            assertEq(res.downtrackDistance, -500, 1e-6);     // downtrack is negative
            assertEq(res.crosstrackDistance, 20, 1e-6);
        }
        void TestLineCircleInt() {
            //bool GeoLibLineDIntersectsCircleD(MaplyCoordinateD startPt, MaplyCoordinateD endPt, MaplyCoordinateD center, double radiusMeters)
            assert( GeoLibLineDIntersectsCircleD(cd(0,0), cd(1,1), cd(0,0), 1000));
            assert( GeoLibLineDIntersectsCircleD(cd(0,0.0001), cd(1,1), cd(0,0), 1000));
            assert( GeoLibLineDIntersectsCircleD(cd(1,1), cd(0,0), cd(0,0), 1000));
            assert( GeoLibLineDIntersectsCircleD(cd(1,1), cd(0.0001,0), cd(0,0), 1000));
            assert(!GeoLibLineDIntersectsCircleD(cd(0,1), cd(1,1), cd(0,0), 1000));
            
            auto p0 = GeoLibCalcDirectD(cd(0,0), /*az=*/M_PI_2, /*dist=*/1000 - 1e-6);
            auto p1 = GeoLibCalcDirectD(p0, /*az=*/0, /*dist=*/100);
            auto p2 = GeoLibCalcDirectD(p0, /*az=*/M_PI, /*dist=*/100);
            assert( GeoLibLineDIntersectsCircleD(p1, p2, cd(0,0), 1000));

            p0 = GeoLibCalcDirectD(cd(0,0), /*az=*/M_PI_2, /*dist=*/1000 + 1e-6);
            p1 = GeoLibCalcDirectD(p0, /*az=*/0, /*dist=*/100);
            p2 = GeoLibCalcDirectD(p0, /*az=*/M_PI, /*dist=*/100);
            assert(!GeoLibLineDIntersectsCircleD(p1, p2, cd(0,0), 1000));

            p0 = GeoLibCalcDirectD(cd(0,0), /*az=*/3*M_PI_2, /*dist=*/1000 - 1e-6);
            p1 = GeoLibCalcDirectD(p0, /*az=*/0, /*dist=*/100);
            p2 = GeoLibCalcDirectD(p0, /*az=*/M_PI, /*dist=*/100);
            assert( GeoLibLineDIntersectsCircleD(p1, p2, cd(0,0), 1000));

            p0 = GeoLibCalcDirectD(cd(0,0), /*az=*/3*M_PI_2, /*dist=*/1000 + 1e-6);
            p1 = GeoLibCalcDirectD(p0, /*az=*/0, /*dist=*/100);
            p2 = GeoLibCalcDirectD(p0, /*az=*/M_PI, /*dist=*/100);
            assert(!GeoLibLineDIntersectsCircleD(p1, p2, cd(0,0), 1000));

            p1 = GeoLibCalcDirectD(p0, /*az=*/0, /*dist=*/1);
            p2 = GeoLibCalcDirectD(p0, /*az=*/0, /*dist=*/100);
            assert(!GeoLibLineDIntersectsCircleD(p1, p2, cd(0,0), 1000));
            
            p1 = GeoLibCalcDirectD(p0, /*az=*/M_PI, /*dist=*/100);
            p2 = GeoLibCalcDirectD(p0, /*az=*/M_PI, /*dist=*/1);
            assert(!GeoLibLineDIntersectsCircleD(p1, p2, cd(0,0), 1000));
        }
        void TestSampleArc() {
            MaplyCoordinateD p[11];
            assertEq(  M_PI/20, GeoLibSampleArcD(cd(0,0), 1000.,  0.,      M_PI_2, true,  p, sizeof(p)/sizeof(p[0])), 1e-6);
            assertEq(3*M_PI/20, GeoLibSampleArcD(cd(0,0), 1000.,  0.,      M_PI_2, false, p, sizeof(p)/sizeof(p[0])), 1e-6);
            assertEq(3*M_PI/20, GeoLibSampleArcD(cd(0,0), 1000.,  M_PI_2,  0,      true,  p, sizeof(p)/sizeof(p[0])), 1e-6);
            assertEq(  M_PI/20, GeoLibSampleArcD(cd(0,0), 1000.,  M_PI_2,  0,      false, p, sizeof(p)/sizeof(p[0])), 1e-6);
            assertEq(  M_PI/20, GeoLibSampleArcD(cd(0,0), 1000., -M_PI_4,  M_PI_4, true,  p, sizeof(p)/sizeof(p[0])), 1e-6);
            assertEq(3*M_PI/20, GeoLibSampleArcD(cd(0,0), 1000., -M_PI_4,  M_PI_4, false, p, sizeof(p)/sizeof(p[0])), 1e-6);
            assertEq(3*M_PI/20, GeoLibSampleArcD(cd(0,0), 1000.,  M_PI_4, -M_PI_4, true,  p, sizeof(p)/sizeof(p[0])), 1e-6);
            assertEq(  M_PI/20, GeoLibSampleArcD(cd(0,0), 1000.,  M_PI_4, -M_PI_4, false, p, sizeof(p)/sizeof(p[0])), 1e-6);
        }
        void TestInitialHeading() {
            // TODO
        }
        void TestLineIntersectsCircle() {
            // TODO
        }
        void TestLineIntersectCircle() {
            // TODO
        }
    } test;
}
#endif

