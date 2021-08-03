/*  VectorObject_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2021 mousebird consulting
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

#import "Maply_jni.h"
#import "Vectors_jni.h"
#import "Geometry_jni.h"
#import "CoordSystem_jni.h"
#import "com_mousebird_maply_VectorObject.h"

using namespace WhirlyKit;

template<> VectorObjectClassInfo *VectorObjectClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_nativeInit(JNIEnv *env, jclass cls)
{
	VectorObjectClassInfo::getClassInfo(env,cls);
}

JNIEXPORT jobject JNICALL MakeVectorObject(JNIEnv *env,const VectorObjectRef &vec)
{
    VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo(env,"com/mousebird/maply/VectorObject");
    return MakeVectorObjectWrapper(env,classInfo,vec);
}

JNIEXPORT jobject JNICALL MakeVectorObjectWrapper(JNIEnv *env,VectorObjectClassInfo *classInfo,const VectorObjectRef &vec)
{
    jobject newObj = classInfo->makeWrapperObject(env);
    VectorObjectRef *oldRef = classInfo->getObject(env,newObj);
    vec->setId((*oldRef)->getId());
    classInfo->setHandle(env,newObj,new VectorObjectRef(vec));
    delete oldRef;
    return newObj;
}

extern "C"
void Java_com_mousebird_maply_VectorObject_initialise(JNIEnv *env, jobject obj, jlong ident)
{
	try
	{
		VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        auto inst = new VectorObjectRef(new VectorObject(ident));
		classInfo->setHandle(env,obj,inst);
	}
    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
void Java_com_mousebird_maply_VectorObject_dispose(JNIEnv *env, jobject obj)
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
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_VectorObject_getVectorTypeNative(JNIEnv *env, jobject obj)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        if (!vecObj)
            return VectorNoneType;
		return (*vecObj)->getVectorType();
    }
    MAPLY_STD_JNI_CATCH()
	return VectorNoneType;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_setSelectable(JNIEnv *env, jobject obj, jboolean newVal)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        if (!vecObj)
            return;
        (*vecObj)->selectable = newVal;
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_getSelectable(JNIEnv *env, jobject obj)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        if (!vecObj)
            return false;
        return (*vecObj)->selectable;
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_getAttributes
  (JNIEnv *env, jobject obj)
{
	try
	{
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto vecAttrs = (*vecObj)->getAttributes())
        {
            // Cast shared pointer type, or make a copy if that fails.  If we make a copy, any
            // Java-side changes will not be applied to this vector object unless the resulting
            // attribute dictionary is later passed back to setAttributes()
            return MakeAttrDictionaryRefOrCopy(env, vecAttrs);
        }
	}
	MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_getAttributesRef
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto vecAttrs = (*vecObj)->getAttributes())
        if (const auto dict = std::dynamic_pointer_cast<MutableDictionary_Android>(vecAttrs))
        {
            // Make a new JNI-wrapped reference to the same object
            return MakeAttrDictionaryRef(env, dict);
        }
    }
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_setAttributes(JNIEnv *env, jobject obj, jobject attrObj)
{
    try
    {
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
		if (auto dict = AttrDictClassInfo::get(env,attrObj))
        {
		    (*vecObj)->setAttributes(*dict);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_addPoint(JNIEnv *env, jobject obj, jobject ptObj)
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
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_addLinear(JNIEnv *env, jobject obj, jobjectArray ptsObj)
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
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_addAreal___3Lcom_mousebird_maply_Point2d_2(JNIEnv *env, jobject obj, jobjectArray ptsObj)
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
    MAPLY_STD_JNI_CATCH()
}

extern "C"
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
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_mergeVectorsFrom(JNIEnv *env, jobject obj, jobject otherObj)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        VectorObjectRef *otherVecObj = classInfo->getObject(env,otherObj);
        if (!vecObj || !otherVecObj)
            return;

        (*vecObj)->mergeVectorsFrom(*(*otherVecObj).get());
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_center(JNIEnv *env, jobject obj)
{
	try
	{
        VectorObjectRef *vecObj = VectorObjectClassInfo::getClassInfo()->getObject(env,obj);
		if (!vecObj)
			return nullptr;

		Point2d center;
		if ((*vecObj)->center(center))
		{
			return MakePoint2d(env,center);
		} else
			return nullptr;
	}
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_centroid(JNIEnv *env, jobject obj)
{
	try
	{
        VectorObjectRef *vecObj = VectorObjectClassInfo::getClassInfo()->getObject(env,obj);
		if (!vecObj)
			return nullptr;

		Point2d center;
		if ((*vecObj)->centroid(center))
		{
			return MakePoint2d(env,center);
		} else
			return nullptr;
	}
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
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
			return nullptr;

		Point2d center;
		if ((*vecObj)->largestLoopCenter(center,*ll,*ur))
		{
			return MakePoint2d(env,center);
		}
		else
        {
		    return nullptr;
        }
	}
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorObject_linearMiddle__Lcom_mousebird_maply_Point2d_2(JNIEnv *env, jobject obj, jobject midObj)
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
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    linearMiddle
 * Signature: (Lcom/mousebird/maply/Point2d;Lcom/mousebird/maply/CoordSystem;)D
 */
extern "C"
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
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_middleCoordinate(JNIEnv *env, jobject obj, jobject midObj)
{
	try
	{
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        {
            if (const auto mid = Point2dClassInfo::get(env,midObj))
            {
                return (*vecObj)->middleCoordinate(*mid);
            }
		}
	}
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_pointInside(JNIEnv *env, jobject obj, jobject ptObj)
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
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorObject_areaOfOuterLoops(JNIEnv *env, jobject obj)
{
    try
    {
        VectorObjectRef *vecObj = VectorObjectClassInfo::getClassInfo()->getObject(env,obj);
        if (!vecObj)
            return 0.0;

        return (*vecObj)->areaOfOuterLoops();
    }
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_VectorObject_countPoints(JNIEnv *env, jobject obj)
{
    try
    {
        //Point2dClassInfo *pt2dClassInfo = Point2dClassInfo::getClassInfo();
        VectorObjectRef *vecObj = VectorObjectClassInfo::getClassInfo()->getObject(env,obj);
        if (!vecObj)
            return 0;

        int numPts = 0;
        for (const auto &shape : (*vecObj)->shapes)
        {
            if (const auto linear = dynamic_cast<VectorLinear*>(shape.get()))
            {
                numPts += linear->pts.size();
            //} else if(const auto vec3d = dynamic_cast<VectorLinear3d*>(shape.get())) {
            } else if(const auto ar = dynamic_cast<VectorAreal*>(shape.get())) {
                for (auto & loop : ar->loops)
                {
                    numPts += loop.size();
                }
            } else if (const auto points = dynamic_cast<VectorPoints*>(shape.get())) {
                numPts += points->pts.size();
            }
        }

        return numPts;
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_boundingBox(JNIEnv *env, jobject obj, jobject llObj, jobject urObj)
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
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}

extern "C"
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
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_subdivideToGlobeGreatCircleNative
    (JNIEnv *env, jobject obj, jobject retObj, jdouble epsilon)
{
    try
    {
        const auto classInfo = VectorObjectClassInfo::getClassInfo();
        if (const auto vecObj = classInfo->getObject(env,obj))
        {
            if (const auto retVecObj = classInfo->getObject(env, retObj))
            {
                *retVecObj = *vecObj;
                (*retVecObj)->subdivideToGlobeGreatCircle(epsilon);
                return true;
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_subdivideToFlatGreatCircleNative
    (JNIEnv *env, jobject obj, jobject retObj, jdouble epsilon)
{
    try
    {
        const auto classInfo = VectorObjectClassInfo::getClassInfo();
        if (const auto vecObj = classInfo->getObject(env,obj))
        {
            if (const auto retVecObj = classInfo->getObject(env, retObj))
            {
                *retVecObj = *vecObj;
                (*retVecObj)->subdivideToFlatGreatCircle(epsilon);
                return true;
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_subdivideToGlobeGreatCirclePreciseNative
        (JNIEnv *env, jobject obj, jobject retObj, jdouble epsilon)
{
    try
    {
        const auto classInfo = VectorObjectClassInfo::getClassInfo();
        if (const auto vecObj = classInfo->getObject(env,obj))
        {
            if (const auto retVecObj = classInfo->getObject(env, retObj))
            {
                *retVecObj = *vecObj;
                (*retVecObj)->subdivideToGlobeGreatCirclePrecise(epsilon);
                return true;
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_subdivideToFlatGreatCirclePreciseNative
        (JNIEnv *env, jobject obj, jobject retObj, jdouble epsilon)
{
    try
    {
        const auto classInfo = VectorObjectClassInfo::getClassInfo();
        if (const auto vecObj = classInfo->getObject(env,obj))
        {
            if (const auto retVecObj = classInfo->getObject(env, retObj))
            {
                *retVecObj = *vecObj;
                (*retVecObj)->subdivideToFlatGreatCirclePrecise(epsilon);
                return true;
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
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
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
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
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
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
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
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
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
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
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
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
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
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
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
jboolean Java_com_mousebird_maply_VectorObject_fromGeoJSON(JNIEnv *env, jobject obj, jstring jstr)
{
	try
	{
		VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
		VectorObjectRef *vecObj = classInfo->getObject(env,obj);
		if (!vecObj)
			return false;

		const char *cStr = env->GetStringUTFChars(jstr,nullptr);
		if (!cStr)
			return false;
		std::string jsonStr(cStr);
		env->ReleaseStringUTFChars(jstr, cStr);

        std::string crs;
		return (*vecObj)->fromGeoJSON(jsonStr,crs);
	}
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_fromShapeFile(JNIEnv *env, jobject obj, jstring jstr)
{
    try
    {
        VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        VectorObjectRef *vecObj = classInfo->getObject(env,obj);
        if (!vecObj)
            return false;

        const char *cStr = env->GetStringUTFChars(jstr,nullptr);
        if (!cStr)
            return false;
        std::string jsonStr(cStr);
        env->ReleaseStringUTFChars(jstr, cStr);

        return (*vecObj)->fromShapeFile(jsonStr);
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    canSplit
 * Signature: ()Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_canSplit(JNIEnv *env, jobject obj) {
    try {
        if (auto vecObjPtr = VectorObjectClassInfo::getClassInfo()->getObject(env,obj)) {
            return ((*vecObjPtr)->shapes.size() > 1);
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
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
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_FromGeoJSONAssembly(JNIEnv *env, jclass /*vecObjClass*/, jstring jstr)
{
	try
	{
		const char *cStr = env->GetStringUTFChars(jstr,nullptr);
		if (!cStr)
			return nullptr;
		std::string jsonStr(cStr);
		env->ReleaseStringUTFChars(jstr, cStr);

		std::map<std::string,VectorObject *> vecData;
		if (VectorObject::FromGeoJSONAssembly(jsonStr,vecData))
		{
			JavaHashMapInfo *hashMapClassInfo = JavaHashMapInfo::getClassInfo(env);
			jobject hashMap = hashMapClassInfo->makeHashMap(env);
			for (const auto &kvp : vecData)
			{
				jstring key = env->NewStringUTF(kvp.first.c_str());
				jobject vecObj = MakeVectorObject(env,VectorObjectRef(kvp.second));
				hashMapClassInfo->addObject(env, hashMap, key, vecObj);
			}

			return hashMap;
		}
	}
    MAPLY_STD_JNI_CATCH()
	return nullptr;
}



