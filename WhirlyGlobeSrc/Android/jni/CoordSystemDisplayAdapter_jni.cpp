/*
 *  CoordSystemDisplayAdapter_jni.cpp
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
#import "Maply_utils_jni.h"
#import "com_mousebird_maply_CoordSystemDisplayAdapter.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;


JNIEXPORT void JNICALL Java_com_mousebird_maply_CoordSystemDisplayAdapter_nativeInit
  (JNIEnv *env, jclass cls)
{
	CoordSystemDisplayAdapterInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_CoordSystemDisplayAdapter_initialise
  (JNIEnv *env, jobject obj, jobject coordSysObj)
{
	try
	{
		CoordSystemDisplayAdapterInfo *classInfo = CoordSystemDisplayAdapterInfo::getClassInfo();
		// Note: Porting.  Just a spherical mercator display adapter for now
		CoordSystemDisplayAdapter *coordAdapter = coordAdapter = new SphericalMercatorDisplayAdapter(0.0, GeoCoord::CoordFromDegrees(-180.0,-90.0), GeoCoord::CoordFromDegrees(180.0,90.0));
		classInfo->setHandle(env,obj,coordAdapter);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystemDisplayAdapter::initialise()");
	}
}

/*
 * Class:     com_mousebird_maply_CoordSystemDisplayAdapter
 * Method:    dispose
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_CoordSystemDisplayAdapter_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		CoordSystemDisplayAdapterInfo *classInfo = CoordSystemDisplayAdapterInfo::getClassInfo();
		CoordSystemDisplayAdapter *coordAdapter = classInfo->getObject(env,obj);
		if (!coordAdapter)
			return;
		delete coordAdapter;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystemDisplayAdapter::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_CoordSystemDisplayAdapter_getBounds
  (JNIEnv *env, jobject obj, jobject llObj, jobject urObj)
{
	try
	{
		CoordSystemDisplayAdapterInfo *classInfo = CoordSystemDisplayAdapterInfo::getClassInfo();
		CoordSystemDisplayAdapter *coordAdapter = classInfo->getObject(env,obj);
		Point3dClassInfo *pt3dClassInfo = Point3dClassInfo::getClassInfo();
		Point3d *ll = pt3dClassInfo->getObject(env,llObj);
		Point3d *ur = pt3dClassInfo->getObject(env,urObj);
		if (!coordAdapter || !ll || !ur)
			return;
		Point3f ll3f,ur3f;
		coordAdapter->getBounds(ll3f,ur3f);
		ll->x() = ll3f.x();  ll->y() = ll3f.y();
		ur->x() = ur3f.x();  ur->y() = ur3f.y();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystemDisplayAdapter::getBounds()");
	}
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_CoordSystemDisplayAdapter_displayToLocal
  (JNIEnv *env, jobject obj, jobject dispPtObj)
{
	try
	{
		CoordSystemDisplayAdapterInfo *classInfo = CoordSystemDisplayAdapterInfo::getClassInfo();
		CoordSystemDisplayAdapter *coordAdapter = classInfo->getObject(env,obj);
		Point3dClassInfo *pt3dClassInfo = Point3dClassInfo::getClassInfo();
		Point3d *dispPt = pt3dClassInfo->getObject(env,dispPtObj);
		if (!coordAdapter || !dispPt)
			return NULL;
		Point3d localPt = coordAdapter->displayToLocal(*dispPt);
		jobject localPtObj = MakePoint3d(env,localPt);

		return localPtObj;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystemDisplayAdapter::displayToLocal()");
	}
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_CoordSystemDisplayAdapter_localToDisplay
  (JNIEnv *env, jobject obj, jobject localPtObj)
{
	try
	{
		CoordSystemDisplayAdapterInfo *classInfo = CoordSystemDisplayAdapterInfo::getClassInfo();
		CoordSystemDisplayAdapter *coordAdapter = classInfo->getObject(env,obj);
		Point3dClassInfo *pt3dClassInfo = Point3dClassInfo::getClassInfo();
		Point3d *localPt = pt3dClassInfo->getObject(env,localPtObj);
		if (!coordAdapter || !localPt)
			return NULL;
		Point3d dispPt = coordAdapter->localToDisplay(*localPt);
		jobject dispPtObj = MakePoint3d(env,dispPt);

		return dispPtObj;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystemDisplayAdapter::localToDisplay()");
	}
    
    return NULL;
}
