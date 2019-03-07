/*
 *  ComponentManager_Android.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/4/19.
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

#import "ComponentManager_Android.h"

namespace WhirlyKit
{
    
// The scene wants a component manager early in the process
// This gives it an Android specific one
ComponentManager *MakeComponentManager()
{
    return new ComponentManager_Android();
}
    
}
