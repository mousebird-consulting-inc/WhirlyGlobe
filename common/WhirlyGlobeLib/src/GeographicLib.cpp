//
//  GeographicLib.cpp
//  WhirlyGlobeMaplyComponent
//
//  Created by Tim Sylvester on 1/14/22.
//  Copyright © 2022 mousebird consulting. All rights reserved.
//

#import "GeographicLib/Geodesic.hpp"
#import "GeographicLib/Geocentric.hpp"

#import "../include/GeographicLib.h"
#import "WhirlyVector.h"

#import <cmath>

#if !defined(M_2PI)
# define M_2PI (2 * M_PI)
#endif

namespace WhirlyKit {

namespace detail {
    // These have to be functions for now; if they're global variables,
    // they don't get initialized early enough for the static unit tests.
    const GeographicLib::Geodesic &wgs84Geodesic() { return GeographicLib::Geodesic::WGS84(); }
    const GeographicLib::Geocentric &wgs84Geocentric() { return GeographicLib::Geocentric::WGS84(); }
}

using namespace detail;
namespace Geocentric {

template <int N>
inline static void combine(double* eqA, double* eqB)
{
    if (eqB[0] == 0.0)
    {
        return;
    }
    if (std::fabs(eqA[0]) >= std::fabs(eqB[0]))
    {
        auto const f = eqB[0] / eqA[0];
        eqB[0] = 0;
        for (auto i = 1; i < N; ++i)
        {
            eqB[i] -= f * eqA[i];
        }
    }
    else
    {
        auto const f = eqA[0] / eqB[0];
        eqA[0] = eqB[0];
        eqB[0] = 0;
        for (auto i = 1; i < N; ++i)
        {
            std::swap(eqA[i], eqB[i]);
            eqB[i] -= f * eqA[i];
        }
    }
}

// for n = 3, partially solve, zeroing the coefficients below the diagonal.
inline static void solveLinear3A(double* eq0, double* eq1, double* eq2)
{
    combine<4>(eq1, eq2);
    combine<4>(eq0, eq1);
    combine<3>(eq1 + 1, eq2 + 1);

    if (eq2[3] != 0.0)
    {
        eq2[3] = (eq2[2] == 0 ? INFINITY : eq2[3] / eq2[2]);
    }
    eq2[2] = 1;
}

// Complete the solution started by solveLinear3Lower
inline static void solveLinear3B(double* eq0, double* eq1, double* eq2)
{
    eq1[3] -= eq1[2] * eq2[3];
    eq1[2] = 0;
    if (eq1[3] != 0.0)
    {
        eq1[3] = (eq1[1] == 0.0 ? INFINITY : eq1[3] / eq1[1]);
    }
    eq1[1] = 1;
    eq0[3] -= eq0[2] * eq2[3] + eq0[1] * eq1[3];
    eq0[1] = 0;
    eq0[2] = 0;
    if (eq0[3] != 0)
    {
        eq0[3] = (eq0[0] == 0.0 ? INFINITY : eq0[3] / eq0[0]);
    }
    eq0[0] = 1;
}

// Project a vector onto the ellipsoid surface
static Point3d project(const Point3d &p, const GeographicLib::Geodesic &geo)
{
    const auto re = geo.EquatorialRadius();
    const auto rp = re - re * geo.Flattening();
    const auto x = p.x();
    const auto y = p.y();
    const auto z = p.z();
    const auto r = std::sqrt((x * x + y * y) / (re * re) + z * z / (rp * rp));
    return (r == 0) ? Point3d{ rp, y, z } :            // it's a pole
                      Point3d{ x / r, y / r, z / r };  // anything else
}

const static Point3d ptZero = { 0.0, 0.0, 0.0 };

// Find the intersection of geodesics A-B and C-D.
// Returns the intersection point iff CalcInt==true
template <bool CalcInt>
static inline std::tuple<bool,Point3d> intersection(
    const Point3d &a, const Point3d &b, const Point3d &c, const Point3d &d,
    const GeographicLib::Geodesic &geo)
{
    // Set equal the equations defining the two geodesics, as defined by the
    // intersection of the ellipsoid and a plane through its center, and solve
    // the resulting system of equations, yielding two antipodal solutions.
    //
    // ((x4 - x3) * s + x3) * t = (x2 - x1) * r + x1
    // ((y4 - y3) * s + y3) * t = (y2 - y1) * r + y1
    // ((z4 - z3) * s + z3) * t = (z2 - z1) * r + z1

    double e0[4] = { d.x() - c.x(), c.x(), a.x() - b.x(), a.x() };
    double e1[4] = { d.y() - c.y(), c.y(), a.y() - b.y(), a.y() };
    double e2[4] = { d.z() - c.z(), c.z(), a.z() - b.z(), a.z() };
    solveLinear3A(e0, e1, e2);

    const auto r = e2[3];
    if (r >= 0.0 && r <= 1.0)
    {
        solveLinear3B(e0, e1, e2);

        // Solution on the right side, and within the segments?
        const auto t = e1[3];
        if (t > 0.0)
        {
            const auto s = e0[3] / t;
            if (s >= 0.0 && s <= 1.0)
            {
                // Yes! Project the solution vector onto the ellipsoid
                return std::make_tuple(true,
                    CalcInt ? project((b - a) * r + a, geo) : ptZero);
            }
        }
    }

    return std::make_tuple(false, ptZero);
}

// Concrete instances for export
bool checkIntersection(const Point3d &a, const Point3d &b, const Point3d &c, const Point3d &d)                                                        { return checkIntersection(a, b, c, d, wgs84Geodesic()); }
bool checkIntersection(const Point3d &a, const Point3d &b, const Point3d &c, const Point3d &d, const GeographicLib::Geodesic &geo)                    { return std::get<0>(intersection<false>(a, b, c, d, geo)); }
std::tuple<bool,Point3d> findIntersection(const Point3d &a, const Point3d &b, const Point3d &c, const Point3d &d, const GeographicLib::Geodesic &geo) { return intersection<true>(a, b, c, d, geo); }
std::tuple<bool,Point3d> findIntersection(const Point3d &a, const Point3d &b, const Point3d &c, const Point3d &d)                                     { return findIntersection(a, b, c, d, wgs84Geodesic()); }


// Project a geocentric point into the orthographic projection defined by the origin
static Point3d projectOrtho(const Point3d &origin, const Point3d &p)
{
    const double sinLat =  origin.z();
    const double cosLat =  std::sqrt(origin.x() * origin.x() + origin.y() * origin.y());
    const double sinLon =  origin.x() / cosLat;
    const double cosLon = -origin.y() / cosLat;
    const double x1 =  p.x() * cosLon + p.y() * sinLon;
    const double y1 = -p.x() * sinLon + p.y() * cosLon;
    return { x1, y1 * sinLat + p.z() * cosLat,  -y1 * cosLat + p.z() * sinLat };
}

// Invert orthographic projection
static Point3d unprojectOrtho(const Point3d &origin, const Point3d &p)
{
    const double sinLat =  origin.z();
    const double cosLat =  std::sqrt(origin.x() * origin.x() + origin.y() * origin.y());
    const double sinLon =  origin.x() / cosLat;
    const double cosLon = -origin.y() / cosLat;
    const double y1  =  p.y() * sinLat  -  p.z() * cosLat;
    const double z1  =  p.y() * cosLat  +  p.z() * sinLat;
    return { p.x() * cosLon - y1 * sinLon, p.x() * sinLon + y1 * cosLon, z1 };
}

static const double sinpi4 = std::sin(M_PI_4);

// Calculate the angle between two vectors.
// Both vectors must be normalized!
static double angle(Point3d a, Point3d b)
{
    const double dp = a.dot(b);
    if (std::fabs(dp) < sinpi4)
    {
        return std::acos(dp);
    }

    const auto m = a.cross(b).norm();
    return (dp < 0) ? M_PI - std::asin(m) : std::asin(m);
}

// TODO: This isn't fully accounting for eccentricity, so it's not super precise.
// (start,end,other)=>(downtrack,crosstrack,length)
std::tuple<double,double,double> OrthoDist(const Point3d &gca, const Point3d &gcb, const Point3d &gcc)
{
    // Calculate the unit normal of the geodesic segment
    const auto geoNorm = gca.cross(gcb).normalized();

    // Calculate the unit normal of that and the point of interest
    const auto orthoNorm = gcc.cross(geoNorm).normalized();

    // Find the the point where the line to the target point is perpendicular
    const auto cp = geoNorm.cross(orthoNorm);

    // Calculate the angles along and aside the segment
    const auto t0 = angle(gca, gcb);
    const auto t1 = angle(cp, gca);
    const auto t2 = angle(cp, gcc);

    // Work out which quadrant we're in and fix the signs
    const auto s1 = (cp.dot(geoNorm.cross(gca)) < 0) ? -1. : 1.;
    const auto s2 = (gcc.dot(geoNorm) > 0) ? -1. : 1.;

    // Convert the angles to distances
    const auto rad = GeographicLib::Constants::WGS84_a();
    return { rad * t1 * s1, rad * t2 * s2, rad * t0 };
}

double initialHeading(const Point3d &startPt, const Point3d &endPt)
{
    // Find the location of the endpoint in the orthographic projection defined by the start point
    const auto end = projectOrtho(startPt, endPt);

    // If x==y==0 they are the same point and the heading is undefined
    return std::atan2(end.x(), end.y());
}
double finalHeading(const Point3d &startPt, const Point3d &endPt)
{
    return std::fmod(initialHeading(endPt, startPt) + M_2PI, M_2PI);
}

Point3d orthoDirect(const Point3d &start, double azimuthRad, double distMeters)
{
    const double theta = distMeters / wgs84Geodesic().EquatorialRadius();
    const double r     = sin(theta);
    const double hdg   = M_PI_2 - azimuthRad;
    return unprojectOrtho(start, { r * std::cos(hdg), r * std::sin(hdg), std::cos(theta) });
}

}}
