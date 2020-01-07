/*
 *  BasicDrawableInstanceGLES.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/10/19.
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

#import "BasicDrawableInstance.h"
#import "BasicDrawableGLES.h"
#import "WrapperGLES.h"

namespace WhirlyKit
{

/// OpenGL variant of BasicDrawableInstance
class BasicDrawableInstanceGLES : virtual public BasicDrawableInstance, virtual public DrawableGLES
{
friend class BasicDrawableInstanceBuilderGLES;
public:
    BasicDrawableInstanceGLES(const std::string &name);
    
    /// Set up local rendering structures (e.g. VBOs)
    virtual void setupForRenderer(const RenderSetupInfo *setupInfo);
    
    /// Clean up any rendering objects you may have (e.g. VBOs).
    virtual void teardownForRenderer(const RenderSetupInfo *setupInfo,Scene *scene);
    
    /// Some drawables have a pre-render phase that uses the GPU for calculation
    virtual void calculate(RendererFrameInfoGLES *frameInfo,Scene *scene);

    /// Set up what you need in the way of context and draw.
    virtual void draw(RendererFrameInfoGLES *frameInfo,Scene *scene);

protected:
    GLuint setupVAO(RendererFrameInfoGLES *frameInfo);
    
    int centerSize,matSize,colorInstSize,colorSize,instSize,modelDirSize;
    GLuint instBuffer;
    GLuint vertArrayObj;
    std::vector<BasicDrawableGLES::VertAttrDefault> vertArrayDefaults;
};

}
