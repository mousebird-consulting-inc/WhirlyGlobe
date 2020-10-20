/*
 *  WorkRegion_private.h
 *  WhirlyGlobeComponent
 *
 *  Created by Tim Sylvester on 10/19/2020.
 *  Copyright 2020 mousebird consulting
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

#ifndef WorkRegion_private_h
#define WorkRegion_private_h

namespace WhirlyKit {

/// This class provides RAII support for startOfWork/endOfWork calls
class WorkRegion
{
public:
    WorkRegion() { }
    
    // Copy constructor creates a new work item, i.e., the worker count increases
    WorkRegion(WorkRegion const &other) {
        if ([other._layer startOfWork]) {
            _layer = other._layer;
            _render = other._render;
        }
    }

    // Move constructor takes ownership, the worker count does not increase.
    // We will release the work item instead of the original `WorkRegion`.
    WorkRegion(WorkRegion &&other) noexcept {
        if (other._layer) {
            _layer = other._layer;
            _render = other._render;
            other._layer = nullptr;
            other._render = nullptr;
        }
    }
    
    WorkRegion(MaplyBaseInteractionLayer* layer,
               NSObject<MaplyRenderControllerProtocol>* renderController = nullptr)
    {
        if (layer && [layer startOfWork]) {
            _layer = layer;
            _render = renderController;
        }
    }

    WorkRegion(NSObject<MaplyRenderControllerProtocol>* renderController)
    {
        if (renderController) {
            // Use the result of getRenderControl, if any, but keep reference to the original.
            const auto rc = [renderController getRenderControl];
            const auto layer = (rc ? rc->interactLayer : nil);
            if (layer && [layer startOfWork]) {
                _layer = layer;
                _render = renderController;
            }
        }
    }

    ~WorkRegion() {
        EndWork();
    }
    
    // Same as copy constructor
    WorkRegion& operator=(WorkRegion const& other) {
        if (&other != this) {
            // Release our work item and acquire one from the other.
            EndWork();
            if ([other._layer startOfWork]) {
                _layer = other._layer;
                _render = other._render;
            }
        }
        return *this;
    }

    // Same as move constructor
    WorkRegion& operator=(WorkRegion&& other) {
        if (&other != this) {
            // Release our work item and take the other one
            EndWork();
            _layer = other._layer;
            _render = other._render;
            other._layer = nullptr;
            other._render = nullptr;
        }
        return *this;
    }

    bool IsWorkStarted() const noexcept { return (_layer != nullptr); }

    operator bool() const noexcept { return (_layer != nullptr); }
    
    operator MaplyBaseInteractionLayer*() const noexcept { return _layer; }
    
    /// End the work region early
    void EndWork() {
        if (_layer) {
            auto layer = _layer;
            _layer = nullptr;
            _render = nullptr;
            [layer endOfWork];
        }
    }
    
private:
    __strong MaplyBaseInteractionLayer* _layer = nullptr;
    
    // We don't do anything with the render controller, we just keep
    // a strong reference so that it doesn't get cleaned up (and
    // destroy the interaction layer) before we can call `endOfWork`.
    __strong NSObject<MaplyRenderControllerProtocol>* _render = nullptr;
};

} // namespace WhirlyKit

#endif /* WorkRegion_private_h */
