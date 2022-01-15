/*  VectorObject_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
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

#import "Maply_jni.h"
#import "Vectors_jni.h"
#import "Geometry_jni.h"
#import "CoordSystem_jni.h"
#import "com_mousebird_maply_VectorObject.h"

using namespace WhirlyKit;

template<> VectorObjectClassInfo *VectorObjectClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_nativeInit
  (JNIEnv *env, jclass cls)
{
	VectorObjectClassInfo::getClassInfo(env,cls);
}

JNIEXPORT jobject JNICALL MakeVectorObject(JNIEnv *env,const VectorObjectRef &vec)
{
    auto classInfo = VectorObjectClassInfo::getClassInfo(env,"com/mousebird/maply/VectorObject");
    return MakeVectorObjectWrapper(env,classInfo,vec);
}

JNIEXPORT jobject JNICALL MakeVectorObjectWrapper(JNIEnv *env,VectorObjectClassInfo *classInfo,const VectorObjectRef &vec)
{
    if (jobject newObj = classInfo->makeWrapperObject(env))
    if (VectorObjectRef *oldRef = classInfo->getObject(env,newObj))
    {
        vec->setId((*oldRef)->getId());
        classInfo->setHandle(env, newObj, new VectorObjectRef(vec));
        delete oldRef;
        return newObj;
    }
    return nullptr;
}

extern "C"
void Java_com_mousebird_maply_VectorObject_initialise
  (JNIEnv *env, jobject obj, jlong ident)
{
	try
	{
        VectorObjectClassInfo::set(env, obj, new VectorObjectRef(std::make_shared<VectorObject>(ident)));
	}
    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
void Java_com_mousebird_maply_VectorObject_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorObjectClassInfo *classInfo = VectorObjectClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        VectorObjectRef *inst = classInfo->getObject(env,obj);
        delete inst;
        classInfo->clearHandle(env,obj);
	}
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_VectorObject_getVectorTypeNative
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto vecObj = VectorObjectClassInfo::get(env,obj))
        {
            return (*vecObj)->getVectorType();
        }
    }
    MAPLY_STD_JNI_CATCH()
	return VectorNoneType;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_setSelectable
  (JNIEnv *env, jobject obj, jboolean newVal)
{
    try
    {
        if (auto vecObj = VectorObjectClassInfo::get(env,obj))
        {
            (*vecObj)->selectable = newVal;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_getSelectable
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto vecObj = VectorObjectClassInfo::get(env,obj))
        {
            return (*vecObj)->selectable;
        }
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
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_setAttributes
  (JNIEnv *env, jobject obj, jobject attrObj)
{
    try
    {
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
		if (const auto dict = AttrDictClassInfo::get(env,attrObj))
        {
		    (*vecObj)->setAttributes(*dict);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_addPoint
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto pt = Point2dClassInfo::get(env, ptObj))
        {
            VectorPointsRef pts = VectorPoints::createPoints();
            pts->pts.push_back(GeoCoord(pt->x(), pt->y()));
            pts->initGeoMbr();
            (*vecObj)->shapes.insert(pts);
            return true;
        }
	}
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_addLinear
  (JNIEnv *env, jobject obj, jobjectArray ptsObj)
{
	try
	{
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        {
            const auto ptClassInfo = Point2dClassInfo::getClassInfo();

            VectorLinearRef lin = VectorLinear::createLinear();

            JavaObjectArrayHelper ptsHelp(env, ptsObj);
            lin->pts.reserve(ptsHelp.numObjects());

            while (jobject ptObj = ptsHelp.getNextObject())
            {
                const Point2d *pt = ptClassInfo->getObject(env, ptObj);
                lin->pts.emplace_back(pt->x(), pt->y());
            }
            lin->initGeoMbr();
            (*vecObj)->shapes.insert(lin);
            return true;
        }
	}
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_addAreal___3Lcom_mousebird_maply_Point2d_2
  (JNIEnv *env, jobject obj, jobjectArray ptsObj)
{
	try
	{
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        {
            const auto ptClassInfo = Point2dClassInfo::getClassInfo();

            VectorArealRef ar = VectorAreal::createAreal();
            ar->loops.resize(1);
            auto &loop = ar->loops[0];

            JavaObjectArrayHelper ptsHelp(env, ptsObj);
            loop.reserve(ptsHelp.numObjects());

            while (jobject ptObj = ptsHelp.getNextObject())
            {
                const Point2d *pt = ptClassInfo->getObject(env, ptObj);
                ar->loops[0].emplace_back(pt->x(), pt->y());
            }
            ar->initGeoMbr();
            (*vecObj)->shapes.insert(ar);
            return true;
        }
	}
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_addAreal___3Lcom_mousebird_maply_Point2d_2_3_3Lcom_mousebird_maply_Point2d_2
  (JNIEnv *env, jobject obj, jobjectArray outerLoopObj, jobjectArray holesArray)
{
    try
    {
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        {
            const auto point2dClassInfo = Point2dClassInfo::getClassInfo();

            VectorArealRef ar = VectorAreal::createAreal();
            ar->loops.resize(1 + env->GetArrayLength(holesArray));

            for (int loop = 0; loop < ar->loops.size(); loop++)
            {
                jobjectArray ptsObj =
                        (loop == 0) ? outerLoopObj :
                        (jobjectArray) env->GetObjectArrayElement(holesArray, loop - 1);

                JavaObjectArrayHelper ptsHelp(env, ptsObj);
                ar->loops[loop].reserve(ptsHelp.numObjects());

                while (jobject ptObj = ptsHelp.getNextObject())
                {
                    Point2d *pt = point2dClassInfo->getObject(env, ptObj);
                    ar->loops[loop].emplace_back(pt->x(), pt->y());
                }

                if (loop > 0)
                {
                    env->DeleteLocalRef(ptsObj);
                }
            }

            ar->initGeoMbr();
            (*vecObj)->shapes.insert(ar);
            return true;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_mergeVectorsFrom
  (JNIEnv *env, jobject obj, jobject otherObj)
{
    try
    {
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto otherVec = VectorObjectClassInfo::get(env,otherObj))
        {
            (*vecObj)->mergeVectorsFrom(**otherVec);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_center
  (JNIEnv *env, jobject obj)
{
	try
	{
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        {
            Point2d center;
            if ((*vecObj)->center(center))
            {
                return MakePoint2d(env, center);
            }
        }
	}
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_centroid
  (JNIEnv *env, jobject obj)
{
	try
	{
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        {
            Point2d center;
            if ((*vecObj)->centroid(center))
            {
                return MakePoint2d(env, center);
            }
        }
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
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto ll = Point2dClassInfo::get(env, llObj))
        if (const auto ur = Point2dClassInfo::get(env, urObj))
        {
            Point2d center;
            if ((*vecObj)->largestLoopCenter(center, *ll, *ur))
            {
                return MakePoint2d(env, center);
            }
        }
	}
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorObject_linearMiddle__Lcom_mousebird_maply_Point2d_2
  (JNIEnv *env, jobject obj, jobject midObj)
{
	try
	{
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto mid = Point2dClassInfo::get(env,midObj))
        {
            double rot;
            if ((*vecObj)->linearMiddle(*mid, rot))
            {
                return rot;
            }
        }
	}
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorObject_linearMiddle__Lcom_mousebird_maply_Point2d_2Lcom_mousebird_maply_CoordSystem_2
  (JNIEnv *env, jobject obj, jobject midObj, jobject coordSysObj)
{
    try
    {
        const auto pt2dClassInfo = Point2dClassInfo::getClassInfo();
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto mid = pt2dClassInfo->getObject(env,midObj))
        if (const auto coordSys = CoordSystemRefClassInfo::get(env,coordSysObj))
        {
            double rot;
            if ((*vecObj)->linearMiddle(*mid,rot,coordSys->get()))
            {
                return rot;
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_middleCoordinate
  (JNIEnv *env, jobject obj, jobject midObj)
{
	try
	{
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto mid = Point2dClassInfo::get(env,midObj))
        {
            return (*vecObj)->middleCoordinate(*mid);
        }
	}
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_pointInside
  (JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto pt = Point2dClassInfo::get(env,ptObj))
        {
            return (*vecObj)->pointInside(*pt);
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorObject_areaOfOuterLoops
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        {
            return (*vecObj)->areaOfOuterLoops();
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_reverseAreals
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        {
            (*vecObj)->reverseAreals();
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_reversedAreals
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        {
            if ((*vecObj)->countAreals() == 0)
            {
                return obj;
            }
            if (auto newObj = (*vecObj)->reversedAreals())
            {
                return MakeVectorObjectWrapper(env, VectorObjectClassInfo::getClassInfo(), newObj);
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_closeLoops
        (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        {
            (*vecObj)->closeLoops();
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_closedLoops
        (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        {
            if ((*vecObj)->countUnClosedLoops() == 0)
            {
                // No un-closed loops, don't create a new object
                return obj;
            }
            if (auto newObj = (*vecObj)->closedLoops())
            {
                return MakeVectorObjectWrapper(env, VectorObjectClassInfo::getClassInfo(), newObj);
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_unCloseLoops
        (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        {
            (*vecObj)->unCloseLoops();
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_unClosedLoops
        (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        {
            if ((*vecObj)->countClosedLoops() == 0)
            {
                // No closed loops, don't create a new object
                return obj;
            }
            if (auto newObj = (*vecObj)->unClosedLoops())
            {
                return MakeVectorObjectWrapper(env, VectorObjectClassInfo::getClassInfo(), newObj);
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_clone
        (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        {
            if (auto newObj = (*vecObj)->deepCopy())
            {
                return MakeVectorObjectWrapper(env, VectorObjectClassInfo::getClassInfo(), newObj);
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_VectorObject_countPoints
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        {
            int numPts = 0;
            for (const auto &shape : (*vecObj)->shapes)
            {
                if (const auto linear = dynamic_cast<VectorLinear*>(shape.get()))
                {
                    numPts += linear->pts.size();
                }
              //} else if(const auto vec3d = dynamic_cast<VectorLinear3d*>(shape.get())) {
                else if(const auto ar = dynamic_cast<VectorAreal*>(shape.get()))
                {
                    for (auto & loop : ar->loops)
                    {
                        numPts += loop.size();
                    }
                }
                else if (const auto points = dynamic_cast<VectorPoints*>(shape.get()))
                {
                    numPts += points->pts.size();
                }
            }

            return numPts;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_anyIntersections
        (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        {
            return (*vecObj)->anyIntersections();
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_boundingBox
  (JNIEnv *env, jobject obj, jobject llObj, jobject urObj)
{
    try
    {
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto ll = Point2dClassInfo::get(env,llObj))
        if (const auto ur = Point2dClassInfo::get(env,urObj))
        {
            return (*vecObj)->boundingBox(*ll,*ur);
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_subdivideToGlobeNative
  (JNIEnv *env, jobject obj, jobject retObj, jdouble epsilon)
{
    try
    {
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto retVec = VectorObjectClassInfo::get(env,retObj))
        {
            *retVec = *vecObj;
            (*retVec)->subdivideToGlobe(epsilon);
            return true;
        }
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
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto retVecObj = VectorObjectClassInfo::get(env, retObj))
        {
            *retVecObj = *vecObj;
            (*retVecObj)->subdivideToGlobeGreatCircle(epsilon);
            return true;
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
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto retVecObj = VectorObjectClassInfo::get(env, retObj))
        {
            *retVecObj = *vecObj;
            (*retVecObj)->subdivideToFlatGreatCircle(epsilon);
            return true;
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
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto retVecObj = VectorObjectClassInfo::get(env, retObj))
        {
            *retVecObj = *vecObj;
            (*retVecObj)->subdivideToGlobeGreatCirclePrecise(epsilon);
            return true;
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
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto retVecObj = VectorObjectClassInfo::get(env, retObj))
        {
            *retVecObj = *vecObj;
            (*retVecObj)->subdivideToFlatGreatCirclePrecise(epsilon);
            return true;
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
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto retVecObj = VectorObjectClassInfo::get(env, retObj))
        {
            VectorObjectRef newVecObj = (*vecObj)->tesselate();
            *retVecObj = newVecObj;
            return true;
        }
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
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto retVecObj = VectorObjectClassInfo::get(env, retObj))
        {
            VectorObjectRef newVecObj = (*vecObj)->clipToGrid(Point2d(sizeX, sizeY));
            *retVecObj = newVecObj;
            return true;
        }
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
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto retVecObj = VectorObjectClassInfo::get(env, retObj))
        {
            VectorObjectRef newVecObj = (*vecObj)->clipToMbr({llX,llY}, {urX,urY});
            *retVecObj = newVecObj;
            return true;
        }
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
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto retVecObj = VectorObjectClassInfo::get(env, retObj))
        if (const auto srcSystem = CoordSystemRefClassInfo::get(env,srcSystemObj))
        if (const auto destSystem = CoordSystemRefClassInfo::get(env,destSystemObj))
        {
            VectorObjectRef newVecObj = (*vecObj)->deepCopy();
            newVecObj->reproject(srcSystem->get(), scale, destSystem->get());
            *retVecObj = newVecObj;
            return true;
        }
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
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto retVecObj = VectorObjectClassInfo::get(env, retObj))
        {
            VectorObjectRef newVecObj = (*vecObj)->filterClippedEdges();
            *retVecObj = newVecObj;
            return true;
        }
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
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto retVecObj = VectorObjectClassInfo::get(env, retObj))
        {
            VectorObjectRef newVecObj = (*vecObj)->linearsToAreals();
            *retVecObj = newVecObj;
            return true;
        }
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
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto retVecObj = VectorObjectClassInfo::get(env, retObj))
        {
            VectorObjectRef newVecObj = (*vecObj)->arealsToLinears();
            *retVecObj = newVecObj;
            return true;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
jboolean Java_com_mousebird_maply_VectorObject_fromGeoJSON
  (JNIEnv *env, jobject obj, jstring jstr)
{
	try
	{
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const char *cStr = env->GetStringUTFChars(jstr, nullptr))
        {
            std::string jsonStr(cStr);
            env->ReleaseStringUTFChars(jstr, cStr);

            std::string crs;
            return (*vecObj)->fromGeoJSON(jsonStr, crs);
        }
	}
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jobject Java_com_mousebird_maply_VectorObject_createLineString
  (JNIEnv* env, jclass, jobjectArray ptsObjs, jobject attrObj)
{
    try
    {
        if (auto obj = MakeVectorObject(env, std::make_shared<VectorObject>()))
        {
            if (Java_com_mousebird_maply_VectorObject_addLinear(env, obj, ptsObjs))
            {
                if (attrObj)
                {
                    Java_com_mousebird_maply_VectorObject_setAttributes(env, obj, attrObj);
                }
                return obj;
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jobject Java_com_mousebird_maply_VectorObject_createAreal
  (JNIEnv* env, jclass, jobjectArray ptsObjs, jobject attrObj)
{
    try
    {
        if (auto obj = MakeVectorObject(env, std::make_shared<VectorObject>()))
        {
            if (Java_com_mousebird_maply_VectorObject_addAreal___3Lcom_mousebird_maply_Point2d_2(env, obj, ptsObjs))
            {
                if (attrObj)
                {
                    Java_com_mousebird_maply_VectorObject_setAttributes(env, obj, attrObj);
                }
                return obj;
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_fromShapeFile
  (JNIEnv *env, jobject obj, jstring jstr)
{
    try
    {
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const char *cStr = env->GetStringUTFChars(jstr, nullptr))
        {
            std::string jsonStr(cStr);
            env->ReleaseStringUTFChars(jstr, cStr);

            return (*vecObj)->fromShapeFile(jsonStr);
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_canSplit
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto vecObjPtr = VectorObjectClassInfo::get(env,obj))
        {
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
        if (const auto vecObj = VectorObjectClassInfo::get(env,obj))
        if (const auto destVecObj = VectorObjectClassInfo::get(env, destObj))
        {
            VectorObjectRef newVecObj = (*vecObj)->deepCopy();
            *destVecObj = newVecObj;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_FromGeoJSONAssembly
  (JNIEnv *env, jclass /*vecObjClass*/, jstring jstr)
{
	try
	{
		if (const char *cStr = env->GetStringUTFChars(jstr,nullptr))
		{
            std::string jsonStr(cStr);
            env->ReleaseStringUTFChars(jstr, cStr);

            std::map<std::string, VectorObjectRef> vecData;
            if (VectorObject::FromGeoJSONAssembly(jsonStr, vecData))
            {
                JavaHashMapInfo *hashMapClassInfo = JavaHashMapInfo::getClassInfo(env);
                jobject hashMap = hashMapClassInfo->makeHashMap(env);
                for (const auto &kvp : vecData)
                {
                    if (jstring key = env->NewStringUTF(kvp.first.c_str()))
                    if (jobject vecObj = MakeVectorObject(env, kvp.second))
                    {
                        hashMapClassInfo->addObject(env, hashMap, key, vecObj);
                    }
                }
                return hashMap;
            }
        }
	}
    MAPLY_STD_JNI_CATCH()
	return nullptr;
}



