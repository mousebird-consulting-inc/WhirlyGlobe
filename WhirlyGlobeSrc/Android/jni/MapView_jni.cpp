/*
 *  MapView_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
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
#import "com_mousebird_maply_MapView.h"
#import "Maply_utils_jni.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_MapView_nativeInit
  (JNIEnv *env, jclass cls)
{
	MapViewClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MapView_initialise
  (JNIEnv *env, jobject obj, jobject coordAdapterObj)
{
	try
	{
		CoordSystemDisplayAdapter *coordAdapter = CoordSystemDisplayAdapterInfo::getClassInfo()->getObject(env,coordAdapterObj);
		Maply::MapView *inst = new Maply::MapView(coordAdapter);
		MapViewClassInfo::getClassInfo()->setHandle(env,obj,inst);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapView::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MapView_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		MapViewClassInfo *classInfo = MapViewClassInfo::getClassInfo();
		Maply::MapView *inst = classInfo->getObject(env,obj);
		if (!inst)
			return;
		delete inst;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapView::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MapView_nativeClone
  (JNIEnv *env, jobject obj, jobject destObj)
{
	try
	{
		MapViewClassInfo *classInfo = MapViewClassInfo::getClassInfo();
		Maply::MapView *src = classInfo->getObject(env,obj);
		Maply::MapView *dest = new Maply::MapView(*src);
		MapViewClassInfo::getClassInfo()->setHandle(env,destObj,dest);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapView::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MapView_setLoc
  (JNIEnv *env, jobject obj, jdouble x, jdouble y, jdouble z)
{
	try
	{
		MapViewClassInfo *classInfo = MapViewClassInfo::getClassInfo();
		Maply::MapView *view = classInfo->getObject(env,obj);
		if (!view)
			return;

		view->setLoc(Point3d(x,y,z));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapView::setLoc()");
	}
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_MapView_getLoc
  (JNIEnv *env, jobject obj)
{
	try
	{
		MapViewClassInfo *classInfo = MapViewClassInfo::getClassInfo();
		Maply::MapView *view = classInfo->getObject(env,obj);
		if (!view)
			return NULL;

		Point3d pt = view->getLoc();

		return MakePoint3d(env,pt);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapView::getLoc()");
	}

	return NULL;
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_MapView_minHeightAboveSurface
  (JNIEnv *env, jobject obj)
{
	try
	{
		MapViewClassInfo *classInfo = MapViewClassInfo::getClassInfo();
		Maply::MapView *view = classInfo->getObject(env,obj);
		if (!view)
			return 0.0;

		return view->minHeightAboveSurface();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapView::minHeightAboveSurface()");
	}
    
    return 0.0;
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_MapView_maxHeightAboveSurface
  (JNIEnv *env, jobject obj)
{
	try
	{
		MapViewClassInfo *classInfo = MapViewClassInfo::getClassInfo();
		Maply::MapView *view = classInfo->getObject(env,obj);
		if (!view)
			return 0.0;

		return view->maxHeightAboveSurface();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapView::maxHeightAboveSurface()");
	}

    return 0.0;
}


JNIEXPORT void JNICALL Java_com_mousebird_maply_MapView_setRot
  (JNIEnv *env, jobject obj, jdouble rot)
{
	try
	{
		MapViewClassInfo *classInfo = MapViewClassInfo::getClassInfo();
		Maply::MapView *view = classInfo->getObject(env,obj);
		if (!view)
			return;

		view->setRotAngle(rot);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapView::setRot()");
	}
}


JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_MapView_getRot
  (JNIEnv *env, jobject obj)
{
	try
	{
		MapViewClassInfo *classInfo = MapViewClassInfo::getClassInfo();
		Maply::MapView *view = classInfo->getObject(env,obj);
		if (!view)
			return 0.0;

		return view->getRotAngle();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapView::getRot()");
	}

    return 0.0;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_MapView_pointOnPlaneFromScreen
  (JNIEnv *env, jobject obj, jobject screenPtObj, jobject viewModelMatObj, jobject frameObj, jboolean clip)
{
	try
	{
		MapViewClassInfo *classInfo = MapViewClassInfo::getClassInfo();
		Maply::MapView *view = classInfo->getObject(env,obj);
		if (!view)
			return NULL;
		Point2d *screenPt = Point2dClassInfo::getClassInfo()->getObject(env,screenPtObj);
		Matrix4d *viewModelMat = Matrix4dClassInfo::getClassInfo()->getObject(env,viewModelMatObj);
		Point2d *framePt = Point2dClassInfo::getClassInfo()->getObject(env,frameObj);
		Point3d hit;

		Point2f screenPt2f(screenPt->x(),screenPt->y());
		Point2f framePt2f(framePt->x(),framePt->y());
		if (view->pointOnPlaneFromScreen(screenPt2f,viewModelMat,framePt2f,&hit,clip))
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

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_MapView_pointOnScreenFromPlane
  (JNIEnv *env, jobject obj, jobject dispPtObj, jobject viewModelMatObj, jobject frameObj)
{
	try
	{
		MapViewClassInfo *classInfo = MapViewClassInfo::getClassInfo();
		Maply::MapView *view = classInfo->getObject(env,obj);
		if (!view)
			return NULL;
		Point3d *dispPt = Point3dClassInfo::getClassInfo()->getObject(env,dispPtObj);
		Matrix4d *viewModelMat = Matrix4dClassInfo::getClassInfo()->getObject(env,viewModelMatObj);
		Point2d *framePt = Point2dClassInfo::getClassInfo()->getObject(env,frameObj);
		Point3d hit;

		Point2f framePt2f(framePt->x(),framePt->y());
		Point2f retPt = view->pointOnScreenFromPlane(*dispPt,viewModelMat,framePt2f);

		return MakePoint2d(env,Point2d(retPt.x(),retPt.y()));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapView::pointOnPlaneFromScreen()");
	}
    
    return NULL;
}

