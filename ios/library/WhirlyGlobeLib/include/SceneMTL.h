/*
 *  SceneMTL.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
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

#import "Scene.h"
#import "WrapperMTL.h"

namespace WhirlyKit
{

// Maximum number of frames we could possibly have rendering at once
#define WKMaxFramesInFlight 4

/** Metal verson of Scene.
  */
class SceneMTL : public Scene
{
public:
    SceneMTL(CoordSystemDisplayAdapter *adapter);
    
    /// Look for a valid texture
    /// If it's missing, we probably won't draw the associated geometry
    id<MTLTexture> getMTLTexture(SimpleIdentity texIdent);
            
    /// Explicitly tear everything down in OpenGL ES.
    /// We're assuming the context has been set.
    virtual void teardown(PlatformThreadInfo*) override;
};
    
}
