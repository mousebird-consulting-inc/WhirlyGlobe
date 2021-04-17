/*  ActiveModel.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 8/16/11.
 *  Copyright 2012-2021 mousebird consulting.
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

#import <map>
#import <list>
#import "Texture.h"
#import "BasicDrawable.h"
#import "GlobeView.h"

namespace WhirlyKit {

/** Fill in the active model protocol to provide an active model to
 the scene.  Active models are called before rendering and can update
 their associated objects.  Use this sparingly, for things like viewer
 controlled models (e.g. things that move every frame).
 */
class ActiveModel : public Identifiable
{
public:
    ActiveModel() = default;
    virtual ~ActiveModel() = default;
    
    /// Create the stuff you need to manipulate in the scene
    virtual void startWithScene(Scene *scene) { }

    /// Return true if you have an update that needs to be processed.
    /// Return false if you don't, otherwise we'll be constantly rendering.
    virtual bool hasUpdate() const { return false; }

    /// Update your stuff for display, but be quick!
    virtual void updateForFrame(RendererFrameInfo *frameInfo) { }

    /// Time to clean up your toys
    virtual void teardown(PlatformThreadInfo*) { }
};
    
typedef std::shared_ptr<ActiveModel> ActiveModelRef;
    
}
