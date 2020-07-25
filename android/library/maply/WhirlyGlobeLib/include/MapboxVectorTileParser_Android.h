/*
 *  MapboxVectorTileParser_Android.h
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

#import <jni.h>
#import "WhirlyGlobe.h"

namespace WhirlyKit
{

// TODO: Turn this into a wrapper
class MapboxVectorTileParser_Android : public MapboxVectorTileParser
{
public:
    MapboxVectorTileParser_Android(VectorStyleDelegateImplRef styleDelegate);
    ~MapboxVectorTileParser_Android();

    // Set the Java environment and object for callbacks
    void setupJavaMethods(JNIEnv *env);

    // Return an Android version of the vector tile data that contains the same info
    virtual VectorTileDataRef makeTileDataCopy(VectorTileData *inTileData);

    // Filter out layers we don't care about
    virtual bool layerShouldParse(const std::string &layerName,VectorTileData *tileData);

    // Return a set of styles that will parse the given feature
    virtual SimpleIDSet stylesForFeature(MutableDictionaryRef attributes,const std::string &layerName,VectorTileData *tileData);

    // Build the objects for the appropriate style
    virtual void buildForStyle(long long styleID,std::vector<VectorObjectRef> &vecObjs,VectorTileDataRef data);

    // Parse the data and add specific iOS level stuff on top
    virtual bool parse(JNIEnv *env,RawData *rawData,VectorTileData *tileData);

protected:
    jmethodID shouldParseMethod;
    jmethodID styleForFeatureMethod;
    jmethodID buildForStyleMethod;
};

}