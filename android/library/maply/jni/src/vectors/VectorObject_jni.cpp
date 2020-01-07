/*
 *  VectorObject_jni.cpp
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

#import "Vectors_jni.h"
#import "Geometry_jni.h"
#import "CoordSystem_jni.h"
#import "com_mousebird_maply_VectorObject.h"

using namespace WhirlyKit;

template<> VectorObjectClassInfo *VectorObjectClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_nativeInit
  (JNIEnv *env, jclass cls)
{
	VectorObjectClassInfo::getClassInfo(env,cls);
}

JNIEXPORT jobject JNICALL MakeVectorObject(JNIEnv *env,VectorObjectRef vec)
{
    VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo(env,"com/mousebird/maply/VectorObject");
    return classInfo->makeWrapperObject(env,new VectorObjectRef(vec));
}

JNIEXPORT jobject JNICALL MakeVectorObjectWrapper(JNIEnv *env,VectorObjectClassInfo *classInfo,VectorObjectRef vecObj)
{
    return classInfo->makeWrapperObject(env,new VectorObjectRef(vecObj));
}

void Java_com_mousebird_maply_VectorObject_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *inst = new VectorObjectRef(new VectorObject());
		classInfo->setHandle(env,obj,inst);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::initialise()");
	}
}

static std::mutex disposeMutex;

void Java_com_mousebird_maply_VectorObject_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            VectorObjectRef *inst = classInfo->getObject(env,obj);
            if (!inst)
                return;
            delete inst;

            classInfo->clearHandle(env,obj);
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::dispose()");
	}
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_VectorObject_getVectorTypeNative
(JNIEnv *env, jobject obj)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        if (!vecObj)
            return VectorNoneType;
		return (*vecObj)->getVectorType();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::clipToMbrNative()");
    }
	return VectorNoneType;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_setSelectable
  (JNIEnv *env, jobject obj, jboolean newVal)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        if (!vecObj)
            return;
        (*vecObj)->selectable = newVal;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::setSelectable()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_getSelectable
  (JNIEnv *env, jobject obj)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        if (!vecObj)
            return false;
        return (*vecObj)->selectable;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::getSelectable()");
    }

    return false;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_getAttributes
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
		if (!vecObj)
			return NULL;

		MutableDictionary_AndroidRef dict = std::dynamic_pointer_cast<MutableDictionary_Android>((*vecObj)->getAttributes());
		if (!dict)
		  return NULL;
		jobject dictObj = MakeAttrDictionary(env,dict);
	    return dictObj;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::getAttributes()");
	}

    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_setAttributes
  (JNIEnv *env, jobject obj, jobject attrObj)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
		MutableDictionary_AndroidRef *dict = AttrDictClassInfo::getClassInfo()->getObject(env,obj);
		if (!vecObj || !dict)
            return;
        (*vecObj)->setAttributes(*dict);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::getAttributes()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_addPoint
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
        VectorObjectRef *vecObj = VectorObjectClassInfo::getClassInfo()->getObject(env,obj);
		Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env,ptObj);
		if (!vecObj)
			return;

		VectorPointsRef pts = VectorPoints::createPoints();
		pts->pts.push_back(GeoCoord(pt->x(),pt->y()));
		pts->initGeoMbr();
        (*vecObj)->shapes.insert(pts);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::addPoint()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_addLinear
  (JNIEnv *env, jobject obj, jobjectArray ptsObj)
{
	try
	{
		VectorObjectRef *vecObj = VectorObjectClassInfo::getClassInfo()->getObject(env,obj);
		if (!vecObj)
			return;

		VectorLinearRef lin = VectorLinear::createLinear();
		JavaObjectArrayHelper ptsHelp(env,ptsObj);
		while (jobject ptObj = ptsHelp.getNextObject()) {
            Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env,ptObj);
            lin->pts.push_back(GeoCoord(pt->x(),pt->y()));
		}
		lin->initGeoMbr();
        (*vecObj)->shapes.insert(lin);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::addLinear()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_addAreal___3Lcom_mousebird_maply_Point2d_2
  (JNIEnv *env, jobject obj, jobjectArray ptsObj)
{
	try
	{
        VectorObjectRef *vecObj = VectorObjectClassInfo::getClassInfo()->getObject(env,obj);
		if (!vecObj)
			return;

		VectorArealRef ar = VectorAreal::createAreal();
		ar->loops.resize(1);

		JavaObjectArrayHelper ptsHelp(env,ptsObj);
		while (jobject ptObj = ptsHelp.getNextObject()) {
            Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env,ptObj);
            ar->loops[0].push_back(GeoCoord(pt->x(),pt->y()));
		}
		ar->initGeoMbr();
        (*vecObj)->shapes.insert(ar);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::addAreal()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_addAreal___3Lcom_mousebird_maply_Point2d_2_3_3Lcom_mousebird_maply_Point2d_2
(JNIEnv *env, jobject obj, jobjectArray outerLoopObj, jobjectArray holesArray)
{
    try
    {
        VectorObjectRef *vecObj = VectorObjectClassInfo::getClassInfo()->getObject(env,obj);
        if (!vecObj)
            return;
        Point2dClassInfo *point2dClassInfo = Point2dClassInfo::getClassInfo();

        VectorArealRef ar = VectorAreal::createAreal();
        ar->loops.resize(1+env->GetArrayLength(holesArray));

        for (int loop=0;loop<ar->loops.size();loop++)
        {
            jobjectArray ptsObj = loop == 0 ? outerLoopObj : (jobjectArray)env->GetObjectArrayElement(holesArray,loop-1);

            JavaObjectArrayHelper ptsHelp(env,ptsObj);
            while (jobject ptObj = ptsHelp.getNextObject()) {
                Point2d *pt = point2dClassInfo->getObject(env,ptObj);
                ar->loops[loop].push_back(GeoCoord(pt->x(),pt->y()));
            }

            if (loop > 0)
                env->DeleteLocalRef(ptsObj);
        }

        ar->initGeoMbr();
        (*vecObj)->shapes.insert(ar);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::addArealWithHoles()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_mergeVectorsFrom
  (JNIEnv *env, jobject obj, jobject otherObj)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        VectorObjectRef *otherVecObj = classInfo->getObject(env,otherObj);
        if (!vecObj || !otherVecObj)
            return;

        (*vecObj)->mergeVectorsFrom((*otherVecObj).get());
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::mergeVectorsFrom()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_center
  (JNIEnv *env, jobject obj)
{
	try
	{
        VectorObjectRef *vecObj = VectorObjectClassInfo::getClassInfo()->getObject(env,obj);
		if (!vecObj)
			return NULL;

		Point2d center;
		if ((*vecObj)->center(center))
		{
			return MakePoint2d(env,center);
		} else
			return NULL;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::center()");
	}

    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_centroid
  (JNIEnv *env, jobject obj)
{
	try
	{
        VectorObjectRef *vecObj = VectorObjectClassInfo::getClassInfo()->getObject(env,obj);
		if (!vecObj)
			return NULL;

		Point2d center;
		if ((*vecObj)->centroid(center))
		{
			return MakePoint2d(env,center);
		} else
			return NULL;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::centroid()");
	}

    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_largestLoopCenter
  (JNIEnv *env, jobject obj, jobject llObj, jobject urObj)
{
	try
	{
		Point2dClassInfo *pt2dClassInfo = Point2dClassInfo::getClassInfo();
        VectorObjectRef *vecObj = VectorObjectClassInfo::getClassInfo()->getObject(env,obj);
		Point2d *ll = pt2dClassInfo->getObject(env,llObj);
		Point2d *ur = pt2dClassInfo->getObject(env,urObj);
		if (!vecObj || !ll || !ur)
			return NULL;

		Point2d center;
		if ((*vecObj)->largestLoopCenter(center,*ll,*ur))
		{
			return MakePoint2d(env,center);
		} else
			return NULL;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::largestLoopCenter()");
	}

    return NULL;
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorObject_linearMiddle__Lcom_mousebird_maply_Point2d_2
  (JNIEnv *env, jobject obj, jobject midObj)
{
	try
	{
		Point2dClassInfo *pt2dClassInfo = Point2dClassInfo::getClassInfo();
        VectorObjectRef *vecObj = VectorObjectClassInfo::getClassInfo()->getObject(env,obj);
		Point2d *mid = pt2dClassInfo->getObject(env,midObj);
		if (!vecObj || !mid)
			return 0.0;

		double rot;
		if ((*vecObj)->linearMiddle(*mid,rot))
		{
			return rot;
		} else
			return 0.0;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::linearMiddle()");
	}

    return 0.0;
}

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    linearMiddle
 * Signature: (Lcom/mousebird/maply/Point2d;Lcom/mousebird/maply/CoordSystem;)D
 */
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorObject_linearMiddle__Lcom_mousebird_maply_Point2d_2Lcom_mousebird_maply_CoordSystem_2
  (JNIEnv *env, jobject obj, jobject midObj, jobject coordSysObj)
{
    try
    {
        Point2dClassInfo *pt2dClassInfo = Point2dClassInfo::getClassInfo();
        VectorObjectRef *vecObj = VectorObjectClassInfo::getClassInfo()->getObject(env,obj);
        Point2d *mid = pt2dClassInfo->getObject(env,midObj);
        CoordSystemRef *coordSys = CoordSystemRefClassInfo::getClassInfo()->getObject(env,coordSysObj);
        if (!vecObj || !mid || !coordSys)
            return 0.0;

        double rot;
        if ((*vecObj)->linearMiddle(*mid,rot,coordSys->get()))
        {
            return rot;
        } else
            return 0.0;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::linearMiddle()");
    }

    return 0.0;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_middleCoordinate
  (JNIEnv *env, jobject obj, jobject midObj)
{
	try
	{
		Point2dClassInfo *pt2dClassInfo = Point2dClassInfo::getClassInfo();
        VectorObjectRef *vecObj = VectorObjectClassInfo::getClassInfo()->getObject(env,obj);
		Point2d *mid = pt2dClassInfo->getObject(env,midObj);
		if (!vecObj || !mid)
			return false;

		if ((*vecObj)->middleCoordinate(*mid))
		{
			return true;
		} else
			return false;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::middleCoordinate()");
	}

    return false;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_pointInside
(JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        Point2dClassInfo *pt2dClassInfo = Point2dClassInfo::getClassInfo();
        VectorObjectRef *vecObj = VectorObjectClassInfo::getClassInfo()->getObject(env,obj);
        Point2d *pt = pt2dClassInfo->getObject(env,ptObj);
        if (!vecObj || !pt)
            return false;

        return (*vecObj)->pointInside(*pt);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::pointInside()");
    }

    return false;
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorObject_areaOfOuterLoops
  (JNIEnv *env, jobject obj)
{
    try
    {
        VectorObjectRef *vecObj = VectorObjectClassInfo::getClassInfo()->getObject(env,obj);
        if (!vecObj)
            return 0.0;

        return (*vecObj)->areaOfOuterLoops();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::areaOfOuterLoops()");
    }

    return 0.0;
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_VectorObject_countPoints
(JNIEnv *env, jobject obj)
{
    try
    {
        Point2dClassInfo *pt2dClassInfo = Point2dClassInfo::getClassInfo();
        VectorObjectRef *vecObj = VectorObjectClassInfo::getClassInfo()->getObject(env,obj);
        if (!vecObj)
            return 0;

        int numPts = 0;
        for (auto shape : (*vecObj)->shapes)
        {
            if(std::dynamic_pointer_cast<VectorLinear>(shape) != NULL)
            {
                VectorLinearRef linear = std::dynamic_pointer_cast<VectorLinear>(shape);
                numPts += linear->pts.size();
            } else if(std::dynamic_pointer_cast<VectorLinear3d>(shape) != NULL)
            {
            } else if(std::dynamic_pointer_cast<VectorAreal>(shape) != NULL)
            {
                VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(shape);
                if (ar)
                {
                    for (int ii=0;ii<ar->loops.size();ii++)
                    {
                        numPts += ar->loops[ii].size();
                    }
                }
            } else if(std::dynamic_pointer_cast<VectorPoints>(shape) != NULL)
            {
                VectorPointsRef points = std::dynamic_pointer_cast<VectorPoints>(shape);
                numPts += points->pts.size();
            }
        }

        return numPts;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::countPoints()");
    }

    return false;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_boundingBox
  (JNIEnv *env, jobject obj, jobject llObj, jobject urObj)
{
    try
    {
        Point2dClassInfo *pt2dClassInfo = Point2dClassInfo::getClassInfo();
        VectorObjectRef *vecObj = VectorObjectClassInfo::getClassInfo()->getObject(env,obj);
        Point2d *ll = pt2dClassInfo->getObject(env,llObj);
        Point2d *ur = pt2dClassInfo->getObject(env,urObj);
        if (!vecObj || !ll || !ur)
            return 0.0;

        return (*vecObj)->boundingBox(*ll,*ur);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::boundingBox()");
    }

    return 0.0;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_subdivideToGlobeNative
(JNIEnv *env, jobject obj, jobject retObj, jdouble epsilon)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        VectorObjectRef *retVecObj = classInfo->getObject(env,retObj);
        if (!vecObj || !retVecObj)
            return false;

        *retVecObj = *vecObj;
        (*retVecObj)->subdivideToGlobe(epsilon);

        return true;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::subdivideToGlobeNative()");
    }

    return false;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_subdivideToGlobeGreatCircleNative
(JNIEnv *env, jobject obj, jobject retObj, jdouble epsilon)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        VectorObjectRef *retVecObj = classInfo->getObject(env,retObj);
        if (!vecObj || !retVecObj)
            return false;

        *retVecObj = *vecObj;
        (*retVecObj)->subdivideToGlobeGreatCircle(epsilon);

        return true;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::subdivideToGlobeGreatCircleNative()");
    }

    return false;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_subdivideToFlatGreatCircleNative
(JNIEnv *env, jobject obj, jobject retObj, jdouble epsilon)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        VectorObjectRef *retVecObj = classInfo->getObject(env,retObj);
        if (!vecObj || !retVecObj)
            return false;

        *retVecObj = *vecObj;
        (*retVecObj)->subdivideToFlatGreatCircle(epsilon);

        return true;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::subdivideToFlatGreatCircleNative()");
    }

    return false;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_tesselateNative
(JNIEnv *env, jobject obj, jobject retObj)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        VectorObjectRef *retVecObj = classInfo->getObject(env,retObj);
        if (!vecObj || !retVecObj)
            return false;

        VectorObjectRef newVecObj = (*vecObj)->tesselate();
        *retVecObj = newVecObj;

        return true;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::tesselateNative()");
    }

    return false;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_clipToGridNative
(JNIEnv *env, jobject obj, jobject retObj, jdouble sizeX, jdouble sizeY)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        VectorObjectRef *retVecObj = classInfo->getObject(env,retObj);
        if (!vecObj || !retVecObj)
            return false;

        VectorObjectRef newVecObj = (*vecObj)->clipToGrid(Point2d(sizeX,sizeY));
        *retVecObj = newVecObj;

        return true;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::clipToGridNative()");
    }

    return false;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_clipToMbrNative
(JNIEnv *env, jobject obj, jobject retObj, jdouble llX, jdouble llY, jdouble urX, jdouble urY)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        VectorObjectRef *retVecObj = classInfo->getObject(env,retObj);
        if (!vecObj || !retVecObj)
            return false;

        Point2d ll(llX,llY),ur(urX,urY);
        VectorObjectRef newVecObj = (*vecObj)->clipToMbr(ll,ur);
        *retVecObj = newVecObj;

        return true;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::clipToMbrNative()");
    }

    return false;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_reprojectNative
  (JNIEnv *env, jobject obj, jobject retObj, jobject srcSystemObj, jdouble scale, jobject destSystemObj)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        VectorObjectRef *retVecObj = classInfo->getObject(env,retObj);
        CoordSystemRefClassInfo *coordSystemInfo = CoordSystemRefClassInfo::getClassInfo();
        CoordSystemRef *srcSystem = coordSystemInfo->getObject(env,srcSystemObj);
        CoordSystemRef *destSystem = coordSystemInfo->getObject(env,destSystemObj);
        if (!vecObj || !retVecObj || !srcSystem || !destSystem)
            return false;

        VectorObjectRef newVecObj = (*vecObj)->deepCopy();
        newVecObj->reproject(srcSystem->get(),scale,destSystem->get());
        *retVecObj = newVecObj;

        return true;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::reprojectNative()");
    }

    return false;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_filterClippedEdgesNative
  (JNIEnv *env, jobject obj, jobject retObj)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        VectorObjectRef *retVecObj = classInfo->getObject(env,retObj);
        if (!vecObj || !retVecObj)
            return false;

        VectorObjectRef newVecObj = (*vecObj)->filterClippedEdges();
        *retVecObj = newVecObj;

        return true;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::filterClippedEdgesNative()");
    }

    return false;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_linearsToArealsNative
  (JNIEnv *env, jobject obj, jobject retObj)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        VectorObjectRef *retVecObj = classInfo->getObject(env,retObj);
        if (!vecObj || !retVecObj)
            return false;

        VectorObjectRef newVecObj = (*vecObj)->linearsToAreals();
        *retVecObj = newVecObj;

        return true;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::linearsToArealsNative()");
    }

    return false;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_arealsToLinearsNative
  (JNIEnv *env, jobject obj, jobject retObj)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        VectorObjectRef *retVecObj = classInfo->getObject(env,retObj);
        if (!vecObj || !retVecObj)
            return false;

        VectorObjectRef newVecObj = (*vecObj)->arealsToLinears();
        *retVecObj = newVecObj;

        return true;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::arealsToLinearsNative()");
    }

    return false;
}

jboolean Java_com_mousebird_maply_VectorObject_fromGeoJSON
  (JNIEnv *env, jobject obj, jstring jstr)
{
	try
	{
		VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
		VectorObjectRef *vecObj = classInfo->getObject(env,obj);
		if (!vecObj)
			return false;

		const char *cStr = env->GetStringUTFChars(jstr,0);
		if (!cStr)
			return false;
		std::string jsonStr(cStr);
		env->ReleaseStringUTFChars(jstr, cStr);

        std::string crs;
		bool ret = (*vecObj)->fromGeoJSON(jsonStr,crs);

		return ret;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::fromGeoJSON()");
	}

    return false;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_fromShapeFile
(JNIEnv *env, jobject obj, jstring jstr)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        if (!vecObj)
            return false;

        const char *cStr = env->GetStringUTFChars(jstr,0);
        if (!cStr)
            return false;
        std::string jsonStr(cStr);
        env->ReleaseStringUTFChars(jstr, cStr);

        bool ret = (*vecObj)->fromShapeFile(jsonStr);

        return true;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::fromShapeFile()");
    }

    return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_deepCopyNative
        (JNIEnv *env, jobject obj, jobject destObj)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        VectorObjectRef *destVecObj = classInfo->getObject(env,destObj);
        if (!vecObj || !destVecObj)
            return;

        VectorObjectRef newVecObj = (*vecObj)->deepCopy();
        *destVecObj = newVecObj;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::deepCopyNative()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_FromGeoJSONAssembly
  (JNIEnv *env, jclass vecObjClass, jstring jstr)
{
	try
	{
		const char *cStr = env->GetStringUTFChars(jstr,0);
		if (!cStr)
			return NULL;
		std::string jsonStr(cStr);
		env->ReleaseStringUTFChars(jstr, cStr);

		std::map<std::string,VectorObject *> vecData;
		bool ret = VectorObject::FromGeoJSONAssembly(jsonStr,vecData);

		if (ret)
		{
			JavaHashMapInfo *hashMapClassInfo = JavaHashMapInfo::getClassInfo(env);
			jobject hashMap = hashMapClassInfo->makeHashMap(env);
			for (std::map<std::string,VectorObject *>::iterator it = vecData.begin();
					it != vecData.end(); ++it)
			{
				jstring key = env->NewStringUTF(it->first.c_str());
				jobject vecObj = MakeVectorObject(env,VectorObjectRef(it->second));
				hashMapClassInfo->addObject(env, hashMap, key, vecObj);
			}

			return hashMap;
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::FromGeoJSONAssembly()");
	}

	return NULL;
}



