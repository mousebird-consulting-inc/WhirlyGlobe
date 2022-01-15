/*  GeographicLib.h
 *  WhirlyGlobeLib
 *
 *  Created by Tim Sylvester on 1/14/22.
 *  Copyright 2022 mousebird consulting
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

#ifndef GeographicLib_h
#define GeographicLib_h

#import "WhirlyVector.h"
#import <tuple>

namespace GeographicLib {
    class Geodesic;
    class Geocentric;
}
namespace WhirlyKit {
    namespace detail {
        // Generic geodesic initialized for WGS84 ellipsoid.
        // We assume this is thread-safe because we only read from it.
        extern const GeographicLib::Geodesic &wgs84Geodesic();
        extern const GeographicLib::Geocentric &wgs84Geocentric();
    }

// Use an extra namespace to clarify that `Point3d`s are geocentric, not 3D Cartesian
namespace Geocentric {

extern bool checkIntersection(
    const Point3d &a, const Point3d &b,
    const Point3d &c, const Point3d &d);

#if defined(GEOGRAPHICLIB_GEODESIC_HPP)
extern bool checkIntersection(
    const Point3d &a, const Point3d &b,
    const Point3d &c, const Point3d &d,
    const GeographicLib::Geodesic &geo);
#endif

extern std::tuple<bool,Point3d> findIntersection(
    const Point3d &a, const Point3d &b,
    const Point3d &c, const Point3d &d);

#if defined(GEOGRAPHICLIB_GEODESIC_HPP)
extern std::tuple<bool,Point3d> findIntersection(
    const Point3d &a, const Point3d &b,
    const Point3d &c, const Point3d &d,
    const GeographicLib::Geodesic &geo);
#endif

extern double initialHeading(const Point3d &startPt, const Point3d &endPt);
extern double finalHeading(const Point3d &startPt, const Point3d &endPt);
extern Point3d orthoDirect(const Point3d &start, double azimuthRad, double distMeters);

extern std::tuple<double,double,double> OrthoDist(const Point3d &gca, const Point3d &gcb, const Point3d &gcc);

}}

#if defined __cplusplus
extern "C" {
#endif

#if defined __cplusplus
}   // extern "C"
#endif

#endif /* GeographicLib_h */
