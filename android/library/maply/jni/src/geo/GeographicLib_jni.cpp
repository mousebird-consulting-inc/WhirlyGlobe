/*  GeographicLib_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Tim Sylvester on 24 Sep. 2021
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

#import <jni.h>
#import "GeographicLib_jni.h"
#import "Geometry_jni.h"
#import "GeographicLib/Geodesic.hpp"
#import "GeographicLib/Geocentric.hpp"

using namespace Eigen;
using namespace WhirlyKit;

#import <tuple>
#import <cmath>

#if !defined(M_2PI)
# define M_2PI (2 * M_PI)
#endif

namespace
{
	// Generic geodesic initialized for WGS84 ellipsoid.
	// We assume this is thread-safe because we only read from it.
	const GeographicLib::Geodesic &wgs84Geodesic = GeographicLib::Geodesic::WGS84();	//NOLINT
	const GeographicLib::Geocentric &wgs84Geocentric = GeographicLib::Geocentric::WGS84();	//NOLINT

	// Class info looked up on-demand and kept with weak refs.
	// Classes can be unloaded by the JVM if no objects are allocated, and they never are for this class.
	// However this serves as an automatic cache that we don't have to worry about cleaning up.
	jweak clsInverse = nullptr;
	jmethodID clsInverseInit = nullptr;

	jclass getInverseClass(JNIEnv *env)
	{
		if (auto c = env->NewLocalRef(clsInverse))
		{
			return (jclass)c;
		}
		if (auto c = env->FindClass("com/mousebird/maply/GeographicLib$Inverse"))
		{
			clsInverse = env->NewWeakGlobalRef(c);
			clsInverseInit = env->GetMethodID(c, "<init>", "(DDD)V");
			return c;
		}
		clsInverse = nullptr;
		clsInverseInit = nullptr;
		return nullptr;
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GeographicLib_nativeInit
  (JNIEnv *env, jclass cls)
{
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_GeographicLib_calcDirect
  (JNIEnv *env, jclass, jdouble lon, jdouble lat, jdouble az, jdouble dist)
{
	try
	{
		const auto lat1 = WhirlyKit::RadToDeg(lat);
		const auto lon1 = WhirlyKit::RadToDeg(lon);
		const auto azDeg = WhirlyKit::RadToDeg(az);

		double lat2 = 0.0, lon2 = 0.0;
		const auto res = wgs84Geodesic.Direct(lat1, lon1, azDeg, dist, lat2, lon2);

		if (std::isfinite(res))
		{
			const auto info = Point2dClassInfo::getClassInfo();
			const auto pt = new Point2d { DegToRad(lon2), DegToRad(lat2) };
			return info->makeWrapperObject(env, pt);
		}
	}
	MAPLY_STD_JNI_CATCH()
	return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_GeographicLib_calcDirectPt
  (JNIEnv *env, jclass cls, jobject originObj, jdouble az, jdouble dist)
{
	try
	{
		if (const auto pt = Point2dClassInfo::get(env, originObj))
		{
			return Java_com_mousebird_maply_GeographicLib_calcDirect(env, cls, pt->x(), pt->y(), az, dist);
		}
	}
	MAPLY_STD_JNI_CATCH()
	return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_GeographicLib_calcInverse
		(JNIEnv *env, jclass, jdouble lon1, jdouble lat1, jdouble lon2, jdouble lat2)
{
	try
	{
		if (auto clsInv = getInverseClass(env))
		{
			const auto dLat1 = WhirlyKit::RadToDeg(lat1);
			const auto dLon1 = WhirlyKit::RadToDeg(lon1);
			const auto dLat2 = WhirlyKit::RadToDeg(lat2);
			const auto dLon2 = WhirlyKit::RadToDeg(lon2);
			double dist = 0.0, az1 = 0.0, az2 = 0.0;
			wgs84Geodesic.Inverse(dLat1, dLon1, dLat2, dLon2, dist, az1, az2);
			return env->NewObject(clsInv, clsInverseInit, dist, DegToRad(az1), DegToRad(az2));
		}
	}
	MAPLY_STD_JNI_CATCH()
	return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_GeographicLib_calcInversePt
		(JNIEnv *env, jclass cls, jobject po1, jobject po2)
{
	try
	{
		if (const auto pt1 = Point2dClassInfo::get(env, po1))
		if (const auto pt2 = Point2dClassInfo::get(env, po2))
		{
			return Java_com_mousebird_maply_GeographicLib_calcInverse(env, cls, pt1->x(), pt1->y(), pt2->x(), pt2->y());
		}
	}
	MAPLY_STD_JNI_CATCH()
	return nullptr;
}
