/*  QuadLoaderBase_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg on 3/25/19.
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

#import "QuadLoading_jni.h"
#import "Geometry_jni.h"
#import "Scene_jni.h"
#import "com_mousebird_maply_QuadLoaderBase.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> QuadImageFrameLoaderClassInfo *QuadImageFrameLoaderClassInfo::classInfoObj = nullptr;

static jclass mbrClass = nullptr;
static jfieldID llID = nullptr;
static jfieldID urID = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_nativeInit(JNIEnv *env, jclass cls)
{
    QuadImageFrameLoaderClassInfo::getClassInfo(env, cls);

    if (!mbrClass)
    {
        mbrClass = env->FindClass("com/mousebird/maply/Mbr");
        llID = env->GetFieldID(mbrClass, "ll", "Lcom/mousebird/maply/Point2d;");
        urID = env->GetFieldID(mbrClass, "ur", "Lcom/mousebird/maply/Point2d;");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_initialise
        (JNIEnv *env, jobject obj, jobject sampleObj, jint numFrames, jint mode)
{
    try {
        auto info = QuadImageFrameLoaderClassInfo::getClassInfo();
        auto params = SamplingParamsClassInfo::getClassInfo()->getObject(env,sampleObj);
        PlatformInfo_Android platformInfo(env);
        auto loader = new QuadImageFrameLoader_AndroidRef(
                new QuadImageFrameLoader_Android(&platformInfo,*params,numFrames,(QuadImageFrameLoader::Mode)mode));
        (*loader)->frameLoaderObj = env->NewGlobalRef(obj);
        (*loader)->setFlipY(true);
        info->setHandle(env, obj, loader);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadLoaderBase::initialise()");
    }
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_dispose(JNIEnv *env, jobject obj)
{
    try {
        QuadImageFrameLoaderClassInfo *info = QuadImageFrameLoaderClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            QuadImageFrameLoader_AndroidRef *loader = info->getObject(env,obj);
            if (!loader)
                return;
            if ((*loader)->frameLoaderObj) {
                env->DeleteGlobalRef((*loader)->frameLoaderObj);
                (*loader)->frameLoaderObj = nullptr;
            }
            delete loader;

            info->clearHandle(env, obj);
        }

    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadLoaderBase::dispose()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_setFlipY(JNIEnv *env, jobject obj, jboolean flipY)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (!loader)
            return;
        (*loader)->setFlipY(flipY);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadLoaderBase::setFlipY()");
    }
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_QuadLoaderBase_getFlipY(JNIEnv *env, jobject obj)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (!loader)
            return false;
        return (*loader)->getFlipY();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadLoaderBase::getFlipY()");
    }

    return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_setDebugMode(JNIEnv *env, jobject obj, jboolean debugMode)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (!loader)
            return;
        (*loader)->setDebugMode(debugMode);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadLoaderBase::setDebugMode()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_geoBoundsForTileNative
        (JNIEnv *env, jobject obj, jint tileX, jint tileY, jint tileLevel, jobject llObj, jobject urObj)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        Point2dClassInfo *point2dClassInfo = Point2dClassInfo::getClassInfo();
        Point2d *ll = point2dClassInfo->getObject(env,llObj);
        Point2d *ur = point2dClassInfo->getObject(env,urObj);
        if (!loader || !ll || !ur)
            return;

        QuadDisplayControllerNew *control = (*loader)->getController();
        if (!control)
            return;
        MbrD mbrD = control->getQuadTree()->generateMbrForNode(WhirlyKit::QuadTreeNew::Node(tileX,tileY,tileLevel));

        CoordSystem *wkCoordSys = control->getCoordSys();
        if (!wkCoordSys)
            return;
        Point2d pts[4];
        pts[0] = wkCoordSys->localToGeographicD(Point3d(mbrD.ll().x(),mbrD.ll().y(),0.0));
        pts[1] = wkCoordSys->localToGeographicD(Point3d(mbrD.ur().x(),mbrD.ll().y(),0.0));
        pts[2] = wkCoordSys->localToGeographicD(Point3d(mbrD.ur().x(),mbrD.ur().y(),0.0));
        pts[3] = wkCoordSys->localToGeographicD(Point3d(mbrD.ll().x(),mbrD.ur().y(),0.0));
        Point2d minPt(pts[0].x(),pts[0].y()),  maxPt(pts[0].x(),pts[0].y());
        for (unsigned int ii=1;ii<4;ii++)
        {
            minPt.x() = std::min(minPt.x(),pts[ii].x());
            minPt.y() = std::min(minPt.y(),pts[ii].y());
            maxPt.x() = std::max(maxPt.x(),pts[ii].x());
            maxPt.y() = std::max(maxPt.y(),pts[ii].y());
        }
        ll->x() = minPt.x();  ll->y() = minPt.y();
        ur->x() = maxPt.x();  ur->y() = maxPt.y();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadLoaderBase::geoBoundsForTileNative()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_boundsForTileNative
        (JNIEnv *env, jobject obj, jint tileX, jint tileY, jint tileLevel, jobject llObj, jobject urObj)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        Point2dClassInfo *point2dClassInfo = Point2dClassInfo::getClassInfo();
        Point2d *ll = point2dClassInfo->getObject(env,llObj);
        Point2d *ur = point2dClassInfo->getObject(env,urObj);
        if (!loader || !ll || !ur)
            return;

        QuadDisplayControllerNew *control = (*loader)->getController();
        if (!control)
            return;
        MbrD mbrD = control->getQuadTree()->generateMbrForNode(WhirlyKit::QuadTreeNew::Node(tileX,tileY,tileLevel));

        ll->x() = mbrD.ll().x();  ll->y() = mbrD.ll().y();
        ur->x() = mbrD.ur().x();  ur->y() = mbrD.ur().y();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadLoaderBase::boundsForTileNative()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_displayCenterForTileNative
        (JNIEnv *env, jobject obj, jint tileX, jint tileY, jint tileLevel, jobject ptObj)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        Point3d *outPt = Point3dClassInfo::getClassInfo()->getObject(env,ptObj);
        if (!loader || !outPt)
            return;
        QuadDisplayControllerNew *control = (*loader)->getController();
        if (!control)
            return;

        MbrD mbrD = control->getQuadTree()->generateMbrForNode(WhirlyKit::QuadTreeNew::Node(tileX,tileY,tileLevel));

        Point2d pt((mbrD.ll().x()+mbrD.ur().x())/2.0,(mbrD.ll().y()+mbrD.ur().y())/2.0);
        Scene *scene = control->getScene();
        Point3d locCoord = CoordSystemConvert3d(control->getCoordSys(), scene->getCoordAdapter()->getCoordSystem(), Point3d(pt.x(),pt.y(),0.0));
        *outPt = scene->getCoordAdapter()->localToDisplay(locCoord);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadLoaderBase::displayCenterForTileNative()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_cleanupNative
        (JNIEnv *env, jobject obj, jobject changeObj)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        ChangeSetRef *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changeObj);
        if (!loader || !changes)
            return;

        if (loader->get()->getMode() == QuadImageFrameLoader::Mode::MultiFrame) {
            loader->get()->getController()->getScene()->removeActiveModel(*loader);
        }

        PlatformInfo_Android platformInfo(env);
        (*loader)->cleanup(&platformInfo,*(changes->get()));
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadLoaderBase::cleanupNative()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_mergeLoaderReturn
        (JNIEnv *env, jobject obj, jobject loadRetObj, jobject changeObj)
{
    try {
        if (!loadRetObj) {
            // Load failed, add any changes appropriate for a failure.
            // For now, that's ... nothing.
            return;
        }

        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        QuadLoaderReturnRef *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,loadRetObj);
        ChangeSetRef *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changeObj);
        if (!loader || !loadReturn || !changes)
            return;
        PlatformInfo_Android platformInfo(env);
        (*changes)->insert((*changes)->end(),(*loadReturn)->changes.begin(),(*loadReturn)->changes.end());
        (*loadReturn)->changes.clear();
        (*loader)->mergeLoadedTile(&platformInfo,loadReturn->get(),*(changes->get()));
        LoaderReturnClassInfo::getClassInfo()->clearHandle(env,loadRetObj);
        delete loadReturn;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadLoaderBase::mergeLoaderReturn()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_samplingLayerConnectNative
        (JNIEnv *env, jobject obj, jobject layerObj, jobject changeObj)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        QuadSamplingController_Android *control = QuadSamplingControllerInfo::getClassInfo()->getObject(env,layerObj);
        ChangeSetRef *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changeObj);
        if (!loader || !control || !changes)
            return;

        PlatformInfo_Android platformInfo(env);
        if (control->addBuilderDelegate(&platformInfo,*loader)) {
            // This will result in callbacks to the Java side
            control->notifyDelegateStartup(&platformInfo,((QuadTileBuilderDelegate *) (*loader).get())->getId(),
                                           *(changes->get()));
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadLoaderBase::samplingLayerConnectNative()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_samplingLayerDisconnectNative
        (JNIEnv *env, jobject obj, jobject layerObj, jobject changeObj)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        QuadSamplingController_Android *control = QuadSamplingControllerInfo::getClassInfo()->getObject(env,layerObj);
        //ChangeSetRef *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changeObj);
        if (!loader || !control)
            return;

        PlatformInfo_Android platformInfo(env);
        control->removeBuilderDelegate(&platformInfo,*loader);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadLoaderBase::samplingLayerDisconnectNative()");
    }
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_QuadLoaderBase_getFrameID
        (JNIEnv *env, jobject obj, jint frameIndex)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);

        if (frameIndex < 0 || frameIndex >= (*loader)->getNumFrames())
            return 0;

        return (*loader)->getFrameInfo(frameIndex)->getId();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadLoaderBase::getFrameID()");
    }

    return 0;
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadLoaderBase_getGeneration(JNIEnv *env, jobject obj)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (!loader)
            return 0;

        return (*loader)->getGeneration();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadLoaderBase::getGeneration()");
    }

    return 0;
}

/*
 * Class:     com_mousebird_maply_QuadLoaderBase
 * Method:    getZoomSlot
 * Signature: ()I
 */
extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadLoaderBase_getZoomSlot(JNIEnv *env, jobject obj)
{
    try
    {
        const auto ptr = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (const auto inst = ptr ? *ptr : nullptr)
        {
            if (const auto dc = inst->getController())
            {
                return dc->getZoomSlot();
            }
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::getGeneration()");
    }

    return -1;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_reloadNative
        (JNIEnv *env, jobject obj, jobject changeSetObj)
{
    Java_com_mousebird_maply_QuadLoaderBase_reloadAreaNative(env, obj, changeSetObj, nullptr);
}

/*
 * Class:     com_mousebird_maply_QuadLoaderBase
 * Method:    reloadAreaNative
 * Signature: (Lcom/mousebird/maply/ChangeSet;[Lcom/mousebird/maply/Mbr;)V
 */
extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_reloadAreaNative
        (JNIEnv *env, jobject obj, jobject changeSetObj, jobjectArray mbrArrayObj)
{
    try
    {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!loader || !changeSet)
            return;

        std::vector<Mbr> mbrs;
        for (JavaObjectArrayHelper mbrObjs(env, mbrArrayObj); mbrObjs.getNextObject(); )
        {
            const auto llObj = env->GetObjectField(mbrObjs.getCurrentObject(), llID);
            const auto urObj = env->GetObjectField(mbrObjs.getCurrentObject(), urID);

            const auto llx = Java_com_mousebird_maply_Point2d_getX(env, llObj);
            const auto lly = Java_com_mousebird_maply_Point2d_getY(env, llObj);
            const auto urx = Java_com_mousebird_maply_Point2d_getX(env, urObj);
            const auto ury = Java_com_mousebird_maply_Point2d_getY(env, urObj);

            env->DeleteLocalRef(llObj);
            env->DeleteLocalRef(urObj);

            if (mbrs.empty())
            {
                mbrs.reserve(mbrObjs.numObjects());
            }
            mbrs.emplace_back(Point2f(llx, lly), Point2f(urx, ury));
        }

        PlatformInfo_Android platformInfo(env);
        (*loader)->reload(&platformInfo,-1,mbrs.empty() ? nullptr : &mbrs[0],(int)mbrs.size(), **changeSet);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::reloadAreaNative()");
    }
}
