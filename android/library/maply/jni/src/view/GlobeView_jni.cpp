/*  GlobeView_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/17/15.
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

#import "View_jni.h"
#import "CoordSystem_jni.h"
#import "Geometry_jni.h"
#import "com_mousebird_maply_GlobeView.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> GlobeViewClassInfo *GlobeViewClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeView_nativeInit
  (JNIEnv *env, jclass cls)
{
	GlobeViewClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeView_initialise
  (JNIEnv *env, jobject obj, jobject coordAdapterObj)
{
	try
	{
		CoordSystemDisplayAdapter *coordAdapter = CoordSystemDisplayAdapterInfo::getClassInfo()->getObject(env,coordAdapterObj);
		WhirlyGlobe::GlobeView *inst = new WhirlyGlobe::GlobeView(coordAdapter);
		GlobeViewClassInfo::getClassInfo()->setHandle(env,obj,inst);
	}
    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeView_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		GlobeViewClassInfo *classInfo = GlobeViewClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        WhirlyGlobe::GlobeView *inst = classInfo->getObject(env,obj);
        delete inst;
        classInfo->clearHandle(env,obj);
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeView_nativeClone
  (JNIEnv *env, jobject obj, jobject destObj)
{
	try
	{
		const auto classInfo = GlobeViewClassInfo::getClassInfo();
        const auto src = classInfo->getObject(env,obj);
        if (src)
        {
            auto const clone = new WhirlyGlobe::GlobeView(*src);
            Java_com_mousebird_maply_GlobeView_dispose(env, destObj);
            classInfo->setHandle(env, destObj, clone);
        }
	}
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_GlobeView_getRotQuat
  (JNIEnv *env, jobject obj)
{
	try
	{
        if (const auto view = GlobeViewClassInfo::get(env,obj))
        {
		    return MakeQuaternion(env,view->getRotQuat());
        }
	}
    MAPLY_STD_JNI_CATCH()
	return nullptr;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeView_setRotQuatNative
  (JNIEnv *env, jobject obj, jobject quatObj)
{
	try
	{
        if (const auto view = GlobeViewClassInfo::get(env,obj))
        {
            if (const auto q = QuaternionClassInfo::get(env, quatObj))
            {
                view->setRotQuat(*q);
            }
        }
	}
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_GlobeView_getLoc
  (JNIEnv *env, jobject obj)
{
	try
	{
        if (const auto view = GlobeViewClassInfo::get(env,obj))
        {
            const Point3d localPt = view->currentUp();
            const double height = view->heightAboveSurface();
            const auto adapter = view->getCoordAdapter();
            const GeoCoord geoCoord = adapter->getCoordSystem()->localToGeographic(
                                        adapter->displayToLocal(localPt));
            return MakePoint3d(env, Point3d(geoCoord.lon(), geoCoord.lat(), height));
        }
	}
    MAPLY_STD_JNI_CATCH()
	return nullptr;
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_GlobeView_minHeightAboveSurface
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (const auto view = GlobeViewClassInfo::get(env,obj))
        {
		    return view->minHeightAboveGlobe();
        }
	}
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_GlobeView_getTilt
(JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto view = GlobeViewClassInfo::get(env,obj))
        {
            return view->getTilt();
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}


extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeView_setTilt
(JNIEnv *env, jobject obj, jdouble newTilt)
{
    try
    {
        if (const auto view = GlobeViewClassInfo::get(env,obj))
        {
            view->setTilt(newTilt);
        }
    }
    MAPLY_STD_JNI_CATCH()
}


extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_GlobeView_maxHeightAboveSurface
  (JNIEnv *env, jobject obj)
{
	try
	{
        if (const auto view = GlobeViewClassInfo::get(env,obj))
        {
            return view->maxHeightAboveGlobe();
        }
	}
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_GlobeView_pointOnSphereFromScreen
  (JNIEnv *env, jobject obj, jobject screenPtObj, jobject viewModelMatObj, jobject frameObj, jboolean clip)
{
	try
	{
        if (const auto view = GlobeViewClassInfo::get(env,obj))
        {
            Point2d *screenPt = Point2dClassInfo::getClassInfo()->getObject(env, screenPtObj);
            Matrix4d *viewModelMat = Matrix4dClassInfo::getClassInfo()->getObject(env,viewModelMatObj);
            Point2d *framePt = Point2dClassInfo::getClassInfo()->getObject(env, frameObj);
            Point3d hit;
            Point2f screenPt2f(screenPt->x(), screenPt->y());
            Point2f framePt2f(framePt->x(), framePt->y());
            if (view->pointOnSphereFromScreen(screenPt2f, *viewModelMat, framePt2f, hit, clip))
            {
                return MakePoint3d(env, hit);
            }
        }
	}
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_GlobeView_pointOnScreenFromSphere
  (JNIEnv *env, jobject obj, jobject dispPtObj, jobject viewModelMatObj, jobject frameObj)
{
	try
	{
        if (const auto view = GlobeViewClassInfo::get(env,obj))
        {
            Point3d *dispPt = Point3dClassInfo::getClassInfo()->getObject(env, dispPtObj);
            Matrix4d *viewModelMat = Matrix4dClassInfo::getClassInfo()->getObject(env,viewModelMatObj);
            Point2d *framePt = Point2dClassInfo::getClassInfo()->getObject(env, frameObj);
            Point3d hit;
            Point2f framePt2f(framePt->x(), framePt->y());
            Point2f retPt = view->pointOnScreenFromSphere(*dispPt, viewModelMat, framePt2f);
            return MakePoint2d(env, Point2d(retPt.x(), retPt.y()));
        }
	}
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_GlobeView_getHeight
  (JNIEnv *env, jobject obj)
{
	try
	{
        if (const auto view = GlobeViewClassInfo::get(env,obj))
        {
            return view->heightAboveSurface();
        }
	}
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}


extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeView_setHeight
  (JNIEnv *env, jobject obj, jdouble newHeight)
{
	try
	{
        if (const auto view = GlobeViewClassInfo::get(env,obj))
        {
            view->setHeightAboveGlobe(newHeight);
        }
	}
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeView_setContinuousZoom
(JNIEnv *env, jobject obj, jboolean newVal)
{
    try
    {
        if (const auto view = GlobeViewClassInfo::get(env,obj))
        {
            view->setContinuousZoom(newVal);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_GlobeView_makeRotationToGeoCoord
  (JNIEnv *env, jobject obj, jdouble x, jdouble y, jboolean northUp)
{
	try
	{
        if (const auto view = GlobeViewClassInfo::get(env,obj))
        {
            return MakeQuaternion(env, view->makeRotationToGeoCoord(GeoCoord(x, y),(bool) northUp));
        }
	}
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_GlobeView_prospectiveUp
  (JNIEnv *env, jobject obj, jobject quatObj)
{
	try
	{
        if (const auto view = GlobeViewClassInfo::get(env,obj))
        {
            if (const auto quat = QuaternionClassInfo::get(env, quatObj))
            {
                return MakePoint3d(env, view->prospectiveUp(*quat));
            }
        }
	}
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_GlobeView_pointUnproject
  (JNIEnv *env, jobject obj, jobject touchObj,jobject frameSizeObj,jboolean clip)
{
	try
	{
        if (const auto view = GlobeViewClassInfo::get(env,obj))
        {
            Point2dClassInfo *ptClassInfo = Point2dClassInfo::getClassInfo();
            Point2d *touch = ptClassInfo->getObject(env, touchObj);
            Point2d *frameSize = ptClassInfo->getObject(env, frameSizeObj);
            if (!touch || !frameSize)
                return nullptr;

            Point3d modelPt = view->pointUnproject(Point2f(touch->x(), touch->y()),
                                                   (int) frameSize->x(), (int) frameSize->y(),
                                                   clip);
            return MakePoint3d(env, modelPt);
        }
	}
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}
