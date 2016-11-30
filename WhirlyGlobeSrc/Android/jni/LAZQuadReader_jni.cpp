/*
 *  LAZQuadReader_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg
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
#import "com_mousebird_maply_LAZQuadReader.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;

class LAZQuadReader
{
public:
    LAZQuadReader()
    : coordSys(NULL)
    {        
    }
    
    Point3d ll,ur;
    double zOffset;
    int minZoom,maxZoom;
    int minTilePoints,maxTilePoints;
    float pointSize;
    int colorScale;
    int pointType;
    CoordSystem *coordSys;
};

typedef JavaClassInfo<LAZQuadReader> LAZQuadReaderClassInfo;
template<> LAZQuadReaderClassInfo *LAZQuadReaderClassInfo::classInfoObj = NULL;


JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_nativeInit
(JNIEnv *env, jclass cls)
{
    LAZQuadReaderClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReader *readerInst = new LAZQuadReader();
        LAZQuadReaderClassInfo::getClassInfo()->setHandle(env,obj,readerInst);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        classInfo->clearHandle(env, obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::dispose()");
    }
}


JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_setBounds
(JNIEnv *env, jobject obj, jdouble minX, jdouble minY, jdouble minZ, jdouble maxX, jdouble maxY, jdouble maxZ)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return;
        
        lazReader->ll = Point3d(minX,minY,minZ);
        lazReader->ur = Point3d(maxX,maxY,maxZ);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::setBounds()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_LAZQuadReader_getLL
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return NULL;
        
        return MakePoint3d(env,lazReader->ll);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::getLL()");
    }
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_LAZQuadReader_getUR
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return NULL;

        return MakePoint3d(env,lazReader->ur);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::getUR()");
    }

    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_setZOffset
(JNIEnv *env, jobject obj, jdouble zOffset)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return;
        
        lazReader->zOffset = zOffset;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::setZOffset()");
    }

}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_LAZQuadReader_getZOffset
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return 0.0;
        
        return lazReader->zOffset;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::getZOffset()");
    }

    return 0.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_setZoomLevels
(JNIEnv *env, jobject obj, jint minZoom, jint maxZoom)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return;
        
        lazReader->minZoom = minZoom;
        lazReader->maxZoom = maxZoom;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::setZoomLevels()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_LAZQuadReader_getMinZoom
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return -1;
        
        return lazReader->minZoom;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::getMinZoom()");
    }
    
    return -1;
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_LAZQuadReader_getMaxZoom
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return -1;
        
        return lazReader->maxZoom;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::getMaxZoom()");
    }
    
    return -1;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_setTilePoints
(JNIEnv *env, jobject obj, jint minTilePoints, jint maxTilePoints)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return;
        
        lazReader->minTilePoints = minTilePoints;
        lazReader->maxTilePoints = maxTilePoints;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::setTilePoints()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_LAZQuadReader_getMinTilePoints
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return -1;
        
        return lazReader->minTilePoints;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::getMinTilePoints()");
    }
    
    return -1;
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_LAZQuadReader_getMaxTilePoints
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return -1;
        
        return lazReader->maxTilePoints;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::getMaxTilePoints()");
    }
    
    return -1;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_setPointSize
(JNIEnv *env, jobject obj, jfloat pointSize)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return;
        
        lazReader->pointSize = pointSize;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::setPointSize()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_LAZQuadReader_getPointSize
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return 0.0;
        
        return lazReader->pointSize;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::getPointSize()");
    }
    
    return 0.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_setColorScale
(JNIEnv *env, jobject obj, jint colorScale)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return;
        
        lazReader->colorScale = colorScale;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::setColorScale()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_LAZQuadReader_getColorScale
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return 0;
        
        return lazReader->colorScale;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::getColorScale()");
    }
    
    return 0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_setPointType
(JNIEnv *env, jobject obj, jint pointType)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return;
        
        lazReader->pointType = pointType;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::setPointType()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_LAZQuadReader_getPointType
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return 0;
        
        return lazReader->pointType;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::getPointType()");
    }
    
    return 0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_setCoordSystemNative
(JNIEnv *env, jobject obj, jobject coordSysObj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        CoordSystem *coordSys = CoordSystemClassInfo::getClassInfo()->getObject(env,coordSysObj);
        if (!lazReader || !coordSys)
            return;

        lazReader->coordSys = coordSys;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::setCoordSystemNative()");
    }
}
