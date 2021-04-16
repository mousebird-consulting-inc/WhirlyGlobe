/*  MaplyActiveObject.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 4/3/13.
 *  Copyright 2011-2021 mousebird consulting
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

#import "MaplyActiveObject_private.h"

namespace WhirlyKit {
    
// Interface between the c++ and Obj-C sides
class ActiveModelInterface : public ActiveModel {
public:
    ActiveModelInterface(MaplyActiveObject *activeObject)
        : activeObject(activeObject)
    {
    }
    
    virtual bool hasUpdate() const override {
        return [activeObject hasUpdate];
    }
    
    virtual void updateForFrame(RendererFrameInfo *frameInfo) override {
        [activeObject updateForFrame:frameInfo];
    }
    
    virtual void teardown(PlatformThreadInfo *) override {
        [activeObject teardown];
    }
    
    MaplyActiveObject *activeObject;
};
    
typedef std::shared_ptr<ActiveModelInterface> ActiveModelInterfaceRef;
    
}

using namespace WhirlyKit;

@implementation MaplyActiveObject
{
    WhirlyKit::ActiveModelInterfaceRef activeInter;
}

/// Default initialization.  Updates will happen on the main queue.
- (instancetype)initWithViewController:(NSObject<MaplyRenderControllerProtocol> *)inViewC
{
    self = [super init];
    _viewC = inViewC;
    if (activeInter)
        activeInter = ActiveModelInterfaceRef(new ActiveModelInterface(self));
    
    return self;
}

- (void)registerWithScene
{
    if (!activeInter)
        activeInter = ActiveModelInterfaceRef(new ActiveModelInterface(self));
    scene->addActiveModel(activeInter);
}

- (void)removeFromScene
{
    if (activeInter)
        scene->removeActiveModel(nullptr, activeInter);
}

- (void)startWithScene:(WhirlyKit::Scene *)inScene
{
    scene = inScene;
}

- (bool)hasUpdate
{
    return false;
}

- (void)updateForFrame:(void *)frameInfo
{
    
}

- (void)teardown
{
    
}

@end
