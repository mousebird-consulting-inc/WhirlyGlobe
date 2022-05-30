/*  Defer.h
 *  WhirlyGlobeLib
 *
 *  Created by Tim Sylvester on 4/29/22.
 *  Copyright 2022-2022 Wet Dog Weather
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

#ifndef MAPLY_DEFER_H
#define MAPLY_DEFER_H

namespace WhirlyKit
{
    template <typename T>
    struct DeferWrapper
    {
        DeferWrapper(T f) : f(f) { }
        DeferWrapper(DeferWrapper<T> &&other) : f(std::move(other.f)) { other.enable = false; }
        ~DeferWrapper() { if (enable) { try { f(); } catch (...) { } } }
    protected:
        T f;
        bool enable = true;
    };

    /// Run some code when exiting the current scope
    /// `auto _ = WhirlyKit::Defer([](){ ... })`
    template <typename T> static DeferWrapper<T> Defer(T f) { return DeferWrapper<T>(f); }
}

#endif //MAPLY_DEFER_H
