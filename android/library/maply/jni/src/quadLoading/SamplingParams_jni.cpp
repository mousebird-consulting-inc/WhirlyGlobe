/*
 *  SamplingParams_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg on 3/20/19.
 *  Copyright 2011-2019 mousebird consulting
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

#import "QuadLoading_jni.h"
#import "CoordSystem_jni.h"
#import "Geometry_jni.h"
#import "com_mousebird_maply_SamplingParams.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> SamplingParamsClassInfo *SamplingParamsClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_nativeInit
  (JNIEnv *env, jclass cls)
{
	SamplingParamsClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
	    SamplingParams *params = new SamplingParams();
		SamplingParamsClassInfo::getClassInfo()->setHandle(env,obj,params);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::initialise()");
	}
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		SamplingParamsClassInfo *classInfo = SamplingParamsClassInfo::getClassInfo();
        {
            SamplingParams *params = classInfo->getObject(env,obj);
            std::lock_guard<std::mutex> lock(disposeMutex);
            if (!params)
                return;
            delete params;
            classInfo->clearHandle(env,obj);
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setCoordSystemNative
		(JNIEnv *env, jobject obj, jobject coordSysObj, jobject llObj, jobject urObj)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
        CoordSystemRef *coordSys = CoordSystemRefClassInfo::getClassInfo()->getObject(env,coordSysObj);
        Point3dClassInfo *pt3dClassInfo = Point3dClassInfo::getClassInfo();
        Point3d *ll = pt3dClassInfo->getObject(env,llObj);
        Point3d *ur = pt3dClassInfo->getObject(env,urObj);
		if (!params || !coordSys || !ll || !ur)
		    return;
		params->coordSys = *coordSys;
		params->coordBounds.addPoint(Point2d(ll->x(),ll->y()));
		params->coordBounds.addPoint(Point2d(ur->x(),ur->y()));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::setCoordSystem()");
	}
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_SamplingParams_getCoordSystem
  (JNIEnv *env, jobject obj)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params || !params->coordSys)
		    return NULL;
		return MakeCoordSystem(env,params->coordSys);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::getCoordSystem()");
	}

	return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setMinZoom
  (JNIEnv *env, jobject obj, jint minZoom)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return;
		params->minZoom = minZoom;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::setMinZoom()");
	}
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_SamplingParams_getMinZoom
  (JNIEnv *env, jobject obj)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return 0;
		return params->minZoom;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::getMinZoom()");
	}

	return 0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setMaxZoom
  (JNIEnv *env, jobject obj, jint maxZoom)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return;
		params->maxZoom = maxZoom;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::setMaxZoom()");
	}
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_SamplingParams_getMaxZoom
  (JNIEnv *env, jobject obj)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return 0;
		return params->maxZoom;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::getMaxZoom()");
	}

	return 0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setMaxTiles
  (JNIEnv *env, jobject obj, jint maxTiles)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return;
		params->maxTiles = maxTiles;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::setMaxTiles()");
	}
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_SamplingParams_getMaxTiles
  (JNIEnv *env, jobject obj)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return 0;
		return params->maxTiles;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::getMaxTiles()");
	}

	return 0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setMinImportance__D
  (JNIEnv *env, jobject obj, jdouble minImport)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return;
		params->minImportance = minImport;
		if (params->minImportanceTop == 0.0)
			params->minImportanceTop = minImport;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::setMinImportance()");
	}
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_SamplingParams_getMinImportance
  (JNIEnv *env, jobject obj)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return 0.0;
		return params->minImportance;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::getMinImportance()");
	}

	return 0.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setMinImportanceTop
  (JNIEnv *env, jobject obj, jdouble minImportanceTop)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return;
		params->minImportanceTop = minImportanceTop;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::setMinImportanceTop()");
	}
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_SamplingParams_getMinImportanceTop
  (JNIEnv *env, jobject obj)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return 0.0;
		return params->minImportanceTop;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::getMinImportanceTop()");
	}

	return 0.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setMinImportance__DI
  (JNIEnv *env, jobject obj, jdouble minImport, jint level)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return;
		params->setImportanceLevel(minImport,level);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::setMinImportance()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setCoverPoles
  (JNIEnv *env, jobject obj, jboolean coverPoles)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return;
		params->coverPoles = coverPoles;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::setCoverPoles()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_SamplingParams_getCoverPoles
  (JNIEnv *env, jobject obj)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return false;
		return params->coverPoles;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::getCoverPoles()");
	}

	return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setEdgeMatching
  (JNIEnv *env, jobject obj, jboolean edgeMatching)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return;
		params->edgeMatching = edgeMatching;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::setEdgeMatching()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_SamplingParams_getEdgeMatching
  (JNIEnv *env, jobject obj)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return false;
		return params->edgeMatching;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::getEdgeMatching()");
	}

	return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setTesselation
  (JNIEnv *env, jobject obj, jint tessX, jint tessY)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return;
		params->tessX = tessX;
		params->tessY = tessY;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::setTesselation()");
	}
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_SamplingParams_getTesselationX
  (JNIEnv *env, jobject obj)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return 0;
		return params->tessX;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::getTesselationX()");
	}

	return 0;
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_SamplingParams_getTesselationY
  (JNIEnv *env, jobject obj)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return 0;
		return params->tessY;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::getTesselationY()");
	}

	return 0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setForceMinLevel
		(JNIEnv *env, jobject obj, jboolean forceMinLevel)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
			return;
		params->forceMinLevel = forceMinLevel;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::setForceMinLevel()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setSingleLevel
  (JNIEnv *env, jobject obj, jboolean singleLevel)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return;
		params->singleLevel = singleLevel;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::setSingleLevel()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_SamplingParams_getSingleLevel
  (JNIEnv *env, jobject obj)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return false;
		return params->singleLevel;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::getSingleLevel()");
	}

	return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setLevelLoads
  (JNIEnv *env, jobject obj, jintArray levelArray)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return;
		std::vector<int> levels;
        ConvertIntArray(env,levelArray,levels);
		params->levelLoads = levels;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::setLevelLoads()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setClipBounds
  (JNIEnv *env, jobject obj, jdouble llx, jdouble lly, jdouble urx, jdouble ury)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
		    return;
		MbrD mbr;
		mbr.addPoint(Point2d(llx,lly));
		mbr.addPoint(Point2d(urx,ury));
		params->clipBounds = mbr;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::setClipBounds()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_SamplingParams_equalsNative
		(JNIEnv *env, jobject obj, jobject otherObj)
{
	try
	{
		SamplingParamsClassInfo *classInfo = SamplingParamsClassInfo::getClassInfo();
		SamplingParams *paramsA = classInfo->getObject(env,obj);
		SamplingParams *paramsB = classInfo->getObject(env,otherObj);
		if (!paramsA || !paramsB)
			return false;
		return *paramsA == *paramsB;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SamplingParams::equalsNative()");
	}

	return false;
}