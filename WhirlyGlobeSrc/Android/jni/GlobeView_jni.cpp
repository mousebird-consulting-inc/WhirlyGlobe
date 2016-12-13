/*
 *  GlobeView_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/17/15.
 *  Copyright 2011-2016 mousebird consulting
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

#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_GlobeView.h"
#import "Maply_utils_jni.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeView_nativeInit
  (JNIEnv *env, jclass cls)
{
	GlobeViewClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeView_initialise
  (JNIEnv *env, jobject obj, jobject coordAdapterObj)
{
	try
	{
		CoordSystemDisplayAdapter *coordAdapter = CoordSystemDisplayAdapterInfo::getClassInfo()->getObject(env,coordAdapterObj);
		WhirlyGlobe::GlobeView *inst = new WhirlyGlobe::GlobeView(coordAdapter);
		GlobeViewClassInfo::getClassInfo()->setHandle(env,obj,inst);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeView::initialise()");
	}
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeView_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		GlobeViewClassInfo *classInfo = GlobeViewClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            WhirlyGlobe::GlobeView *inst = classInfo->getObject(env,obj);
            if (!inst)
                return;
            delete inst;

            classInfo->clearHandle(env,obj);
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeView::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeView_nativeClone
  (JNIEnv *env, jobject obj, jobject destObj)
{
	try
	{
		GlobeViewClassInfo *classInfo = GlobeViewClassInfo::getClassInfo();
		WhirlyGlobe::GlobeView *src = classInfo->getObject(env,obj);
		WhirlyGlobe::GlobeView *dest = new WhirlyGlobe::GlobeView(*src);
		GlobeViewClassInfo::getClassInfo()->setHandle(env,destObj,dest);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeView::dispose()");
	}
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_GlobeView_getRotQuat
  (JNIEnv *env, jobject obj)
{
	try
	{
		GlobeViewClassInfo *classInfo = GlobeViewClassInfo::getClassInfo();
		WhirlyGlobe::GlobeView *view = classInfo->getObject(env,obj);
		if (!view)
			return NULL;

		return MakeQuaternion(env,view->getRotQuat());
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeView::getRotQuat()");
	}

	return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeView_setRotQuatNative
  (JNIEnv *env, jobject obj, jobject quatObj)
{
	try
	{
		GlobeViewClassInfo *classInfo = GlobeViewClassInfo::getClassInfo();
		WhirlyGlobe::GlobeView *view = classInfo->getObject(env,obj);
		QuaternionClassInfo *qClassInfo = QuaternionClassInfo::getClassInfo();
		Quaterniond *q = qClassInfo->getObject(env,quatObj);
		if (!view || !q)
			return;

		view->setRotQuat(*q);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeView::setRotQuat()");
	}
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_GlobeView_getLoc
  (JNIEnv *env, jobject obj)
{
	try
	{
		GlobeViewClassInfo *classInfo = GlobeViewClassInfo::getClassInfo();
		WhirlyGlobe::GlobeView *view = classInfo->getObject(env,obj);
		if (!view)
			return NULL;

	    Point3d localPt = view->currentUp();
	    double height = view->heightAboveSurface();
	    GeoCoord geoCoord = view->coordAdapter->getCoordSystem()->localToGeographic(view->coordAdapter->displayToLocal(localPt));
	    Point3d pos;
	    pos.x() = geoCoord.lon();  pos.y() = geoCoord.lat();
	    pos.z() = height;

		return MakePoint3d(env,pos);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeView::getLoc()");
	}

	return NULL;
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_GlobeView_minHeightAboveSurface
  (JNIEnv *env, jobject obj)
{
	try
	{
		GlobeViewClassInfo *classInfo = GlobeViewClassInfo::getClassInfo();
		WhirlyGlobe::GlobeView *view = classInfo->getObject(env,obj);
		if (!view)
			return 0.0;

		return view->minHeightAboveGlobe();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeView::minHeightAboveSurface()");
	}
    
    return 0.0;
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_GlobeView_getTilt
(JNIEnv *env, jobject obj)
{
    try
    {
        GlobeViewClassInfo *classInfo = GlobeViewClassInfo::getClassInfo();
        WhirlyGlobe::GlobeView *view = classInfo->getObject(env,obj);
        if (!view)
            return 0.0;
        
        return view->getTilt();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeView::getTilt()");
    }
    
    return 0.0;
}


JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeView_setTilt
(JNIEnv *env, jobject obj, jdouble newTilt)
{
    try
    {
        GlobeViewClassInfo *classInfo = GlobeViewClassInfo::getClassInfo();
        WhirlyGlobe::GlobeView *view = classInfo->getObject(env,obj);
        if (!view)
            return;
        
        view->setTilt(newTilt);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeView::setTilt()");
    }
}


JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_GlobeView_maxHeightAboveSurface
  (JNIEnv *env, jobject obj)
{
	try
	{
		GlobeViewClassInfo *classInfo = GlobeViewClassInfo::getClassInfo();
		WhirlyGlobe::GlobeView *view = classInfo->getObject(env,obj);
		if (!view)
			return 0.0;

		return view->maxHeightAboveGlobe();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeView::maxHeightAboveSurface()");
	}
    
    return 0.0;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_GlobeView_pointOnSphereFromScreen
  (JNIEnv *env, jobject obj, jobject screenPtObj, jobject viewModelMatObj, jobject frameObj, jboolean clip)
{
	try
	{
		GlobeViewClassInfo *classInfo = GlobeViewClassInfo::getClassInfo();
		WhirlyGlobe::GlobeView *view = classInfo->getObject(env,obj);
		if (!view)
			return NULL;
		Point2d *screenPt = Point2dClassInfo::getClassInfo()->getObject(env,screenPtObj);
		Matrix4d *viewModelMat = Matrix4dClassInfo::getClassInfo()->getObject(env,viewModelMatObj);
		Point2d *framePt = Point2dClassInfo::getClassInfo()->getObject(env,frameObj);
		Point3d hit;

		Point2f screenPt2f(screenPt->x(),screenPt->y());
		Point2f framePt2f(framePt->x(),framePt->y());
		if (view->pointOnSphereFromScreen(screenPt2f,viewModelMat,framePt2f,&hit,clip))
		{
			return MakePoint3d(env,hit);
		} else
			return NULL;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapView::pointOnPlaneFromScreen()");
	}
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_GlobeView_pointOnScreenFromSphere
  (JNIEnv *env, jobject obj, jobject dispPtObj, jobject viewModelMatObj, jobject frameObj)
{
	try
	{
		GlobeViewClassInfo *classInfo = GlobeViewClassInfo::getClassInfo();
		WhirlyGlobe::GlobeView *view = classInfo->getObject(env,obj);
		if (!view)
			return NULL;
		Point3d *dispPt = Point3dClassInfo::getClassInfo()->getObject(env,dispPtObj);
		Matrix4d *viewModelMat = Matrix4dClassInfo::getClassInfo()->getObject(env,viewModelMatObj);
		Point2d *framePt = Point2dClassInfo::getClassInfo()->getObject(env,frameObj);
		Point3d hit;

		Point2f framePt2f(framePt->x(),framePt->y());
		Point2f retPt = view->pointOnScreenFromSphere(*dispPt,viewModelMat,framePt2f);

		return MakePoint2d(env,Point2d(retPt.x(),retPt.y()));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapView::pointOnPlaneFromScreen()");
	}
    
    return NULL;
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_GlobeView_getHeight
  (JNIEnv *env, jobject obj)
{
	try
	{
		GlobeViewClassInfo *classInfo = GlobeViewClassInfo::getClassInfo();
		WhirlyGlobe::GlobeView *view = classInfo->getObject(env,obj);
		if (!view)
			return 0.0;

	    return view->heightAboveSurface();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeView::getHeight()");
	}
    
    return 0.0;
}


JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeView_setHeight
  (JNIEnv *env, jobject obj, jdouble newHeight)
{
	try
	{
		GlobeViewClassInfo *classInfo = GlobeViewClassInfo::getClassInfo();
		WhirlyGlobe::GlobeView *view = classInfo->getObject(env,obj);
		if (!view)
			return;

	    view->setHeightAboveGlobe(newHeight);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeView::setHeight()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeView_setContinuousZoom
(JNIEnv *env, jobject obj, jboolean newVal)
{
    try
    {
        GlobeViewClassInfo *classInfo = GlobeViewClassInfo::getClassInfo();
        WhirlyGlobe::GlobeView *view = classInfo->getObject(env,obj);
        if (!view)
            return;
        
        view->continuousZoom = newVal;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeView::setHeight()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_GlobeView_makeRotationToGeoCoord
  (JNIEnv *env, jobject obj, jdouble x, jdouble y, jboolean northUp)
{
	try
	{
		GlobeViewClassInfo *classInfo = GlobeViewClassInfo::getClassInfo();
		WhirlyGlobe::GlobeView *view = classInfo->getObject(env,obj);
		if (!view)
			return NULL;

	    Eigen::Quaterniond newRot = view->makeRotationToGeoCoord(GeoCoord(x,y),(bool)northUp);

	    return MakeQuaternion(env,newRot);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeView::makeRotationToGeoCoord()");
	}
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_GlobeView_prospectiveUp
  (JNIEnv *env, jobject obj, jobject quatObj)
{
	try
	{
		GlobeViewClassInfo *classInfo = GlobeViewClassInfo::getClassInfo();
		WhirlyGlobe::GlobeView *view = classInfo->getObject(env,obj);
		QuaternionClassInfo *quatClassInfo = QuaternionClassInfo::getClassInfo();
		Quaterniond *quat = quatClassInfo->getObject(env,quatObj);
		if (!view || !quat)
			return NULL;

		Vector3d newUp = view->prospectiveUp(*quat);

		return MakePoint3d(env,Point3d(newUp.x(),newUp.y(),newUp.z()));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeView::prospectiveUp()");
	}
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_GlobeView_pointUnproject
  (JNIEnv *env, jobject obj, jobject touchObj,jobject frameSizeObj,jboolean clip)
{
	try
	{
		GlobeViewClassInfo *classInfo = GlobeViewClassInfo::getClassInfo();
		WhirlyGlobe::GlobeView *view = classInfo->getObject(env,obj);
		Point2dClassInfo *ptClassInfo = Point2dClassInfo::getClassInfo();
		Point2d *touch = ptClassInfo->getObject(env,touchObj);
		Point2d *frameSize = ptClassInfo->getObject(env,frameSizeObj);
		if (!view || !touch || !frameSize)
			return NULL;

		Point3d modelPt = view->pointUnproject(Point2f(touch->x(),touch->y()),(int)frameSize->x(),(int)frameSize->y(),clip);
		return MakePoint3d(env,modelPt);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeView::pointUnproject()");
	}
    
    return NULL;
}
