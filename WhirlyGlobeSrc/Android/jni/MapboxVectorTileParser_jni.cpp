/*
 *  MapboxVectorTileParser_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/25/16.
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
#import "com_mousebird_maply_MapboxVectorTileParser.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace Maply;

JNIEXPORT void JNICALL Java_com_mousebird_maply_MapboxVectorTileParser_nativeInit
(JNIEnv *env, jclass cls)
{
    MapboxVectorTileParserClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MapboxVectorTileParser_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        MapboxVectorTileParserClassInfo *classInfo = MapboxVectorTileParserClassInfo::getClassInfo();
        MapboxVectorTileParser *inst = new MapboxVectorTileParser();
        classInfo->setHandle(env,obj,inst);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapboxVectorTileParser::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_MapboxVectorTileParser_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        MapboxVectorTileParserClassInfo *classInfo = MapboxVectorTileParserClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            MapboxVectorTileParser *inst = classInfo->getObject(env,obj);
            if (!inst)
                return;
            delete inst;
            
            classInfo->clearHandle(env,obj);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapboxVectorTileParser::dispose()");
    }
}

JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_MapboxVectorTileParser_parseDataNative
(JNIEnv *env, jobject obj, jbyteArray data, jdouble minX, jdouble minY, jdouble maxX, jdouble maxY)
{
    try
    {
        MapboxVectorTileParserClassInfo *classInfo = MapboxVectorTileParserClassInfo::getClassInfo();
        MapboxVectorTileParser *inst = classInfo->getObject(env,obj);
        if (!inst || !data)
            return NULL;
        
        Mbr mbr;
        mbr.addPoint(Point2f(minX,minY));
        mbr.addPoint(Point2f(maxX,maxY));
        
        // Parse vector tile and create vector objects
        jbyte *bytes = env->GetByteArrayElements(data,NULL);
        RawDataWrapper rawData(bytes,env->GetArrayLength(data),false);
        std::vector<VectorObject *> vecObjs;
        bool ret = inst->parseVectorTile(&rawData,vecObjs,mbr);
        env->ReleaseByteArrayElements(data,bytes, 0);
        
        if (vecObjs.empty())
        {
            return NULL;
        } else {
            VectorObjectClassInfo *vecClassInfo = VectorObjectClassInfo::getClassInfo();
            if (!vecClassInfo)
                vecClassInfo = VectorObjectClassInfo::getClassInfo(env,"com/mousebird/maply/VectorObject");
            jobjectArray retArr = env->NewObjectArray(vecObjs.size(), vecClassInfo->getClass(), NULL);

            int which = 0;
            for (VectorObject *vecObj : vecObjs)
            {
                jobject vecObjObj = MakeVectorObject(env,vecObj);
                env->SetObjectArrayElement( retArr, which, vecObjObj);
                env->DeleteLocalRef( vecObjObj);
                which++;
            }
            
            return retArr;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapboxVectorTileParser::parseDataNative()");
    }
    
    return NULL;
}
