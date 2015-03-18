/*
 *  GlobeView_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/17/15.
 *  Copyright 2011-2015 mousebird consulting
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeView_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		GlobeViewClassInfo *classInfo = GlobeViewClassInfo::getClassInfo();
		WhirlyGlobe::GlobeView *inst = classInfo->getObject(env,obj);
		if (!inst)
			return;
		delete inst;

		classInfo->clearHandle(env,obj);
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeView_setLoc
  (JNIEnv *env, jobject obj, jdouble x, jdouble y, jdouble z)
{
	try
	{
		GlobeViewClassInfo *classInfo = GlobeViewClassInfo::getClassInfo();
		WhirlyGlobe::GlobeView *view = classInfo->getObject(env,obj);
		if (!view)
			return;

	    Eigen::Quaterniond newRot = view->makeRotationToGeoCoord(GeoCoord(x,y),true);
	    view->setRotQuat(newRot,true);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeView::setLoc()");
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
}
