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
MapboxVectorTileParser_Android::MapboxVectorTileParser_Android(VectorStyleDelegateImplRef styleDelegate)
: shouldParseMethod(NULL), styleForFeatureMethod(NULL), buildForStyleMethod(NULL), MapboxVectorTileParser(styleDelegate)
{
}

MapboxVectorTileParser_Android::~MapboxVectorTileParser_Android()
{
}

void MapboxVectorTileParser_Android::setupJavaMethods(JNIEnv *env)
{
    if (!shouldParseMethod) {
        jclass thisClass = MapboxVectorTileParserClassInfo::getClassInfo()->getClass();
        shouldParseMethod = env->GetMethodID(thisClass,"layerShouldParse","(Ljava/lang/String;Lcom/mousebird/maply/VectorTileData;)Z");
        styleForFeatureMethod = env->GetMethodID(thisClass,"stylesForFeature","(Lcom/mousebird/maply/AttrDictionary;Ljava/lang/String;Lcom/mousebird/maply/VectorTileData;)[J");
        buildForStyleMethod = env->GetMethodID(thisClass,"buildForStyle","(J[Lcom/mousebird/maply/VectorObject;Lcom/mousebird/maply/VectorTileData;)V");
    }
}

VectorTileDataRef MapboxVectorTileParser_Android::makeTileDataCopy(VectorTileData *inTileData)
{
    VectorTileData_Android *theTileData = (VectorTileData_Android *)inTileData;
    VectorTileData_AndroidRef tileData(new VectorTileData_Android(*theTileData));

    return tileData;
}

bool MapboxVectorTileParser_Android::layerShouldParse(const std::string &layerName,VectorTileData *inTileData)
{
    VectorTileData_Android *tileData = (VectorTileData_Android *)inTileData;

    jstring layerNameStr = tileData->env->NewStringUTF(layerName.c_str());
    bool ret = tileData->env->CallBooleanMethod(tileData->parserObj,shouldParseMethod,layerNameStr,tileData->vecTileDataObj);
    tileData->env->DeleteLocalRef(layerNameStr);

    return ret;
}

SimpleIDSet MapboxVectorTileParser_Android::stylesForFeature(MutableDictionaryRef inAttrs,const std::string &layerName,VectorTileData *inTileData)
{
    VectorTileData_Android *tileData = (VectorTileData_Android *)inTileData;
    MutableDictionary_AndroidRef attrs = std::dynamic_pointer_cast<MutableDictionary_Android>(inAttrs);

    // Call the Java side to get a list of style IDs
    jobject attrObj = MakeAttrDictionary(tileData->env,attrs);
    jstring layerNameStr = tileData->env->NewStringUTF(layerName.c_str());
    jlongArray idArray = (jlongArray)tileData->env->CallObjectMethod(tileData->parserObj,styleForFeatureMethod,attrObj,layerNameStr,tileData->vecTileDataObj);
    tileData->env->DeleteLocalRef(layerNameStr);
    tileData->env->DeleteLocalRef(attrObj);

    SimpleIDSet styleIDs;
    ConvertLongArrayToSet(tileData->env,idArray,styleIDs);
    tileData->env->DeleteLocalRef(idArray);

    return styleIDs;
}

void MapboxVectorTileParser_Android::buildForStyle(long long styleID,std::vector<VectorObjectRef> &vecObjs,VectorTileDataRef inTileData)
{
    VectorTileData_AndroidRef tileData = std::dynamic_pointer_cast<VectorTileData_Android>(inTileData);

    // Wrap the vectors in individual Java objects
    std::vector<jobject> objsJava;
    for (VectorObjectRef vecObj : vecObjs) {
        jobject objJava = MakeVectorObject(tileData->env,vecObj);
        objsJava.push_back(objJava);
    }
    jobjectArray objsArrJava = BuildObjectArray(tileData->env,VectorObjectClassInfo::getClassInfo()->getClass(),objsJava);

    tileData->env->CallVoidMethod(tileData->parserObj,buildForStyleMethod,styleID,objsArrJava,tileData->vecTileDataObj);

    // Tear down the Java objects we built up
    tileData->env->DeleteLocalRef(objsArrJava);
    for (jobject objJava : objsJava)
        tileData->env->DeleteLocalRef(objJava);
}

bool MapboxVectorTileParser_Android::parse(JNIEnv *env,RawData *rawData,VectorTileData *tileData)
{
    PlatformInfo_Android inst(env);

    return MapboxVectorTileParser::parse(&inst,rawData,tileData);
}

}