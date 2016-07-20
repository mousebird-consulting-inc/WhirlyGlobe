/*
 *  CoordSystemDisplayAdapter_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
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

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_CoordSystemDisplayAdapter_screenPointFromGeoBatch
(JNIEnv *env, jobject obj,jobject viewObj,int frameSizeX,int frameSizeY,jdoubleArray gxArray, jdoubleArray gyArray, jdoubleArray gzArray, jdoubleArray sxArray, jdoubleArray syArray)
{
    try
    {
        CoordSystemDisplayAdapterInfo *classInfo = CoordSystemDisplayAdapterInfo::getClassInfo();
        CoordSystemDisplayAdapter *coordAdapter = classInfo->getObject(env,obj);
        ViewClassInfo *viewClassInfo = ViewClassInfo::getClassInfo();
        View *view = viewClassInfo->getObject(env,viewObj);
        if (!coordAdapter || !view)
            return false;
        Maply::MapView *mapView = dynamic_cast<Maply::MapView *>(view);
        WhirlyGlobe::GlobeView *globeView = dynamic_cast<WhirlyGlobe::GlobeView *>(view);
        
        JavaDoubleArray geoX(env,gxArray), geoY(env,gyArray);
        JavaDoubleArray sx(env,sxArray), sy(env,syArray);
        
        if (geoX.len != geoY.len || geoX.len != sx.len || geoX.len != sy.len)
            return false;

        CoordSystem *coordSys = coordAdapter->getCoordSystem();
        Matrix4d modelMat = view->calcModelMatrix();
        Matrix4d viewMat = view->calcViewMatrix();
        Matrix4d fullMat = viewMat * modelMat;
        Matrix4d fullNormalMat = fullMat.inverse().transpose();
        Point2f frameSize(frameSizeX,frameSizeY);
        
        for (unsigned int ii=0;ii<geoX.len;ii++)
        {
            Point3d localPt = coordSys->geographicToLocal3d(GeoCoord(geoX.rawDouble[ii],geoY.rawDouble[ii]));
            Point3d dispPt = coordAdapter->localToDisplay(localPt);
            Point2f screenPt;
            bool valid = true;
            if (globeView)
            {
                if (CheckPointAndNormFacing(dispPt,dispPt.normalized(),modelMat,fullNormalMat) < 0.0)
                    valid = false;
                else {
                    screenPt = globeView->pointOnScreenFromSphere(dispPt,&fullMat,frameSize);
                }
            } else {
                if (mapView)
                    screenPt = mapView->pointOnScreenFromPlane(dispPt,&fullMat,frameSize);
            }
            if (valid)
            {
                sx.rawDouble[ii] = screenPt.x();  sy.rawDouble[ii] = screenPt.y();
            } else {
                sx.rawDouble[ii] = MAXFLOAT;  sy.rawDouble[ii] = MAXFLOAT;
            }
        }
        
        return true;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystemDisplayAdapter::screenPointFromGeoBatch()");
    }

    return true;
}

/*
 * Class:     com_mousebird_maply_CoordSystemDisplayAdapter
 * Method:    geoPointFromScreenBatch
 * Signature: ([D[D[D[D[D)Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_CoordSystemDisplayAdapter_geoPointFromScreenBatch
(JNIEnv *env, jobject obj, jobject viewObj,int frameSizeX,int frameSizeY, jdoubleArray sxArray, jdoubleArray syArray, jdoubleArray gxArray, jdoubleArray gyArray)
{
    try
    {
        CoordSystemDisplayAdapterInfo *classInfo = CoordSystemDisplayAdapterInfo::getClassInfo();
        CoordSystemDisplayAdapter *coordAdapter = classInfo->getObject(env,obj);
        ViewClassInfo *viewClassInfo = ViewClassInfo::getClassInfo();
        View *view = viewClassInfo->getObject(env,viewObj);
        if (!coordAdapter || !view)
            return false;
        Maply::MapView *mapView = dynamic_cast<Maply::MapView *>(view);
        WhirlyGlobe::GlobeView *globeView = dynamic_cast<WhirlyGlobe::GlobeView *>(view);
        
        JavaDoubleArray sx(env,sxArray), sy(env,syArray);
        JavaDoubleArray geoX(env,gxArray), geoY(env,gyArray);
        
        if (geoX.len != geoY.len || geoX.len != sx.len || geoX.len != sy.len)
            return false;
        
        CoordSystem *coordSys = coordAdapter->getCoordSystem();
        Matrix4d modelMat = view->calcModelMatrix();
        Matrix4d viewMat = view->calcViewMatrix();
        Matrix4d fullMat = viewMat * modelMat;
        Point2f frameSize(frameSizeX,frameSizeY);
        
        for (unsigned int ii=0;ii<sx.len;ii++)
        {
            GeoCoord outCoord;
            bool valid = true;
            Point2f screenPt(sx.rawDouble[ii],sy.rawDouble[ii]);
            Point3d hit;
            if (globeView)
            {
                if (!globeView->pointOnSphereFromScreen(screenPt,&fullMat,frameSize,&hit,true))
                    valid = false;
                else
                    outCoord = coordSys->localToGeographic(coordAdapter->displayToLocal(hit));
            } else {
                if (mapView)
                {
                    if (!mapView->pointOnPlaneFromScreen(screenPt,&fullMat,frameSize,&hit,false))
                        valid = false;
                    else
                        outCoord = coordSys->localToGeographic(coordAdapter->displayToLocal(hit));
                }
            }
            
            if (valid)
            {
                geoX.rawDouble[ii] = outCoord.x();  geoY.rawDouble[ii] = outCoord.y();
            } else {
                geoX.rawDouble[ii] = MAXFLOAT;  geoY.rawDouble[ii] = MAXFLOAT;
            }
        }
        
        return true;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystemDisplayAdapter::screenPointFromGeoBatch()");
    }
    
    return true;
}
