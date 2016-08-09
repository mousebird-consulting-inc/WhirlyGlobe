/*
 *  MaplyScene.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/11/12.
 *  Copyright 2011-2016 mousebird consulting
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

#import "Scene.h"

namespace Maply
{

/** The Map Scene is the subclass of Scene that deals with flat maps.
    At present it's a dumb container and doesn't do culling, as the globe scene
    does.
  */
class MapScene : public WhirlyKit::Scene
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    MapScene(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter);
    
    /// Add a drawable
    virtual void addDrawable(WhirlyKit::DrawableRef drawable);
    
    /// Remove a drawable
    virtual void remDrawable(WhirlyKit::DrawableRef drawable);
};

}
