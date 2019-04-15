/*
 *  MapboxVectorTileParser_Android.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/12/19.
 *  Copyright 2011-2019 mousebird consulting
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

#import "MapboxVectorTileParser_Android.h"
#import "Formats_jni.h"
#import "Vectors_jni.h"

namespace WhirlyKit
{
MapboxVectorTileParser_Android::MapboxVectorTileParser_Android()
: env(NULL), shouldParseMethod(NULL), styleForFeatureMethod(NULL), buildForStyleMethod(NULL)
{
}

MapboxVectorTileParser_Android::~MapboxVectorTileParser_Android()
{
    env = NULL;
}

void MapboxVectorTileParser_Android::setupJavaMethods(JNIEnv *inEnv)
{
    if (!shouldParseMethod) {
        jclass thisClass = MapboxVectorTileParserClassInfo::getClassInfo()->getClass();
        shouldParseMethod = env->GetMethodID(thisClass,"layerShouldParse","(Ljava/lang/String;Lcom/mousebird/maply/VectorTileData;)Z");
        styleForFeatureMethod = env->GetMethodID(thisClass,"stylesForFeature","(Lcom/mousebird/maply/AttrDictionary;Ljava/lang/String;Lcom/mousebird/maply/VectorTileData;)[J");
        buildForStyleMethod = env->GetMethodID(thisClass,"buildForStyle","(J[Lcom/mousebird/maply/VectorObject;Lcom/mousebird/maply/VectorTileData;)V");
    }
}

bool MapboxVectorTileParser_Android::layerShouldParse(const std::string &layerName,VectorTileData *inTileData)
{
    VectorTileData_Android *tileData = (VectorTileData_Android *)inTileData;

    jstring layerNameStr = env->NewStringUTF(layerName.c_str());
    bool ret = env->CallBooleanMethod(tileData->parserObj,shouldParseMethod,layerNameStr,tileData->vecTileDataObj);
    env->DeleteLocalRef(layerNameStr);

    return ret;
}

SimpleIDSet MapboxVectorTileParser_Android::stylesForFeature(MutableDictionaryRef inAttrs,const std::string &layerName,VectorTileData *inTileData)
{
    VectorTileData_Android *tileData = (VectorTileData_Android *)inTileData;
    MutableDictionary_AndroidRef attrs = std::dynamic_pointer_cast<MutableDictionary_Android>(inAttrs);

    // Call the Java side to get a list of style IDs
    jobject attrObj = MakeAttrDictionary(tileData->env,attrs);
    jlongArray idArray = (jlongArray)env->CallObjectMethod(tileData->parserObj,styleForFeatureMethod,attrObj,tileData->vecTileDataObj);
    env->DeleteLocalRef(attrObj);

    SimpleIDSet styleIDs;
    ConvertLongArrayToSet(tileData->env,idArray,styleIDs);
    env->DeleteLocalRef(idArray);

    return styleIDs;
}

void MapboxVectorTileParser_Android::buildForStyle(long long styleID,std::vector<VectorObjectRef> &vecObjs,VectorTileDataRef inTileData)
{
    VectorTileData_AndroidRef tileData = std::dynamic_pointer_cast<VectorTileData_Android>(inTileData);

    // Wrap the vectors in individual Java objects
    std::vector<jobject> objsJava;
    for (VectorObjectRef vecObj : vecObjs) {
        jobject objJava = MakeVectorObject(env,vecObj);
        objsJava.push_back(objJava);
    }
    jobjectArray objsArrJava = BuildObjectArray(env,VectorObjectClassInfo::getClassInfo()->getClass(),objsJava);

    env->CallVoidMethod(tileData->parserObj,buildForStyleMethod,objsArrJava,tileData->vecTileDataObj);

    // Tear down the Java objects we built up
    env->DeleteLocalRef(objsArrJava);
    for (jobject objJava : objsJava)
        env->DeleteLocalRef(objJava);
}

bool MapboxVectorTileParser_Android::parse(RawData *rawData,VectorTileData *tileData)
{
    return MapboxVectorTileParser::parse(rawData,tileData);
}

}