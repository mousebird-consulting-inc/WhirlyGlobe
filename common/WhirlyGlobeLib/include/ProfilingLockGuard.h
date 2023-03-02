/*  ProfilingLockGuard.h
 *  WhirlyGlobeLib
 *
 *  Created by Tim Sylvester on 2/23/2023
 *  Copyright 2023-2023 mousebird consulting
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

#import <WhirlyTypes.h>
#import <mutex>

namespace WhirlyKit
{

/// Wraps `std::lock_guard` with timing information
struct ProfilingLockGuard : std::lock_guard<std::mutex>
{
    ProfilingLockGuard(std::mutex &mutex, TimeInterval t0 = PerfTime());
    ~ProfilingLockGuard();
    const TimeInterval t0;
    
    static inline TimeInterval PerfTime();
};

}

