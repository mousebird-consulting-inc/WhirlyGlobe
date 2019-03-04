/*
 *  platform.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 12/18/13.
 *  Copyright 2011-2013 mousebird consulting
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

#import "Platform.h"

namespace WhirlyKit
{

// Return current time in seconds as a double
TimeInterval TimeGetCurrent()
{
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    
    return (double)tp.tv_sec + tp.tv_nsec * (double)1e-9;
    
}

// No retina on Android to mess with scale
float DeviceScreenScale()
{
	return 1.0;
}

}
