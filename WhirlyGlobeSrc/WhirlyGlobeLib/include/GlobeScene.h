/*
 *  GlobeScene.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/3/11.
 *  Copyright 2011-2012 mousebird consulting
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
#import "Cullable.h"

namespace WhirlyGlobe
{
    
/** GlobeScene is the top level scene object for WhirlyGlobe.
    The main addition to the basic WhirlyKit Scene is sorting into
    geo MBR oriented cullables.
 */
class GlobeScene : public WhirlyKit::Scene
{
public:
    /// Construct with the geo coordinate system and the quad tree depth for the culling.
    GlobeScene(WhirlyKit::CoordSystem *coordSystem,int depth);
    
    /// Add a drawable, taking overlap into account
    virtual void addDrawable(WhirlyKit::DrawableRef drawable);
    
    /// Remove a drawable
    virtual void remDrawable(WhirlyKit::DrawableRef drawable);
    
protected:
};
    
}
