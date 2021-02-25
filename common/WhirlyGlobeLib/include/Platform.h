/*
 *  Platform.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 12/13/13.
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

#include <stdlib.h>
#import <ctime>
#import <vector>
#ifdef __ANDROID__
#import <jni.h>
#include <android/log.h>
#endif
#import "WhirlyTypes.h"

namespace WhirlyKit
{

// Wrapper for platform specific time function
extern TimeInterval TimeGetCurrent();

/**
 * On Android we have to pass around per-thread information.
 * So this is a base class passed in to the parse and build methods.  Platforms put whatever per-thread info they need into it.
*/
class PlatformThreadInfo
{
public:
};

}
