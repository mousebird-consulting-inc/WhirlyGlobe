/*
 *  VectorTileData_Android.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/15/19.
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

// Android version of VectorTileData.  References a Java-side object
class VectorTileData_Android : public VectorTileData {
public:
    VectorTileData_Android();
    VectorTileData_Android(const VectorTileData_Android &that);
    ~VectorTileData_Android();

    void setEnv(JNIEnv *env,jobject parserObj,jobject vectorTileDataObj);

public:
    JNIEnv *env;         // Set by the JNI side for each call that uses this
    jobject parserObj;   // The MapboxVectorTileParser_Android
    jobject vecTileDataObj;  // Java side for this object
};

typedef std::shared_ptr<VectorTileData_Android> VectorTileData_AndroidRef;
}