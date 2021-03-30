/*  WideVectorDrawableBuilderGLES.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/14/19.
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

#import "WideVectorDrawableBuilder.h"
#import "BasicDrawableBuilderGLES.h"

namespace WhirlyKit
{
 
/// GLES version modifies uniforms
struct WideVectorTweakerGLES : public WideVectorTweaker
{
    virtual void tweakForFrame(Drawable *inDraw,RendererFrameInfo *frameInfo) override;
};

// Shader name
//#define kWideVectorShaderName "Wide Vector Shader"
//#define kWideVectorGlobeShaderName "Wide Vector Shader Globe"

/// Construct and return the wide vector shader program
ProgramGLES *BuildWideVectorProgramGLES(const std::string &name,SceneRenderer *renderer);
/// This version is for the 3D globe
ProgramGLES *BuildWideVectorGlobeProgramGLES(const std::string &name,SceneRenderer *renderer);

/// OpenGL version of the WideVectorDrawable Builder
class WideVectorDrawableBuilderGLES : virtual public WideVectorDrawableBuilder
{
public:
    // Initialize with an estimate on the number of vertices and triangles
    WideVectorDrawableBuilderGLES(const std::string &name,const SceneRenderer *sceneRenderer,Scene *scene);
    
    virtual void Init(unsigned int numVertex,unsigned int numTri,unsigned int numCenterline,
            WideVecImplType implType,
            bool globeMode,
            const WideVectorInfo *vecInfo) override;
    
    virtual int addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int slot = -1,int numThings = -1) override;

    // Return the basic drawable for the simple and complex cases
    virtual BasicDrawableRef getBasicDrawable() override;

    // Return the drawable instance for the complex case
    virtual BasicDrawableInstanceRef getInstanceDrawable() override;

    virtual DrawableTweakerRef makeTweaker() const override;

    bool drawableGotten;
    bool instanceGotten;
};

}
