/*  ProfilingLockGuard.cpp
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

#import <ProfilingLockGuard.h>
#import <Platform.h>
#import <WhirlyKitLog.h>

using namespace WhirlyKit;

static constexpr double acquireWarnThreshold = 0.001;
static constexpr double holdWarnThreshold = 0.001;
static constexpr double extraHoldTime = 0.0;

ProfilingLockGuard::ProfilingLockGuard(std::mutex &mutex, TimeInterval t0) :
    std::lock_guard<std::mutex>(mutex),
    t0(t0)
{
    const auto t = PerfTime() - t0;
    if (t >= acquireWarnThreshold)
    {
        wkLogLevel(Warn, "Lock acquisition took %f", t);
    }
}

ProfilingLockGuard::~ProfilingLockGuard()
{
    const auto t = PerfTime() - t0;
    if (t >= holdWarnThreshold)
    {
        wkLogLevel(Warn, "Lock held for %f", t);
    }
    if (extraHoldTime > 0)
    {
        const timespec t = { (int)extraHoldTime, (int64_t)(extraHoldTime * 1.0e9 + 0.5) };
        nanosleep(&t, nullptr);
    }
}

TimeInterval ProfilingLockGuard::PerfTime()
{
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return (double)tp.tv_sec + tp.tv_nsec * (double)1e-9;
}
