/*  SamplingParams_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg on 3/20/19.
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

#import "QuadLoading_jni.h"
#import "CoordSystem_jni.h"
#import "Geometry_jni.h"
#import "com_mousebird_maply_SamplingParams.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> SamplingParamsClassInfo *SamplingParamsClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_nativeInit
  (JNIEnv *env, jclass cls)
{
	SamplingParamsClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
	    SamplingParams *params = new SamplingParams();
		SamplingParamsClassInfo::getClassInfo()->setHandle(env,obj,params);
	}
	MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		SamplingParamsClassInfo *classInfo = SamplingParamsClassInfo::getClassInfo();
		std::lock_guard<std::mutex> lock(disposeMutex);
		SamplingParams *params = classInfo->getObject(env,obj);
		delete params;
		classInfo->clearHandle(env,obj);
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
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
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_SamplingParams_getCoordSystem
  (JNIEnv *env, jobject obj)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::get(env,obj);
		if (params && params->coordSys)
		{
			return MakeCoordSystem(env,params->coordSys);
		}
	}
	MAPLY_STD_JNI_CATCH()
	return nullptr;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setReportedMaxZoom
		(JNIEnv *env, jobject obj, jint maxZoom)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
			return;
		params->reportedMaxZoom = maxZoom;
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_SamplingParams_getReportedMaxZoom
		(JNIEnv *env, jobject obj)
{
	try
	{
		SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,obj);
		if (!params)
			return 0;
		return params->reportedMaxZoom;
	}
	MAPLY_STD_JNI_CATCH()
	return 0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setMinZoom
  (JNIEnv *env, jobject obj, jint minZoom)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			// See [MaplySamplingParams setMinZoom:]
			if (minZoom > 1)
			{
				wkLogLevel(Error, "\n============Error===============\n"
				                  "Do not set MaplySamplingParams minZoom to anything more than 1.\n"
				                  "Instead, set the minZoom of your tileSource to the right number.\n"
				                  "============Error===============\n");
			}

			params->minZoom = std::max(0, std::min(1, minZoom));
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_SamplingParams_getMinZoom
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			return params->minZoom;
		}
	}
	MAPLY_STD_JNI_CATCH()
	return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setMaxZoom
  (JNIEnv *env, jobject obj, jint maxZoom)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			params->maxZoom = maxZoom;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_SamplingParams_getMaxZoom
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			return params->maxZoom;
		}
	}
	MAPLY_STD_JNI_CATCH()
	return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setMaxTiles
  (JNIEnv *env, jobject obj, jint maxTiles)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			params->maxTiles = maxTiles;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_SamplingParams_getMaxTiles
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			return params->maxTiles;
		}
	}
	MAPLY_STD_JNI_CATCH()
	return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setMinImportance__D
  (JNIEnv *env, jobject obj, jdouble minImport)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			params->minImportance = minImport;
			if (params->minImportanceTop == 0.0)
				params->minImportanceTop = minImport;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_SamplingParams_getMinImportance
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			return params->minImportance;
		}
	}
	MAPLY_STD_JNI_CATCH()
	return 0.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setMinImportanceTop
  (JNIEnv *env, jobject obj, jdouble minImportanceTop)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			params->minImportanceTop = minImportanceTop;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_SamplingParams_getMinImportanceTop
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			return params->minImportanceTop;
		}
	}
	MAPLY_STD_JNI_CATCH()
	return 0.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setMinImportance__DI
  (JNIEnv *env, jobject obj, jdouble minImport, jint level)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			params->setImportanceLevel(minImport,level);
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setCoverPoles
  (JNIEnv *env, jobject obj, jboolean coverPoles)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			params->coverPoles = coverPoles;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_SamplingParams_getCoverPoles
  (JNIEnv *env, jobject obj)
{
	try
	{
		const auto params = SamplingParamsClassInfo::get(env,obj);
		return params && params->coverPoles;
	}
	MAPLY_STD_JNI_CATCH()
	return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setEdgeMatching
  (JNIEnv *env, jobject obj, jboolean edgeMatching)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			params->edgeMatching = edgeMatching;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_SamplingParams_getEdgeMatching
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			return params->edgeMatching;
		}
	}
	MAPLY_STD_JNI_CATCH()
	return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setTesselation
  (JNIEnv *env, jobject obj, jint tessX, jint tessY)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			params->tessX = tessX;
			params->tessY = tessY;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_SamplingParams_getTesselationX
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			return params->tessX;
		}
	}
	MAPLY_STD_JNI_CATCH()
	return 0;
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_SamplingParams_getTesselationY
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			return params->tessY;
		}
	}
	MAPLY_STD_JNI_CATCH()
	return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setForceMinLevel
		(JNIEnv *env, jobject obj, jboolean forceMinLevel)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			params->forceMinLevel = forceMinLevel;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setSingleLevel
  (JNIEnv *env, jobject obj, jboolean singleLevel)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			params->singleLevel = singleLevel;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_SamplingParams_getSingleLevel
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			return params->singleLevel;
		}
	}
	MAPLY_STD_JNI_CATCH()
	return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setLevelLoads
  (JNIEnv *env, jobject obj, jintArray levelArray)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			std::vector<int> levels;
			ConvertIntArray(env,levelArray,levels);
			params->levelLoads = levels;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setBoundsScale
		(JNIEnv *env, jobject obj, jfloat scale)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			params->boundsScale = scale;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SamplingParams_setClipBounds
  (JNIEnv *env, jobject obj, jdouble llx, jdouble lly, jdouble urx, jdouble ury)
{
	try
	{
		if (const auto params = SamplingParamsClassInfo::get(env,obj))
		{
			MbrD mbr;
			mbr.addPoint(Point2d(llx,lly));
			mbr.addPoint(Point2d(urx,ury));
			params->clipBounds = mbr;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_SamplingParams_equalsNative
		(JNIEnv *env, jobject obj, jobject otherObj)
{
	try
	{
		SamplingParamsClassInfo *classInfo = SamplingParamsClassInfo::getClassInfo();
		const auto paramsA = classInfo->getObject(env,obj);
		const auto paramsB = classInfo->getObject(env,otherObj);
		return paramsA && paramsB && *paramsA == *paramsB;
	}
	MAPLY_STD_JNI_CATCH()
	return false;
}