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

namespace WhirlyKit
{
MapboxVectorTileParser_Android::MapboxVectorTileParser_Android()
: env(NULL), obj(NULL)
{
}

MapboxVectorTileParser_Android::~MapboxVectorTileParser_Android()
{
    env = NULL;
    obj = NULL;
}

void MapboxVectorTileParser_Android::setJavaObject(JNIEnv *inEnv,jobject inObj)
{
    env = inEnv;
    obj = inObj;

    if (!env) {
        jclass thisClass = MapboxVectorTileParserClassInfo::getClassInfo()->getClass();
        shouldParseMethod = env->GetMethodID(thisClass,"layerShouldParse","(Ljava/lang/String;Lcom/mousebird/maply/VectorTileData;)Z");
        styleForFeatureMethod = env->GetMethodID(thisClass,"stylesForFeature","(Lcom/mousebird/maply/AttrDictionary;Ljava/lang/String;Lcom/mousebird/maply/VectorTileData;)[J");
        buildForStyleMethod = env->GetMethodID(thisClass,"buildForStyle","(J[Lcom/mousebird/maply/VectorObject;Lcom/mousebird/maply/VectorTileData;)V");
    }
}

bool MapboxVectorTileParser_Android::layerShouldParse(const std::string &layerName,VectorTileData *tileData)
{
    // TODO: Need the java side object for the tileData
    return env->CallVoidMethod(obj,shouldParseMethod,)
}

SimpleIDSet MapboxVectorTileParser_Android::stylesForFeature(MutableDictionaryRef attributes,const std::string &layerName,VectorTileData *tileData)
{
    // TODO: Call the Java side
}

void MapboxVectorTileParser_Android::buildForStyle(long long styleID,std::vector<VectorObjectRef> &vecObjs,VectorTileDataRef data)
{
    // TODO: Call the Java side
}

bool MapboxVectorTileParser_Android::parse(RawData *rawData,VectorTileData *tileData)
{
    return MapboxVectorTileParser::parse(rawData,tileData);
}

}