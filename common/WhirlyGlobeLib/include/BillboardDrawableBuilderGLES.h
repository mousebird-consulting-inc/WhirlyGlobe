/*
 *  BillboardDrawableBuilderGLES.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/15/19.
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

#import "BillboardDrawableBuilder.h"
#import "BasicDrawableBuilderGLES.h"

namespace WhirlyKit
{

// Shader name for accessing within the scene
//#define kBillboardShaderName "Billboard Shader"

/// Construct and return the Billboard shader program
ProgramGLES *BuildBillboardGroundProgramGLES(const std::string &name,SceneRenderer *render);
ProgramGLES *BuildBillboardEyeProgramGLES(const std::string &name,SceneRenderer *render);

/** OpenGL version of BillboardDrawable Builder
 */
class BillboardDrawableBuilderGLES : public BasicDrawableBuilderGLES, public BillboardDrawableBuilder
{
public:
    BillboardDrawableBuilderGLES(const std::string &name,Scene *scene);
    
    virtual int addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int slot = -1,int numThings = -1) override;

    virtual BasicDrawableRef getDrawable() override;
};

}
