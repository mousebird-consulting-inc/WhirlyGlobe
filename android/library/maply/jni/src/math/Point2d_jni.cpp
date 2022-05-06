/*  Point2d_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2022 mousebird consulting
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
#import "Geometry_jni.h"
#import "com_mousebird_maply_Point2d.h"
#import "GeographicLib.h"
#import <GeographicLib/Geodesic.hpp>

template<> Point2dClassInfo *Point2dClassInfo::classInfoObj = nullptr;

using namespace Eigen;
using namespace WhirlyKit;

JNIEXPORT jobject JNICALL MakePoint2d(JNIEnv *env)
{
	auto classInfo = Point2dClassInfo::getClassInfo(env,"com/mousebird/maply/Point2d");
	return classInfo->makeWrapperObject(env,nullptr);
}

JNIEXPORT jobject JNICALL MakePoint2d(JNIEnv *env,const WhirlyKit::Point2d &pt)
{
	jobject newObj = MakePoint2d(env);
	if (auto inst = Point2dClassInfo::get(env,newObj))
	{
		*inst = pt;
		return newObj;
	}
	return nullptr;
}


extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Point2d_nativeInit(JNIEnv *env, jclass cls)
{
	Point2dClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Point2d_initialise(JNIEnv *env, jobject obj)
{
	try
	{
		if (auto *pt = Point2dClassInfo::get(env,obj))
		{
			pt->x() = pt->y() = 0;
		}
		else
		{
			Point2dClassInfo::set(env, obj, new Point2d(0, 0));
		}
	}
	MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Point2d_dispose(JNIEnv *env, jobject obj)
{
	try
	{
		Point2dClassInfo *classInfo = Point2dClassInfo::getClassInfo();
		std::lock_guard<std::mutex> lock(disposeMutex);
		delete classInfo->getObject(env,obj);
		classInfo->clearHandle(env,obj);
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point2d_getX(JNIEnv *env, jobject obj)
{
	try
	{
		if (auto *pt = Point2dClassInfo::get(env,obj))
		{
			return pt->x();
		}
	}
	MAPLY_STD_JNI_CATCH()
	return 0.0;
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point2d_getY(JNIEnv *env, jobject obj)
{
	try
	{
		if (auto *pt = Point2dClassInfo::get(env,obj))
		{
			return pt->y();
		}
	}
	MAPLY_STD_JNI_CATCH()
	return 0.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Point2d_setValue(JNIEnv *env, jobject obj, jdouble x, jdouble y)
{
	try
	{
		Point2dClassInfo *classInfo = Point2dClassInfo::getClassInfo();
		if (auto pt = classInfo->getObject(env,obj))
		{
			pt->x() = x;
			pt->y() = y;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point2d_getGeoDist
	(JNIEnv *env, jobject obj, jobject otherObj)
{
	if (env->IsSameObject(obj, otherObj))
	{
		return 0;
	}
	try
	{
		if (const auto pt = Point2dClassInfo::get(env, obj))
		if (const auto other = Point2dClassInfo::get(env, otherObj))
		{
			double dist = 0.0, t = 0.0;
			using WhirlyKit::detail::wgs84Geodesic;
			wgs84Geodesic().GenInverse(RadToDeg(pt->y()), RadToDeg(pt->x()),
									   RadToDeg(other->y()), RadToDeg(other->x()),
									   GeographicLib::Geodesic::DISTANCE,
									   dist, t, t, t, t, t, t);
			return dist;
		}
	}
	MAPLY_STD_JNI_CATCH()
	return -1.0;
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point2d_getGeoAzimuth
		(JNIEnv *env, jobject obj, jobject otherObj)
{
	if (env->IsSameObject(obj, otherObj))
	{
		return 0;
	}
	try
	{
		if (const auto pt = Point2dClassInfo::get(env, obj))
		if (const auto other = Point2dClassInfo::get(env, otherObj))
		{
			// Azimuth from a point to itself is undefined, return zero for consistency.
			if (*pt == *other)
			{
				return 0;
			}
			double az1 = 0.0, t = 0.0;
			using WhirlyKit::detail::wgs84Geodesic;
			wgs84Geodesic().GenInverse(RadToDeg(pt->y()), RadToDeg(pt->x()),
									   RadToDeg(other->y()), RadToDeg(other->x()),
									   GeographicLib::Geodesic::AZIMUTH,
									   t, az1, t, t, t, t, t);
			return DegToRad(az1);
		}
	}
	MAPLY_STD_JNI_CATCH()
	return std::numeric_limits<double>::quiet_NaN();
}

static constexpr double EARTH_RADIUS_METERS = 6371009.0;
static inline double haversine(double lat1, double lon1, double lat2, double lon2)
{
	using namespace std;
	const double dPhi = sin((lat2 - lat1) / 2);
	const double dLambda = sin((lon2 - lon1) / 2);
	const double h = dPhi*dPhi + dLambda*dLambda * cos(lat1) * cos(lat2);
	if (h == 1.0)	// todo: >0.9999... ?
	{
		return M_PI_2;	// antipode
	}
	return atan2(sqrt(h), sqrt(1 - h));
}
static inline double haversineDist(double lat1, double lon1, double lat2, double lon2)
{
	return haversine(lat1, lon1, lat2, lon2) * 2.0 * EARTH_RADIUS_METERS;
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point2d_getGeoDistApprox
		(JNIEnv *env, jobject obj, jobject otherObj)
{
	if (env->IsSameObject(obj, otherObj))
	{
		return 0;
	}
	try
	{
		if (const auto pt = Point2dClassInfo::get(env, obj))
		if (const auto other = Point2dClassInfo::get(env, otherObj))
		{
			return haversineDist(pt->y(), pt->x(), other->y(), other->x());
		}
	}
	MAPLY_STD_JNI_CATCH()
	return -1.0;
}
