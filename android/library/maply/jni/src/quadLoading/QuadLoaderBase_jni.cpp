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
#import <Exceptions_jni.h>

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
        (*loader)->setFrameLoaderObj(env->NewGlobalRef(obj));
        (*loader)->setFlipY(true);
        info->setHandle(env, obj, loader);
    } catch (...) {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::initialise()");
    }
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_dispose(JNIEnv *env, jobject obj)
{
    try {
        const auto info = QuadImageFrameLoaderClassInfo::getClassInfo();

        std::lock_guard<std::mutex> lock(disposeMutex);
        const auto loader = info->getObject(env,obj);
        delete loader;
        info->clearHandle(env, obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::dispose()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_setFlipY(JNIEnv *env, jobject obj, jboolean flipY)
{
    try
    {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            (*loader)->setFlipY(flipY);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::setFlipY()");
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
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::getFlipY()");
    }

    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_QuadLoaderBase_getDebugMode(JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            return (*loader)->getDebugMode();
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::getDebugMode()");
    }
    return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_setDebugMode(JNIEnv *env, jobject obj, jboolean debugMode)
{
    try
    {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            (*loader)->setDebugMode(debugMode);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::setDebugMode()");
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
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::geoBoundsForTileNative()");
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
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::boundsForTileNative()");
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
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::displayCenterForTileNative()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_cleanupNative
        (JNIEnv *env, jobject obj, jobject changeObj)
{
    try
    {
        if (const auto loader = QuadImageFrameLoaderClassInfo::get(env,obj))
        {
            if (const auto changes = ChangeSetClassInfo::get(env,changeObj))
            {
                PlatformInfo_Android platformInfo(env);
                if ((*loader)->getMode() == QuadImageFrameLoader::Mode::MultiFrame)
                {
                    (*loader)->getController()->getScene()->removeActiveModel(&platformInfo,*loader);
                }

                (*loader)->cleanup(&platformInfo,**changes);
                (*loader)->teardown(&platformInfo);
            }
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::cleanupNative()");
    }
}

static void setLoadReturnRef(JNIEnv *env, jobject obj, jobject loadReturnObj,bool clear)
{
    if (const auto loaderPtr = QuadImageFrameLoaderClassInfo::get(env,obj))
    {
        if (const auto loader = *loaderPtr)
        {
            if (const auto loadReturnPtr = LoaderReturnClassInfo::get(env,loadReturnObj))
            {
                if (const auto loadReturn = *loadReturnPtr)
                {
                    const auto ptr = clear ? nullptr : loadReturn;
                    loader->setLoadReturnRef(loadReturn->ident,loadReturn->frame,ptr);
                }
            }
        }
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_setLoadReturn
        (JNIEnv *env, jobject obj, jobject loadReturnObj)
{
    try
    {
        setLoadReturnRef(env,obj,loadReturnObj,false);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::setLoadReturn()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_clearLoadReturn
        (JNIEnv *env, jobject obj, jobject loadReturnObj)
{
    try
    {
        setLoadReturnRef(env,obj,loadReturnObj,true);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::clearLoadReturn()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_mergeLoaderReturn
        (JNIEnv *env, jobject obj, jobject loadRetObj, jobject changeObj)
{
    try {
        if (!loadRetObj || !changeObj) {
            // Load failed, add any changes appropriate for a failure.
            // For now, that's ... nothing.
            return;
        }

        auto loaderPtr = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        auto loader = loaderPtr ? *loaderPtr : nullptr;
        auto loadReturnPtr = LoaderReturnClassInfo::getClassInfo()->getObject(env,loadRetObj);
        auto loadReturn = loadReturnPtr ? *loadReturnPtr : nullptr;
        auto changesPtr = ChangeSetClassInfo::getClassInfo()->getObject(env,changeObj);
        auto changes = changesPtr ? *changesPtr : nullptr;
        if (!loader || !loadReturn || !changes)
            return;

        // Move the change requests
        changes->insert(changes->end(),loadReturn->changes.begin(),loadReturn->changes.end());
        loadReturn->changes.clear();

        // Merge the objects
        PlatformInfo_Android platformInfo(env);
        loader->mergeLoadedTile(&platformInfo,loadReturn.get(),*changes);
        loadReturn->clear();

        // Detach the loader return from the frame
        loader->setLoadReturnRef(loadReturn->ident,loadReturn->frame,nullptr);

        // Destroy the loader return root reference
        LoaderReturnClassInfo::getClassInfo()->clearHandle(env,loadRetObj);
        delete loadReturnPtr;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::mergeLoaderReturn()");
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
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::samplingLayerConnectNative()");
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
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::samplingLayerDisconnectNative()");
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
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::getFrameID()");
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
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::getGeneration()");
    }

    return 0;
}

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
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::getZoomSlot()");
    }

    return -1;
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadLoaderBase_getNumFrames(JNIEnv *env, jobject obj)
{
    try
    {
        const auto ptr = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (const auto inst = ptr ? *ptr : nullptr)
        {
            return inst->getNumFrames();
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::getNumFrames()");
    }

    return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_reloadNative
        (JNIEnv *env, jobject obj, jobject changeSetObj)
{
    Java_com_mousebird_maply_QuadLoaderBase_reloadAreaNative(env, obj, changeSetObj, nullptr);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadLoaderBase_reloadAreaNative
        (JNIEnv *env, jobject obj, jobject changeSetObj, jobjectArray mbrArrayObj)
{
    try
    {
        auto loader = QuadImageFrameLoaderClassInfo::get(env,obj);
        auto changeSet = ChangeSetClassInfo::get(env,changeSetObj);
        if (!loader || !changeSet)
            return;

        JavaObjectArrayHelper mbrObjs(env, mbrArrayObj);
        std::vector<Mbr> mbrs;
        mbrs.reserve(mbrObjs.numObjects());
        while (mbrObjs.getNextObject())
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

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadLoaderBase_getModeNative(JNIEnv *env, jobject obj)
{
    try
    {
        const auto loaderPtr = QuadImageFrameLoaderClassInfo::get(env,obj);
        if (const auto loader = loaderPtr ? *loaderPtr : nullptr)
        {
            return loader->getMode();
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::getModeNative()");
    }
    return -1;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_QuadLoaderBase_isFrameLoading
        (JNIEnv *env, jobject obj, jobject identObj, jlong frameID)
{
    try
    {
        const auto loaderPtr = QuadImageFrameLoaderClassInfo::get(env,obj);
        if (const auto loader = loaderPtr ? *loaderPtr : nullptr)
        {
            const auto tileID = loader->getTileID(env, identObj);
            return loader->isFrameLoading(tileID,frameID);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::isFrameLoading()");
    }
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_QuadLoaderBase_mergeLoadedFrame
        (JNIEnv *env, jobject obj, jobject identObj, jlong frameID, jbyteArray rawData, jobject rawDataArray)
{
    try
    {
        const auto loaderPtr = QuadImageFrameLoaderClassInfo::get(env,obj);
        const auto loader = loaderPtr ? *loaderPtr : nullptr;
        if (!loader || !rawData)
        {
            return false;
        }

        // Get the raw bytes, not making a copy, if possible, since we're going to
        // copy it anyway.  Note that the critical section must be exited as soon as
        // possible and no JNI calls calls or blocking may be executed while it is open.
        const int rawDataSize = env->GetArrayLength(rawData);
        jboolean isCopy = false;
        if (auto rawBytes = (jbyte*)env->GetPrimitiveArrayCritical(rawData, &isCopy))
        {
            try
            {
                // We need to make a copy because this data may be held over until a later load
                // completes the frame.
                auto dataWrapper = std::make_shared<RawDataWrapper>(new jbyte[rawDataSize], rawDataSize, /*free=*/true);
                memcpy((jbyte*)dataWrapper->getRawData(), rawBytes, rawDataSize);
                env->ReleasePrimitiveArrayCritical(rawData, rawBytes, JNI_ABORT); // do not copy back
                rawBytes = nullptr; // cleanup complete, don't repeat on exception

                const auto tileID = loader->getTileID(env, identObj);

                // Do the merge.  If we're not the last fetch needed for the frame, this will
                // store the data, if we are, it will produce all the results for the frame.
                // todo: could we just use a `RawData` type that holds on to the array jobject?
                std::vector<RawDataRef> allData;
                const auto res = loader->mergeLoadedFrame(tileID, frameID, std::move(dataWrapper), allData);

                // Copy any produced data back to the caller's array
                for (const auto &data : allData)
                {
                    const auto len = (jsize)data->getLen();
                    if (const auto newArray = env->NewByteArray(len))
                    {
                        env->SetByteArrayRegion(newArray, 0, len, (jbyte*)data->getRawData());
                        env->CallBooleanMethod(rawDataArray, loader->arrayListAdd, newArray);
                        env->DeleteLocalRef(newArray);
                    }
                    else
                    {
                        __android_log_print(ANDROID_LOG_WARN, "Maply",
                                            "QuadLoaderBase::mergeLoadedFrame failed to create byte array");
                        logAndClearJVMException(env);
                        return false;
                    }
                }
                return res;
            }
            catch (...)
            {
                if (rawBytes)
                {
                    // since we can't `finally{}`, handle and re-throw.  todo: RAII wrapper
                    env->ReleasePrimitiveArrayCritical(rawData, rawBytes, JNI_ABORT);
                    logAndClearJVMException(env);
                }
                throw;
            }
        }
        else
        {
            __android_log_print(ANDROID_LOG_WARN, "Maply",
                                "QuadLoaderBase::mergeLoadedFrame failed to get input bytes");
            logAndClearJVMException(env);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in QuadLoaderBase::mergeLoadedFrame()");
    }
    return false;
}
